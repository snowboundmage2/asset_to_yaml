#ifndef TEXTURE_H
#define TEXTURE_H

#include "ImgFmt.h"
#include <vector>
#include <optional>
#include <cstdint>
#include <stdexcept>

class Texture {
public:
    ImgFmt texture_type;
    size_t width;
    size_t height;
    std::optional<std::vector<uint8_t>> palette;
    std::vector<uint8_t> pixel_data;

    Texture(ImgFmt texture_type, size_t w, size_t h, const std::vector<uint8_t>& bin);

    std::vector<uint8_t> to_rgba32() const;
    static std::vector<uint8_t> rgba16_to_rgba32(const std::vector<uint8_t>& rgba16);
    static std::vector<uint8_t> ci4_to_rgba32(const std::vector<uint8_t>& ci4, const std::vector<uint8_t>& palette);
    static std::vector<uint8_t> ci8_to_rgba32(const std::vector<uint8_t>& ci8, const std::vector<uint8_t>& palette);
    static std::vector<uint8_t> i4_to_rgba32(const std::vector<uint8_t>& i4);
    static std::vector<uint8_t> i8_to_rgba32(const std::vector<uint8_t>& i8);
    static std::vector<uint8_t> ia4_to_rgba32(const std::vector<uint8_t>& ia4);
    static std::vector<uint8_t> ia8_to_rgba32(const std::vector<uint8_t>& ia8);
};

#endif // TEXTURE_H