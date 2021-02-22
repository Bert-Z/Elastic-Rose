#pragma once

#include <string>
#include <vector>

#include "filter.hpp"
#include "elastic_rosetta.hpp"

namespace bench
{
    using namespace elastic_rose;

    class FilterElasticRosetta : public Filter
    {
    public:
        // Requires that keys are sorted
        FilterElasticRosetta(std::vector<std::string> keys, u32 num, u32 levels, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys)
        {
            filter_ = new Elastic_Rosetta(keys, num, levels, bits_per_key, last_level_bits_per_keys);
        }

        ~FilterElasticRosetta()
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
        Elastic_Rosetta *filter_;
    };

} // namespace bench
