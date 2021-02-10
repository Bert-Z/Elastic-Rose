#include "../include/rosetta.hpp"

using namespace elastic_rose;

static void test_rose(Rosetta &rose, u64 low, u64 high)
{
    bool exist = rose.range_query(low, high);
    printf("low: %ld hight: %ld ", low, high);
    printf("%s\n", exist ? "exist" : "not exist");
}

int main(int argc, char **argv)
{

    // u64 keys[] = {2019, 10, 2, 2020, 486, 10000, 2135, 987321, 123};
    std::vector<uint64_t> keys = {2, 3, 13, 19, 23, 29, 31, 37, 123, 202};
    // u64 keys[] = {123};
    Rosetta rose = Rosetta(keys, keys.size());

    printf("%d %s\n", 2, rose.lookupKey(2) ? "exist" : "not exist");
    printf("%d %s\n", 13, rose.lookupKey(13) ? "exist" : "not exist");
    printf("%d %s\n", 202, rose.lookupKey(202) ? "exist" : "not exist");
    printf("%d %s\n", 203, rose.lookupKey(203) ? "exist" : "not exist");

    printf("\n");
    // close range query
    test_rose(rose, 20, 30);
    test_rose(rose, 23, 24);
    test_rose(rose, 24, 29);
    test_rose(rose, 24, 28);
    test_rose(rose, 40, 73);
    test_rose(rose, 100, 130);
    test_rose(rose, 140, 201);
    test_rose(rose, 210, 220);

    return 0;
}
