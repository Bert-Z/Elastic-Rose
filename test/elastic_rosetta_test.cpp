#include "./new/elastic_rosetta.hpp"

using namespace elastic_rose;
using namespace std;

static void test_rose(Elastic_Rosetta &rose, string low, string high)
{
    std::cout << "===============================" << std::endl;
    bool exist = rose.range_query(low, high);
    printf("low: %s high: %s ", low.c_str(), high.c_str());
    printf("%s\n", exist ? "exist" : "not exist");
}

static void test_rose(Elastic_Rosetta &rose, u64 low, u64 high)
{
    std::cout << "===============================" << std::endl;
    bool exist = rose.range_query(low, high);
    printf("low: %ld high: %ld ", low, high);
    printf("%s\n", exist ? "exist" : "not exist");
}

void string_test(Elastic_Rosetta &rose2)
{
    printf("%s %s\n", "a", rose2.lookupKey("a") ? "exist" : "not exist");
    printf("%s %s\n", "aa", rose2.lookupKey("aa") ? "exist" : "not exist");
    printf("%s %s\n", "cat", rose2.lookupKey("cat") ? "exist" : "not exist");
    printf("%s %s\n", "e", rose2.lookupKey("e") ? "exist" : "not exist");
    printf("%s %s\n", "m", rose2.lookupKey("m") ? "exist" : "not exist");

    test_rose(rose2, "a", "b");
    test_rose(rose2, "a", "ad");
    test_rose(rose2, "g", "h");
    test_rose(rose2, "e", "mark");
}

void u64_test(Elastic_Rosetta &rose)
{
    printf("%d %s\n", 2, rose.lookupKey(2) ? "exist" : "not exist");
    printf("%d %s\n", 13, rose.lookupKey(13) ? "exist" : "not exist");
    printf("%d %s\n", 202, rose.lookupKey(202) ? "exist" : "not exist");
    printf("%d %s\n", 203, rose.lookupKey(203) ? "exist" : "not exist");
    printf("%ld %s\n", 2929836923455405461, rose.lookupKey(2929836923455405461) ? "exist" : "not exist");

    // printf("\n");

    // close range query
    test_rose(rose, 20, 30);
    test_rose(rose, 23, 24);
    test_rose(rose, 24, 29);
    test_rose(rose, 24, 28);
    test_rose(rose, 40, 73);
    test_rose(rose, 100, 130);
    test_rose(rose, 140, 201);
    test_rose(rose, 210, 220);
    test_rose(rose, 2929836872643288022, 2929836964876985333);
}

int main(int argc, char **argv)
{
    std::vector<u64> bits_per_keys = {3, 3, 3, 3};
    std::cout << "=========u64=========" << std::endl;

    std::vector<uint64_t> keys = {2, 3, 13, 19, 23, 29, 31, 37, 123, 202, 2929836923455405461};
    // std::vector<uint64_t> keys = {2929836923455405461};
    Elastic_Rosetta rose = Elastic_Rosetta(keys, 10, bits_per_keys);

    std::cout << "=========before=========" << std::endl;
    u64_test(rose);

    // serialize
    // u64 rose_size = rose.serializedSize();
    char *dst = rose.serialize();

    std::cout << "=========after=========" << std::endl;
    Elastic_Rosetta *new_rose = Elastic_Rosetta::deSerialize(dst, 3);
    u64_test(*new_rose);

    std::cout << "seek: " << new_rose->seek(2) << std::endl;
    std::cout << "seek: " << new_rose->seek(10) << std::endl;
    std::cout << "seek: " << new_rose->seek(100) << std::endl;
    std::cout << "seek: " << new_rose->seek(1000) << std::endl;

    delete (new_rose);

    std::cout << "=========string=========" << std::endl;

    std::vector<string> strkeys = {"a", "cat", "dog", "egg", "mark"};
    Elastic_Rosetta rose2 = Elastic_Rosetta(strkeys, 10, 128, bits_per_keys);
    std::cout << "=========before=========" << std::endl;
    string_test(rose2);

    std::cout << "seek: " << rose2.seek("e") << std::endl;

    // serialize
    // u64 rose2_size = rose2.serializedSize();
    char *dst2 = rose2.serialize();

    std::cout << "=========after=========" << std::endl;
    Elastic_Rosetta *new_rose2 = Elastic_Rosetta::deSerialize(dst2, 3);
    string_test(*new_rose2);

    std::cout << "seek: " << rose2.seek("e") << std::endl;
    std::cout << "seek: " << rose2.seek("b") << std::endl;
    delete (new_rose2);

    // std::vector<uint64_t> keys = {6989586621679009792, 7017452644373364736, 7061644215716937728};
    // Elastic_Rosetta rose = Elastic_Rosetta(keys, keys.size());
    // test_rose(rose, 6989586621679009792, 7061644215716937728);

    // std::vector<string> strkeys = {
    //     "a", "ac", "b"};
    // Elastic_Rosetta rose2 = Elastic_Rosetta(strkeys, strkeys.size());
    // test_rose(rose2, "a", "b");

    return 0;
}
