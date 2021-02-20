#include <iostream>
#include <stdlib.h>
#include <vector>
#include "../include/lib.hpp"
#include "../include/hash.hpp"

using namespace std;
using namespace elastic_rose;

int main()
{

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

    return 0;
}