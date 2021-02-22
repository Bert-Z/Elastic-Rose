#include "../include/elastic_rosetta.hpp"

using namespace elastic_rose;
using namespace std;

static void test_rose(Elastic_Rosetta &rose, string low, string high)
{
    std::cout << "===============================" << std::endl;
    bool exist = rose.range_query(low, high);
    printf("low: %s high: %s ", low.c_str(), high.c_str());
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

int main(int argc, char **argv)
{
    std::cout << "=========string=========" << std::endl;

    std::vector<string> strkeys = {"a", "cat", "dog", "egg", "mark"};
    std::vector<u64> bits_per_keys = {3, 3, 3, 3};
    Elastic_Rosetta rose2 = Elastic_Rosetta(strkeys, strkeys.size(), 128, 10, bits_per_keys);
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
