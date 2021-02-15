#include "../include/rosetta.hpp"

using namespace elastic_rose;
using namespace std;

static void test_rose(Rosetta &rose, u64 low, u64 high)
{
    std::cout << "===============================" << std::endl;
    bool exist = rose.range_query(low, high);
    printf("low: %ld high: %ld ", low, high);
    printf("%s\n", exist ? "exist" : "not exist");
}

static void test_rose(Rosetta &rose, string low, string high)
{
    std::cout << "===============================" << std::endl;
    bool exist = rose.range_query(low, high);
    printf("low: %s high: %s ", low.c_str(), high.c_str());
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

    std::vector<string> strkeys = {"a", "cat", "dog", "egg", "mark"};
    Rosetta rose2 = Rosetta(strkeys, strkeys.size(), 128);
    printf("%s %s\n", "a", rose2.lookupKey("a") ? "exist" : "not exist");
    printf("%s %s\n", "aa", rose2.lookupKey("aa") ? "exist" : "not exist");
    printf("%s %s\n", "cat", rose2.lookupKey("cat") ? "exist" : "not exist");
    printf("%s %s\n", "e", rose2.lookupKey("e") ? "exist" : "not exist");
    printf("%s %s\n", "m", rose2.lookupKey("m") ? "exist" : "not exist");

    test_rose(rose2, "a", "b");
    test_rose(rose2, "a", "ad");
    test_rose(rose2, "g", "h");
    test_rose(rose2, "e", "mark");

    // std::vector<uint64_t> keys = {6989586621679009792, 7017452644373364736, 7061644215716937728};
    // Rosetta rose = Rosetta(keys, keys.size());
    // test_rose(rose, 6989586621679009792, 7061644215716937728);

    // std::vector<string> strkeys = {
    //     "a", "ac", "b"};
    // Rosetta rose2 = Rosetta(strkeys, strkeys.size());
    // test_rose(rose2, "a", "b");

    return 0;
}