#ifndef SPRITE_CHUNK_H
#define SPRITE_CHUNK_H

#include "ImgFmt.h"
#include <vector>
#include <cstddef>
#include <cstdint>

class SpriteChunk {
public:
    int x;
    int y;
    size_t w;
    size_t h;
    std::vector<uint8_t> pixel_data;

    SpriteChunk(int x, int y, size_t w, size_t h, std::vector<uint8_t> pixel_data)
        : x(x), y(y), w(w), h(h), pixel_data(std::move(pixel_data)) {}

    static SpriteChunk from_bytes(const std::vector<uint8_t>& bin, size_t& file_offset, const ImgFmt& format) {
        const uint8_t* chunk_bin = bin.data() + file_offset;
        
        int x = static_cast<int16_t>((chunk_bin[0] << 8) | chunk_bin[1]);
        int y = static_cast<int16_t>((chunk_bin[2] << 8) | chunk_bin[3]);
        size_t w = static_cast<uint16_t>((chunk_bin[4] << 8) | chunk_bin[5]);
        size_t h = static_cast<uint16_t>((chunk_bin[6] << 8) | chunk_bin[7]);
        
        file_offset += 8;
        file_offset = (file_offset + (8 - 1)) & ~(8 - 1); // Align to 8 bytes

        size_t pxl_size = 0;
        switch (format) {
            case ImgFmt::I4: case ImgFmt::IA4: case ImgFmt::CI4:
                pxl_size = 4; break;
            case ImgFmt::I8: case ImgFmt::IA8: case ImgFmt::CI8:
                pxl_size = 8; break;
            case ImgFmt::RGBA16:
                pxl_size = 16; break;
            case ImgFmt::RGBA32:
                pxl_size = 32; break;
            default:
                pxl_size = 0;
        }
        
        size_t data_size = (w * h * pxl_size) / 8;
        std::vector<uint8_t> data(bin.begin() + file_offset, bin.begin() + file_offset + data_size);
        file_offset += data_size;

        return SpriteChunk(x, y, w, h, data);
    }
};

#endif // SPRITE_CHUNK_H
