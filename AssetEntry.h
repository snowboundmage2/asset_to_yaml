#ifndef ASSET_ENTRY_H
#define ASSET_ENTRY_H

#include "AssetMeta.h"
#include "Asset.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include "yaml-cpp/yaml.h"

class AssetEntry {
public:
    size_t uid;
    size_t seg;
    AssetMeta meta;
    std::optional<std::shared_ptr<Asset>> data;

    AssetEntry(size_t uid)
        : uid(uid), seg(0), meta(0, false, 4), data(std::nullopt) {}

    AssetEntry(size_t uid, size_t seg, AssetMeta meta, std::optional<std::shared_ptr<Asset>> data)
        : uid(uid), seg(seg), meta(meta), data(std::move(data)) {}

    static AssetEntry from_yaml(const YAML::Node& yaml) {
        if (!yaml["uid"].IsDefined() || !yaml["uid"].IsScalar()) {
            throw std::runtime_error("Could not read uid as integer");
        }
        size_t uid = yaml["uid"].as<size_t>();
        bool c_type = yaml["compressed"].as<bool>();
        uint16_t t_type = yaml["flags"].as<uint16_t>();
        
        AssetMeta meta(0, c_type, t_type);
        return AssetEntry(uid, 0, meta, std::nullopt);
    }

    int get_type2() const {
        //AssetType 
        return 0; // Placeholder implementation
    }
};

#endif // ASSET_ENTRY_H