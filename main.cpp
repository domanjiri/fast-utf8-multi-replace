#include <iostream>
#include <chrono>

#include "replace.hpp"


namespace {

constexpr std::string_view kDataFileName = "Bijankhan_Corpus.txt";
constexpr std::string_view kDictFileName = "dict.tsv";

} // namespace

int main(int argc, char *argv[])
{
    auto start_time = std::chrono::high_resolution_clock::now();

    auto [search_table, search_ascii] =
        utf8mr::CreateDictionary(static_cast<std::string>(kDictFileName));
    auto input_file = utf8mr::TouchFile(static_cast<std::string>(kDataFileName));

    auto result = utf8mr::ProcessByWorkers(std::move(input_file),
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

