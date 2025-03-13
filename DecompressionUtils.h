#ifndef DECOMPRESSION_UTILS_H
#define DECOMPRESSION_UTILS_H

#include <vector>
#include <stdexcept>
#include <zlib.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include <stdio.h>

constexpr size_t MAX_MATCH = 258;  // Define MAX_MATCH if needed

// Function to inflate a compressed input buffer
std::vector<uint8_t> inflate_bytes(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        throw std::invalid_argument("Input data is empty");
    }

    z_stream strm = {};
    strm.next_in = const_cast<uint8_t*>(data.data());
    strm.avail_in = data.size();

    if (inflateInit2(&strm, 15 + 32) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib");
    }

    std::vector<uint8_t> decoded;
    std::vector<uint8_t> buffer(4096);

    int ret;
    do {
        strm.next_out = buffer.data();
        strm.avail_out = buffer.size();

        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) {
            inflateEnd(&strm);
            throw std::runtime_error("Decompression failed");
        }

        decoded.insert(decoded.end(), buffer.data(), buffer.data() + (buffer.size() - strm.avail_out));
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("Incomplete decompression");
    }

    return decoded;
}

void check_bk_header(const std::vector<uint8_t>& in_buffer) {
    if (in_buffer.size() < 2 || in_buffer[0] != 0x11 || in_buffer[1] != 0x72) {
        throw std::invalid_argument("in_buffer does not have bk header");
    }
}

size_t get_expected_len(const std::vector<uint8_t>& in_buffer) {
    if (in_buffer.size() < 6) {
        throw std::invalid_argument("Input buffer is too small");
    }

    uint32_t expected_len = (in_buffer[2] << 24) | (in_buffer[3] << 16) | (in_buffer[4] << 8) | in_buffer[5];
    return static_cast<size_t>(expected_len);
}

void check_expected_len(size_t expected_len, const std::vector<uint8_t>& out_buffer) {
    if (expected_len != out_buffer.size()) {
        std::cout << "Expected length: " << expected_len << std::endl;
        std::cout << "Output length: " << out_buffer.size() << std::endl;
        std::cout << "Output size does not match expected size" << std::endl;
    }
}

// Function to unzip a buffer with a specific header check
std::vector<uint8_t> unzip_bk(const std::vector<uint8_t>& in_buffer) {
    std::cout << "Starting unzip_bk function" << std::endl;

    // Check the header
    std::cout << "Checking header" << std::endl;
    check_bk_header(in_buffer);
    std::cout << "Header check passed" << std::endl;

    // Get the expected length
    std::cout << "Getting expected length" << std::endl;
    size_t expected_len = static_cast<size_t>( // Convert to size_t
        (static_cast<uint32_t>(in_buffer[2]) << 24) | 
        (static_cast<uint32_t>(in_buffer[3]) << 16) | 
        (static_cast<uint32_t>(in_buffer[4]) << 8)  | 
        (static_cast<uint32_t>(in_buffer[5]))
    );
    std::cout << "Expected length: " << expected_len << std::endl;

    std::cout << "Starting decompression loop" << std::endl;
    //std::vector<uint8_t> out_buffer = inflate_bytes(std::vector<uint8_t>(in_buffer.begin() + 6, in_buffer.end()));
    std::vector<uint8_t> out_buffer;
    try{
        out_buffer = inflate_bytes(std::vector<uint8_t>(in_buffer.begin() + 6, in_buffer.end()));
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    std::cout << "Decompression loop completed" << std::endl;

    // Check the length
    std::cout << "Checking output length" << std::endl;
    check_expected_len(expected_len, out_buffer);

    std::cout << "unzip_bk function completed successfully" << std::endl;
    return out_buffer;
}

#endif // DECOMPRESSION_UTILS_H
