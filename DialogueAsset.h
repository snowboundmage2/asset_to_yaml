#ifndef DIALOG_H
#define DIALOG_H

#include "Asset.h"
#include "BKString.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "yaml-cpp/yaml.h"

class Dialog : public Asset {
private:
    std::vector<BKString> bottom;
    std::vector<BKString> top;

public:
    Dialog(std::vector<BKString> bottom, std::vector<BKString> top)
        : bottom(std::move(bottom)), top(std::move(top)) {
            a_type = AssetType::Dialog;
        }

    static Dialog from_bytes(const std::vector<uint8_t>& in_bytes) {
        size_t offset = 3;
        
        std::vector<BKString> bottom;
        uint8_t bottom_size = in_bytes[offset++];
        
        for (uint8_t i = 0; i < bottom_size; ++i) {
            uint8_t cmd = in_bytes[offset];
            uint8_t str_size = in_bytes[offset + 1];
            std::vector<uint8_t> str(in_bytes.begin() + offset + 2, in_bytes.begin() + offset + 2 + str_size);
            bottom.emplace_back(cmd, std::move(str));
            offset += 2 + str_size;
        }
        
        std::vector<BKString> top;
        uint8_t top_size = in_bytes[offset++];
        
        for (uint8_t i = 0; i < top_size; ++i) {
            uint8_t cmd = in_bytes[offset];
            uint8_t str_size = in_bytes[offset + 1];
            std::vector<uint8_t> str(in_bytes.begin() + offset + 2, in_bytes.begin() + offset + 2 + str_size);
            top.emplace_back(cmd, std::move(str));
            offset += 2 + str_size;
        }
        
        return Dialog(bottom, top);
    }

    static Dialog read(const std::filesystem::path& path) {
        YAML::Node doc = YAML::LoadFile(path.string());
        if (doc["type"].as<std::string>() != "Dialog") {
            throw std::runtime_error("Invalid Dialog type");
        }
        
        std::vector<BKString> bottom;
        for (const auto& item : doc["bottom"]) {
            bottom.push_back(BKString::from_yaml(item));
        }
        
        std::vector<BKString> top;
        for (const auto& item : doc["top"]) {
            top.push_back(BKString::from_yaml(item));
        }
        
        return Dialog(bottom, top);
    }

    std::vector<uint8_t> to_bytes() const override {
        std::vector<uint8_t> out = {0x01, 0x03, 0x00};
        out.push_back(static_cast<uint8_t>(bottom.size()));
        for (const auto& text : bottom) {
            out.push_back(text.get_cmd());
            out.push_back(static_cast<uint8_t>(text.get_string().size()));
            out.insert(out.end(), text.get_string().begin(), text.get_string().end());
        }
        out.push_back(static_cast<uint8_t>(top.size()));
        for (const auto& text : top) {
            out.push_back(text.get_cmd());
            out.push_back(static_cast<uint8_t>(text.get_string().size()));
            out.insert(out.end(), text.get_string().begin(), text.get_string().end());
        }
        return out;
    }

    AssetType get_type() const override {
        std::cout << "dialogue::get_type() called" << std::endl;
        return a_type;
    }

    void write(const std::filesystem::path& path) const override {
        std::ofstream bin_file(path);
        if (!bin_file) {
            throw std::runtime_error("Failed to create file");
        }
        
        bin_file << "type: Dialog\n";
        bin_file << "bottom:\n";
        for (const auto& text : bottom) {
            bin_file << "  - { cmd: 0x" << std::hex << static_cast<int>(text.get_cmd()) << ", string: \"";
            bin_file.write(reinterpret_cast<const char*>(text.get_string().data()), text.get_string().size());
            bin_file << "\" }\n";
        }
        bin_file << "top:\n";
        for (const auto& text : top) {
            bin_file << "  - { cmd: 0x" << std::hex << static_cast<int>(text.get_cmd()) << ", string: \"";
            bin_file.write(reinterpret_cast<const char*>(text.get_string().data()), text.get_string().size());
            bin_file << "\" }\n";
        }
    }
};

#endif // DIALOG_H