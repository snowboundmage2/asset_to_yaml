#ifndef ANIMATION_H
#define ANIMATION_H

#include "Asset.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>

class Animation : public Asset {
    private:

        std::vector<uint8_t> bytes;
    
    public:
        // Constructor from bytes
        explicit Animation(const std::vector<uint8_t>& in_bytes) : bytes(in_bytes) {
        }
    
        // Static function to create an Animation from raw bytes
        static Animation from_bytes(const std::vector<uint8_t>& in_bytes) {
            return Animation(in_bytes);
        }
    
        // Static function to read from file
        static Animation read(const std::filesystem::path& path) {
            std::cout << "Animation::read() called" << std::endl;
            std::ifstream file(path, std::ios::binary);
            if (!file) {
                throw std::runtime_error("Failed to open file: " + path.string());
            }
            return Animation(std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {}));
        }
    
        // Convert to bytes
        std::vector<uint8_t> to_bytes() const override {
            std::cout << "Animation::to_bytes() called" << std::endl;
            return bytes;
        }
    
        // Get asset type
        AssetType get_type() const override {
            return AssetType::Animation;
        }
    
        // Write to file
        void write(const std::filesystem::path& path) const override {

            std::ofstream file(path, std::ios::binary);
            if (!file) {
                throw std::runtime_error("Failed to create file: " + path.string());
            }
            file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        }
};

#endif // ANIMATION_H
