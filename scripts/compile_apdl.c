#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    char name[64];
    uint32_t target;
    uint32_t value;
} apdl_task_t;

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: compile_apdl <input.apdl> <output.xpl>\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        fprintf(stderr, "Error: Cannot open input file %s\n", argv[1]);
        return 1;
    }

    char line[256];
    apdl_task_t tasks[2];
    int task_count = 0;
    bool in_concurrent = false;

    while (fgets(line, sizeof(line), in)) {
        // Strip newline
        line[strcspn(line, "\r\n")] = 0;

        if (strstr(line, "CONCURRENT")) {
            in_concurrent = true;
            continue;
        }
        if (strstr(line, "END")) {
            in_concurrent = false;
            continue;
        }

        if (in_concurrent && task_count < 2) {
            // Parse format: TASK_1: WRITE 64410 = 99
            char task_name[64];
            uint32_t target_addr = 0;
            uint32_t val = 0;
            
            int parsed = sscanf(line, "%[^:]: WRITE %u = %u", task_name, &target_addr, &val);
            if (parsed == 3) {
                strcpy(tasks[task_count].name, task_name);
                tasks[task_count].target = target_addr;
                tasks[task_count].value = val;
                task_count++;
            }
        }
    }
    fclose(in);

    if (task_count != 2) {
        fprintf(stderr, "Error: APDL parallel compiler requires exactly 2 task declarations.\n");
        return 1;
    }

    // Horning Disjointness Audit: Check write target address spaces
    if (tasks[0].target == tasks[1].target) {
        fprintf(stderr, "Horning Check Failed: Collision detected on write space: %u\n", tasks[0].target);
        return 1;
    }

    printf("Horning Check Passed: Disjoint write segments verified.\n");

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "Error: Cannot open output file %s\n", argv[2]);
        return 1;
    }

    fprintf(out, "/* Generated Auncient XPL Block from APDL Compiler */\n");
    fprintf(out, "DECLARE MUTEX_REG LITERALLY '64400';\n\n");
    
    fprintf(out, "/* Task 1 (%s) */\n", tasks[0].name);
    fprintf(out, "IF BYTE(MUTEX_REG) == 0 THEN\n");
    fprintf(out, "    BYTE(MUTEX_REG) = 1;\n");
    fprintf(out, "    BYTE(%u) = %u;\n", tasks[0].target, tasks[0].value);
    fprintf(out, "    BYTE(MUTEX_REG) = 0;\n");
    fprintf(out, "END;\n\n");

    fprintf(out, "/* Task 2 (%s) */\n", tasks[1].name);
    fprintf(out, "IF BYTE(MUTEX_REG) == 0 THEN\n");
    fprintf(out, "    BYTE(MUTEX_REG) = 2;\n");
    fprintf(out, "    BYTE(%u) = %u;\n", tasks[1].target, tasks[1].value);
    fprintf(out, "    BYTE(MUTEX_REG) = 0;\n");
    fprintf(out, "END;\n");

    fclose(out);
    printf("Compilation succeeded. Emitted XPL block to: %s\n", argv[2]);
    return 0;
}
