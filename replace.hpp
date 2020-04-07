#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <future>
#include <utility>

#include <tbb/concurrent_unordered_map.h>


namespace utf8mr {

typedef tbb::concurrent_unordered_map<std::string, std::string> StringDictionay;

// Returns the length of one's block at leftmost of char.
inline uint8_t LeftmostBlockSize(const uint8_t);

// Returns s string which seek code points replced in it.
std::string Replace(const std::string&&,    
                    uint64_t,
                    const StringDictionay&,
                    bool);

// Load dictionary file from disc and store its tab separated rows in a hashtable.
// Also, it will be checked that if there exist any ASCII char in seek list.
std::pair<StringDictionay, bool> CreateDictionary(const std::string&);

// Returns file handler. Any validation and preprocessing goes here.
std::ifstream TouchFile(const std::string&);

// Returns number of chars in file
uint64_t GetFileLength(std::ifstream&);

// Returns Vector of asynchronous tasks.
// Read text file in chunks and pass each chunk to one task/worker for processing.
std::vector<std::future<std::string>> ProcessByWorkers(std::ifstream&&,
                                                       const StringDictionay&,
                                                       bool);

} // namespace

