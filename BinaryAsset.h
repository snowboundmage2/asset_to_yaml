#ifndef BINARY_H
#define BINARY_H

#include "Asset.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>

class Binary : public Asset {
private:
    std::vector<uint8_t> bytes;

public:
    explicit Binary(const std::vector<uint8_t>& in_bytes) : bytes(in_bytes) {
        a_type = AssetType::Binary;
    }

    static Binary from_bytes(const std::vector<uint8_t>& in_bytes) {
        return Binary(in_bytes);
    }

    static Binary read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
        
        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return Binary(buffer);
    }

    std::vector<uint8_t> to_bytes() const override {
        return bytes;
    }

    AssetType get_type() const override {
        //std::cout << "binaryasset::get_type() called" << std::endl;
        return a_type;
    }

    void write(const std::filesystem::path& path) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to create file");
        }
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
};

#endif // BINARY_H
