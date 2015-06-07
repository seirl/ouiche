#pragma once

#include <unistd.h>

struct CompactRadixTrie
{
    unsigned freq;
    size_t nb_children;
    size_t offset[0];
};

struct Children
{
    size_t key_len;
    char key[0];
};
