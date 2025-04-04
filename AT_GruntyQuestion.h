#ifndef GRUNTY_QUESTION_H
#define GRUNTY_QUESTION_H

#include "Asset.h"
#include "BKString.h"
#include <vector>
#include <array>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "yaml-cpp/yaml.h"

class GruntyQuestion : public Asset {
private:
    std::vector<BKString> question;
    std::array<BKString, 3> options;

public:
    // Constructor
    GruntyQuestion(std::vector<BKString> question, std::array<BKString, 3> options)
        : question(std::move(question)), options(options) {
        }

    // Static function to construct from binary data
    static GruntyQuestion from_bytes(const std::vector<uint8_t>& in_bytes) {
        size_t offset = 6;
        uint8_t str_cnt = in_bytes[5];
        
        std::vector<BKString> texts;
        for (uint8_t i = 0; i < str_cnt; ++i) {
            uint8_t cmd = in_bytes[offset];
            uint8_t str_size = in_bytes[offset + 1];
            std::vector<uint8_t> str(in_bytes.begin() + offset + 2, in_bytes.begin() + offset + 2 + str_size);
            texts.emplace_back(cmd, std::move(str));
            offset += 2 + str_size;
        }
        
        if (texts.size() < 3) {
            throw std::runtime_error("Not enough options in GruntyQuestion data.");
        }

        std::vector<BKString> q_text(texts.begin(), texts.end() - 3);
        std::array<BKString, 3> o_text = {texts[texts.size() - 3], texts[texts.size() - 2], texts[texts.size() - 1]};
        
        return GruntyQuestion(q_text, o_text);
    }

    // Static function to construct from a YAML file
    static GruntyQuestion read(const std::filesystem::path& path) {
        YAML::Node doc = YAML::LoadFile(path.string());
        if (doc["type"].as<std::string>() != "GruntyQuestion") {
            throw std::runtime_error("Invalid GruntyQuestion type");
        }

        std::vector<BKString> q;
        for (const auto& item : doc["question"]) {
            q.push_back(BKString::from_yaml(item));
        }

        std::array<BKString, 3> a = {
            BKString::from_yaml(doc["options"][0]),
            BKString::from_yaml(doc["options"][1]),
            BKString::from_yaml(doc["options"][2])
        };

        return GruntyQuestion(q, a);
    }

    // Convert to binary format
    std::vector<uint8_t> to_bytes() const override {
        std::vector<uint8_t> out = {0x01, 0x03, 0x00, 0x05, 0x00};
        out.push_back(static_cast<uint8_t>(question.size() + options.size()));
        for (const auto& text : question) {
            out.push_back(text.get_cmd());
            out.push_back(static_cast<uint8_t>(text.get_string().size()));
            out.insert(out.end(), text.get_string().begin(), text.get_string().end());
        }
        for (const auto& text : options) {
            out.push_back(text.get_cmd());
            out.push_back(static_cast<uint8_t>(text.get_string().size()));
            out.insert(out.end(), text.get_string().begin(), text.get_string().end());
        }
        return out;
    }

    // Get asset type
    AssetType get_type() const override {
        return AssetType::GruntyQuestion;
    }

    // Write asset to YAML file
    void write(const std::filesystem::path& path) const override {
        std::ofstream bin_file(path);
        if (!bin_file) {
            throw std::runtime_error("Failed to create file");
        }

        bin_file << "type: GruntyQuestion\n";
        bin_file << "question:\n";
        for (const auto& text : question) {
            bin_file << "  - { cmd: 0x" << std::hex << static_cast<int>(text.get_cmd()) << ", string: \"";
            bin_file.write(reinterpret_cast<const char*>(text.get_string().data()), text.get_string().size());
            bin_file << "\" }\n";
        }
        bin_file << "options:\n";
        for (const auto& text : options) {
            bin_file << "  - { cmd: 0x" << std::hex << static_cast<int>(text.get_cmd()) << ", string: \"";
            bin_file.write(reinterpret_cast<const char*>(text.get_string().data()), text.get_string().size());
            bin_file << "\" }\n";
        }
    }
};

#endif // GRUNTY_QUESTION_H