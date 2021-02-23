#pragma once

#include "filter.hpp"
#include "filter_rosetta.hpp"
#include "filter_elastic_bf.hpp"
#include "filter_bloom.hpp"
#include "filter_surf.hpp"
#include "filter_elastic_rosetta.hpp"

namespace bench
{

	class FilterFactory
	{
	public:
		static Filter *createFilter(const std::string &filter_type,
									const uint32_t suffix_len,
									const std::vector<std::string> &keys)
		{
			if (filter_type.compare(std::string("SuRF")) == 0)
				return new FilterSuRF(keys, surf::kNone, 0, 0);
			else if (filter_type.compare(std::string("SuRFHash")) == 0)
				return new FilterSuRF(keys, surf::kHash, suffix_len, 0);
			else if (filter_type.compare(std::string("SuRFReal")) == 0)
				return new FilterSuRF(keys, surf::kReal, 0, suffix_len);
			else if (filter_type.compare(std::string("SuRFMixed")) == 0)
				return new FilterSuRF(keys, surf::kMixed, suffix_len, suffix_len);
			else if (filter_type.compare(std::string("Bloom")) == 0)
				return new FilterBloom(keys);
			else
				return new FilterSuRF(keys, surf::kReal, 0, suffix_len); // default
		}

		static Filter *createFilter(std::vector<std::string> &keys, uint32_t num, uint32_t levels, uint64_t bits_per_key)
		{
			return new FilterRosetta(keys, num, levels, bits_per_key);
		}

		static Filter *createFilter(std::vector<std::string> &keys, u32 levels, u64 bits_per_key, std::vector<u64> last_level_bits_per_keys)
		{
			return new FilterElasticRosetta(keys, levels, bits_per_key, last_level_bits_per_keys);
		}

		static Filter *createFilter(std::vector<std::string> &keys, std::vector<u64> bits_per_keys)
		{
			return new FilterElasticBF(keys, bits_per_keys);
		}
	};

} // namespace bench
