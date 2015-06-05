#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "damerau-levenshtein.hh"

#ifndef NDEBUG
# define DEBUG(fmt, ...) fprintf(stderr, "debug: " fmt "\n", ## __VA_ARGS__)
#else
# define DEBUG(...)
#endif

class Trie
{
public:
    // word, distance, freq
    struct match_t
    {
        std::string word;
        unsigned distance;
        unsigned freq;
    };
    using matches_t = std::vector<match_t>;
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

    static std::unique_ptr<Trie> deserialize_mem(const char* start)
    {
        return deserialize_mem_(&start);
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
        DamerauLevenshtein dl(word, max_distance);
        matches_(res, dl);
        std::sort(res.begin(), res.end(), [](match_t a, match_t b) -> bool {
                if (a.distance < b.distance)
                    return true;
                if (a.distance > b.distance)
                    return false;
                if (b.freq < a.freq)
                    return true;
                if (b.freq > a.freq)
                    return false;
                return (a.word.compare(b.word) < 0);
        });
        return res;
    }

    void format_dot(std::ostream& out) const
    {
        out << "digraph Trie {" << std::endl;
        format_dot_here_(out);
        out << "}" << std::endl;
    }

private:
    static std::unique_ptr<Trie> deserialize_mem_(const char** start)
    {
        auto res = std::make_unique<Trie>();

        res->freq_ = *reinterpret_cast<const unsigned*>(*start);
        *start += sizeof (unsigned);
        size_t nb_children = *reinterpret_cast<const size_t*>(*start);
        *start += sizeof (size_t);
        for (size_t i = 0; i < nb_children; i++)
        {
            size_t lsize = *reinterpret_cast<const size_t*>(*start);
            *start += sizeof (size_t);
            std::string e(*start, lsize);
            *start += lsize;
            res->children_.emplace_back(e, deserialize_mem_(start));
        }
        return res;
    }

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

    void matches_(matches_t& res, DamerauLevenshtein& dl) const
    {
        unsigned baselen = dl.current().size();
        for (const auto& edge : children_)
        {
            dl.rollback(baselen);
            matches_edge_(res, dl, edge);
        }
    }

    void matches_edge_(matches_t& res, DamerauLevenshtein& dl,
                       const edge_t& edge) const
    {
        bool accept = false;
        for (char c: edge.first)
        {
            auto res_feed = dl.feed(c);
            if (!res_feed.first) // should not continue
                return;
            accept = res_feed.second;
        }
        if (accept && edge.second->freq_ != 0)
            res.push_back(match_t{dl.current(), dl.dist(), edge.second->freq_});
        edge.second->matches_(res, dl);
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
