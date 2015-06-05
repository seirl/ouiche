#pragma once

#include <algorithm>
#include <string>
#include <vector>

class DamerauLevenshtein
{
public:
    DamerauLevenshtein(const std::string& word, unsigned max_dist)
      : word_(word)
      , max_dist_(max_dist)
      , current_()
      , table_(0)
    {
        table_.reserve(word_.size() + 1);
        for (unsigned j = 0; j < word_.size() + 1; j++)
            table_.push_back(j);
    }

    void rollback(unsigned new_len)
    {
        current_.resize(new_len);
        table_.resize((new_len + 1) * (word_.size() + 1));
    }

    unsigned dist() const
    {
        return table_[table_.size() - 1];
    }

    const std::string& current() const
    {
        return current_;
    }

    // Returns a pair (continue searching, accept this one)
    std::pair<bool, bool> feed(char c)
    {
        bool cont = false;
        current_.push_back(c);
        table_.push_back(current_.size());

        for (unsigned j = 0; j < word_.size(); j++)
        {
            unsigned ts = table_.size();
            unsigned ws = word_.size();

            unsigned i = current_.size();
            unsigned left = table_[ts - 1];
            unsigned up = table_[ts - (ws + 1)];
            unsigned diag = table_[ts - (ws + 1) - 1];

            unsigned dist = std::min({
                    left + 1,
                    up + 1,
                    diag + ((c == word_[j]) ? 0 : 1),
            });

            if (i > 1 && j > 0 &&
                current_[i - 1] == word_[j - 1] &&
                current_[i - 2] == word_[j])
            {
                unsigned diag2 = table_[ts - 2 * (ws + 1) - 2];
                dist = std::min(dist, diag2 + 1);
            }
            table_.push_back(dist);

            if (dist <= max_dist_)
                cont = true;
        }
        return {cont, dist() <= max_dist_};
    }

private:
    const std::string& word_;
    unsigned max_dist_;
    std::string current_;
    std::vector<unsigned> table_;
};
