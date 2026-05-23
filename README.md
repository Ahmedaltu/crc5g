# crc5g

CRC-24 implementation in C for 5G NR, with GTest unit tests and performance benchmarks.

## Background

CRC-24 is used in 5G NR polar codes for error detection (3GPP TS 38.212 Section 5.1).  
Generator polynomial: `g(x) = x^24 + x^23 + x^6 + x^5 + x + 1`

Two implementations are provided:
- **Naive** — bit-by-bit loop, 8 iterations per byte
- **Table-driven** — 256-entry LUT, 1 lookup per byte (~2.4x faster)

This mirrors the V0 → optimised progression from CS-E4580 Programming Parallel Computers.

## Project structure

```
src/
  crc.h / crc.c          # Naive bit-by-bit CRC-24
  crc_table.h / crc_table.c  # Table-driven CRC-24 (LUT optimised)
tests/
  test_crc.cpp           # 8 GTest correctness tests
  bench_crc.cpp          # Throughput benchmark (MB/s)
CMakeLists.txt
```

## Build & Test

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
ctest --output-on-failure   # run correctness tests
./bench_crc                 # run benchmark
```

## GTest results

```
8/8 tests passed

CRC24Test.EmptyInput               PASSED
CRC24Test.SingleZeroByte           PASSED
CRC24Test.SingleFFByte             PASSED
CRC24Test.ResultFitsIn24Bits       PASSED
CRC24Test.DifferentInputsDifferentCRC  PASSED
CRC24Test.SingleBitErrorDetected   PASSED
CRC24Test.OrderMatters             PASSED
CRC24Test.ConsistencyAcrossRuns    PASSED
```

## Benchmark results

| Input size | Naive (bit loop) | Table-driven (LUT) | Speedup |
|------------|------------------|--------------------|---------|
| 64 B       | 122 MB/s         | 320 MB/s           | 2.6x    |
| 1 KB       | 122 MB/s         | 294 MB/s           | 2.4x    |
| 8 KB       | 122 MB/s         | 291 MB/s           | 2.4x    |
| 64 KB      | 122 MB/s         | 294 MB/s           | 2.4x    |
| 1 MB       | 121 MB/s         | 294 MB/s           | 2.4x    |

The theoretical speedup is ~8x (LUT replaces 8 bit iterations per byte).  
Achieved ~2.4x — the gap is due to the LUT introducing a data-dependent memory load  
per byte (cache miss pressure), while the naive loop is pure arithmetic with no memory traffic.  
This is a classic **compute vs memory bandwidth tradeoff** — the same bottleneck analysed  
in CS-E4580 when comparing V1 (memory-bound) vs V2 (ILP-bound) implementations.
