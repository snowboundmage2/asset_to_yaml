#ifndef ASSET_ENTRY_H
#define ASSET_ENTRY_H

#include "AssetMeta.h"
#include "Asset.h"
#include <memory>
#include <stdexcept>
#include "yaml-cpp/yaml.h"
#include "AnimationAsset.h"
#include "BinaryAsset.h"
#include "DemoButton.h"
#include "DialogueAsset.h"
#include "GruntyQuestion.h"
#include "LevelSetup.h"
#include "MidiSeq.h"
#include "ModelAsset.h"
#include "QuizQuestion.h"
#include "Sprite.h"
#include "TextureAsset.h"

class AssetEntry {
public:
    size_t uid;
    size_t seg;
    AssetMeta meta;
    std::shared_ptr<Asset> data;
    AssetType ae_type;

    AssetEntry(size_t uid, size_t seg, AssetMeta meta, std::shared_ptr<Asset> data)
        : uid(uid), seg(seg), meta(meta), data(data) {
        // std::cout << "AssetEntry created with uid: " << uid << ", data: " << (data ? "yes" : "no") << std::endl;
    }

    bool has_data() const {
        //std::cout << "checking " << meta.offset << " for has_data" << std::endl;
        return data && data.get() != nullptr;
    }
    
    std::shared_ptr<Asset> get_asset() const {
        //std::cout << "getting " << meta.offset << "'s asset data" << std::endl;
        return data;
    }

    static AssetEntry from_yaml(const YAML::Node& yaml) {
        std::cout << "AssetEntry::from_yaml(): " << yaml << std::endl;
        if (!yaml["uid"].IsDefined() || !yaml["uid"].IsScalar()) {
            throw std::runtime_error("Could not read uid as integer");
        }
        size_t uid = yaml["uid"].as<size_t>();
        bool c_type = yaml["compressed"].as<bool>();
        uint16_t t_type = yaml["flags"].as<uint16_t>();
        
        AssetMeta meta(0, c_type, t_type);
        return AssetEntry(uid, 0, meta, nullptr);
    }

    //this never gets any data
    AssetType get_asset_type() const {
        if (data != nullptr) {
            //std::cout << "AssetEntry::get_asset_type() - data has value" << std::endl;
            return data->a_type;
        } else {
            //std::cout << "AssetEntry::get_asset_type() - data has no value" << std::endl;
            return AssetType::Binary;
        }
    }

    AssetType string_to_type(const std::string& typeStr) const {
        if (typeStr == "Animation") return AssetType::Animation;
        if (typeStr == "Binary") return AssetType::Binary;
        if (typeStr == "DemoInput") return AssetType::DemoInput;
        if (typeStr == "Dialog") return AssetType::Dialog;
        if (typeStr == "GruntyQuestion") return AssetType::GruntyQuestion;
        if (typeStr == "LevelSetup") return AssetType::LevelSetup;
        if (typeStr == "Midi") return AssetType::Midi;
        if (typeStr == "Model") return AssetType::Model;
        if (typeStr == "QuizQuestion") return AssetType::QuizQuestion;
        if (typeStr == "Sprite") return AssetType::Sprite;
        if (typeStr == "Texture") return AssetType::Texture;
        return (AssetType)NULL;  // Default case
    }

    std::string type_to_string(const AssetType& ass_type) const {
        if (ass_type == AssetType::Animation) return "Animation";
        if (ass_type == AssetType::Binary) return "Binary";
        if (ass_type == AssetType::DemoInput) return "DemoInput";
        if (ass_type == AssetType::Dialog) return "Dialog";
        if (ass_type == AssetType::GruntyQuestion) return "GruntyQuestion";
        if (ass_type == AssetType::LevelSetup) return "LevelSetup";
        if (ass_type == AssetType::Midi) return "Midi";
        if (ass_type == AssetType::Model) return "Model";
        if (ass_type == AssetType::QuizQuestion) return "QuizQuestion";
        if (ass_type == AssetType::Sprite) return "Sprite";
        if (ass_type == AssetType::Texture) return "Texture";
        return "broken";  // Default case
    }

private:
    std::shared_ptr<Animation> animation_;
    std::shared_ptr<Binary> binary_;
    std::shared_ptr<DemoButtonFile> demobuttonfile_;
    std::shared_ptr<Dialog> dialogue_;
    std::shared_ptr<GruntyQuestion> gruntyquestion_;
    std::shared_ptr<LevelSetup> levelsetup_;
    std::shared_ptr<MidiSeqFile> midiseqfile_;
    std::shared_ptr<Model> model_;
    std::shared_ptr<QuizQuestion> quizquestion_;
    std::shared_ptr<Sprite> sprite_;
    std::shared_ptr<Texture> texture_;
};

#endif // ASSET_ENTRY_H