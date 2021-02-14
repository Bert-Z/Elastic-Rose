#pragma once

#include <vector>
#include <bitset>
#include <string>

namespace elastic_rose
{
    template <size_t bit_size>
    class Bitvec : public std::bitset<bit_size>
    {
    public:
        Bitvec() : std::bitset<bit_size>(){};
        Bitvec(std::string str) : std::bitset<bit_size>(str){};
        Bitvec(unsigned long value) : std::bitset<bit_size>(value){};
        ~Bitvec() = default;

        bool operator<(Bitvec &bitvec2)
        {
            return this->to_string() < bitvec2.to_string();
        }
        bool operator<=(Bitvec &bitvec2)
        {
            return this->to_string() <= bitvec2.to_string();
        }
        bool operator>(Bitvec &bitvec2)
        {
            return this->to_string() > bitvec2.to_string();
        }
        bool operator>=(Bitvec &bitvec2)
        {
            return this->to_string() >= bitvec2.to_string();
        }

        bool operator<(std::string str)
        {
            return this->to_string() < str;
        }
        bool operator<=(std::string str)
        {
            return this->to_string() <= str;
        }
        bool operator>(std::string str)
        {
            return this->to_string() > str;
        }
        bool operator>=(std::string str)
        {
            return this->to_string() >= str;
        }
    };
} // namespace elastic_rose