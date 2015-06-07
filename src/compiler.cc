#include <cstdlib>
#include <fstream>

#include "radix-trie.hh"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] <<
            " /path/to/word/freq.txt /path/to/output/dict.bin" << std::endl;
        std::abort();
    }

    std::cout << "Size structure: " << 4 << std::endl; //FIXME(seirl): wtf

    std::string line;
    std::ifstream words_f(argv[1]);
    if (!words_f.is_open())
    {
        std::cerr << "File not found: " << argv[1] << std::endl;
        return 1;
    }

    auto trie = std::make_unique<RadixTrie>();
    trie->load(words_f);
    words_f.close();

    std::ofstream dict_f(argv[2]);
    if (!dict_f.is_open())
    {
        std::cerr << "File not found: " << argv[2] << std::endl;
        return 1;
    }

    trie->serialize_compact(dict_f);
    dict_f.close();
    return 0;
}
