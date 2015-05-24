#include <cstdlib>
#include <fstream>

#include "trie.hh"

int main(int argc, char *argv[])
{
    if (argc < 2)
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

    Trie trie;
    std::string word;
    unsigned freq;
    while (!words_f.eof())
    {
        words_f >> word;
        words_f >> freq;
        trie.add_word(freq, word);
    }
    words_f.close();

    std::ofstream dict_f(argv[2]);
    if (!dict_f.is_open())
    {
        std::cerr << "File not found: " << argv[2] << std::endl;
        return 1;
    }

    trie.serialize(dict_f);
    dict_f.close();
    return 0;
}
