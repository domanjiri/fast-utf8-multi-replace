#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <future>
#include <utility>

#include <smmintrin.h>
#include <tbb/concurrent_unordered_map.h>


namespace utf8mr {

typedef tbb::concurrent_unordered_map<std::string, std::string> StringDictionary;

// Returns the length of one's block at leftmost of char.
inline uint8_t LeftmostBlockSize(const uint8_t);

// Returns start points of unicode code points in vector of 16 char
inline std::unique_ptr<uint8_t[]> GetStartPoint(__m128i&);

inline std::string ReallocateIfNotEnough(std::string& str,
                                         const uint64_t current_pos,
                                         const uint64_t len_required);

std::string ReplaceCharByChar(std::string& src,
                              const StringDictionary& search_table);

// Returns s string which seek code points replaced in it.
std::string Replace(const std::string&&,    
                    uint64_t,
                    const StringDictionary&,
                    bool);

// Load dictionary file from disc and store its tab separated rows in a hashtable.
// Also, it will be checked that if there exist any ASCII char in seek list.
std::pair<StringDictionary, bool> CreateDictionary(const std::string&);

// Returns file handler. Any validation and preprocessing goes here.
std::ifstream TouchFile(const std::string&);

// Returns number of chars in file
uint64_t GetFileLength(std::ifstream&);

// Returns Vector of asynchronous tasks.
// Read text file in chunks and pass each chunk to one task/worker for processing.
std::vector<std::future<std::string>> ProcessByWorkers(std::ifstream&&,
                                                       const StringDictionary&,
                                                       bool);

} // namespace

