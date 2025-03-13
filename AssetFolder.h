#ifndef ASSET_FOLDER_H
#define ASSET_FOLDER_H

#include "AssetEntry.h"
#include "Asset.h"
#include "AssetMeta.h"
#include <iostream>
#include "AssetFactory.h"
#include <optional>

#include "DecompressionUtils.h"

#include <vector>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "yaml-cpp/yaml.h"
#include <iomanip>

namespace fs = std::filesystem;

class AssetFolder {
public:

    std::vector<AssetEntry> v_asset_entries;
    AssetType asset_type;

    AssetFolder() : v_asset_entries() {}

    static AssetFolder create() {
        return AssetFolder();
    }

    static AssetFolder from_bytes(const std::vector<uint8_t>& in_bytes) {
        std::cout << "AssetFolder::from_bytes() called" << std::endl;
        // Ensure the input has at least 4 bytes
        if (in_bytes.size() < 4) {
            throw std::runtime_error("Invalid input: Not enough data.");
        }

        // Read asset slot count (first 4 bytes)
        size_t asset_slot_count = (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3];

        // Ensure the input is large enough
        if (in_bytes.size() < 8 + (8 * asset_slot_count)) {
            throw std::runtime_error("Invalid input: Data too small.");
        }

        // Split metadata and actual asset data
        std::vector<uint8_t> table_bytes(in_bytes.begin() + 8, in_bytes.begin() + 8 + (8 * asset_slot_count));
        std::vector<uint8_t> data_bytes(in_bytes.begin() + 8 + (8 * asset_slot_count), in_bytes.end());

        // Parse metadata
        std::vector<AssetMeta> meta_info;
        for (size_t i = 0; i < asset_slot_count; ++i) {
            std::vector<uint8_t> chunk(table_bytes.begin() + i * 8, table_bytes.begin() + (i + 1) * 8);
            meta_info.push_back(AssetMeta::from_bytes(chunk));
        }

        std::vector<AssetEntry> asset_list;
        size_t segment = 0;
        uint16_t prev_t = 0x3; // Used for segment detection

        for (size_t i = 0; i < meta_info.size() - 1; ++i) {
            const AssetMeta& this_meta = meta_info[i];
            const AssetMeta& next_meta = meta_info[i + 1];

            if (this_meta.t_flag == 4) { // Empty entry
                asset_list.emplace_back(i, 0, this_meta, std::nullopt);
                continue;
            } else if ((this_meta.t_flag != 2) && ((prev_t & 2) != (this_meta.t_flag & 2))) {
                segment += 1;
                prev_t = this_meta.t_flag;
            }

            // Ensure valid offset range
            if (this_meta.offset >= data_bytes.size() || next_meta.offset > data_bytes.size() || this_meta.offset >= next_meta.offset) {
                throw std::runtime_error("Invalid offsets in metadata.");
            }

            // Extract compressed binary
            std::vector<uint8_t> comp_bin(data_bytes.begin() + this_meta.offset, data_bytes.begin() + next_meta.offset);

            // Decompress if needed
            std::cout << "Decompressing data" << this_meta.offset << std::endl;
            std::vector<uint8_t> decomp_bin = this_meta.c_flag ? unzip_bk(comp_bin) : comp_bin;
            std::cout << "Decompressed data" << this_meta.offset << std::endl;

            // Create asset using factory
            std::unique_ptr<Asset> asset = AssetFactory::from_seg_index_and_bytes(segment, i, decomp_bin);

            // Store asset entry
            asset_list.emplace_back(i, segment, this_meta, std::move(asset));
        }

        return AssetFolder{ asset_list };
    }
    
    std::vector<uint8_t> to_bytes() {
        // Ensure the last asset is an empty entry if needed
        if (!v_asset_entries.empty() && v_asset_entries.back().data.has_value()) {
            v_asset_entries.emplace_back(v_asset_entries.size(), 0, AssetMeta{0, false, 4}, std::nullopt);
        }

        // Get compressed version if compressed
        std::vector<std::vector<uint8_t>> comp_bins;
        for (const auto& asset : v_asset_entries) {
            if (!asset.data.has_value()) {
                comp_bins.emplace_back(); // Empty vector for missing assets
            } else {
                const auto& asset_data = asset.data.value();
                if (asset.meta.c_flag) {
                    comp_bins.push_back(compress_data(asset_data->to_bytes())); // Compress
                } else {
                    comp_bins.push_back(asset_data->to_bytes()); // Raw bytes
                }
            }
        }

        // Update asset offsets
        size_t offset = 0;
        for (size_t i = 0; i < v_asset_entries.size(); ++i) {
            v_asset_entries[i].meta.offset = offset;
            offset += comp_bins[i].size();
        }

        // Convert everything to bytes
        std::vector<uint8_t> out;
        uint32_t asset_count = static_cast<uint32_t>(v_asset_entries.size());
        
        // Write asset count (big-endian)
        out.push_back((asset_count >> 24) & 0xFF);
        out.push_back((asset_count >> 16) & 0xFF);
        out.push_back((asset_count >> 8) & 0xFF);
        out.push_back(asset_count & 0xFF);

        // Add padding bytes
        out.insert(out.end(), {0xFF, 0xFF, 0xFF, 0xFF});

        // Write metadata
        for (const auto& asset : v_asset_entries) {
            std::vector<uint8_t> meta_bytes = asset.meta.to_bytes();
            out.insert(out.end(), meta_bytes.begin(), meta_bytes.end());
        }

        // Write data
        for (const auto& bin : comp_bins) {
            out.insert(out.end(), bin.begin(), bin.end());
        }

        // Remove last placeholder asset
        v_asset_entries.pop_back();

        return out;
    }

    void write(const fs::path& out_dir_path) {
        fs::path asset_yaml_path = out_dir_path / "assets.yaml";

        // Open YAML file
        std::ofstream asset_yaml(asset_yaml_path);
        if (!asset_yaml.is_open()) {
            throw std::runtime_error("Could not write file: " + asset_yaml_path.string());
        }

        // Iterate over assets that contain data
        for (const auto& elem : v_asset_entries) {
            if (!elem.data.has_value()) continue;  // Skip empty assets

            std::shared_ptr<Asset> data = elem.data.value();  // Get the asset data
            if (!data) {
                throw std::runtime_error("Asset data is null for UID: " + std::to_string(elem.uid));
            }

            // Determine asset type as string
            std::string data_type_str;
            switch (data->get_type()) {
                case AssetType::Animation: data_type_str = "ANIMATION"; break;
                case AssetType::Binary: data_type_str = "BINARY"; break;
                case AssetType::DemoInput: data_type_str = "DEMOINPUT"; break;
                case AssetType::Dialog: data_type_str = "DIALOG"; break;
                case AssetType::GruntyQuestion: data_type_str = "GRUNTYQUESTION"; break;
                case AssetType::Midi: data_type_str = "MIDI"; break;
                case AssetType::Model: data_type_str = "MODEL"; break;
                case AssetType::LevelSetup: data_type_str = "LEVELSETUP"; break;
                case AssetType::QuizQuestion: data_type_str = "QUIZQUESTION"; break;
                case AssetType::Sprite: data_type_str = "SPRITE"; break;
                default: data_type_str = "BINARY"; break;
            }

            // Determine file extension
            std::string file_ext;
            switch (data->get_type()) {
                case AssetType::Binary: file_ext = ".bin"; break;
                case AssetType::Dialog: file_ext = ".dialog"; break;
                case AssetType::GruntyQuestion: file_ext = ".grunty_q"; break;
                case AssetType::QuizQuestion: file_ext = ".quiz_q"; break;
                case AssetType::DemoInput: file_ext = ".demo"; break;
                case AssetType::Midi: file_ext = ".midi.bin"; break;
                case AssetType::Model: file_ext = ".model.bin"; break;
                case AssetType::LevelSetup: file_ext = ".lvl_setup.bin"; break;
                case AssetType::Animation: file_ext = ".anim.bin"; break;
                case AssetType::Sprite: file_ext = ".sprite.bin"; break;
                default: file_ext = ".bin"; break;
            }

            // Determine containing folder based on asset type
            std::string containing_folder;
            switch (data->get_type()) {
                case AssetType::Binary: containing_folder = "bin"; break;
                case AssetType::Dialog: containing_folder = "dialog"; break;
                case AssetType::GruntyQuestion: containing_folder = "grunty_q"; break;
                case AssetType::QuizQuestion: containing_folder = "quiz_q"; break;
                case AssetType::DemoInput: containing_folder = "demo"; break;
                case AssetType::Midi: containing_folder = "midi"; break;
                case AssetType::Model: containing_folder = "model"; break;
                case AssetType::LevelSetup: containing_folder = "lvl_setup"; break;
                case AssetType::Animation: containing_folder = "anim"; break;
                case AssetType::Sprite: containing_folder = "sprite"; break;
                default: containing_folder = "bin"; break;
            }

            // Create the directory if it doesn't exist
            fs::path elem_folder = out_dir_path / containing_folder;
            fs::create_directories(elem_folder);

            // Construct full file path
            fs::path elem_path = elem_folder / (std::to_string(elem.uid) + file_ext);
            std::string relative_path = fs::relative(elem_path, out_dir_path).string();

            // Write asset information to the YAML file
            asset_yaml << containing_folder << "_" << std::setw(4) << std::setfill('0') << std::hex << elem.uid << ": \n";
            asset_yaml << " {type: " << data_type_str << ", offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << (0x5E90 + elem.meta.offset) 
                       << ", Symbol: " << std::setw(4) << std::setfill('0') << std::hex << elem.uid << "}\n";

            // Write the asset data to a file
            data->write(elem_path);
        }

        asset_yaml.close();
    }

private:

    AssetFolder(std::vector<AssetEntry> v_asset_entries) : v_asset_entries(std::move(v_asset_entries)) {}

    static std::vector<uint8_t> decompress_data(const std::vector<uint8_t>& compressed_data) {
        // Decompression logic needs to be implemented here
        return compressed_data; // Placeholder: return original data if no decompression is available
    }
    
    static std::vector<uint8_t> compress_data(const std::vector<uint8_t>& data) {
        // Placeholder for compression logic (implement actual compression if needed)
        return data; // Currently returns uncompressed data
    }

    
};

#endif // ASSET_FOLDER_H
