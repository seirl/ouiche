#include "trie.hh"

#include <iostream>

int main(void) {
    std::unique_ptr<Trie> trie = Trie::deserialize(std::cin);
    trie->format_dot(std::cout);
    return 0;
}
