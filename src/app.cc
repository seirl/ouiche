#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "compact-radix-trie.hh"
#include "radix-trie.hh"

void print_matches(std::ostream& out, const RadixTrie::matches_t& matches)
{
    out << "[";
    for (unsigned i = 0; i < matches.size(); i++)
    {
        const auto& r = matches[i];
        out << "{" <<
            "\"word\":\"" << r.word << "\","
            "\"freq\":" << r.freq << ","
            "\"distance\":" << r.distance << "}";
        if (i != matches.size() - 1)
            out << ",";
    }
    out << "]" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " /path/to/compiled/dict.bin";
        std::cout << std::endl;
        std::abort();
    }

    int fd = -1;
    if ((fd = open(argv[1], 0)) == -1)
        abort();

    struct stat s;
    if (fstat(fd, &s) < 0)
        abort();

    void* file = mmap(NULL, s.st_size, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
    char* start = reinterpret_cast<char*>(file);

    while (!std::cin.eof())
    {
        std::string approx;
        std::string word;
        int max_dist = -1;

        std::cin >> approx;
        std::cin >> max_dist;
        std::cin >> word;

        if (!word.size())
            continue;

        if (max_dist >= 0)
        {
            auto res = CompactRadixTrie::matches(word, start, max_dist);
            print_matches(std::cout, res);
        }
        else
            print_matches(std::cout, {});
    }
    return 0;
}
