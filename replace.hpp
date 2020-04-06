#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <future>
#include <utility>

#include <tbb/concurrent_unordered_map.h>


namespace {

typedef tbb::concurrent_unordered_map<std::string, std::string> StringDictionay;

inline uint8_t LeftmostBlockSize(const uint8_t);
std::string Replace(const std::string&&,    
                    uint64_t,
                    const StringDictionay&,
                    bool);
std::pair<StringDictionay, bool> CreateDictionary();
std::ifstream TouchFile(const std::string&);
std::vector<std::future<std::string>> ProcessByWorkers(std::ifstream&&,
                                                       const StringDictionay&,
                                                       bool);

} // namespace

