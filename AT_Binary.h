#ifndef BINARY_ASSET_H
#define BINARY_ASSET_H

#include <vector>
#include <fstream>
#include <filesystem>
#include "Asset.h"

class Binary : public Asset {
private:
    std::vector<uint8_t> bytes;

public:
    // Constructor from bytes
    explicit Binary(const std::vector<uint8_t>& in_bytes) : bytes(in_bytes) {}

    // Static function to create from bytes (equivalent to Rust `from_bytes`)
    static Binary from_bytes(const std::vector<uint8_t>& in_bytes) {
        return Binary(in_bytes);
    }

    // Static function to read from a file (equivalent to Rust `read`)
    static Binary read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }
        return Binary(std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {}));
    }

    // Override to_bytes function (equivalent to Rust `to_bytes`)
    std::vector<uint8_t> to_bytes() const override {
        return bytes;
    }

    // Override get_type function (equivalent to Rust `get_type`)
    AssetType get_type() const override {
        return AssetType::Binary;
    }

    // Override write function (equivalent to Rust `write`)
    void write(const std::filesystem::path& path) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to create file: " + path.string());
        }
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
};

#endif // BINARY_ASSET_H
