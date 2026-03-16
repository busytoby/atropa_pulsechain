#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tsfi_io.h"
#include "lau_memory.h"

#define MAX_MATCHES 1024

static const char *TAGS[] = {"[IMMEDIATE]", "[CRITICAL]", "[BUG]", "[TODO]"};
static const int TAG_COUNT = 4;

void scan_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) return;

    struct stat st;
    if (fstat(fd, &st) == -1 || st.st_size == 0) {
        close(fd);
        return;
    }

    void *ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        return;
    }

    for (int i = 0; i < TAG_COUNT; i++) {
        size_t starts[MAX_MATCHES];
        size_t ends[MAX_MATCHES];
        size_t count = tsfi_io_grep_context((const uint8_t*)ptr, st.st_size, TAGS[i], strlen(TAGS[i]), starts, ends, MAX_MATCHES, 0, 0);
        
        for (size_t j = 0; j < count; j++) {
            size_t len = ends[j] - starts[j];
            printf("[%s] %s: %.*s\n", TAGS[i], path, (int)len, (char*)ptr + starts[j]);
        }
    }

    munmap(ptr, st.st_size);
    close(fd);
}

void scan_dir(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(path, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_dir(path);
        } else if (S_ISREG(st.st_mode)) {
            if (strstr(path, ".md")) {
                scan_file(path);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("[Discovery] Starting native context analysis...\n");
    
    printf("\n--- 1. Notes & Mandate Review ---\n");
    scan_dir("atropa_pulsechain/firmware/notes");
    scan_dir("."); 

    printf("\n--- 2. Test State Analysis ---\n");
    FILE *log = fopen("test_run.log", "r");
    if (log) {
        char line[4096];
        int fatals = 0;
        while (fgets(line, sizeof(line), log)) {
            if (strstr(line, "[FATAL]")) {
                fatals++;
                printf("[CRITICAL] Previous test run had failure: %s", line);
            }
        }
        if (fatals == 0) printf("[OK] Last test run passed successfully.\n");
        fclose(log);
    } else {
        printf("[WARN] No previous test log found.\n");
    }

    if (access("test_all.sh", F_OK) == 0) {
        printf("[*] checking for orphaned tests...\n");
        DIR *tdir = opendir("tests");
        if (tdir) {
            struct dirent *te;
            int sfd = open("test_all.sh", O_RDONLY);
            if (sfd != -1) {
                struct stat sst;
                fstat(sfd, &sst);
                char *sbuf = mmap(NULL, sst.st_size, PROT_READ, MAP_PRIVATE, sfd, 0);
                if (sbuf != MAP_FAILED) {
                    while ((te = readdir(tdir))) {
                        char tpath[512];
                        snprintf(tpath, sizeof(tpath), "tests/%s", te->d_name);
                        struct stat tst;
                        if (stat(tpath, &tst) == 0 && S_ISREG(tst.st_mode) && (tst.st_mode & S_IXUSR) && strncmp(te->d_name, "test_", 5) == 0) {
                            if (!strstr(sbuf, te->d_name)) {
                                printf("[WARN] Orphaned Test Detected: %s (Not in test_all.sh)\n", tpath);
                            }
                        }
                    }
                    munmap(sbuf, sst.st_size);
                }
                close(sfd);
            }
            closedir(tdir);
        }
    }

    printf("\n[Discovery] Analysis complete.\n");
    return 0;
}
