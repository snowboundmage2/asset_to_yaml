#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include "SpriteChunk.h"
#include "AT_Texture.h"
#include "ImgFmt.h"
#include <vector>
#include <optional>
#include <cstdint>
#include <array>

class SpriteFrame {
public:
    size_t width;
    size_t height;
    std::vector<uint8_t> header;
    std::vector<std::vector<uint8_t>> chunk_headers;
    std::optional<std::vector<uint8_t>> palette;
    std::vector<uint8_t> pixel_data;

    SpriteFrame(size_t w, size_t h, std::vector<uint8_t> header,
                std::vector<std::vector<uint8_t>> chk_hdrs,
                std::optional<std::vector<uint8_t>> palette,
                std::vector<uint8_t> pixel_data)
        : width(w), height(h), header(std::move(header)),
          chunk_headers(std::move(chk_hdrs)), palette(std::move(palette)),
          pixel_data(std::move(pixel_data)) {}

    static SpriteFrame from_bytes(const std::vector<uint8_t>& bin, size_t file_offset, const ImgFmt& format) {
        std::vector<uint8_t> header(bin.begin() + file_offset, bin.begin() + file_offset + 0x14);
        const uint8_t* frame_bin = bin.data() + file_offset;

        size_t w = (frame_bin[4] << 8) | frame_bin[5];
        size_t h = (frame_bin[6] << 8) | frame_bin[7];
        uint16_t chunk_cnt = (frame_bin[8] << 8) | frame_bin[9];

        size_t offset = file_offset + 0x14;
        offset = (offset + (8 - 1)) & ~(8 - 1); // Align to 8 bytes
        std::optional<std::vector<uint8_t>> palette;
        std::vector<std::vector<uint8_t>> chk_hdrs;
        std::vector<SpriteChunk> chunks;

        if (format == ImgFmt::CI4) {
            palette = std::vector<uint8_t>(bin.begin() + offset, bin.begin() + offset + 0x20);
            offset += 0x20;
        } else if (format == ImgFmt::CI8) {
            palette = std::vector<uint8_t>(bin.begin() + offset, bin.begin() + offset + 0x200);
            offset += 0x200;
        }

        for (size_t i = 0; i < chunk_cnt; ++i) {
            chk_hdrs.emplace_back(bin.begin() + offset, bin.begin() + offset + 8);
            chunks.push_back(SpriteChunk::from_bytes(bin, offset, format));
        }

        std::vector<std::vector<std::array<uint8_t, 4>>> pxl_data(h, std::vector<std::array<uint8_t, 4>>(w, {0, 0, 0, 0}));

        for (const auto& chnk : chunks) {
            std::vector<uint8_t> raw_data;
            switch (format) {
                case ImgFmt::CI4:
                    raw_data = Texture::ci4_to_rgba32(chnk.pixel_data, *palette);
                    break;
                case ImgFmt::CI8:
                    raw_data = Texture::ci8_to_rgba32(chnk.pixel_data, *palette);
                    break;
                case ImgFmt::I4:
                    raw_data = Texture::i4_to_rgba32(chnk.pixel_data);
                    break;
                case ImgFmt::I8:
                    raw_data = Texture::i8_to_rgba32(chnk.pixel_data);
                    break;
                case ImgFmt::RGBA16:
                    raw_data = Texture::rgba16_to_rgba32(chnk.pixel_data);
                    break;
                case ImgFmt::RGBA32:
                    raw_data = chnk.pixel_data;
                    break;
                case ImgFmt::IA4:
                    raw_data = Texture::ia4_to_rgba32(chnk.pixel_data);
                    break;
                case ImgFmt::IA8:
                    raw_data = Texture::ia8_to_rgba32(chnk.pixel_data);
                    break;
                default:
                    throw std::runtime_error("Unsupported image format");
            }

            auto row_data = raw_data;
            size_t chunk_w = chnk.w;
            for (size_t j = 0; j < row_data.size() / (4 * chunk_w); ++j) {
                for (size_t i = 0; i < chunk_w; ++i) {
                    size_t fx = chnk.x + i;
                    size_t fy = chnk.y + j;
                    if (fx < w && fy < h) {
                        std::array<uint8_t, 4> pxl = {row_data[(j * chunk_w + i) * 4],
                                                      row_data[(j * chunk_w + i) * 4 + 1],
                                                      row_data[(j * chunk_w + i) * 4 + 2],
                                                      row_data[(j * chunk_w + i) * 4 + 3]};
                        pxl_data[fy][fx] = pxl;
                    }
                }
            }
        }

        std::vector<uint8_t> final_pixel_data;
        for (const auto& row : pxl_data) {
            for (const auto& pxl : row) {
                final_pixel_data.insert(final_pixel_data.end(), pxl.begin(), pxl.end());
            }
        }

        return SpriteFrame(w, h, header, chk_hdrs, palette, final_pixel_data);
    }
};

#endif // SPRITE_FRAME_H
