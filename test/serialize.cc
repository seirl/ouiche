#include "trie.hh"

#include <iostream>

int main(void) {
    Trie trie;
    trie.load(std::cin);
    trie.serialize(std::cout);
    return 0;
}