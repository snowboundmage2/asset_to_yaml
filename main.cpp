#include "AssetFolder.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <iomanip>

#include "AT_Animation.h"
#include "AT_Binary.h"
#include "AT_DemoButton.h"
#include "AT_Dialogue.h"
#include "AT_GruntyQuestion.h"
#include "AT_LevelSetup.h"
#include "AT_MidiSeq.h"
#include "AT_Model.h"
#include "AT_QuizQuestion.h"
#include "AT_Sprite.h"
#include "AT_Texture.h"

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

            /**
             * @brief Reads the contents of the input file into a vector of bytes.
             * 
             * This line of code initializes a std::vector<uint8_t> named in_bytes by reading
             * all the bytes from the input_file stream. It uses an istreambuf_iterator to 
             * iterate over the characters in the input file and constructs the vector with 
             * those bytes.
             * 
             * @param input_file The input file stream from which bytes are read.
             * @return A vector containing the bytes read from the input file.
             */
            std::vector<uint8_t> in_bytes((std::istreambuf_iterator<char>(input_file)), {});
/* 
            std::ofstream logfile("output_log.txt");
            if (!logfile) {
                std::cerr << "Failed to open output_log.txt for writing." << std::endl;
                return 1;
            }

            int i;
            i=0;

            size_t asset_slot_count = (in_bytes[0] << 24) | (in_bytes[1] << 16) | (in_bytes[2] << 8) | in_bytes[3];
            logfile << "Asset Slot Count: " << static_cast<int>(asset_slot_count) << std::endl;
            logfile << "asset_slot_count: 0x" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(asset_slot_count) << std::endl;
            std::vector<uint8_t> table_bytes(in_bytes.begin() + 8, in_bytes.begin() + 8 + (8 * asset_slot_count));
            logfile << "Table Bytes: " << std::endl;
            for (const auto& byte : table_bytes) {
                logfile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                while (i < 8){
                    i++;
                    if (i == 8){
                        logfile << std::endl;
                        i = 0;
                    }
                    break;
                }
            }
            std::vector<uint8_t> data_bytes(in_bytes.begin() + 8 + (8 * asset_slot_count), in_bytes.end());
            logfile << "Data Bytes: " << std::endl;
            for (const auto& byte : data_bytes) {
                logfile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                while (i < 8){
                    i++;
                    if (i == 8){
                        logfile << std::endl;
                        i = 0;
                    }
                    break;
                }
            }

            logfile << "raw in_bytes: " << std::endl;
            for (const auto& byte : in_bytes) {
                logfile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                while (i < 8){
                    i++;
                    if (i == 8){
                        logfile << std::endl;
                        i = 0;
                    }
                    break;
                }
            }

            // Close the file
            logfile.close(); */

            // Create AssetFolder(af) variable from input_file>vector(aka in_bytes) returning from assetfolder::from_bytes function
            //this is only done once
            AssetFolder af = AssetFolder::from_bytes(in_bytes);

            std::filesystem::create_directories(out_path);
            if (!std::filesystem::is_directory(out_path)) {
                throw std::runtime_error("Output path is not a directory");
            }
            //std::cout << "Created driectory from outpath." << std::endl;

            //this is also done once
            af.write(out_path);
            

        } else {/* 
            disabled compact/create for now
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
            out_bin.write(reinterpret_cast<const char*>(decomp_buffer.data()), decomp_buffer.size()); */
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
