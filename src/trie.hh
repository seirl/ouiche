#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>

class Trie
{
public:
    // word -> distance, freq
    using matches_t =
        std::unordered_map<std::string, std::pair<unsigned, unsigned>>;

    Trie()
      : children_()
      , freq_(0)
    {
    }

    void add_word(unsigned freq, const std::string& word, size_t start = 0)
    {
        assert(start <= word.size());
        if (word.size() == start)
        {
            freq_ = freq;
            return;
        }

        auto it = children_.find(word[start]);
        if (it == children_.end())
            children_[word[start]] = std::make_unique<Trie>();
        children_[word[start]]->add_word(freq, word, start + 1);
    }

    void serialize(std::ostream& out) const
    {
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
        out << "digraph Trie {" << std::endl;
        format_dot_here_(out);
        out << "}" << std::endl;
    }

private:
    void format_dot_here_(std::ostream& out) const
    {
        out << "    " << this << " [label=\"";
        if (freq_)
            out << freq_;
        out << "\"];"  << std::endl;
        for (const auto& p: children_)
        {
            out << "    n" << this << " -> " << p.second.get() <<
                " [label=\"" << p.first << "\"];" << std::endl;
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

    std::unordered_map<char, std::unique_ptr<Trie>> children_;
    unsigned freq_; // 0 if not final
};
