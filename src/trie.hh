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
        size_t node = node_start_(word[start]);
        if (node == children_.size()) // no edge with same starting char
            children_.push_back({word.substr(start),
                                 std::make_unique<Trie>(freq)});
        else
        {
            auto& edge = children_[node];
            size_t pos = std::distance(edge.first.begin(),
                    std::mismatch(word.begin() + start, word.end(),
                                  edge.first.begin()).second);
            if (pos == edge.first.size()) // edge is the prefix
                edge.second->add_word(freq, word, start + edge.first.size());
            else if (pos == word.size() - start)
            {
                // word is a prefix of the label, we need to put a node in the
                // middle
                auto node_split = std::make_unique<Trie>(freq);
                node_split->children_.push_back({
                        edge.first.substr(pos),
                        std::move(edge.second)
                });
                edge = {edge.first.substr(0, pos), std::move(node_split)};
            }
            else
            {
                // common prefix, we need to split and fork the edge
                auto node_split = std::make_unique<Trie>();
                node_split->children_.push_back({
                        edge.first.substr(pos),
                        std::move(edge.second)
                });
                node_split->children_.push_back({
                        word.substr(start + pos),
                        std::make_unique<Trie>(freq)
                });
                edge = {edge.first.substr(0, pos), std::move(node_split)};
            }
        }
    }

    void serialize(std::ostream& out) const
    {
        size_t nb_children = children_.size();
        out.write(reinterpret_cast<const char*>(&freq_), sizeof (unsigned));
        out.write(reinterpret_cast<const char*>(&nb_children), sizeof (size_t));
        for (const auto& p : children_)
        {
            size_t lsize = p.first.size();
            out.write(reinterpret_cast<const char*>(&lsize), sizeof (size_t));
            out.write(p.first.c_str(), p.first.size());
            p.second->serialize(out);
        }
    }

    static std::unique_ptr<Trie> deserialize(std::istream& in)
    {
        auto res = std::make_unique<Trie>();

        size_t nb_children;
        in.read(reinterpret_cast<char*>(&res->freq_), sizeof (unsigned));
        in.read(reinterpret_cast<char*>(&nb_children), sizeof (size_t));
        for (size_t i = 0; i < nb_children; i++)
        {
            size_t lsize;
            in.read(reinterpret_cast<char*>(&lsize), sizeof (size_t));
            std::vector<char> buf(lsize);
            in.read(&buf[0], lsize);
            res->children_[i] = {std::string(&buf[0]), deserialize(in)};
        }
        return res;
    }

    unsigned lookup(const std::string& word, size_t start = 0) const
    {
        assert(start <= word.size());
        if (start == word.size())
            return freq_;

        size_t node = node_start_(word[start]);
        if (node == children_.size())
            return 0;
        const auto& edge = children_[node];
        if (edge.first.find(word, start) != 0)
            return 0;
        return edge.second->lookup(word, start + 1);
    }

    matches_t matches(const std::string& word, unsigned max_distance = 0) const
    {
        matches_t res;
        matches_in_(res, word, 0, max_distance, max_distance);
        return res;
    }


    void format_dot(std::ostream& out) const
    {
        out << "digraph Trie {" << std::endl;
        format_dot_here_(out);
        out << "}" << std::endl;
    }

private:
    void format_dot_here_(std::ostream& out) const
    {
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

    size_t node_start_(char c) const
    {
        size_t node;
        for (node = 0; node < children_.size(); node++)
            if (children_[node].first[0] == c)
                break;
        return node;
    }

    std::vector<std::pair<std::string, std::unique_ptr<Trie>>> children_;
    unsigned freq_; // 0 if not final
};
