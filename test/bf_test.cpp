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
    struct bf *bf = bf_create(10, keys.size());

    // add keys
    for (auto key : keys)
        bf_add(bf, u64hash(key));

    cout << "=========before=========" << endl;
    for (auto key : keys)
        cout << bf_test(bf, u64hash(key)) << ' ';
    cout << endl;

    u64 bf_size = bf_serialized_size(bf);
    char *dst = (char *)malloc(bf_size);
    bf_serialize(bf, dst);

    // dst has changed
    dst -= bf_size;

    cout << "=========after=========" << endl;
    struct bf *new_bf = bf_deserialize(dst);
    for (auto key : keys)
        cout << bf_test(new_bf, u64hash(key)) << ' ';
    cout << endl;

    return 0;
}