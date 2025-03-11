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
private:
    //AssetEntry current_asset;
    //std::vector<uint8_t> out_bytes;
public:
    std::vector<AssetEntry> v_asset_entries;
    AssetType asset_type;

    AssetFolder() : v_asset_entries() {}

    static AssetFolder create() {
        return AssetFolder();
    }

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
            
        AssetFolder folder;
        folder.v_asset_entries = std::move(asset_list);
        return folder;

    }
    
    std::vector<uint8_t> to_bytes() {
        std::vector<uint8_t> out;
        size_t asset_count = v_asset_entries.size();
        out.push_back((asset_count >> 24) & 0xFF);
        out.push_back((asset_count >> 16) & 0xFF);
        out.push_back((asset_count >> 8) & 0xFF);
        out.push_back(asset_count & 0xFF);
        out.insert(out.end(), {0xFF, 0xFF, 0xFF, 0xFF});
        
        std::vector<uint8_t> meta_bytes;
        std::vector<uint8_t> data_bytes;
        size_t offset = 0;
        for (auto& asset : v_asset_entries) {
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
        
        std::cout << "Starting write loop" << std::endl;
        for (const auto& asset_entry : v_asset_entries) {
            std::shared_ptr<Asset> data = asset_entry.data;
            std::vector<uint8_t> out_bytes = data ? data->to_bytes() : std::vector<uint8_t>();
            std::string out(out_bytes.begin(), out_bytes.end());
            // Debug: Print asset entry meta information
            //std::cout << std::hex << asset_entry.meta.from_bytes << std::endl;
            //std::cout << std::hex << asset_entry.meta.to_bytes << std::endl;
            //std::cout << out << std::endl;
            std::cout << "Asset entry meta - Offset: " << std::to_string(asset_entry.meta.offset) << std::endl;

            asset_yaml << "  - { Symbol: " << std::hex << std::setw(4) << \
            std::setfill('0') << asset_entry.uid << std::dec << ", Offset: "  << \
            std::hex << std::setw(4) << std::setfill('0') << asset_entry.meta.offset << \
            std::dec << ", frombytes: " << asset_entry.meta.from_bytes << " }" << \
            std::endl;
        }
    }

    AssetType determine_asset_type(const AssetEntry& current_asset) {
        std::vector<uint8_t> out_bytes;
        // Implement logic to determine the asset type based on the binary data
        out_bytes = current_asset.data->to_bytes();
        if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Animation)}) {
                return AssetType::Animation;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Binary)}) {
                return AssetType::Binary;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::DemoInput)}) {
                return AssetType::DemoInput;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Dialog)}) {
                return AssetType::Dialog;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::GruntyQuestion)}) {
                return AssetType::GruntyQuestion;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::LevelSetup)}) {
                return AssetType::LevelSetup;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Midi)}) {
                return AssetType::Midi;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Model)}) {
                return AssetType::Model;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::QuizQuestion)}) {
                return AssetType::QuizQuestion;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Sprite)}) {
                return AssetType::Sprite;
            } else if (out_bytes == std::vector<uint8_t>{static_cast<uint8_t>(AssetType::Texture)}) {
                return AssetType::Texture;
            } else {
                return AssetType::Binary;
            }
    } 
/*         if (Animation::is_valid(data)) return AssetType::Animation;
        if (Binary::is_valid(data)) return AssetType::Binary;
        if (DemoButton::is_valid(data)) return AssetType::DemoInput;
        if (Dialog::is_valid(data)) return AssetType::Dialog;
        if (GruntyQuestion::is_valid(data)) return AssetType::GruntyQuestion;
        if (LevelSetup::is_valid(data)) return AssetType::LevelSetup;
        if (MidiSeqFile::is_valid(data)) return AssetType::Midi;
        if (Model::is_valid(data)) return AssetType::Model;
        if (QuizQuestion::is_valid(data)) return AssetType::QuizQuestion;
        if (Sprite::is_valid(data)) return AssetType::Sprite;
        if (Texture::is_valid(data)) return AssetType::Texture;
        return AssetType::Binary; */

    void read(const std::filesystem::path& yaml_path) {
    }

    std::string get_asset_type_string(const AssetType& type) {
        switch (type) {
            case AssetType::Animation: return "Animation";
            case AssetType::Binary: return "Binary";
            case AssetType::DemoInput: return "DemoInput";
            case AssetType::Dialog: return "Dialog";
            case AssetType::GruntyQuestion: return "GruntyQuestion";
            case AssetType::Midi: return "Midi";
            case AssetType::Model: return "Model";
            case AssetType::LevelSetup: return "LevelSetup";
            case AssetType::QuizQuestion: return "QuizQuestion";
            case AssetType::Sprite: return "Sprite"; // Sprite needs special handling
            default: return "Binary";
        }
    }
    
};

#endif // ASSET_FOLDER_H
