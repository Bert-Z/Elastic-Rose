#include "bitvec.hpp"
#include <iostream>

using namespace std;
using namespace elastic_rose;

int main()
{

    Bitvec<4> foo("0010");
    for (int i = 0; i < 4; ++i)
        cout << foo[i] << endl;
    cout << foo.to_string() << endl;

    Bitvec<4> bar("0100");
    cout << (foo < bar) << endl;
    cout << (foo < string("1000")) << endl;

    string pow_1 = "1111";
    Bitvec<6> x("100000");
    cout << x.to_string().substr(0, 6 - pow_1.size()) + pow_1 << endl;

    return 0;
}