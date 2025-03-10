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
/* 
    void read(const std::filesystem::path& yaml_path) {
        if (yaml_path.extension() != ".yaml") {
            throw std::runtime_error("Invalid file extension. Expected .yaml");
        }
        
        std::filesystem::path containing_folder = yaml_path.parent_path();
        std::ifstream file(yaml_path);
        if (!file) {
            throw std::runtime_error("Failed to open YAML file");
        }
        
        YAML::Node doc = YAML::LoadFile(yaml_path.string());
        
        std::vector<AssetEntry> asset_meta;
        for (const auto& entry : doc["files"]) {
            asset_meta.push_back(AssetEntry::from_yaml(entry));
        }
        
        size_t expect_len = doc["tbl_len"].as<size_t>();
        size_t max_id = 0;
        for (const auto& a : asset_meta) {
            max_id = std::max(max_id, a.uid);
        }
        
        expect_len = std::max(expect_len, max_id + 1);
        
        if (assets.size() < expect_len) {
            assets.resize(expect_len, AssetEntry(0));
            for (size_t i = 0; i < expect_len; ++i) {
                assets[i] = AssetEntry(i);
            }
        }
        
        for (const auto& a : asset_meta) {
            assets[a.uid] = a;
        }
    } */
        //TODO: Fix includes needed for this function
/*     void load_assets(const std::filesystem::path& containing_folder, const YAML::Node& doc) {
        for (const auto& y : doc["files"]) {
            size_t uid = y["uid"].as<size_t>();
            std::string relative_path = y["relative_path"].as<std::string>();
            
            std::shared_ptr<Asset> data;
            std::string type = y["type"].as<std::string>();
            std::filesystem::path asset_path = containing_folder / relative_path;
            
            if (type == "Binary") data = std::make_shared<Binary>(Binary::read(asset_path));
            else if (type == "Dialog") data = std::make_shared<Dialog>(Dialog::read(asset_path));
            else if (type == "GruntyQuestion") data = std::make_shared<GruntyQuestion>(GruntyQuestion::read(asset_path));
            else if (type == "QuizQuestion") data = std::make_shared<QuizQuestion>(QuizQuestion::read(asset_path));
            else if (type == "DemoInput") data = std::make_shared<DemoButtonFile>(DemoButtonFile::read(asset_path));
            else if (type == "Midi") data = std::make_shared<MidiSeqFile>(MidiSeqFile::read(asset_path));
            else if (type == "Model") data = std::make_shared<Model>(Model::read(asset_path));
            else if (type == "LevelSetup") data = std::make_shared<LevelSetup>(LevelSetup::read(asset_path));
            else if (type == "Animation") data = std::make_shared<Animation>(Animation::read(asset_path));
            else if (type.rfind("Sprite_", 0) == 0) data = std::make_shared<Sprite>(Sprite::read(asset_path));
            else data = std::make_shared<Binary>(Binary::read(asset_path));
            
            assets[uid].data = data;
        }
    } */
};

#endif // ASSET_FOLDER_H
