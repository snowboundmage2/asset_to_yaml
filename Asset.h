#ifndef ASSET_H
#define ASSET_H

#include <vector>
#include <string>
#include "AssetType.h"
#include <filesystem>

class Asset {
public:
    virtual ~Asset() = default;
    
    virtual std::vector<uint8_t> to_bytes() const = 0;
    virtual AssetType get_type() const = 0;
    virtual void write(const std::filesystem::path& path) const = 0;
};

#endif // ASSET_H
