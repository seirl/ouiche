#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "compact-radix-trie.hh"
#include "damerau-levenshtein.hh"

#ifndef NDEBUG
# define DEBUG(fmt, ...) fprintf(stderr, "debug: " fmt "\n", __VA_ARGS__)
#else
# define DEBUG(...)
#endif

class RadixTrie
{
public:
    using match_t = CompactRadixTrie::match_t;
    using matches_t = CompactRadixTrie::matches_t;
    using edge_t = std::pair<std::string, std::unique_ptr<RadixTrie>>;

    RadixTrie(unsigned freq = 0)
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
                                 std::make_unique<RadixTrie>(freq)});
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
                auto node_split = std::make_unique<RadixTrie>();
                node_split->children_.push_back({
                        edge.first.substr(pos),
                        std::move(edge.second)
                });
                if (pos == word.size() - start) // word is prefix of edge label
                    node_split->freq_ = freq;
                else // word and edge have a common prefix
                    node_split->children_.push_back({
                            word.substr(start + pos),
                            std::make_unique<RadixTrie>(freq)
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

    static std::unique_ptr<RadixTrie> deserialize(std::istream& in)
    {
        auto res = std::make_unique<RadixTrie>();

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

    static std::unique_ptr<RadixTrie> deserialize_mem(const char* start)
    {
        return deserialize_mem_(&start);
    }

    void serialize_compact(std::ostream& out) const
    {
        size_t nb_children = children_.size();
        size_t zero = 0;
        int start = out.tellp();

        // CompactHead
        out.write(reinterpret_cast<const char*>(&freq_), sizeof (unsigned));
        out.write(reinterpret_cast<const char*>(&nb_children), sizeof (size_t));
        int chpos = out.tellp();
        for (size_t i = 0; i < nb_children; ++i)
            out.write(reinterpret_cast<const char*>(&zero), sizeof (size_t));

        for (const auto& p : children_)
        {
            // offset
            size_t curpos = out.tellp();
            size_t offset = curpos - start;
            out.seekp(chpos);
            out.write(reinterpret_cast<const char*>(&offset), sizeof (size_t));
            out.seekp(curpos);
            chpos += sizeof (size_t);

            // label_len
            size_t lsize = p.first.size();
            out.write(reinterpret_cast<const char*>(&lsize), sizeof (size_t));

            // label
            out.write(p.first.c_str(), lsize);

            // child
            p.second->serialize_compact(out);
        }
    }

    static std::unique_ptr<RadixTrie> deserialize_compact(const char* start)
    {
        using CompactHead = CompactRadixTrie::CompactHead;
        using CompactChild = CompactRadixTrie::CompactChild;

        auto res = std::make_unique<RadixTrie>();

        const CompactHead* h = reinterpret_cast<const CompactHead*>(start);
        res->freq_ = h->freq;

        for (size_t c = 0; c < h->nb_children; ++c)
        {
            const char* cha = start + h->offset[c];
            const CompactChild* ch = reinterpret_cast<const CompactChild*>(cha);
            const char* caddr = cha + sizeof (size_t) + ch->label_len;
            res->children_.push_back({std::string(ch->label, ch->label_len),
                                      deserialize_compact(caddr)});
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
        DamerauLevenshtein dl(word, max_distance);
        matches_(res, dl);
        std::sort(res.begin(), res.end(), [](match_t a, match_t b) -> bool {
                if (a.distance != b.distance)
                    return (a.distance < b.distance);
                if (b.freq != a.freq)
                    return (b.freq < a.freq);
                return (a.word.compare(b.word) < 0);
        });
        return res;
    }

    void format_dot(std::ostream& out) const
    {
        out << "digraph RadixTrie {" << std::endl;
        format_dot_here_(out);
        out << "}" << std::endl;
    }

private:
    static std::unique_ptr<RadixTrie> deserialize_mem_(const char** start)
    {
        auto res = std::make_unique<RadixTrie>();

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
