# ElasticRosetta

Based on unofficial implementation of Rosetta range filter, published in [SIGMOD2020](https://stratos.seas.harvard.edu/files/stratos/files/rosetta.pdf).

## Build

`rose_test.c` provides a simple demo on how to use the filter.

```
mkdir build
cd build
cmake ..
make -j8
```

## Benchmark

### Step 1: Download YCSB

```
cd bench/workload_gen
bash ycsb_download.sh
```

### Step 2: Generate Workloads

```
cd bench/workload_gen
bash gen_workload.sh
```

You must provide your own email list to generate email-key workloads.

### Step 3: Run Workloads

```
cd bench
./run.sh
```

Note that `run.sh` only includes several representative runs. Refer to `bench/workload.cpp`, `bench/workload_multi_thread.cpp` and `bench/workload_arf.cpp` for more experiment configurations.

## Optimizations

Work in progress...
