#include <iostream>
#include <vector>
#include <string.h>

using namespace std;

int main()
{
    int x[] = {0, 1, 2};

    char *p = (char *)malloc(sizeof(x));
    memcpy(p, x, sizeof(x));

    vector<int> v((int *)p, (int *)p + 3);

    for (auto ele : v)
        cout << ele << ' ';
    cout << endl;
    cout << v.size() << endl;

    vector<int *> vec;
    vec.push_back(new int(1));
    vec.push_back(new int(1));

    for(auto ele : vec)
        delete ele;

    return 0;
}