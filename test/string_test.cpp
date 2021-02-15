#include <iostream>
#include <string>

using namespace std;

void str2ascii(const string str)
{
    string ret = "";
    for (auto c : str)
        for (int i = 7; i >= 0; --i)
            ret += ((c >> i) & 1) ? '1' : '0';

    while (ret.size() < 64)
        ret += '0';

    cout << str << endl
         << ret << endl;
}

int main()
{

    string a = "a";
    string b = "ba";
    string c = "hc";

    cout << (b < a) << endl
         << (a < c) << endl
         << (b < c) << endl;

    str2ascii("a");
    str2ascii("ac");
    str2ascii("b");
    // 6989586621679009792
    // 7017452644373364736
    // 7061644215716937728
    cout << (int)'a' << endl;

    cout << string(8, '0') << endl;

    return 0;
}