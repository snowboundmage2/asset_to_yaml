#ifndef DEMO_BUTTON_FILE_H
#define DEMO_BUTTON_FILE_H

#include "Asset.h"
#include "ContInput.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "yaml-cpp/yaml.h"

class DemoButtonFile : public Asset {
private:
    std::vector<ContInput> inputs;
    uint8_t frame1_flag;

public:
    DemoButtonFile(std::vector<ContInput> inputs, uint8_t frame1_flag)
        : inputs(std::move(inputs)), frame1_flag(frame1_flag) {
            a_type = AssetType::DemoInput;
        }

    static DemoButtonFile from_bytes(const std::vector<uint8_t>& in_bytes) {
        if (in_bytes.size() < 4) {
            return DemoButtonFile({}, 0);
        }

        uint32_t expected_length = (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3];
        uint8_t f1f = in_bytes[9];

        std::vector<ContInput> inputs;
        for (size_t i = 4; i + 6 <= in_bytes.size(); i += 6) {
            inputs.emplace_back(static_cast<int8_t>(in_bytes[i]), static_cast<int8_t>(in_bytes[i + 1]),
                                static_cast<uint16_t>((in_bytes[i + 2] << 8) | in_bytes[i + 3]),
                                in_bytes[i + 4]);
        }

        if (expected_length != inputs.size() * 6) {
            throw std::runtime_error("Byte size mismatch in DemoButtonFile");
        }

        return DemoButtonFile(inputs, f1f);
    }

    static DemoButtonFile read(const std::filesystem::path& path) {
        YAML::Node doc = YAML::LoadFile(path.string());
        if (doc["type"].as<std::string>() != "DemoInput") {
            throw std::runtime_error("Invalid DemoButtonFile type");
        }

        uint8_t f1f = static_cast<uint8_t>(doc["flag"].as<int64_t>());
        std::vector<ContInput> inputs;
        for (const auto& item : doc["inputs"]) {
            inputs.push_back(ContInput::from_yaml(item));
        }

        return DemoButtonFile(inputs, f1f);
    }

    std::vector<uint8_t> to_bytes() const override {
        if (inputs.empty()) {
            return {};
        }

        std::vector<uint8_t> output(4);
        uint32_t length = inputs.size() * 6;
        output[0] = (length >> 24) & 0xFF;
        output[1] = (length >> 16) & 0xFF;
        output[2] = (length >> 8) & 0xFF;
        output[3] = length & 0xFF;

        std::vector<uint8_t> input_bytes;
        for (const auto& input : inputs) {
            auto bytes = input.to_bytes();
            input_bytes.insert(input_bytes.end(), bytes.begin(), bytes.end());
        }
        
        if (!input_bytes.empty()) {
            input_bytes[5] = frame1_flag;
        }

        output.insert(output.end(), input_bytes.begin(), input_bytes.end());
        return output;
    }

    AssetType get_type() const override {
        //std::cout << "demobutton::get_type() called" << std::endl;
        return a_type;
    }

    void write(const std::filesystem::path& path) const override {
        std::ofstream demo_file(path);
        if (!demo_file) {
            throw std::runtime_error("Failed to create file");
        }

        demo_file << "type: DemoInput\n";
        demo_file << "flag: 0x" << std::hex << static_cast<int>(frame1_flag) << "\n";
        if (inputs.empty()) {
            demo_file << "inputs: []\n";
            return;
        }
        demo_file << "inputs:\n";
        for (const auto& input : inputs) {
            demo_file << "  - {x: " << static_cast<int>(input.get_x())
                      << ", y: " << static_cast<int>(input.get_y())
                      << ", buttons: 0x" << std::hex << static_cast<int>(input.get_buttons())
                      << ", frames: " << std::dec << static_cast<int>(input.get_frames())
                      << "}\n";
        }
    }
};

#endif // DEMO_BUTTON_FILE_H