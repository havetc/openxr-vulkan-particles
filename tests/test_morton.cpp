#include "acutest.h"
#include "../src/Octree.h"

void test_neighbor(void)
{
  TEST_CHECK(is_neighbour(0b0000000000000000, 0b0000000000000001, 0));

  TEST_CHECK(! is_neighbour(0b0000010000000000, 0b0000000000000001, 0));

  TEST_CHECK(is_neighbour(0b0000000100001000, 0b0000000100001101, 0));

  TEST_CHECK(is_neighbour(0b0000000100001000, 0b0000000100100101, 1));

  TEST_CHECK(is_neighbour(0b0000000000111101, 0b0000000100011011, 0));

  TEST_CHECK(!is_neighbour(0b0000000000111101 << 3, 0b0000000100011011 << 3, 0));

  TEST_CHECK(is_neighbour(0b0000000000011101, 0b0000000000101010, 0));

  TEST_CHECK(!is_neighbour(0b0000000000011101, 0b0000000000101000, 0));

  TEST_CHECK(!is_neighbour(0b0000000000011101, 0b0000000000101110, 0));
}

void test_success(void)
{
    /* Do nothing */
}

TEST_LIST = {
    { "test_neighbor", test_neighbor },
    { "success", test_success },
    { NULL, NULL }
};
