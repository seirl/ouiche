#pragma once

#include <cstdio>
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#ifndef NDEBUG
# define DEBUG(fmt, args...) fprintf(stderr, "debug: " fmt "\n", ## args)
#else
# define DEBUG(...)
#endif

class Trie
{
public:
    // word -> distance, freq
    using matches_t =
        std::unordered_map<std::string, std::pair<unsigned, unsigned>>;

    Trie(unsigned freq = 0)
      : children_()
      , freq_(freq)
    {
    }

    void load(std::istream& in)
    {
        std::string word;
        unsigned freq;
        while (!in.eof())
        {
            in >> word;
            in >> freq;
            DEBUG("adding word %s with freq %d", word.c_str(), freq);
            add_word(freq, word);
        }
    }

    void add_word(unsigned freq, const std::string& word, size_t start = 0)
    {
        assert(start <= word.size());
        if (word.size() == start)
        {
            freq_ = freq;
            return;
        }
        size_t node;
        for (node = 0; node < children_.size(); node++)
            if (children_[node].first[0] == word[start])
                break;
        if (node == children_.size()) // no edge with same starting char
            children_.push_back({word.substr(start),
                                 std::make_unique<Trie>(freq)});
        else
        {
            auto& edge = children_[node];
            size_t pos = *std::mismatch(word.begin() + start, word.end(),
                                       edge.first.begin()).first;
            if (pos == edge.first.size()) // edge is the prefix
                edge.second->add_word(freq, word, start + edge.first.size());
            else if (pos == word.size() - start)
            {
                // word is a prefix of the label, we need to put a node in the
                // middle
                //TODO(seirl)
            }
            else
            {
                // common prefix, we need to split and fork the edge
                //TODO(seirl)
            }
        }
    }

    void serialize(std::ostream& out) const
    {
        //TODO(seirl): adapt to ptrie
        size_t nb_children = children_.size();
        out.write(reinterpret_cast<const char*>(&freq_), sizeof (unsigned));
        out.write(reinterpret_cast<const char*>(&nb_children), sizeof (size_t));
        for (const auto& p : children_)
        {
            out.write(&p.first, sizeof (char));
            p.second->serialize(out);
        }
    }

    static std::unique_ptr<Trie> deserialize(std::istream& in)
    {
        //TODO(seirl): adapt to ptrie
        auto res = std::make_unique<Trie>();

        size_t nb_children;
        in.read(reinterpret_cast<char*>(&res->freq_), sizeof (unsigned));
        in.read(reinterpret_cast<char*>(&nb_children), sizeof (size_t));
        for (size_t i = 0; i < nb_children; i++)
        {
            char l;
            in.read(&l, sizeof (char));
            res->children_[l] = deserialize(in);
        }
        return res;
    }

    unsigned lookup(const std::string& word, size_t start = 0) const
    {
        //TODO(seirl): adapt to ptrie
        assert(start <= word.size());
        if (start == word.size())
            return freq_;

        const auto& it = children_.find(word[start]);
        if (it == children_.end())
            return 0;
        return it->second->lookup(word, start + 1);
    }

    matches_t matches(const std::string& word, unsigned max_distance = 0) const
    {
        matches_t res;
        matches_in_(res, word, 0, max_distance, max_distance);
        return res;
    }


    void format_dot(std::ostream& out) const
    {
        //TODO(seirl): adapt to ptrie
        out << "digraph Trie {" << std::endl;
        format_dot_here_(out);
        out << "}" << std::endl;
    }

private:
    void format_dot_here_(std::ostream& out) const
    {
        //TODO(seirl): adapt to ptrie
        out << "    n" << this << " [label=\"";
        if (freq_)
            out << freq_;
        out << "\"];"  << std::endl;
        for (const auto& p: children_)
        {
            out << "    n" << this << " -> n" << p.second.get() <<
                " [label=\"" << p.first << "\"];" << std::endl;
            p.second->format_dot_here_(out);
        }
    }

    void matches_in_(matches_t& matches, const std::string& word,
                     size_t start = 0, unsigned d_left = 0,
                     unsigned max_d = 0) const
    {
        assert(start <= word.size());
        if (start == word.size())
            matches[word] = {max_d - d_left, freq_};
        //FIXME(seirl): complete the algorithm
    }

    std::vector<std::pair<std::string, std::unique_ptr<Trie>>> children_;
    unsigned freq_; // 0 if not final
};
