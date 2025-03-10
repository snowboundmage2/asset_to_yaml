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

    AssetType string_to_type(const std::string& typeStr) const {
        if (typeStr == "Binary") return AssetType::Binary;
        if (typeStr == "Animation") return AssetType::Animation;
        if (typeStr == "DemoInput") return AssetType::DemoInput;
        if (typeStr == "Dialog") return AssetType::Dialog;
        if (typeStr == "GruntyQuestion") return AssetType::GruntyQuestion;
        if (typeStr == "LevelSetup") return AssetType::LevelSetup;
        if (typeStr == "Midi") return AssetType::Midi;
        if (typeStr == "Model") return AssetType::Model;
        if (typeStr == "QuizQuestion") return AssetType::QuizQuestion;
        if (typeStr == "Sprite") return AssetType::Sprite;
        return (AssetType)NULL;  // Default case
    }

    std::string type_to_string(const AssetType& ass_type) const {
        if (ass_type == AssetType::Binary) return "Binary";
        if (ass_type == AssetType::Animation) return "Animation";
        if (ass_type == AssetType::DemoInput) return "DemoInput";
        if (ass_type == AssetType::Dialog) return "Dialog";
        if (ass_type == AssetType::GruntyQuestion) return "GruntyQuestion";
        if (ass_type == AssetType::LevelSetup) return "LevelSetup";
        if (ass_type == AssetType::Midi) return "Midi";
        if (ass_type == AssetType::Model) return "Model";
        if (ass_type == AssetType::QuizQuestion) return "QuizQuestion";
        if (ass_type == AssetType::Sprite) return "Sprite";
        return "broken";  // Default case
    }
};

#endif // ASSET_ENTRY_H