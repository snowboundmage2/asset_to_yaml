#ifndef ASSET_FOLDER_H
#define ASSET_FOLDER_H

#include "AssetEntry.h"
#include "Asset.h"
#include "AssetMeta.h"
#include <iostream>
#include "AssetFactory.h"
#include <optional>

//#include "librarezip.h"
#include "BKAssetRareZip.h"

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
        //std::cout << "AssetFolder::from_bytes() called" << std::endl;
        // Ensure the input has at least 4 bytes
        if (in_bytes.size() < 4) {
            throw std::runtime_error("Invalid input: Not enough data.");
        }

        // Read asset slot count (first 4 bytes)
        size_t asset_slot_count = (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3];
        std::cout << "Asset slot count: " << asset_slot_count << std::endl;
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
        // Print metadata for debugging
/*         for (size_t i = 0; i < meta_info.size(); ++i) {
            const AssetMeta& meta = meta_info[i];
            std::cout << "Asset " << i << ": offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << (0x010CD0 + meta.offset) << ", c_flag: " << meta.c_flag << ", t_flag: " << std::dec << meta.t_flag << std::endl;
        } */

        std::vector<AssetEntry> asset_list;
        size_t segment = 0;
        uint16_t prev_t = 0x3; // Used for segment detection

        for (size_t i = 0; i < meta_info.size() - 1; ++i) {
            const AssetMeta& this_meta = meta_info[i];
            const AssetMeta& next_meta = meta_info[i + 1];

            //std::cout << "Pre-testing meta offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << (0x010CD0 + this_meta.offset) << std::endl;
            //std::cout << "meta offset without rom offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << this_meta.offset << std::endl;
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
            std::vector<uint8_t> decomp_bin;

            // Decompress if needed
            //if (this_meta.offset == 0x726b0 ||  this_meta.offset == 0x260530 || this_meta.offset == 0x6f4418) {
            std::cout << "Decompressing asset at offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << (0x010CD0 + this_meta.offset) << std::endl;
            try {
                if (this_meta.c_flag == 1) {
                    //std::cout << "unzipping asset at offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << (0x010CD0 + this_meta.offset) << std::endl;
                    size_t expected_len = ((uint32_t)comp_bin[2] << 24) | ((uint32_t)comp_bin[3] << 16) | ((uint32_t)comp_bin[4] << 8) | ((uint32_t)comp_bin[5]);
                    //std::cout << std::dec << "expected length: " << expected_len << std::endl;
                    decomp_bin.resize(expected_len);
                    //std::cout << std::dec << "trying the unzip now" << std::endl;
                    try{
                        decomp_bin = BK64::bk_unzip(comp_bin.data(), comp_bin.size());
                    } catch (const std::exception& e) {
                        std::cout << "tried to unzip " << std::hex << this_meta.offset << std::dec << " but failed." << std::endl;
                        std::cerr << "Error: " << e.what() << std::endl;
                        //print the first 8 bytes of the compressed data
                        std::cout << "compressed data inital bytes: ";
                        for (size_t j = 0; j < 8 && j < comp_bin.size(); ++j) {
                            std::cout << std::hex << (int)comp_bin[j] << " ";
                        }
                        std::cout << std::dec << "expected length: " << expected_len << std::endl;
                        decomp_bin = comp_bin;
                    }
                } else {
                    //std::cout << "not unzipping " << std::hex << this_meta.offset << std::dec << std::endl;
                    decomp_bin = comp_bin;
                }
            } catch (const std::exception& e) {
                //std::cout << "tried to process asset " << std::hex << this_meta.offset << std::dec << " but failed." << std::endl;
                std::cerr << "Error: " << e.what() << std::endl;
            }

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
                case AssetType::Animation: data_type_str = "BK64::ANIMATION"; break;
                case AssetType::Binary: data_type_str = "BK64::BINARY"; break;
                case AssetType::DemoInput: data_type_str = "BK64::DEMOINPUT"; break;
                case AssetType::Dialog: data_type_str = "BK64::DIALOG"; break;
                case AssetType::GruntyQuestion: data_type_str = "BK64::GRUNTYQUIZ"; break;
                case AssetType::Midi: data_type_str = "BK64::MIDI"; break;
                case AssetType::Model: data_type_str = "BK64::MODEL"; break;
                case AssetType::LevelSetup: data_type_str = "BK64::LEVELSETUP"; break;
                case AssetType::QuizQuestion: data_type_str = "BK64::QUESTION"; break;
                case AssetType::Sprite: data_type_str = "BK64::SPRITE"; break;
                default: data_type_str = "BK64::BINARY"; break;
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

            std::string asset_to_enum_names;
            switch (data->get_type()) {
                case AssetType::Animation: asset_to_enum_names = "ANIM"; break;
                case AssetType::Binary: asset_to_enum_names = "SKIP"; break;
                case AssetType::DemoInput: asset_to_enum_names = "SKIP"; break;
                case AssetType::Dialog: asset_to_enum_names = "DIALOG"; break;
                case AssetType::GruntyQuestion: asset_to_enum_names = "SKIP"; break;
                case AssetType::LevelSetup: asset_to_enum_names = "SKIP"; break;
                case AssetType::Midi: asset_to_enum_names = "SKIP"; break;
                case AssetType::Model: asset_to_enum_names = "MODEL"; break;
                case AssetType::QuizQuestion: asset_to_enum_names = "SKIP"; break;
                case AssetType::Sprite: asset_to_enum_names = "SPRITE"; break;
                default: asset_to_enum_names = "SKIP"; break;
            } 
            
            // 'Animation': 'ANIM',
            // 'Model': 'MODEL',
            // 'Sprite_I4': 'SPRITE',
            // 'Sprite_I8': 'SPRITE',
            // 'Sprite_CI4': 'SPRITE',
            // 'Sprite_CI8': 'SPRITE',
            // 'Sprite_RGBA16': 'SPRITE',
            // 'Sprite_RGBA32': 'SPRITE',
            // 'Sprite_UNKNOWN(256)': 'SPRITE',
            // 'Dialog': 'DIALOG',

            // Create the directory if it doesn't exist
            fs::path elem_folder = out_dir_path / containing_folder;
            fs::create_directories(elem_folder);

            // Construct full file path
            /* fs::path elem_path = elem_folder / (std::to_string(elem.uid) + file_ext);
            std::string relative_path = fs::relative(elem_path, out_dir_path).string();           */  

            std::stringstream ss;
            ss << std::setw(4) << std::setfill('0') << std::hex << elem.uid;
            fs::path elem_path = elem_folder / (ss.str() + file_ext);
            std::string relative_path = fs::relative(elem_path, out_dir_path).string();
            
            int assetLength;
            assetLength = (elem.uid < v_asset_entries.size() - 1) ? v_asset_entries[elem.uid + 1].meta.offset - elem.meta.offset : 0;
            
            // temporary limtiting of asset types
            std::string data_type_strv2;
            if (data_type_str != "BK64::SPRITE") {
                data_type_strv2 = "BK64::BINARY";
            } else {
                data_type_strv2 = "BK64::SPRITE";
            }
            // Write asset information to the YAML file
            //name
            asset_yaml << elem.uid << ": \n";
            //needed yaml entries
            asset_yaml << " {OGname: " << containing_folder << "_" << std::setw(4) << std::setfill('0') << std::hex << elem.uid << ", type: " << std::dec << data_type_strv2 << ", offset: 0x" << std::setw(8) << std::setfill('0') << std::hex << (0x010CD0 + elem.meta.offset)    \
                       << ", symbol: " << std::setw(4) << std::setfill('0') << std::hex << elem.uid <<   \
                       ", size: " << std::dec << assetLength << ", subtype: " << data_type_str << ", compressed: " << elem.meta.c_flag << \
                       ", t_flag: " << elem.meta.t_flag << ", index: " << std::dec << elem.uid <<  "}\n";
            
            // Write the asset data to a file
            //std::cout << "Writing asset to: " << elem_path << std::endl;
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
