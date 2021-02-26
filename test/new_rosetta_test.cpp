#include "../include/new/rosetta.hpp"

using namespace elastic_rose;
using namespace std;

std::string str2BitArray(const std::string &str, uint32_t levels)
{
    std::string ret = "";
    for (auto c : str)
        for (int i = 7; i >= 0; --i)
            ret += (((c >> i) & 1) ? '1' : '0');

    // format str size
    ret += std::string(levels - ret.size(), '0');

    return ret;
}

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
    std::string slow = str2BitArray(low, 128);
    std::string shigh = str2BitArray(high, 128);
    bool exist = rose.range_query(slow, shigh);
    printf("low: %s high: %s ", low.c_str(), high.c_str());
    printf("%s\n", exist ? "exist" : "not exist");
}

void string_test(Rosetta &rose2)
{
    printf("%s %s\n", "a", rose2.lookupKey(str2BitArray("a", 128)) ? "exist" : "not exist");
    printf("%s %s\n", "aa", rose2.lookupKey(str2BitArray("aa", 128)) ? "exist" : "not exist");
    printf("%s %s\n", "cat", rose2.lookupKey(str2BitArray("cat", 128)) ? "exist" : "not exist");
    printf("%s %s\n", "e", rose2.lookupKey(str2BitArray("e", 128)) ? "exist" : "not exist");
    printf("%s %s\n", "m", rose2.lookupKey(str2BitArray("m", 128)) ? "exist" : "not exist");

    test_rose(rose2, "a", "b");
    test_rose(rose2, "a", "ad");
    test_rose(rose2, "g", "h");
    test_rose(rose2, "e", "mark");
}

void u64_test(Rosetta &rose)
{
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
}

void preTreatment(vector<string> &str_vec, u32 levels)
{
    size_t size = str_vec.size();

    for (size_t i = 0; i < size; ++i)
        str_vec[i] = str2BitArray(str_vec[i], levels);
}

int main(int argc, char **argv)
{

    std::cout << "=========u64=========" << std::endl;

    std::vector<uint64_t> keys = {2, 3, 13, 19, 23, 29, 31, 37, 123, 202};
    Rosetta rose = Rosetta(keys, keys.size(), 10);

    std::cout << "=========before=========" << std::endl;
    u64_test(rose);

    // serialize
    // u64 rose_size = rose.serializedSize();
    char *dst = rose.serialize();

    std::cout << "=========after=========" << std::endl;
    Rosetta *new_rose = Rosetta::deSerialize(dst);
    u64_test(*new_rose);

    std::cout << "seek: " << new_rose->seek(2) << std::endl;
    std::cout << "seek: " << new_rose->seek(10) << std::endl;
    std::cout << "seek: " << new_rose->seek(100) << std::endl;
    std::cout << "seek: " << new_rose->seek(1000) << std::endl;

    delete (new_rose);

    std::cout << "=========string=========" << std::endl;

    std::vector<string> strkeys = {"a", "cat", "dog", "egg", "mark"};

    preTreatment(strkeys, 128);
    Rosetta rose2 = Rosetta(strkeys, strkeys.size(), 128, 10);
    std::cout << "=========before=========" << std::endl;
    string_test(rose2);

    std::cout << "seek: " << rose2.seek(str2BitArray("e", 128)) << std::endl;

    // serialize
    // u64 rose2_size = rose2.serializedSize();
    char *dst2 = rose2.serialize();

    std::cout << "=========after=========" << std::endl;
    Rosetta *new_rose2 = Rosetta::deSerialize(dst2);
    string_test(*new_rose2);

    std::cout << "seek: " << rose2.seek(str2BitArray("e", 128)) << std::endl;
    std::cout << "seek: " << rose2.seek(str2BitArray("b", 128)) << std::endl;
    delete (new_rose2);

    // std::vector<uint64_t> keys = {6989586621679009792, 7017452644373364736, 7061644215716937728};
    // Rosetta rose = Rosetta(keys, keys.size());
    // test_rose(rose, 6989586621679009792, 7061644215716937728);

    // std::vector<string> strkeys = {
    //     "a", "ac", "b"};
    // Rosetta rose2 = Rosetta(strkeys, strkeys.size());
    // test_rose(rose2, "a", "b");

    return 0;
}
