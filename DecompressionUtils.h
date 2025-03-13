#include <zlib.h>

std::vector<uint8_t> DUunzip(const std::vector<uint8_t>& comp_bin) {
        if (comp_bin.empty()) {
            throw std::invalid_argument("Input compressed data is empty");
        }
    
        // Initialize zlib stream
        z_stream strm = {};
        strm.next_in = const_cast<uint8_t*>(comp_bin.data());
        strm.avail_in = comp_bin.size();
    
        if (inflateInit2(&strm, 15 + 32) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib");
        }
    
        std::vector<uint8_t> decomp_bin;
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
    
            decomp_bin.insert(decomp_bin.end(), buffer.data(), buffer.data() + (buffer.size() - strm.avail_out));
        } while (ret != Z_STREAM_END);
    
        inflateEnd(&strm);
    
        if (ret != Z_STREAM_END) {
            throw std::runtime_error("Incomplete decompression");
        }
    
        return decomp_bin;
}