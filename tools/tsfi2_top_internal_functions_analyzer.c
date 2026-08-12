// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient Internal Function Usage & Fan-In Frequency Analyzer (Excluding Standard C Library)
 * Scans all 837 C source files in `tsfi2-deepseek/src` to tally call frequencies for custom `tsfi_`, `lau_`, and `auncient_`
 * internal project functions, excluding all standard C library utilities (`printf`, `strcmp`, `memset`, `memcpy`, etc.).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

#define MAX_UNIQUE_FUNCS 8192
#define MAX_NAME_LEN 128

typedef struct {
	char func_name[MAX_NAME_LEN];
	int call_count;
} func_frequency_t;

static func_frequency_t func_table[MAX_UNIQUE_FUNCS];
static int unique_func_count = 0;

/* Filter to exclude Standard C Library Functions */
static bool is_standard_c_function(const char *name)
{
	static const char *std_funcs[] = {
		"printf", "fprintf", "sprintf", "snprintf", "vprintf", "vsnprintf", "sscanf", "scanf", "fscanf",
		"malloc", "calloc", "realloc", "free",
		"memset", "memcpy", "memmove", "memcmp", "memchr",
		"strcpy", "strncpy", "strcat", "strncat", "strcmp", "strncmp", "strlen", "strdup", "strchr", "strrchr", "strstr", "strtok",
		"fopen", "fclose", "fread", "fwrite", "fseek", "ftell", "rewind", "fflush", "fgets", "fputs", "fgetc", "fputc", "popen", "pclose",
		"sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sinf", "cosf", "tanf", "atan2f", "sqrt", "sqrtf", "pow", "powf", "fabs", "fabsf", "fmax", "fmaxf", "fmin", "fminf", "log", "log2", "exp", "floor", "ceil",
		"rand", "srand", "exit", "abort", "atoi", "atol", "atof", "strtol", "strtoul", "strtod", "qsort", "bsearch",
		"pthread_create", "pthread_join", "pthread_mutex_lock", "pthread_mutex_unlock", "clock_gettime", "sleep", "usleep",
		"open", "close", "read", "write", "opendir", "readdir", "closedir", "assert",
		"if", "while", "for", "switch", "case", "return", "sizeof", "cast", "typeof"
	};
	size_t num_std = sizeof(std_funcs) / sizeof(std_funcs[0]);
	for (size_t i = 0; i < num_std; i++) {
		if (strcmp(name, std_funcs[i]) == 0) return true;
	}
	return false;
}

static void register_internal_function_call(const char *name)
{
	if (!name || strlen(name) < 3 || is_standard_c_function(name)) return;

	for (int i = 0; i < unique_func_count; i++) {
		if (strcmp(func_table[i].func_name, name) == 0) {
			func_table[i].call_count++;
			return;
		}
	}

	if (unique_func_count < MAX_UNIQUE_FUNCS) {
		strncpy(func_table[unique_func_count].func_name, name, MAX_NAME_LEN - 1);
		func_table[unique_func_count].call_count = 1;
		unique_func_count++;
	}
}

static int compare_funcs(const void *a, const void *b)
{
	const func_frequency_t *fa = (const func_frequency_t *)a;
	const func_frequency_t *fb = (const func_frequency_t *)b;
	return fb->call_count - fa->call_count;
}

int main(void)
{
	const char *src_dir = "tsfi2-deepseek/src";
	DIR *dir = opendir(src_dir);
	if (!dir) return 1;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG && strstr(entry->d_name, ".c") && !strstr(entry->d_name, ".c.inc")) {
			char fullpath[512];
			snprintf(fullpath, sizeof(fullpath), "%s/%s", src_dir, entry->d_name);

			FILE *f = fopen(fullpath, "r");
			if (!f) continue;

			char line[512];
			while (fgets(line, sizeof(line), f)) {
				char func_name[MAX_NAME_LEN];
				const char *p = line;

				while (*p) {
					if (( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_' ) && p > line && *(p-1) != '#') {
						int len = 0;
						while ( (p[len] >= 'a' && p[len] <= 'z') || (p[len] >= 'A' && p[len] <= 'Z') || (p[len] >= '0' && p[len] <= '9') || p[len] == '_' ) {
							len++;
						}
						const char *after = p + len;
						while (*after == ' ' || *after == '\t') after++;
						if (*after == '(') {
							if (len < MAX_NAME_LEN) {
								strncpy(func_name, p, len);
								func_name[len] = '\0';
								register_internal_function_call(func_name);
							}
						}
						p += len;
					} else {
						p++;
					}
				}
			}
			fclose(f);
		}
	}
	closedir(dir);

	qsort(func_table, unique_func_count, sizeof(func_frequency_t), compare_funcs);

	printf("=============================================================\n");
	printf("TOP 10 MOST USED INTERNAL FUNCTIONS (EXCLUDING STANDARD C)  \n");
	printf("=============================================================\n");
	for (int i = 0; i < 10 && i < unique_func_count; i++) {
		printf("  #%-2d %-45s : %d Invocations\n", i + 1, func_table[i].func_name, func_table[i].call_count);
	}
	printf("=============================================================\n");

	return 0;
}
