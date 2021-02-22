#include <iostream>
#include <stdlib.h>
#include <vector>
#include "elastic_bf.hpp"

using namespace std;
using namespace elastic_rose;

int main()
{
    std::vector<u64> bits_per_keys = {3, 3, 3, 3};
    std::vector<std::string> keys = {"a", "cat", "dog", "egg", "mark"};
    Elastic_BF ebf(bits_per_keys, keys.size());

    // add keys
    for (auto key : keys)
        ebf.add(key);

    cout << "=========before=========" << endl;
    std::vector<std::string> test_keys = {"a", "cat", "dog", "egg", "mark", "hello", "world", "black", "ca"};

    for (auto key : test_keys)
        cout << ebf.test(key) << ' ';
    cout << endl;

    u64 bf_size = ebf.serializedSize();
    char *dst = (char *)malloc(bf_size);
    ebf.serialize(dst);

    // dst has changed
    dst -= bf_size;

    cout << "=========after=========" << endl;
    Elastic_BF *new_ebf = Elastic_BF::deserialize(dst, 3);
    for (auto key : test_keys)
        cout << new_ebf->test(key) << ' ';
    cout << endl;

    delete new_ebf;

    return 0;
}