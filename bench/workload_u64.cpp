#include "bench.hpp"
#include "filter_factory.hpp"

int main(int argc, char *argv[])
{
	if (argc != 5)
	{
		std::cout << "Usage:\n";
		std::cout << "1. filter type: Rosetta, ElasticRosetta\n";
		std::cout << "2. percentage of keys inserted: 0 < num <= 100\n";
		std::cout << "3. query type: point, range\n";
		std::cout << "4. distribution: uniform, zipfian, latest\n";
		return -1;
	}

	std::string filter_type = argv[1];
	unsigned percent = atoi(argv[2]);
	std::string query_type = argv[3];
	std::string distribution = argv[4];
	std::string key_type = "randint";

	// check args ====================================================
	if (filter_type.compare(std::string("Rosetta")) != 0 && filter_type.compare(std::string("ElasticRosetta")) != 0)
	{
		std::cout << bench::kRed << "WRONG filter type\n"
				  << bench::kNoColor;
		return -1;
	}

	if (percent > 100)
	{
		std::cout << bench::kRed << "WRONG percentage\n"
				  << bench::kNoColor;
		return -1;
	}

	if (query_type.compare(std::string("point")) != 0 && query_type.compare(std::string("range")) != 0)
	{
		std::cout << bench::kRed << "WRONG query type\n"
				  << bench::kNoColor;
		return -1;
	}

	if (distribution.compare(std::string("uniform")) != 0 && distribution.compare(std::string("zipfian")) != 0 && distribution.compare(std::string("latest")) != 0)
	{
		std::cout << bench::kRed << "WRONG distribution\n"
				  << bench::kNoColor;
		return -1;
	}

	// load keys from files =======================================
	size_t level = 0;
	std::string load_file = "workloads/load_";
	load_file += key_type;
	std::vector<uint64_t> load_keys;
	bench::loadKeysFromFile(load_file, bench::kNumIntRecords, load_keys, level);

	std::string txn_file = "workloads/txn_";
	txn_file += key_type;
	txn_file += "_";
	txn_file += distribution;
	std::vector<uint64_t> txn_keys;
	bench::loadKeysFromFile(txn_file, bench::kNumIntRecords, txn_keys, level);

	std::vector<uint64_t> insert_keys;
	bench::selectIntKeysToInsert(percent, insert_keys, load_keys);

	// create filter ==============================================
	double time1 = bench::getNow();
	bench::Filter *filter = nullptr;
	if (filter_type.compare(std::string("Rosetta")) == 0)
		filter = bench::FilterFactory::createFilter(insert_keys, insert_keys.size(), 10);
	else if (filter_type.compare(std::string("ElasticRosetta")) == 0)
		filter = bench::FilterFactory::createFilter(insert_keys, 10, {5, 5, 5, 5});

	double time2 = bench::getNow();
	std::cout << "Build time = " << (time2 - time1) << std::endl;

	// execute transactions =======================================
	int64_t positives = 0;
	uint64_t count = 0;
	double start_time = bench::getNow();
	if (query_type.compare(std::string("point")) == 0)
	{
		for (int i = 0; i < (int)txn_keys.size(); i++)
			positives += (int)filter->lookup(bench::uint64ToString(txn_keys[i]));
	}
	else if (query_type.compare(std::string("range")) == 0)
	{
		for (int i = 0; i < (int)txn_keys.size(); i++)
		{
			uint64_t left = txn_keys[i];
			uint64_t right = txn_keys[i] + bench::kIntRangeSize;
			if (right > left)
			{
				positives += (int)(filter->lookupRange(bench::uint64ToString(left), bench::uint64ToString(right)));
			}
		}
	}

	double end_time = bench::getNow();

	// compute true positives ======================================
	std::map<uint64_t, bool> ht;
	for (int i = 0; i < (int)insert_keys.size(); i++)
		ht[insert_keys[i]] = true;

	int64_t true_positives = 0;
	std::map<uint64_t, bool>::iterator ht_iter;
	if (query_type.compare(std::string("point")) == 0)
	{
		for (int i = 0; i < (int)txn_keys.size(); i++)
		{
			ht_iter = ht.find(txn_keys[i]);
			true_positives += (ht_iter != ht.end());
		}
	}
	else if (query_type.compare(std::string("range")) == 0)
	{
		for (int i = 0; i < (int)txn_keys.size(); i++)
		{
			uint64_t left = txn_keys[i];
			uint64_t right = txn_keys[i] + bench::kIntRangeSize;
			if (right > left)
			{
				ht_iter = ht.lower_bound(txn_keys[i]);
				if (ht_iter != ht.end())
				{
					uint64_t fetched_key = ht_iter->first;
					true_positives += (int)(fetched_key <= right);
				}
			}
		}
	}

	int64_t false_positives = positives - true_positives;
	// assert(false_positives >= 0);
	int64_t true_negatives = txn_keys.size() - positives;

	// print
	double tput = txn_keys.size() / (end_time - start_time) / 1000000; // Mops/sec
	std::cout << bench::kGreen << "Throughput = " << bench::kNoColor << tput << "\n";

	std::cout << "positives = " << positives << "\n";
	std::cout << "true positives = " << true_positives << "\n";
	std::cout << "false positives = " << false_positives << "\n";
	std::cout << "true negatives = " << true_negatives << "\n";
	std::cout << "count = " << count << "\n";

	double fp_rate = 0;
	if (false_positives > 0)
		fp_rate = false_positives / (true_negatives + false_positives + 0.0);
	std::cout << bench::kGreen << "False Positive Rate = " << bench::kNoColor << fp_rate << "\n";

	if (filter != nullptr)
		std::cout << bench::kGreen << "Memory = " << bench::kNoColor << filter->getMemoryUsage() << "\n\n";

	return 0;
}
