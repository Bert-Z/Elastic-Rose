#include <iostream>
#include <stdlib.h>
#include <vector>
#include "lib.hpp"
#include "hash.hpp"
#include "../include/new/bloom_filter.hpp"

using namespace std;
using namespace elastic_rose;

int main()
{
    cout << "===========old===========" << endl;
    std::vector<uint64_t> keys = {2, 3, 13, 19, 37, 123, 202};
    BF bf(10, keys.size());

    // add keys
    for (auto key : keys)
        bf.add(u64hash(key));

    cout << "=========before=========" << endl;
    for (auto key : keys)
        cout << bf.test(u64hash(key)) << ' ';
    cout << endl;

    u64 bf_size = bf.serializedSize();
    char *dst = (char *)malloc(bf_size);
    bf.serialize(dst);

    // dst has changed
    dst -= bf_size;

    cout << "=========after=========" << endl;
    BF *new_bf = bf.deserialize(dst);
    for (auto key : keys)
        cout << new_bf->test(u64hash(key)) << ' ';
    cout << endl;

    delete new_bf;

    cout << endl;

    cout << "===========new===========" << endl;
    std::vector<std::string> str_keys = {"a", "cat", "dog", "egg", "mark"};
    std::vector<std::string> test_keys = {"a", "cat", "dog", "egg", "mark", "hello", "world", "black", "ca"};

    BloomFilter bloomfilter(str_keys, 10);

    cout << "=========before=========" << endl;
    for (auto str_key : test_keys)
        cout << bloomfilter.test(str_key) << ' ';
    cout << endl;

    u64 new_bf_size = bloomfilter.serializedSize();
    char *new_dst = (char *)malloc(new_bf_size);
    bloomfilter.serialize(new_dst);

    // new_dst has changed
    new_dst -= new_bf_size;

    cout << "=========after=========" << endl;
    BloomFilter *new_bloomfilter = BloomFilter::deserialize(new_dst);
    for (auto str_key : test_keys)
        cout << new_bloomfilter->test(str_key) << ' ';
    cout << endl;

    delete new_bloomfilter;

    return 0;
}