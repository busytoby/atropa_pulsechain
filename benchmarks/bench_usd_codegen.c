#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

static double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000000.0 + (double)tv.tv_usec;
}

int main(void) {
    printf("=== RUNNING AUNCIENT USD CODEGEN & PARSER BENCHMARK ===\n");
    
    // 1. Measure Schema Generation Compiler run time
    double t_start = get_time_us();
    int status = system("./tests/usd_gen_schema tests/schema.usda tests/auncient_cactus_schema.h > /dev/null 2>&1");
    double t_end = get_time_us();
    
    if (status != 0) {
        // Build first if needed
        system("gcc -Wall -Wextra -Werror -std=c11 -O3 tests/usd_gen_schema.c -o tests/usd_gen_schema");
        t_start = get_time_us();
        status = system("./tests/usd_gen_schema tests/schema.usda tests/auncient_cactus_schema.h > /dev/null 2>&1");
        t_end = get_time_us();
    }
    
    if (status != 0) {
        printf("Error: usd_gen_schema execution failed.\n");
        return 1;
    }
    
    printf("- Dynamic schema parse & codegen latency: %.2f us\n", t_end - t_start);
    printf("=== BENCHMARK COMPLETED SUCCESSFULLY ===\n");
    return 0;
}
