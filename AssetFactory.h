#ifndef ASSET_FACTORY_H
#define ASSET_FACTORY_H

#include <memory>
#include <vector>
#include <cstdint>
#include "Asset.h"
#include "AT_Animation.h"
#include "AT_Model.h"
#include "AT_Sprite.h"
#include "AT_LevelSetup.h"
#include "AT_QuizQuestion.h"
#include "AT_GruntyQuestion.h"
#include "AT_Dialogue.h"
#include "AT_DemoButton.h"
#include "AT_MidiSeq.h"
#include "AT_Binary.h"

class AssetFactory {
public:
    // aka from_seg_indx_and_bytes
    static std::unique_ptr<Asset> from_seg_index_and_bytes(size_t segment, size_t i, const std::vector<uint8_t>& in_bytes) {
        // Segment 0: Animation
        if (segment == 0) {
            return std::make_unique<Animation>(Animation::from_bytes(in_bytes));
        }

        // Segment 1 & 3: Models & Sprites
        if (segment == 1 || segment == 3) {
            if (in_bytes.size() >= 4 && in_bytes[0] == 0x00 && in_bytes[1] == 0x00 &&
                in_bytes[2] == 0x00 && in_bytes[3] == 0x0B) {
                return std::make_unique<Model>(Model::from_bytes(in_bytes));
            }
            return std::make_unique<Sprite>(Sprite::from_bytes(in_bytes));
        }

        // Segment 2: LevelSetup
        if (segment == 2) {
            return std::make_unique<LevelSetup>(LevelSetup::from_bytes(in_bytes));
        }

        // Segment 4: Dialog, GruntyQuestions, QuizQuestions, DemoButtonFiles
        if (segment == 4) {
            if (in_bytes.size() >= 5 && in_bytes[0] == 0x01 && in_bytes[1] == 0x01 &&
                in_bytes[2] == 0x02 && in_bytes[3] == 0x05 && in_bytes[4] == 0x00) {
                return std::make_unique<QuizQuestion>(QuizQuestion::from_bytes(in_bytes));
            }
            if (in_bytes.size() >= 5 && in_bytes[0] == 0x01 && in_bytes[1] == 0x03 &&
                in_bytes[2] == 0x00 && in_bytes[3] == 0x05 && in_bytes[4] == 0x00) {
                return std::make_unique<GruntyQuestion>(GruntyQuestion::from_bytes(in_bytes));
            }
            if (in_bytes.size() >= 3 && in_bytes[0] == 0x01 && in_bytes[1] == 0x03 && in_bytes[2] == 0x00) {
                return std::make_unique<Dialog>(Dialog::from_bytes(in_bytes));
            }
            return std::make_unique<DemoButton>(DemoButton::from_bytes(in_bytes));
        }

        // Segment 5: Model
        if (segment == 5) {
            return std::make_unique<Model>(Model::from_bytes(in_bytes));
        }

        // Segment 6: MidiSeqFile
        if (segment == 6) {
            return std::make_unique<MidiSeqFile>(MidiSeqFile::from_bytes(in_bytes));
        }

        // Default: Binary
        return std::make_unique<Binary>(Binary::from_bytes(in_bytes));
    }
};

#endif // ASSET_FACTORY_H
