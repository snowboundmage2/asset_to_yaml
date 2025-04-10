#ifndef DEMO_BUTTON_H
#define DEMO_BUTTON_H

#include <vector>
#include <cstdint>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>  // Ensure you have yaml-cpp installed
#include "Asset.h"          // Asset Base Class

// Struct for Controller Input
struct ContInput {
    int8_t x;
    int8_t y;
    uint16_t buttons;
    uint8_t frames;

    // Convert to byte vector
    std::vector<uint8_t> to_bytes() const {
        return {
            static_cast<uint8_t>(x),
            static_cast<uint8_t>(y),
            static_cast<uint8_t>(buttons >> 8),  // High byte
            static_cast<uint8_t>(buttons & 0xFF),  // Low byte
            frames,
            0x00
        };
    }

    // Create ContInput from YAML
    static ContInput from_yaml(const YAML::Node& yaml) {
        return {
            static_cast<int8_t>(yaml["x"].as<int64_t>()),
            static_cast<int8_t>(yaml["y"].as<int64_t>()),
            static_cast<uint16_t>(yaml["buttons"].as<int64_t>()),
            static_cast<uint8_t>(yaml["frames"].as<int64_t>())
        };
    }
};

// Class for DemoButtonFile
class DemoButtonFile : public Asset {
private:
    std::vector<ContInput> inputs;
    uint8_t frame1_flag;

public:
    // Constructor
    DemoButtonFile(std::vector<ContInput> inputs, uint8_t flag)
        : inputs(std::move(inputs)), frame1_flag(flag) {}

    // Static function to create from bytes
    static DemoButtonFile from_bytes(const std::vector<uint8_t>& in_bytes) {
        if (in_bytes.size() < 4) {
            return DemoButtonFile({}, 0);
        }

        size_t expected_len = static_cast<size_t>(
            (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3]
        );

        uint8_t f1f = in_bytes[9];

        std::vector<ContInput> inputs;
        for (size_t i = 4; i + 6 <= in_bytes.size(); i += 6) {
            inputs.push_back(ContInput{
                static_cast<int8_t>(in_bytes[i]),
                static_cast<int8_t>(in_bytes[i + 1]),
                static_cast<uint16_t>((in_bytes[i + 2] << 8) | in_bytes[i + 3]),
                static_cast<uint8_t>(in_bytes[i + 4])
            });
        }

        assert(expected_len == inputs.size() * 6);
        return DemoButtonFile(inputs, f1f);
    }

    // Static function to read from YAML
    static DemoButtonFile read(const std::filesystem::path& path) {
        YAML::Node doc = YAML::LoadFile(path.string());
        if (!doc["type"] || doc["type"].as<std::string>() != "DemoInput") {
            throw std::runtime_error("Invalid YAML: Missing 'type: DemoInput'");
        }

        uint8_t f1f = static_cast<uint8_t>(doc["flag"].as<int64_t>());
        std::vector<ContInput> inputs;
        
        for (const auto& y : doc["inputs"]) {
            inputs.push_back(ContInput::from_yaml(y));
        }

        return DemoButtonFile(inputs, f1f);
    }

    // Convert to byte vector
    std::vector<uint8_t> to_bytes() const override {
        if (inputs.empty()) return {};

        std::vector<uint8_t> output(4);  // Reserve first 4 bytes for size
        size_t total_size = inputs.size() * 6;
        output[0] = (total_size >> 24) & 0xFF;
        output[1] = (total_size >> 16) & 0xFF;
        output[2] = (total_size >> 8) & 0xFF;
        output[3] = total_size & 0xFF;

        std::vector<uint8_t> input_bytes;
        for (const auto& input : inputs) {
            auto bytes = input.to_bytes();
            input_bytes.insert(input_bytes.end(), bytes.begin(), bytes.end());
        }

        input_bytes[5] = frame1_flag;  // Insert flag at the correct position
        output.insert(output.end(), input_bytes.begin(), input_bytes.end());
        return output;
    }

    // Get asset type
    AssetType get_type() const override {
        return AssetType::DemoInput;
    }

    // Write asset to file
    void write(const std::filesystem::path& path) const override {
        std::ofstream demo_file(path);
        if (!demo_file) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        demo_file << "type: DemoInput\n";
        demo_file << "flag: 0x" << std::hex << std::uppercase << static_cast<int>(frame1_flag) << "\n";

        if (inputs.empty()) {
            demo_file << "inputs: []\n";
            return;
        }

        demo_file << "inputs:\n";
        for (const auto& input : inputs) {
            demo_file << "  - {x: " << std::setw(3) << static_cast<int>(input.x)
                      << ", y: " << std::setw(3) << static_cast<int>(input.y)
                      << ", buttons: 0x" << std::hex << std::setw(4) << std::setfill('0') << input.buttons
                      << ", frames: " << std::dec << static_cast<int>(input.frames) << "}\n";
        }
    }
};

#endif // DEMO_BUTTON_H
