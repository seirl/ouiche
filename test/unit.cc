#define BOOST_TEST_MODULE distance
#include <boost/test/included/unit_test.hpp>

#include "damerau-levenshtein.hh"

int distance_words(const std::string& a, const std::string& b)
{
    DamerauLevenshtein dl(a, 10000);
    for (auto c: b)
        dl.feed(c);
    return dl.dist();
}

#define TEST_WORDS(Word1, Word2, Distance) \
    BOOST_CHECK_EQUAL(distance_words(Word1, Word2), Distance)

BOOST_AUTO_TEST_CASE(TestDistanceWords)
{
    TEST_WORDS("azertyuiop", "aeryuop", 3);
    TEST_WORDS("aeryuop", "azertyuiop", 3);
    TEST_WORDS("azertyuiopqsdfghjklmwxcvbn,", "qwertyuiopasdfghjkl;zxcvbnm", 6);
    TEST_WORDS("1234567890", "1324576809", 3);
}
