#ifndef CONT_INPUT_H
#define CONT_INPUT_H

#include <vector>
#include <cstdint>
#include "yaml-cpp/yaml.h"

class ContInput {
public:
    int8_t x;
    int8_t y;
    uint16_t buttons;
    uint8_t frames;

    ContInput() : x(0), y(0), buttons(0), frames(0) {}

    ContInput(int8_t x, int8_t y, uint16_t buttons, uint8_t frames)
        : x(x), y(y), buttons(buttons), frames(frames) {}

    std::vector<uint8_t> to_bytes() const {
        return {
            static_cast<uint8_t>(x),
            static_cast<uint8_t>(y),
            static_cast<uint8_t>(buttons >> 8), // Big-endian (high byte first)
            static_cast<uint8_t>(buttons & 0xFF), // Low byte
            frames,
            0x00 // Padding byte (same as in Rust)
        };
    }

    static ContInput from_yaml(const YAML::Node& yaml) {
        ContInput input;
        input.x = static_cast<int8_t>(yaml["x"].as<int64_t>());
        input.y = static_cast<int8_t>(yaml["y"].as<int64_t>());
        input.buttons = static_cast<uint16_t>(yaml["buttons"].as<int64_t>());
        input.frames = static_cast<uint8_t>(yaml["frames"].as<int64_t>());
        return input;
    }

    // **Fix: Add Getter Methods**
    /* int8_t get_x() const { return x; }
    int8_t get_y() const { return y; }
    uint16_t get_buttons() const { return buttons; }
    uint8_t get_frames() const { return frames; } */

};

#endif // CONT_INPUT_H
