#ifndef ASSET_FOLDER_H
#define ASSET_FOLDER_H

#include "AssetEntry.h"
#include "Asset.h"
#include <iostream>

#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "yaml-cpp/yaml.h"
#include <iomanip>

class AssetFolder {
public:
    std::vector<AssetEntry> v_asset_entry;
    AssetType asset_type;

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
            asset_list.emplace_back(i, 0, this_meta, nullptr);
        }

        return AssetFolder{asset_list};
    }

    std::vector<uint8_t> to_bytes() {
        std::vector<uint8_t> out;
        size_t asset_count = v_asset_entry.size();
        out.push_back((asset_count >> 24) & 0xFF);
        out.push_back((asset_count >> 16) & 0xFF);
        out.push_back((asset_count >> 8) & 0xFF);
        out.push_back(asset_count & 0xFF);
        out.insert(out.end(), {0xFF, 0xFF, 0xFF, 0xFF});
        
        std::vector<uint8_t> meta_bytes;
        std::vector<uint8_t> data_bytes;
        size_t offset = 0;
        for (auto& asset : v_asset_entry) {
            auto asset_data = asset.data ? asset.data->to_bytes() : std::vector<uint8_t>();
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
        for (auto& asset_entry : v_asset_entry) {
            // Debug: Print asset entry UID
            std::cout << "Writing asset entry UID: " << asset_entry.uid << std::endl;
            //asset_type = asset_entry.data.has_value() ? asset_entry.data.value()->a_type : (AssetType)NULL;
            asset_type = asset_entry.get_asset_type();
            std::cout << "asset_type: " << asset_entry.type_to_string(asset_type) << std::endl;

            std::vector<uint8_t> out_bytes = asset_entry.meta.to_bytes();
            std::string out(out_bytes.begin(), out_bytes.end());

            // Debug: Print asset entry meta information
            std::cout << "Asset entry meta - Offset: " << asset_entry.meta.offset << std::endl;

            asset_yaml << "  - { Symbol: " << std::hex << std::setw(4) << std::setfill('0') << asset_entry.uid << std::dec << ", Offset: "  << std::hex << std::setw(4) << std::setfill('0') << asset_entry.meta.offset << std::dec << ", Type: " << static_cast<int>(asset_type) << " }" << std::endl;
        }
    }

    AssetType determine_asset_type(const std::vector<uint8_t>& data) {
        // Implement logic to determine the asset type based on the binary data

/*      if (Animation::is_valid(data)) return AssetType::Animation;
        if (Binary::is_valid(data)) return AssetType::Binary;
        if (DemoButtonFile::is_valid(data)) return AssetType::DemoInput;
        if (Dialog::is_valid(data)) return AssetType::Dialog;
        if (GruntyQuestion::is_valid(data)) return AssetType::GruntyQuestion;
        if (LevelSetup::is_valid(data)) return AssetType::LevelSetup;
        if (MidiSeqFile::is_valid(data)) return AssetType::Midi;
        if (Model::is_valid(data)) return AssetType::Model;
        if (QuizQuestion::is_valid(data)) return AssetType::QuizQuestion;
        if (Sprite::is_valid(data)) return AssetType::Sprite;
        if (Texture::is_valid(data)) return AssetType::Texture;
        return AssetType::Binary; */
    }

    void read(const std::filesystem::path& yaml_path) {
    }
};

#endif // ASSET_FOLDER_H
