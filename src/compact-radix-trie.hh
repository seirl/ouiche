#pragma once

#include <string>
#include <vector>
#include <unistd.h>

#include "damerau-levenshtein.hh"

class CompactRadixTrie
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

    struct CompactHead
    {
        unsigned freq;
        size_t nb_children;
        size_t offset[1]; // offset from CompactHead address
    } __attribute__((packed));

    struct CompactChild
    {
        size_t label_len;
        char label[1];
    } __attribute__((packed));

    static matches_t matches(const std::string& word, const char* start,
                             unsigned max_distance = 0)
    {
        matches_t res;
        DamerauLevenshtein dl(word, max_distance);
        matches_(res, dl, start);
        std::sort(res.begin(), res.end(), [](match_t a, match_t b) -> bool {
                if (a.distance != b.distance)
                    return (a.distance < b.distance);
                if (b.freq != a.freq)
                    return (b.freq < a.freq);
                return (a.word.compare(b.word) < 0);
                });
        return res;
    }

    static void matches_(matches_t& res, DamerauLevenshtein& dl,
            const char* start)
    {
        const CompactHead* h = reinterpret_cast<const CompactHead*>(start);
        unsigned baselen = dl.current().size();
        for (size_t c = 0; c < h->nb_children; ++c)
        {
            dl.rollback(baselen);
            matches_edge_(res, dl, start + h->offset[c]);
        }
    }

    static void matches_edge_(matches_t& res, DamerauLevenshtein& dl,
                              const char* start)
    {
        const CompactChild* ch = reinterpret_cast<const CompactChild*>(start);
        const char* caddr = start + sizeof (size_t) + ch->label_len;
        const CompactHead* chead = reinterpret_cast<const CompactHead*>(caddr);
        bool accept = false;
        for (size_t i = 0; i < ch->label_len; i++)
        {
            char c = ch->label[i];
            auto res_feed = dl.feed(c);
            if (!res_feed.first)
                return;
            accept = res_feed.second;
        }
        if (accept && chead->freq != 0)
            res.push_back(match_t{dl.current(), dl.dist(), chead->freq});
        matches_(res, dl, caddr);
    }
};
