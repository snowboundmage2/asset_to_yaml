#ifndef CONT_INPUT_H
#define CONT_INPUT_H

#include <vector>
#include <cstdint>
#include "yaml-cpp/yaml.h"

class ContInput {
private:
    int8_t x;
    int8_t y;
    uint16_t buttons;
    uint8_t frames;

public:
    ContInput(int8_t x, int8_t y, uint16_t buttons, uint8_t frames)
        : x(x), y(y), buttons(buttons), frames(frames) {}

    std::vector<uint8_t> to_bytes() const {
        return {static_cast<uint8_t>(x), static_cast<uint8_t>(y),
                static_cast<uint8_t>(buttons >> 8), static_cast<uint8_t>(buttons & 0xFF),
                frames, 0x00};
    }

    static ContInput from_yaml(const YAML::Node& yaml) {
        int8_t x = static_cast<int8_t>(yaml["x"].as<int64_t>());
        int8_t y = static_cast<int8_t>(yaml["y"].as<int64_t>());
        uint16_t buttons = static_cast<uint16_t>(yaml["buttons"].as<int64_t>());
        uint8_t frames = static_cast<uint8_t>(yaml["frames"].as<int64_t>());
        return ContInput(x, y, buttons, frames);
    }

    // **Fix: Add Getter Methods**
    int8_t get_x() const { return x; }
    int8_t get_y() const { return y; }
    uint16_t get_buttons() const { return buttons; }
    uint8_t get_frames() const { return frames; }
};

#endif // CONT_INPUT_H
