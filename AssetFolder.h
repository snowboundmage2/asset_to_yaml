#ifndef ASSET_FOLDER_H
#define ASSET_FOLDER_H

#include "AssetEntry.h"
#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "yaml-cpp/yaml.h"
#include <iomanip>

class AssetFolder {
public:
    std::vector<AssetEntry> assets;

    AssetFolder() = default;

    static AssetFolder from_bytes(const std::vector<uint8_t>& in_bytes) {
        if (in_bytes.size() < 8) {
            throw std::runtime_error("Invalid byte array size for AssetFolder");
        }
        size_t asset_slot_cnt = (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3];
        
        std::vector<AssetEntry> asset_list;
        size_t table_size = asset_slot_cnt * 8;
        auto table_bytes = std::vector<uint8_t>(in_bytes.begin() + 8, in_bytes.begin() + 8 + table_size);
        auto data_bytes = std::vector<uint8_t>(in_bytes.begin() + 8 + table_size, in_bytes.end());
        
        std::vector<AssetMeta> meta_info;
        for (size_t i = 0; i < asset_slot_cnt; i++) {
            meta_info.push_back(AssetMeta::from_bytes({table_bytes.begin() + i * 8, table_bytes.begin() + (i + 1) * 8}));
        }
        
        for (size_t i = 0; i < asset_slot_cnt - 1; i++) {
            const AssetMeta& this_meta = meta_info[i];
            const AssetMeta& next_meta = meta_info[i + 1];
            std::vector<uint8_t> asset_data(data_bytes.begin() + this_meta.offset, data_bytes.begin() + next_meta.offset);
            asset_list.emplace_back(i, 0, this_meta, std::nullopt);
        }

        return AssetFolder{asset_list};
    }

    std::vector<uint8_t> to_bytes() {
        std::vector<uint8_t> out;
        size_t asset_count = assets.size();
        out.push_back((asset_count >> 24) & 0xFF);
        out.push_back((asset_count >> 16) & 0xFF);
        out.push_back((asset_count >> 8) & 0xFF);
        out.push_back(asset_count & 0xFF);
        out.insert(out.end(), {0xFF, 0xFF, 0xFF, 0xFF});
        
        std::vector<uint8_t> meta_bytes;
        std::vector<uint8_t> data_bytes;
        size_t offset = 0;
        for (auto& asset : assets) {
            auto asset_data = asset.data.has_value() ? asset.data.value()->to_bytes() : std::vector<uint8_t>();
            asset.meta.offset = offset;
            meta_bytes.insert(meta_bytes.end(), asset.meta.to_bytes().begin(), asset.meta.to_bytes().end());
            data_bytes.insert(data_bytes.end(), asset_data.begin(), asset_data.end());
            offset += asset_data.size();
        }
        out.insert(out.end(), meta_bytes.begin(), meta_bytes.end());
        out.insert(out.end(), data_bytes.begin(), data_bytes.end());
        return out;
    }

    void write(const std::filesystem::path& out_dir_path) {
        std::ofstream asset_yaml(out_dir_path / "assets.yaml");
        if (!asset_yaml) {
            throw std::runtime_error("Could not write assets.yaml");
        }
        asset_yaml << "files:\n";
        for (auto& asset : assets) {
            auto asset_data = asset.data.has_value() ? asset.data.value()->to_bytes() : std::vector<uint8_t>();
            //asset.meta.offset
            asset_yaml << "  - { FileID: " << std::hex << std::setw(4) << std::setfill('0') << asset.uid << std::dec << ", " << "Offset: "  << std::hex << std::setw(4) << std::setfill('0') << asset.meta.offset << std::dec << " }\n";
        }
    }

    void read(const std::filesystem::path& yaml_path) {
        YAML::Node doc = YAML::LoadFile(yaml_path);
        for (const auto& node : doc["files"]) {
            size_t uid = node["uid"].as<size_t>();
            assets.emplace_back(AssetEntry(uid));
        }
    }
};

#endif // ASSET_FOLDER_H
