#pragma once

#include <string>
#include <vector>

#include "filter.hpp"
#include "rosetta.hpp"

namespace bench
{
    using namespace elastic_rose;

    class FilterRosetta : public Filter
    {
    public:
        // Requires that keys are sorted
        FilterRosetta(std::vector<std::string> keys, u32 num, u32 levels, u64 bits_per_key)
        {
            filter_ = new Rosetta(keys, num, levels, bits_per_key);
        }

        ~FilterRosetta()
        {
            delete filter_;
        }

        bool lookup(const std::string &key)
        {
            return filter_->lookupKey(key);
        }

        bool lookupRange(const std::string &left_key, const std::string &right_key)
        {
            return filter_->range_query(left_key, right_key);
        }

        // bool approxCount(const std::string& left_key, const std::string& right_key) {
        // return filter_->approxCount(left_key, right_key);
        // }

        // uint64_t getMemoryUsage()
        // {
        //     return filter_->getMemoryUsage();
        // }

    private:
        Rosetta *filter_;
    };

} // namespace bench
