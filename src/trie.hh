#pragma once

#include <cstdio>
#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#ifndef NDEBUG
# define DEBUG(fmt, ...) fprintf(stderr, "debug: " fmt "\n", ## __VA_ARGS__)
#else
# define DEBUG(...)
#endif

class Trie
{
public:
    // word -> distance, freq
    using matches_t =
        std::unordered_map<std::string, std::pair<unsigned, unsigned>>;
    using edge_t = std::pair<std::string, std::unique_ptr<Trie>>;

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
        int e = edge_start_(word[start]);
        if (e == -1) // no edge with same starting char
            children_.push_back({word.substr(start),
                                 std::make_unique<Trie>(freq)});
        else
        {
            auto& edge = children_[e];
            size_t pos = std::distance(edge.first.begin(),
                    std::mismatch(word.begin() + start, word.end(),
                                  edge.first.begin()).second);
            if (pos == edge.first.size()) // edge is the prefix
                edge.second->add_word(freq, word, start + edge.first.size());
            else
            {
                auto node_split = std::make_unique<Trie>();
                node_split->children_.push_back({
                        edge.first.substr(pos),
                        std::move(edge.second)
                });
                if (pos == word.size() - start) // word is prefix of edge label
                    node_split->freq_ = freq;
                else // word and edge have a common prefix
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
        res->children_.reserve(nb_children);
        for (size_t i = 0; i < nb_children; i++)
        {
            size_t lsize;
            in.read(reinterpret_cast<char*>(&lsize), sizeof (size_t));
            std::vector<char> buf(lsize);
            in.read(&buf[0], lsize);
            res->children_.emplace_back(std::string(&buf[0], lsize),
                                        deserialize(in));
        }
        return res;
    }

    unsigned lookup(const std::string& word, size_t start = 0) const
    {
        assert(start <= word.size());
        if (start == word.size())
            return freq_;

        int e = edge_start_(word[start]);
        if (e == -1)
            return 0;
        const auto& edge = children_[e];
        if (edge.first.find(word, start) != 0)
            return 0;
        return edge.second->lookup(word, start + 1);
    }

    matches_t matches(const std::string& word, unsigned max_distance = 0) const
    {
        matches_t res;
        std::string wc = word;
        matches_(res, wc, 0, max_distance, max_distance);
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

    bool is_next_(const std::string& prefix, unsigned start, unsigned len,
                  int edge_offset = 0, int e = -1) const
    {
        if (len == 0)
            return true;
        if (e == -1)
            e = edge_start_(prefix[start]);
        if (static_cast<size_t>(e) >= children_.size())
            return false;
        auto& edge = children_[e];
        unsigned i;
        for (i = 1; i + edge_offset < edge.first.size() && i < len; i++)
            if (edge.first[i + edge_offset] != prefix[start + i])
                return false;
        i += 1;
        return edge.second->is_next_(prefix, start + i, len - i);
    }

    void matches_(matches_t& matches, std::string& word,
                  size_t start = 0,
                  int edge_base = -1, int edge_offset = 0,
                  unsigned d_left = 0, unsigned max_d = 0) const
    {
        assert(start <= word.size());
        if (start == word.size() && freq_ != 0)
            matches[word] = {max_d - d_left, freq_};
        if (is_next_(word, start, 1))
            matches_(matches, word, start + 1, d_left, max_d);
    }

    int edge_start_(char c, unsigned edge_offset = 0) const
    {
        for (size_t edge = 0; edge < children_.size(); edge++)
            if (edge_offset < children_[edge].first.size() &&
                children_[edge].first[edge_offset] == c)
                return edge;
        return -1;
    }

    std::vector<edge_t> children_;
    unsigned freq_; // 0 if not final
};
