#include "AssetFolder.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <stdexcept>

enum class Direction {
    Extract,
    Construct
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " (--extract | -e | --construct | -c) <input path> <output path>\n";
        return 1;
    }

    std::string arg1 = argv[1];
    Direction direction;
    if (arg1 == "--extract" || arg1 == "-e") {
        direction = Direction::Extract;
    } else if (arg1 == "--construct" || arg1 == "-c") {
        direction = Direction::Construct;
    } else {
        std::cerr << "Invalid direction: " << arg1 << "\nTry: --extract, -e, --construct, or -c\n";
        return 1;
    }

    std::filesystem::path in_path = argv[2];
    std::filesystem::path out_path = argv[3];

    try {
        if (direction == Direction::Extract) {
            if (!std::filesystem::is_regular_file(in_path)) {
                throw std::runtime_error("Input path is not a file");
            }

            std::ifstream input_file(in_path, std::ios::binary);
            if (!input_file) {
                throw std::runtime_error("Could not open input file");
            }

            std::vector<uint8_t> in_bytes((std::istreambuf_iterator<char>(input_file)), {});
            AssetFolder af = AssetFolder::from_bytes(in_bytes);

            std::filesystem::create_directories(out_path);
            if (!std::filesystem::is_directory(out_path)) {
                throw std::runtime_error("Output path is not a directory");
            }
            
            af.write(out_path);
        } else {
            if (!std::filesystem::is_regular_file(in_path)) {
                throw std::runtime_error("Input path is not a file");
            }
            
            AssetFolder af;
            af.read(in_path);

            std::vector<uint8_t> decomp_buffer = af.to_bytes();
            decomp_buffer.resize((decomp_buffer.size() + 15) & ~15, 0);
            
            std::ofstream out_bin(out_path, std::ios::binary);
            if (!out_bin) {
                throw std::runtime_error("Could not create output binary file");
            }
            out_bin.write(reinterpret_cast<const char*>(decomp_buffer.data()), decomp_buffer.size());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
