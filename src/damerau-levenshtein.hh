#pragma once

#include <string>
#include <vector>

class Damerau_Levenshtein
{
public:
    Damerau_Levenshtein(const std::string& word, unsigned max_dist)
      : word_(word)
      , max_dist_(max_dist)
      , current_()
      , table_(0)
    {
        table_.reserve(word_.size());
    }

    bool feed(char c)
    {
        //TODO
    }

private:
    const std::string& word_;
    unsigned max_dist_;
    std::string current_;
    std::vector<unsigned> table_;
};
