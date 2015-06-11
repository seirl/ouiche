#include "damerau-levenshtein.hh"

int main(void)
{
    DamerauLevenshtein dl("chien", 3);
    dl.feed('n');
    dl.feed('i');
    dl.feed('c');
    dl.feed('h');
    dl.feed('e');
    std::cout << dl << std::endl;
    dl.rollback(4);
    dl.feed('i');
    dl.feed('e');
    dl.feed('n');
    std::cout << dl << std::endl;
    dl.rollback(0);
    dl.feed('c');
    dl.feed('h');
    dl.feed('i');
    dl.feed('e');
    dl.feed('n');
    std::cout << dl << std::endl;
}
