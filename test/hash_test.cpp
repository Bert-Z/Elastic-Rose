#include "../include/hash.hpp"

#include <iostream>

using namespace std;

int main()
{
    cout << elastic_rose::BloomHash("hello") << endl;
    cout << elastic_rose::BloomHash("world") << endl;
}