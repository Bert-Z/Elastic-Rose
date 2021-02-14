#include <iostream>
#include <string>

using namespace std;

void str2ascii(const string str)
{
    for (auto c : str)
        for (int i = 7; i >= 0; --i)
            cout << ((c >> i) & 1) << ' ';

    cout << endl;
}

int main()
{

    string a = "h";
    string b = "ba";
    string c = "hc";

    cout << (b < a) << endl
         << (a < c) << endl
         << (b < c) << endl;

    str2ascii("ba");
    str2ascii("h");
    str2ascii("hc");

    cout << (int)'a' << endl;

    return 0;
}