#ifndef MODEL_H
#define MODEL_H

#include "Asset.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>

class Model : public Asset {
private:
    std::vector<uint8_t> bytes;

public:
    explicit Model(std::vector<uint8_t> bytes) : bytes(std::move(bytes)) {
    }

    static Model from_bytes(const std::vector<uint8_t>& in_bytes) {
        return Model(in_bytes);
    }

    static Model read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
        
        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return Model(buffer);
    }

    std::vector<uint8_t> to_bytes() const override {
        return bytes;
    }

    AssetType get_type() const override {
        //std::cout << "model::get_type() called" << std::endl;
        return AssetType::Model;
    }

    void write(const std::filesystem::path& path) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to create file");
        }
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
};

#endif // MODEL_H
