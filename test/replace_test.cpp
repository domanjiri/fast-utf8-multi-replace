#include <string>

#include "../replace.hpp"
#include "gtest/gtest.h"

class ReplaceTest : public ::testing::Test {
 protected:
  ReplaceTest() {
  }

  virtual ~ReplaceTest() {
  }

};


TEST_F(ReplaceTest, ASCII)
{
    std::string src{"Simple ASCII text!"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("ascii_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Simple ASCII text!", result);
}


TEST_F(ReplaceTest, SingleSelfChar)
{
    std::string src{"Test with utf-8 آب"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("single_self_char_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Test with utf-8 آب", result);
}


TEST_F(ReplaceTest, SingleDiffChar)
{
    std::string src{"Test with utf-8 آب"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("single_diff_char_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Test with utf-8 تب", result);
}


TEST_F(ReplaceTest, SingleDiffLastChar)
{
    std::string src{"Test with utf-8 آب"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("single_diff_last_char_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Test with utf-8 آش", result);
}


TEST_F(ReplaceTest, MultipleChar)
{
    std::string src{"Test with utf-8 آب"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("multiple_char_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Test with utf-8 تاب", result);
}


TEST_F(ReplaceTest, MultipleCharMultipleRow)
{
    std::string src{"Test with utf-8 آب"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("multiple_char_multiple_row_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Test with utf-8 تابلو", result);
}


TEST_F(ReplaceTest, MultipleCharMultipleRowLong)
{
    std::string src{"Test with utf-8 آب with بآ long ب آ"};
    auto [dict, search_ascii] = utf8mr::CreateDictionary("multiple_char_multiple_row_dict.tsv");

    auto result = utf8mr::Replace(std::move(src), src.size(), dict, search_ascii);
    ASSERT_EQ("Test with utf-8 تابلو with بلوتا long بلو تا", result);
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

