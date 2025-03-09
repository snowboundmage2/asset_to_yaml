#ifndef ASSET_META_H
#define ASSET_META_H

#include <vector>
#include <cstdint>
#include <stdexcept>

class AssetMeta {
public:
    size_t offset;
    bool c_flag;
    uint16_t t_flag;

    AssetMeta(size_t offset, bool c_flag, uint16_t t_flag)
        : offset(offset), c_flag(c_flag), t_flag(t_flag) {}

    static AssetMeta from_bytes(const std::vector<uint8_t>& in_bytes) {
        if (in_bytes.size() < 8) {
            throw std::runtime_error("Invalid byte array size for AssetMeta");
        }
        size_t offset = (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3];
        bool c_flag = in_bytes[5] != 0;
        uint16_t t_flag = (in_bytes[6] << 8) | in_bytes[7];
        return AssetMeta(offset, c_flag, t_flag);
    }

    std::vector<uint8_t> to_bytes() const {
        std::vector<uint8_t> out;
        out.push_back((offset >> 24) & 0xFF);
        out.push_back((offset >> 16) & 0xFF);
        out.push_back((offset >> 8) & 0xFF);
        out.push_back(offset & 0xFF);
        out.push_back(0x00);
        out.push_back(c_flag ? 1 : 0);
        out.push_back((t_flag >> 8) & 0xFF);
        out.push_back(t_flag & 0xFF);
        return out;
    }
};

#endif // ASSET_META_H
