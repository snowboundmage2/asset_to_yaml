#ifndef BKSTRING_H
#define BKSTRING_H

#include <vector>
#include <cstdint>
#include <string>
#include "yaml-cpp/yaml.h"

class BKString {
private:
    uint8_t cmd;
    std::vector<uint8_t> string;

public:
    BKString(uint8_t cmd, std::vector<uint8_t> str) : cmd(cmd), string(std::move(str)) {}

    static BKString from_yaml(const YAML::Node& yaml) {
        uint8_t cmd = static_cast<uint8_t>(yaml["cmd"].as<int>());
        std::string str = yaml["string"].as<std::string>();
        return BKString(cmd, std::vector<uint8_t>(str.begin(), str.end()));
    }

    // **Fix: Add getter methods**
    uint8_t get_cmd() const { return cmd; }
    const std::vector<uint8_t>& get_string() const { return string; }
};

#endif // BKSTRING_H
