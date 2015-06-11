#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

class DamerauLevenshtein
{
public:
    const unsigned infty = std::numeric_limits<unsigned>::max() >> 1;

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

        size_t ts = table_.size();
        size_t ws = word_.size();
        size_t i = current_.size();

        table_.resize(ts + ws + 1, infty);
        table_[ts] = i;

        unsigned lb = std::max(0,
                static_cast<int>(i) - static_cast<int>(max_dist_) - 1);
        unsigned rb = std::min(word_.size(), i + max_dist_);

        for (unsigned j = lb; j < rb; j++)
        {
            unsigned left = table_[i * (ws + 1) + j];
            unsigned up = table_[(i - 1) * (ws + 1) + j + 1];
            unsigned diag = table_[(i - 1) * (ws + 1) + j];

            unsigned dist = std::min({
                    left + 1,
                    up + 1,
                    diag + ((c == word_[j]) ? 0 : 1),
            });

            if (i > 1 && j > 0 &&
                current_[i - 1] == word_[j - 1] &&
                current_[i - 2] == word_[j])
            {
                unsigned diag2 = table_[(i - 2) * (ws + 1) + j - 1];
                dist = std::min(dist, diag2 + 1);
            }
            table_[i * (ws + 1) + j + 1] = dist;

            if (dist <= max_dist_)
                cont = true;
        }
        return {cont, dist() <= max_dist_};
    }

    friend std::ostream& operator<< (std::ostream& out,
                                     const DamerauLevenshtein& dl)
    {
        if (dl.word_.empty())
            return out;
        out << "     ";
        for (auto c: dl.word_)
            out << " " << c << " ";
        size_t ws = dl.word_.size();
        out << std::endl << " ";
        for (size_t i = 0; i < dl.table_.size(); i++)
        {
            if (i % (ws + 1) == 0 && i / (ws + 1) > 0)
            {
                out << std::endl;
                out << dl.current_[i / (ws + 1) - 1];
            }
            out << std::setw(3);
            if (dl.table_[i] > (dl.infty >> 1))
                out << "oo";
            else
                out << dl.table_[i];
        }
        out << std::endl;
        return out;
    }

private:
    std::string word_;
    unsigned max_dist_;
    std::string current_;
    std::vector<unsigned> table_;
};
