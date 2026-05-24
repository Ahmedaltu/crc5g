/**
 * Performance benchmark: naive bit-by-bit CRC-24 vs table-driven CRC-24
 *
 * Measures throughput in MB/s across different input sizes.
 * Mirrors the V0 (naive) → V1 (optimised) progression from CS-E4580.
 *
 * Build: cmake --build build --target bench_crc
 * Run:   ./build/bench_crc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

extern "C" {
#include "../src/crc.h"
#include "../src/crc_table.h"
}

/* Returns wall time in seconds */
static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void bench(const char *label,
                  uint32_t (*fn)(const uint8_t *, size_t),
                  const uint8_t *data, size_t len,
                  int iterations)
{
    /* Warmup */
    volatile uint32_t sink = fn(data, len);
    (void)sink;

    double t0 = now();
    for (int i = 0; i < iterations; i++) {
        sink = fn(data, len);
    }
    double elapsed = now() - t0;

    double total_bytes = (double)len * iterations;
    double mbps = (total_bytes / (1024.0 * 1024.0)) / elapsed;

    printf("  %-20s  %6.1f MB/s  (%.3f s for %d iters x %zu bytes)\n",
           label, mbps, elapsed, iterations, len);
}

int main(void)
{
    /* Test at different input sizes to show cache effects */
    size_t sizes[] = {64, 1024, 8192, 65536, 1 * 1024 * 1024};
    int    iters[]  = {500000, 100000, 20000, 2000, 200};
    const char *size_labels[] = {"64 B", "1 KB", "8 KB", "64 KB", "1 MB"};

    printf("CRC-24 throughput benchmark\n");
    printf("Naive (bit-by-bit) vs Table-driven (256-entry LUT)\n");
    printf("%-10s  %-20s  %-20s  Speedup\n", "Size", "Naive", "Table");
    printf("%s\n", "------------------------------------------------------------");

    for (int s = 0; s < 5; s++) {
        size_t len = sizes[s];
        int it     = iters[s];

        uint8_t *data = (uint8_t *)malloc(len);
        /* Fill with pseudo-random data */
        for (size_t i = 0; i < len; i++)
            data[i] = (uint8_t)(i * 6364136223846793005ULL >> 56);

        /* Verify both produce the same result */
        uint32_t r_naive = crc24(data, len);
        uint32_t r_table = crc24_table(data, len);
        if (r_naive != r_table) {
            fprintf(stderr, "MISMATCH at size %zu: naive=0x%06X table=0x%06X\n",
                    len, r_naive, r_table);
            free(data);
            return 1;
        }

        printf("\n[%s] CRC=0x%06X\n", size_labels[s], r_naive);

        double t0, t1, naive_mbps, table_mbps;
        volatile uint32_t sink;

        /* Naive */
        sink = crc24(data, len); (void)sink;
        t0 = now();
        for (int i = 0; i < it; i++) sink = crc24(data, len);
        t1 = now();
        naive_mbps = ((double)len * it / (1024.0*1024.0)) / (t1 - t0);

        /* Table */
        sink = crc24_table(data, len); (void)sink;
        t0 = now();
        for (int i = 0; i < it; i++) sink = crc24_table(data, len);
        t1 = now();
        table_mbps = ((double)len * it / (1024.0*1024.0)) / (t1 - t0);

        printf("  %-20s  %6.1f MB/s\n", "naive (bit loop)", naive_mbps);
        printf("  %-20s  %6.1f MB/s  (%.1fx speedup)\n",
               "table-driven (LUT)", table_mbps, table_mbps / naive_mbps);

        free(data);
    }

    printf("\nNote: speedup ~8x expected (LUT replaces 8 bit iterations per byte)\n");
    return 0;
}
