#ifndef ASSET_TYPE_H
#define ASSET_TYPE_H

#include <variant>
#include "ImgFmt.h"

enum class AssetType {
    Animation,
    Binary,
    DemoInput,
    Dialog,
    GruntyQuestion,
    LevelSetup,
    Midi,
    Model,
    QuizQuestion,
    Sprite,  // Special case, handled separately
    Texture //may also need special handling later
};

struct AssetTypeVariant {
    AssetType type;
    std::variant<std::monostate, ImgFmt> extraData; // Used for Sprite
    
    AssetTypeVariant(AssetType t) : type(t), extraData(std::monostate{}) {}
    AssetTypeVariant(ImgFmt imgFmt) : type(AssetType::Sprite), extraData(imgFmt) {}
};

#endif // ASSET_TYPE_H
