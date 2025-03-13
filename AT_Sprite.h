#ifndef SPRITE_H
#define SPRITE_H

#include "ImgFmt.h"
#include "SpriteFrame.h"
#include "SpriteChunk.h"
#include "Asset.h"
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <optional>
//#include <png.hpp>

class Sprite : public Asset {
private:
    ImgFmt format;
    std::vector<SpriteFrame> frames;
    std::vector<uint8_t> bytes;

public:
    Sprite(ImgFmt format, std::vector<SpriteFrame> frames, std::vector<uint8_t> bytes)
        : format(format), frames(std::move(frames)), bytes(std::move(bytes)) {
        }
        
    Sprite(std::vector<uint8_t> bytes) : bytes(std::move(bytes)) {
        }

    static Sprite from_bytes(const std::vector<uint8_t>& in_bytes) {
       return Sprite(in_bytes);
       /*  uint16_t frame_cnt = (in_bytes[0] << 8) | in_bytes[1];
        uint16_t format_code = (in_bytes[2] << 8) | in_bytes[3];
        
        ImgFmt frmt = static_cast<ImgFmt>(format_code);
        if (frame_cnt > 0x100) {
            size_t offset = 8;
            SpriteChunk chunk = SpriteChunk::from_bytes(in_bytes, offset, ImgFmt::RGBA16);
            SpriteFrame frame(chunk.w, chunk.h, {}, {std::vector<uint8_t>(in_bytes.begin() + 8, in_bytes.begin() + 16)},
                              std::nullopt, Texture::rgba16_to_rgba32(chunk.pixel_data));
            return Sprite(frmt, {frame}, in_bytes);
        }

        std::vector<SpriteFrame> frames;
        size_t base_offset = 0x10 + frame_cnt * 4;
        for (size_t i = 0; i < frame_cnt; ++i) {
            size_t offset = (in_bytes[0x10 + i * 4] << 24) |
                            (in_bytes[0x11 + i * 4] << 16) |
                            (in_bytes[0x12 + i * 4] << 8) |
                            in_bytes[0x13 + i * 4];
            frames.push_back(SpriteFrame::from_bytes(in_bytes, base_offset + offset, frmt));
        }
        
        return Sprite(frmt, frames, in_bytes); */
    }

    static Sprite read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
        std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return Sprite(ImgFmt::Unknown, {}, bytes);
    }

    std::vector<uint8_t> to_bytes() const override {
        return bytes;
    }

    AssetType get_type() const override {
        //std::cout << "sprite::get_type() called" << std::endl;
        return AssetType::Sprite;
    }

    void write(const std::filesystem::path& path) const override {
        std::ofstream bin_file(path, std::ios::binary);
        if (!bin_file) {
            throw std::runtime_error("Failed to create file");
        }
        bin_file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());

        std::filesystem::path base_path = path.parent_path() / path.stem();
        std::filesystem::create_directories(base_path);
        
        std::ofstream desc_file(base_path / "sprite.yaml");
        if (!desc_file) {
            throw std::runtime_error("Failed to create descriptor file");
        }
        desc_file << "type: Sprite\nformat: " << static_cast<int>(format) << "\nframes:\n";

        for (size_t i = 0; i < frames.size(); ++i) {
            std::filesystem::path img_path = base_path / (std::to_string(i) + ".png");
            desc_file << "  - " << img_path.string() << "\n";

            std::ofstream png_file(img_path, std::ios::binary);
            if (!png_file) {
                throw std::runtime_error("Failed to create PNG file");
            }
            
/*             png::image<png::rgb_pixel> image(frames[i].width, frames[i].height);
            for (size_t y = 0; y < frames[i].height; ++y) {
                for (size_t x = 0; x < frames[i].width; ++x) {
                    size_t idx = (y * frames[i].width + x) * 4;
                    png::rgb_pixel rgb_pixel(
                        frames[i].pixel_data[idx],
                        frames[i].pixel_data[idx + 1],
                        frames[i].pixel_data[idx + 2]
                    );
                    image.set_pixel(x, y, rgb_pixel);
                }
            } */
            // image.write(img_path.string());
        }
    }
};

#endif // SPRITE_H