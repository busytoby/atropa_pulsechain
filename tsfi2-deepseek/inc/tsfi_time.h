#ifndef TSFI_TIME_H
#define TSFI_TIME_H

unsigned long long get_time_ns(void);

static inline unsigned long long tsfi_rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}
void tsfi_time_wait_ns(long long ns);
void tsfi_time_burn_avx512_ns(long long ns);
void measure_frequency(void);

#endif
