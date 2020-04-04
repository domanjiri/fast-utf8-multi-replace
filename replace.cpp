#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <array>
#include <chrono>
#include <deque>
#include <thread>

#include <smmintrin.h>
#include <libcuckoo/cuckoohash_map.hh>


inline uint8_t LeftmostBlockSize(const uint8_t chr) {
    uint8_t size{2};
    std::array<uint8_t, 5> shift{0, 1, 2, 3, 4};

    while (size <= 4) {
        uint8_t leftmost_bit = chr << shift[size] & 0x80;
        if(!leftmost_bit)
            break;
        
        ++size;
    }

    return size;
}


std::string Replace(const std::string&& src,
                    libcuckoo::cuckoohash_map<std::string, std::string>& search_table,
                    uint64_t length,
                    bool search_ascii) {
    std::string dest{};
    dest.resize(2 * length);

    auto c_src = src.c_str();
    uint64_t i{0};
    uint64_t src_wr_cursor{0};
    uint64_t dst_wr_cursor{0};
    uint64_t unwritten_bytes{0};
    while (length - i >= 16) {
        auto chunk = _mm_loadu_si128(static_cast<const __m128i*>(static_cast<const void*>(c_src + i))); 
        if (!_mm_movemask_epi8(chunk) && !search_ascii) {
            i += 16;
            unwritten_bytes += 16;
            continue;
        }
        
        auto chunk_signed = _mm_add_epi8(chunk, _mm_set1_epi8(0x80));
        auto state = _mm_set1_epi8(0x0 | 0x80);
        auto w_2_bytes = _mm_cmplt_epi8( _mm_set1_epi8(0xc2 - 1 - 0x80), chunk_signed);
        state = _mm_blendv_epi8(state , _mm_set1_epi8(0x2 | 0xc2),  w_2_bytes);
        auto w_3_bytes = _mm_cmplt_epi8( _mm_set1_epi8(0xe0 - 1 - 0x80), chunk_signed);
        state = _mm_blendv_epi8(state , _mm_set1_epi8(0x3 | 0xe0),  w_3_bytes);
        auto w_4_bytes = _mm_cmplt_epi8(_mm_set1_epi8(0xf0 - 1 - 0x80), chunk_signed);
        state = _mm_blendv_epi8(state , _mm_set1_epi8(0x4 | 0xf0),  w_4_bytes);
        auto start_points_p =  _mm_and_si128(state, _mm_set1_epi8(0x7));

        uint8_t start_points[16]{0};
        _mm_storeu_si128(reinterpret_cast<__m128i*>(start_points), start_points_p);

		uint8_t j{0};
		while (j < 16) {
            if (uint8_t code_point_len = start_points[j]) {
                std::string seek(c_src + i + j, code_point_len);
                std::string result{};
                
                if (search_table.find(seek, result)) {
                    if (unwritten_bytes) {
                        memcpy(dest.data() + dst_wr_cursor,
                               c_src + src_wr_cursor,
                               unwritten_bytes);
                        
                        src_wr_cursor += unwritten_bytes;
                        dst_wr_cursor += unwritten_bytes;
                    }

                    memcpy(dest.data() + dst_wr_cursor,
                           result.data(),
                           result.size());

                    unwritten_bytes = 0;
                    src_wr_cursor += code_point_len;
                    dst_wr_cursor += result.size();
                    j += code_point_len;
                    continue;
                }
                
                unwritten_bytes += code_point_len;
                j += code_point_len;
                continue;
            }
            ++unwritten_bytes;
            ++j;
		}
        i += j;
    }

    if (unwritten_bytes) {
       memcpy(dest.data() + dst_wr_cursor,
              c_src + src_wr_cursor,
              unwritten_bytes);
       
       src_wr_cursor += unwritten_bytes;
       dst_wr_cursor += unwritten_bytes;
       unwritten_bytes = 0;
    }

    while (i < length) {
        if (src[i] & 0x80) { // ASCII 0x0-------
            memcpy(dest.data() + dst_wr_cursor,
                   c_src + src_wr_cursor,
                   1);
            ++src_wr_cursor;
            ++dst_wr_cursor;
            ++i;
            continue;
        }
        
        uint8_t len = LeftmostBlockSize(src[i]);
        std::string seek(c_src + i, len);
        std::string result{};

        if (search_table.find(seek, result)) {
            memcpy(dest.data() + dst_wr_cursor,
                   result.data(),
                   result.size());

            src_wr_cursor += len;
            dst_wr_cursor += result.size();
            i += len;
            continue;
        } else {
            memcpy(dest.data() + dst_wr_cursor,
                   c_src + src_wr_cursor,
                   len);

            src_wr_cursor += len;
            dst_wr_cursor += len;
            i += len;
            continue;
        }

        ++i;
    }
    
    return dest;
}


int main(int argc, char *argv[]) {
    constexpr std::string_view kDataFileName = "Bijankhan_Corpus.txt";
    constexpr std::string_view kDictFileName = "dict.tsv";

    std::ifstream input_file{static_cast<std::string>(kDataFileName)};
    std::string content((std::istreambuf_iterator<char>(input_file)),
                           (std::istreambuf_iterator<char>()));

    std::ifstream dictionary_file{static_cast<std::string>(kDictFileName)};
    bool search_ascii{false};
    libcuckoo::cuckoohash_map<std::string, std::string> search_table{};
    std::string line{};
    while (std::getline(dictionary_file, line)) {
        std::istringstream single_line(line);
        std::vector<std::string> pair((std::istream_iterator<std::string>(single_line)),
                                            std::istream_iterator<std::string>());
        if (pair.size() == 1) {
            pair.emplace_back("");
        }
        search_table.insert(pair[0], pair[1]);
        if (pair[0].size() == 1)
            search_ascii = true;
    }

    std::deque<std::string> chunk_deque;

    auto start_time = std::chrono::high_resolution_clock::now();
    auto result = Replace(std::move(content),
                          search_table,
                          content.size(),
                          search_ascii);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Elapsed time: " << duration << "ms" << std::endl;

    return 0;
}

