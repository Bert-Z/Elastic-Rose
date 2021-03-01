#include <math.h>
#include <iostream>
#include <vector>

using namespace std;

const uint64_t R = 100000000000;
const int logR = log(R);

double g(int x)
{
    if (x < logR)
        return 1;
    else if (x == logR)
        return (double)(R - (1 << x) + 1) / (double)(1 << x);
    else
        return 0;
}

double levelFrequency(int r)
{
    double ret = 0;
    for (int i = 0; i <= logR - r; ++i)
        ret += g(r + i);
    return ret;
}

int main()
{
    cout << logR << endl;

    int bits_per_key = 10;
    int levels_ = 64;

    std::vector<double> bpk_per_level_vec(levels_);

    double fre_min;
    bool isset = false;
    for (int i = 0; i < levels_; ++i)
    {
        bpk_per_level_vec[i] = levelFrequency(levels_ - i - 1);
        if (bpk_per_level_vec[i] != 0 && !isset)
        {
            fre_min = bpk_per_level_vec[i];
            isset = true;
        }
    }
    double fre_max = bpk_per_level_vec[levels_ - 1];

    for (int i = 0; i < levels_; ++i)
    {
        if (bpk_per_level_vec[i] != 0)
            bpk_per_level_vec[i] = (bits_per_key / 2) * (2 - (fre_max - bpk_per_level_vec[i]) / (fre_max - fre_min));
    }

    for (int i = 0; i < levels_; ++i)
        cout << i << " : " << bpk_per_level_vec[i] << endl;

    return 0;
}