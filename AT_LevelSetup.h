#ifndef LEVEL_SETUP_H
#define LEVEL_SETUP_H

#include "Asset.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>

class LevelSetup : public Asset {
private:
    std::vector<uint8_t> bytes;

public:
    explicit LevelSetup(std::vector<uint8_t> bytes) : bytes(std::move(bytes)) {
    }

    static LevelSetup from_bytes(const std::vector<uint8_t>& in_bytes) {
        return LevelSetup(in_bytes);
    }

    static LevelSetup read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
        
        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return LevelSetup(buffer);
    }

    std::vector<uint8_t> to_bytes() const override {
        return bytes;
    }

    AssetType get_type() const override {
        return AssetType::LevelSetup;
    }

    void write(const std::filesystem::path& path) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to create file");
        }
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
};

#endif // LEVEL_SETUP_H