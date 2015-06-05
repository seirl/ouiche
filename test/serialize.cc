#include "trie.hh"

#include <iostream>

int main(void) {
    auto trie = std::make_unique<Trie>();
    trie->load(std::cin);
    trie->serialize(std::cout);
    return 0;
}
