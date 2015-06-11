#include <limits>

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

#undef TEST_WORDS

int max_distance_words(const std::string& a, const std::string& b,
                       unsigned max_distance)
{
    DamerauLevenshtein dl(a, max_distance);
    for (auto c: b)
        dl.feed(c);
    return dl.dist();
}

#define TEST_WORDS(Word1, Word2, MaxDistance, Distance) \
    BOOST_CHECK_EQUAL(max_distance_words(Word1, Word2, MaxDistance), Distance)

BOOST_AUTO_TEST_CASE(TestMaxDistance)
{
    auto infty = std::numeric_limits<unsigned>::max() >> 1;
    TEST_WORDS("azertyuiop", "aeryuop", 3, 3);
    TEST_WORDS("azertyuiop", "aeryuop", 4, 3);
    TEST_WORDS("azertyuiop", "aeryuop", 2, infty);
    TEST_WORDS("aeryuop", "azertyuiop", 0, infty);
    TEST_WORDS("1234567890", "1324576809", 5, 3);
    TEST_WORDS("1234567890", "1324576809", 3, 3);
}

#undef TEST_WORDS

BOOST_AUTO_TEST_CASE(TestDistanceRollback)
{
    DamerauLevenshtein dl("chien", 10000);
    dl.feed('n');
    dl.feed('i');
    dl.feed('c');
    dl.feed('h');
    dl.feed('e');
    BOOST_CHECK_EQUAL(dl.dist(), 4);
    dl.rollback(4);
    dl.feed('i');
    dl.feed('e');
    dl.feed('n');
    BOOST_CHECK_EQUAL(dl.dist(), 2);
    dl.rollback(0);
    dl.feed('c');
    dl.feed('h');
    dl.feed('i');
    dl.feed('e');
    dl.feed('n');
    BOOST_CHECK_EQUAL(dl.dist(), 0);
}
