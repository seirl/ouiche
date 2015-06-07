#include "radix-trie.hh"

#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc < 1)
        abort();

    int fd = -1;
    if ((fd = open(argv[1], 0)) == -1)
        abort();

    struct stat s;
    if (fstat(fd, &s) < 0)
        abort();

    void* file = mmap(NULL, s.st_size, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
    auto trie = RadixTrie::deserialize_mem(reinterpret_cast<char*>(file));
    trie->format_dot(std::cout);
    return 0;
}
