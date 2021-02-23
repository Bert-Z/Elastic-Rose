#pragma once

#include <string>
#include <vector>

#include "filter.hpp"
#include "../include/new/elastic_bf.hpp"

namespace bench
{
    using namespace elastic_rose;

    class FilterElasticBF : public Filter
    {
    public:
        // Requires that keys are sorted
        FilterElasticBF(std::vector<std::string> &keys, std::vector<u64> bits_per_keys)
        {
            filter_ = new elastic_rose::Elastic_BF(keys, bits_per_keys);
        }

        ~FilterElasticBF()
        {
            delete filter_;
        }

        bool lookup(const std::string &key)
        {
            return filter_->test(key);
        }

        bool lookupRange(const std::string &left_key, const std::string &right_key)
        {
            return false;
        }

        // bool approxCount(const std::string& left_key, const std::string& right_key) {
        // return filter_->approxCount(left_key, right_key);
        // }

        // uint64_t getMemoryUsage()
        // {
        //     return filter_->getMemoryUsage();
        // }

    private:
        elastic_rose::Elastic_BF *filter_;
    };

} // namespace bench
