#ifndef HATHITRUST_CACHE_PROFILER_H
#define HATHITRUST_CACHE_PROFILER_H

/*
 * Profiles resumption token lookup latencies to ensure sub-microsecond performance.
 * Returns:
 *   Average lookup latency in nanoseconds.
 */
double hathitrust_profile_cache_latency(void);

#endif /* HATHITRUST_CACHE_PROFILER_H */
