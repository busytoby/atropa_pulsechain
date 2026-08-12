// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient Codebase-Wide Function Usage & Fan-In Frequency Analyzer
 * Scans all 837 C source files in `tsfi2-deepseek/src` to tally call frequencies
 * and identify the Top 10 most called/used functions across the entire codebase.
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

static void register_function_call(const char *name)
{
	if (!name || strlen(name) < 3) return;

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
				/* Extract C function call identifiers (e.g. func_name(...)) */
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

								/* Exclude C language keywords */
								if (strcmp(func_name, "if") != 0 && strcmp(func_name, "while") != 0 &&
								    strcmp(func_name, "for") != 0 && strcmp(func_name, "switch") != 0 &&
								    strcmp(func_name, "return") != 0 && strcmp(func_name, "sizeof") != 0 &&
								    strcmp(func_name, "cast") != 0 && strcmp(func_name, "typeof") != 0) {
									register_function_call(func_name);
								}
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
	printf("TOP 10 MOST USED FUNCTIONS IN TSFI2-DEEPSEEK CODEBASE         \n");
	printf("=============================================================\n");
	for (int i = 0; i < 10 && i < unique_func_count; i++) {
		printf("  #%-2d %-40s : %d Invocations\n", i + 1, func_table[i].func_name, func_table[i].call_count);
	}
	printf("=============================================================\n");

	return 0;
}
