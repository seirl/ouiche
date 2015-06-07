#include "radix-trie.hh"

#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
    if (argc < 2)
        abort();

    std::ofstream f(argv[1]);
    if (!f.is_open())
        abort();

    auto trie = std::make_unique<RadixTrie>();
    trie->load(std::cin);
    trie->serialize_compact(f);
    return 0;
}
