#pragma once

#include <string>
#include <vector>

#include "filter.hpp"
#include "../include/new/elastic_rosetta.hpp"

namespace bench
{
    using namespace elastic_rose;

    class FilterElasticRosetta : public Filter
    {
    public:
        // Requires that keys are sorted
        FilterElasticRosetta(std::vector<std::string> keys, u32 levels, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys)
        {
            filter_ = new elastic_rose::Elastic_Rosetta(keys, levels, bits_per_key, last_level_bits_per_keys);
        }

        FilterElasticRosetta(std::vector<uint64_t> &keys, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys)
        {
            filter_ = new elastic_rose::Elastic_Rosetta(keys, bits_per_key, last_level_bits_per_keys);
        }

        ~FilterElasticRosetta()
        {
            delete filter_;
        }

        bool lookup(const std::string &key)
        {
            uint64_t entry = bench::stringToUint64(key);
            return filter_->lookupKey(entry);
        }

        bool lookupRange(const std::string &left_key, const std::string &right_key)
        {
            uint64_t left_key_entry = bench::stringToUint64(left_key);
            uint64_t right_key_entry = bench::stringToUint64(right_key);
            return filter_->range_query(left_key_entry, right_key_entry);
        }

        // bool approxCount(const std::string& left_key, const std::string& right_key) {
        // return filter_->approxCount(left_key, right_key);
        // }

        uint64_t getMemoryUsage()
        {
            return filter_->serializedSize();
        }

    private:
        elastic_rose::Elastic_Rosetta *filter_;
    };

} // namespace bench
