#include <iostream>
#include <stdlib.h>
#include <vector>
#include "./new/elastic_bf.hpp"

using namespace std;
using namespace elastic_rose;

int main()
{
    std::vector<u64> bits_per_keys = {3, 3, 3, 3};

    // u64
    cout << "===========old===========" << endl;
    std::vector<uint64_t> u64keys = {2, 3, 13, 19, 37, 123, 202};
    Elastic_BF bf(u64keys, bits_per_keys);

    cout << "=========before=========" << endl;
    for (auto key : u64keys)
        cout << bf.test(key) << ' ';
    cout << endl;

    u64 bf_size = bf.serializedSize();
    char *dst = (char *)malloc(bf_size);
    bf.serialize(dst);

    // dst has changed
    dst -= bf_size;

    cout << "=========after=========" << endl;
    Elastic_BF *new_bf = bf.deserialize(dst, 3);
    for (auto key : u64keys)
        cout << new_bf->test(key) << ' ';
    cout << endl;

    delete new_bf;

    cout << endl;

    // string
    std::vector<std::string> keys = {"a", "cat", "dog", "egg", "mark"};
    Elastic_BF ebf(keys, bits_per_keys);

    cout << "=========before=========" << endl;
    std::vector<std::string> test_keys = {"a", "cat", "dog", "egg", "mark", "hello", "world", "black", "ca"};

    for (auto key : test_keys)
        cout << ebf.test(key) << ' ';
    cout << endl;

    bf_size = ebf.serializedSize();
    char *edst = (char *)malloc(bf_size);
    ebf.serialize(edst);

    // dst has changed
    edst -= bf_size;

    cout << "=========after=========" << endl;
    Elastic_BF *new_ebf = Elastic_BF::deserialize(edst, 3);
    for (auto key : test_keys)
        cout << new_ebf->test(key) << ' ';
    cout << endl;

    delete new_ebf;

    return 0;
}