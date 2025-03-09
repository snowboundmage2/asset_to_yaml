#ifndef VECU8_TO_STRING_H
#define VECU8_TO_STRING_H

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

std::string vecu8_to_string(const std::vector<uint8_t>& bytes) {
    if (bytes.empty()) return "";
    
    std::ostringstream out;
    for (size_t i = 0; i < bytes.size() - 1; ++i) {
        uint8_t b = bytes[i];
        if (b < 0x20 || b > 0x7E) { // Non-printable ASCII characters
            out << "\\x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        } else {
            out << static_cast<char>(b);
        }
    }
    return out.str();
}

#endif // VECU8_TO_STRING_H
