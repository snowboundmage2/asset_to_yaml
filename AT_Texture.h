#ifndef TEXTURE_H
#define TEXTURE_H

#include "Asset.h"
#include "ImgFmt.h"
#include <vector>
#include <optional>
#include <cstdint>
#include <stdexcept>
#include <fstream>

class Texture : public Asset {
public:

    std::vector<uint8_t> bytes;

    ImgFmt texture_type;
    size_t width;
    size_t height;
    std::optional<std::vector<uint8_t>> palette;
    std::vector<uint8_t> pixel_data;
    std::vector<uint8_t> data;

    Texture(ImgFmt texture_type, size_t w, size_t h, const std::vector<uint8_t>& bin)
        : texture_type(texture_type), width(w), height(h), data(bin) {
        } 
/*     Texture(ImgFmt texture_type, size_t w, size_t h, const std::vector<uint8_t>& bin)
    : texture_type(texture_type), width(w), height(h) {
        if (texture_type == ImgFmt::CI4) {
            if (bin.size() < 0x20) throw std::runtime_error("Invalid bin size for CI4 format: " + std::to_string(bin.size()));
            palette = std::vector<uint8_t>(bin.begin(), bin.begin() + 0x20);
            pixel_data.assign(bin.begin() + 0x20, bin.end());
        } else if (texture_type == ImgFmt::CI8) {
            if (bin.size() < 0x200) throw std::runtime_error("Invalid bin size for CI8 format");
            palette = std::vector<uint8_t>(bin.begin(), bin.begin() + 0x200);
            pixel_data.assign(bin.begin() + 0x200, bin.end());
        } else {
            pixel_data = bin;
        }
    } */

    std::vector<uint8_t> to_bytes() const override {
        std::vector<uint8_t> bytes;
        if (palette) {
            bytes.insert(bytes.end(), palette->begin(), palette->end());
        }
        bytes.insert(bytes.end(), pixel_data.begin(), pixel_data.end());
        return bytes;
    }

    AssetType get_type() const override {
        return AssetType::Texture;
    }

    std::vector<uint8_t> to_rgba32() const {
        switch (texture_type) {
            case ImgFmt::CI4:
                if (!palette) throw std::runtime_error("Expected CI4 palette, but none found");
                return ci4_to_rgba32(pixel_data, *palette);
            case ImgFmt::CI8:
                if (!palette) throw std::runtime_error("Expected CI8 palette, but none found");
                return ci8_to_rgba32(pixel_data, *palette);
            case ImgFmt::RGBA16:
                return rgba16_to_rgba32(pixel_data);
            case ImgFmt::RGBA32:
                return pixel_data;
            case ImgFmt::I4:
                return i4_to_rgba32(pixel_data);
            case ImgFmt::I8:
                return i8_to_rgba32(pixel_data);
            case ImgFmt::IA4:
                return ia4_to_rgba32(pixel_data);
            case ImgFmt::IA8:
                return ia8_to_rgba32(pixel_data);
            default:
                throw std::runtime_error("Image type not implemented yet");
        }
    }

    static std::vector<uint8_t> rgba16_to_rgba32(const std::vector<uint8_t>& rgba16);
    static std::vector<uint8_t> ci4_to_rgba32(const std::vector<uint8_t>& ci4, const std::vector<uint8_t>& palette);
    static std::vector<uint8_t> ci8_to_rgba32(const std::vector<uint8_t>& ci8, const std::vector<uint8_t>& palette);
    static std::vector<uint8_t> i4_to_rgba32(const std::vector<uint8_t>& i4);
    static std::vector<uint8_t> i8_to_rgba32(const std::vector<uint8_t>& i8);
    static std::vector<uint8_t> ia4_to_rgba32(const std::vector<uint8_t>& ia4);
    static std::vector<uint8_t> ia8_to_rgba32(const std::vector<uint8_t>& ia8);

    void write(const std::filesystem::path& path) const override {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to create file");
        }
        file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }

};

#endif // TEXTURE_H
