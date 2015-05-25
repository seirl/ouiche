#include "trie.hh"

#include <iostream>

int main(void) {
    Trie trie;
    trie.load(std::cin);
    trie.format_dot(std::cout);
    return 0;
}
