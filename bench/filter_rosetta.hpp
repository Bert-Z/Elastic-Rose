#pragma once

#include <string>
#include <vector>

#include "filter.hpp"
#include "../include/new/rosetta.hpp"

namespace bench
{
    class FilterRosetta : public Filter
    {
    public:
        // Requires that keys are sorted
        FilterRosetta(std::vector<std::string> &keys, uint32_t num, uint32_t levels, uint64_t bits_per_key)
        {
            filter_ = new elastic_rose::Rosetta(keys, num, levels, bits_per_key);
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
        elastic_rose::Rosetta *filter_;
    };

} // namespace bench
