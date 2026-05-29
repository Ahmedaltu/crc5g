# crc5g

CRC-24 implementation in C for 5G NR, with GTest unit tests and performance benchmarks.  
Follows the V0 → V1 → V2 optimisation progression from CS-E4580 Programming Parallel Computers.

## Background

CRC-24 is used in 5G NR polar codes for error detection (3GPP TS 38.212 Section 5.1).  
Generator polynomial: `g(x) = x^24 + x^23 + x^6 + x^5 + x + 1`

## Project structure

```
src/
  crc.h / crc.c              # V0: naive bit-by-bit (8 iterations per byte)
  crc_table.h / crc_table.c  # V1: table-driven LUT (1 lookup per byte)
  crc_ilp.h / crc_ilp.c      # V2: ILP with 4 independent accumulators
tests/
  test_crc.cpp               # 8 GTest tests (V0)
  test_crc_table.cpp         # 8 GTest tests (V1) + cross-validation vs V0
  test_crc_ilp.cpp           # 8 GTest tests (V2) + cross-validation vs V0
  bench_crc.cpp              # throughput benchmark (MB/s)
CMakeLists.txt
```

## Build & Test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
ctest --output-on-failure   # 24 tests
./bench_crc                 # throughput benchmark
```

## GTest results

```
24/24 tests passed

CRC24Test:      EmptyInput, SingleZeroByte, SingleFFByte, ResultFitsIn24Bits,
                DifferentInputsDifferentCRC, SingleBitErrorDetected, OrderMatters,
                ConsistencyAcrossRuns

CRC24TableTest: EmptyInput, SingleZeroByte, ResultFitsIn24Bits, ConsistencyAcrossRuns,
                SingleBitErrorDetected, MatchesNaive_AllSingleBytes,
                MatchesNaive_MultiByte, MatchesNaive_LongInput

CRC24ILPTest:   ResultFitsIn24Bits, Deterministic, SingleBitErrorDetected,
                ShortInputMatchesNaive, MatchesNaive_AllSingleBytes,
                MatchesNaive_MultiByte, MatchesNaive_1KB,
                MatchesNaive_NonMultipleOf4
```

## Benchmark results

Measured on WSL2/Ubuntu, Intel CPU, `-O3 -march=native`:

| Input | V0 naive | V1 LUT | V2 ILP | V1/V0 | V2/V0 |
|-------|----------|--------|--------|-------|-------|
| 64 B  | 115 MB/s | 410 MB/s | 259 MB/s | 3.6x | 2.3x |
| 1 KB  | 124 MB/s | 361 MB/s | 220 MB/s | 2.9x | 1.8x |
| 8 KB  | 122 MB/s | 375 MB/s | 219 MB/s | 3.1x | 1.8x |
| 64 KB | 123 MB/s | 365 MB/s | 215 MB/s | 3.0x | 1.7x |
| 1 MB  | 124 MB/s | 356 MB/s | 212 MB/s | 2.9x | 1.7x |

## Analysis

**V0 → V1 (LUT):** replacing 8 bit-loop iterations with one table lookup gives ~3x speedup.
Theoretical is ~8x but the LUT introduces a data-dependent memory load per byte — cache
pressure eats the arithmetic savings. V0 is compute-bound; V1 is memory-bound.

**V1 → V2 (ILP):** applying 4 independent accumulators made things *slower* (1.8x vs 3.0x).

This is the key lesson from CS-E4580: **you must identify the correct bottleneck before
optimising.** ILP helps when the bottleneck is a dependency chain — each iteration waiting
for the previous result. But V1's bottleneck is memory access, not the dependency chain:

- V1: 1 LUT lookup per iteration → L1 cache handles it well
- V2: 4 LUT lookups per iteration → 4× more cache pressure → more misses → slower

Applying the wrong fix made things worse. The correct next step is **V3: hardware CRC**
using the x86 `PCLMULQDQ` (carry-less multiply) instruction, which eliminates memory
traffic entirely by computing CRC in registers.

## Roadmap

| Version | Technique | Status |
|---------|-----------|--------|
| V0 | Naive bit-by-bit | ✅ done |
| V1 | LUT table-driven | ✅ done |
| V2 | ILP 4 accumulators | ✅ done (negative result — wrong bottleneck) |
| V3 | CLMUL hardware instruction | 🔜 next |
| V4 | OpenMP parallel chunks | 🔜 planned |
| V5 | AVX-512 VPCLMULQDQ | 🔜 planned |