#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <chrono>
#include <future>

#include <smmintrin.h>
#include <tbb/concurrent_unordered_map.h>


namespace {

inline uint8_t LeftmostBlockSize(const uint8_t); 

constexpr std::string_view kDataFileName = "Bijankhan_Corpus.txt";
constexpr std::string_view kDictFileName = "dict.tsv";
typedef tbb::concurrent_unordered_map<std::string, std::string> StringDictionay;


std::string Replace(const std::string&& src,
                    uint64_t length,
                    const StringDictionay& search_table,
                    bool search_ascii)
{
    std::string dest{};
    dest.resize(length * 2);
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
                StringDictionay::const_iterator it = search_table.find(seek);

                if (it != search_table.end()) {
                    const std::string result{it->second};
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
        if (!(src[i] & 0x80)) { // ASCII 0x0-------
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
        StringDictionay::const_iterator it = search_table.find(seek);
        
        if (it != search_table.end()) {
            std::string result{it->second};            
            memcpy(dest.data() + dst_wr_cursor,
                   result.data(),
                   result.size());

            dst_wr_cursor += result.size();
        } else {
            memcpy(dest.data() + dst_wr_cursor,
                   c_src + src_wr_cursor,
                   len);

            dst_wr_cursor += len;
        }

        src_wr_cursor += len;
        i += len;
    }
    
    return dest;
}


uint8_t LeftmostBlockSize(const uint8_t chr)
{
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


StringDictionay CreateDictionary(bool& search_ascii)
{
    std::ifstream dictionary_file{static_cast<std::string>(kDictFileName)};
    search_ascii = false;
    
    StringDictionay search_table{};
    std::string line{};
    while (std::getline(dictionary_file, line)) {
        std::istringstream single_line(line);
        std::vector<std::string> pair((std::istream_iterator<std::string>(single_line)),
                                            std::istream_iterator<std::string>());
        if (pair.size() == 1) {
            pair.emplace_back("");
        }
        search_table.insert(std::make_pair(pair[0], pair[1]));

        if (pair[0].size() == 1)
            search_ascii = true;
    }

    return search_table;
}


std::ifstream TouchFile(const std::string& file_path, uint64_t& len) {
    std::ifstream input_file{file_path};

    input_file.seekg(0, input_file.end);
    len = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    return input_file;
}


std::vector<std::future<std::string>> ProcessByWorkers(std::ifstream&& input_file,
                                                       uint64_t input_file_len,
                                                       const StringDictionay& search_table,
                                                       bool search_ascii) {
    std::vector<std::future<std::string>> workers;
    const uint64_t kChunkSize = input_file_len / std::thread::hardware_concurrency();
    
    while (!input_file.eof() && !input_file.fail()) {
        std::vector<char> chunk(kChunkSize);
        input_file.read(chunk.data(), kChunkSize);
        chunk.resize(input_file.gcount());

        workers.emplace_back(std::async([&](std::vector<char>&& data){
                std::string src(data.begin(), data.end());
                return Replace(std::move(src),
                               data.size(),
                               search_table,
                               search_ascii);
            }, std::move(chunk)));
    }

    return workers;
}

} // namespace


int main(int argc, char *argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now();

    bool search_ascii{false};
    auto search_table = CreateDictionary(search_ascii);
    uint64_t input_file_len{0};
    auto input_file = TouchFile(static_cast<std::string>(kDataFileName), input_file_len);

    auto result = ProcessByWorkers(std::move(input_file),
                                   input_file_len,
                                   search_table,
                                   search_ascii);
    for (auto&& worker : result) {
        worker.wait();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Elapsed time: " << duration << "ms" << std::endl;

    return 0;
}

