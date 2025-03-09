#ifndef IMG_FMT_H
#define IMG_FMT_H

#include <variant>
#include <cstdint>

enum class ImgFmt {
    CI4,
    CI8,
    I4,
    I8,
    RGBA16,
    RGBA32,
    IA4,
    IA8,
    Unknown  // Special case, handled separately
};

struct ImgFmtVariant {
    ImgFmt format;
    std::variant<std::monostate, uint16_t> extraData; // Used for Unknown
    
    ImgFmtVariant(ImgFmt fmt) : format(fmt), extraData(std::monostate{}) {}
    ImgFmtVariant(uint16_t value) : format(ImgFmt::Unknown), extraData(value) {}
};

#endif // IMG_FMT_H
