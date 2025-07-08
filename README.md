# SynthMemBench

SynthMemBench is a collection of memory-related synthetic benchmarks designed by Lorenzo Carletti (lorenzo.carletti@unimore.it) and Gianluca Brilli (gianluca.brilli@unimore.it) for the paper [Taking a closer look at memory interference effects in commercial-off-the-shelf multicore SoCs](https://doi.org/10.1016/j.sysarc.2025.103487).
More details on the benchmarks are available in the paper itself.

The collection includes implementations (ARM-based, in this repository) for READ\_MISS, WRITE\_MISS and MEMSET. In both their interfered form and their interfering form (suffixed with _infinite).

The interfered form of a benchmark accepts as possible inputs the footprint size of the buffer and the number of iterations of the benchmark to be executed. Optionally, it is also possible to specify which performance counters to keep track of during the execution via perf, if the proper compilation flag is set (maximum of 6 counters, enabled by default).

The interfering form of a benchmark accepts as possible input the footprint size of the buffer on which to infinitely iterate.

## Dependencies

SynthMemBench has two build dependencies: CMake and gcc.

Optionally, to measure performance counters, there is a dependency on [perf C library](https://github.com/AlexGustafsson/perf) and libcap-dev. Pre-compiled static forms of these libraries are included inside the libs folder for ease of use.

## Compilation

To compile the collection, assuming CMake and gcc are installed on the system, this is the command which should be launched:

```
cmake -B build ; cmake --build build
```

## Notes

- It is possible to disable the perf dependency by setting the CMake flag DO\_PERF\_COMPILATION to false.
- Usage of perf to track performance counters requires extra permissions.
- Default performance counters tracked (on the Xilinx ZU9EG): L2D\_CACHE\_REFILL, L2D\_CACHE\_WB, "Attributable Performance Impact Event Counts every cycle there is a stall in the Wr stage because of a load miss", "Data Write operation that stalls the pipeline because the store buffer is full", LD\_RETIRED, ST\_RETIRED.
- To prevent core migration and task descheduling, it is suggested to make use of the chrt and taskset commands.
- An implementation of MEMCPY is also included, though it was not used in the paper.
- The implementations assume that the cache line size is 64 bytes.
