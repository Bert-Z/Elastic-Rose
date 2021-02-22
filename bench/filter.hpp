#pragma once

#include <string>
#include <vector>

namespace bench
{

    class Filter
    {
    public:
        virtual bool lookup(const std::string &key) = 0;
        virtual bool lookupRange(const std::string &left_key, const std::string &right_key) = 0;
        virtual bool approxCount(const std::string &left_key, const std::string &right_key) { return true; };
        virtual uint64_t getMemoryUsage() { return 0; };
    };

} // namespace bench
