/**
 * Performance benchmark: V0 naive vs V1 LUT vs V2 ILP (4 accumulators)
 *
 * Measures throughput in MB/s across different input sizes.
 * Mirrors the V0 → V1 → V2 progression from CS-E4580.
 *
 * Build: cmake --build build
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
#include "../src/crc_ilp.h"
}

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static double measure(uint32_t (*fn)(const uint8_t *, size_t),
                      const uint8_t *data, size_t len, int iters)
{
    volatile uint32_t sink = fn(data, len); (void)sink;  // warmup
    double t0 = now();
    for (int i = 0; i < iters; i++) sink = fn(data, len);
    double elapsed = now() - t0;
    return ((double)len * iters / (1024.0 * 1024.0)) / elapsed;
}

int main(void)
{
    size_t sizes[] = {64, 1024, 8192, 65536, 1 * 1024 * 1024};
    int    iters[]  = {500000, 100000, 20000, 2000, 200};
    const char *labels[] = {"64 B", "1 KB", "8 KB", "64 KB", "1 MB"};

    printf("CRC-24 throughput benchmark\n");
    printf("V0: naive bit-by-bit  |  V1: LUT table  |  V2: ILP 4 accumulators\n");
    printf("%-8s  %14s  %14s  %14s  %8s  %8s\n",
           "Size", "V0 naive", "V1 LUT", "V2 ILP", "V1/V0", "V2/V0");
    printf("%s\n", "------------------------------------------------------------------------");

    for (int s = 0; s < 5; s++) {
        size_t len = sizes[s];
        int it     = iters[s];

        uint8_t *data = (uint8_t *)malloc(len);
        for (size_t i = 0; i < len; i++)
            data[i] = (uint8_t)(i * 6364136223846793005ULL >> 56);

        /* Verify all produce identical results */
        uint32_t r0 = crc24(data, len);
        uint32_t r1 = crc24_table(data, len);
        uint32_t r2 = crc24_ilp(data, len);

        if (r0 != r1 || r0 != r2) {
            fprintf(stderr, "MISMATCH at %s: V0=0x%06X V1=0x%06X V2=0x%06X\n",
                    labels[s], r0, r1, r2);
            free(data);
            return 1;
        }

        double v0 = measure(crc24,       data, len, it);
        double v1 = measure(crc24_table, data, len, it);
        double v2 = measure(crc24_ilp,   data, len, it);

        printf("%-8s  %11.1f MB/s  %11.1f MB/s  %11.1f MB/s  %7.1fx  %7.1fx\n",
               labels[s], v0, v1, v2, v1/v0, v2/v0);

        free(data);
    }

    printf("\nNote: V2 uses 4 independent accumulators to hide the LUT dependency chain.\n");
    printf("Expected: V2 > V1 at larger inputs where V1 is dependency-chain limited.\n");
    return 0;
}
