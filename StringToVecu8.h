#ifndef STRING_TO_VECU8_H
#define STRING_TO_VECU8_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>

std::vector<uint8_t> string_to_vecu8(const std::string& input) {
    std::vector<uint8_t> bytes(input.begin(), input.end());
    
    std::vector<size_t> squig_indices;
    for (size_t i = 0; i + 1 < bytes.size(); ++i) {
        if (bytes[i] == 0xC3 && bytes[i + 1] == 0xBD) {
            squig_indices.push_back(i);
        }
    }
    
    std::reverse(squig_indices.begin(), squig_indices.end());
    for (size_t i : squig_indices) {
        bytes[i] = 0xFD;
        bytes.erase(bytes.begin() + i + 1);
    }
    
    bytes.push_back(0);
    return bytes;
}

#endif // STRING_TO_VECU8_H