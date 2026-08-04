#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_solr_transcoder.h"
#include "inc/hathitrust_cache_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void test_solr_transcoder(void) {
    printf("[Test] Running Solr to MARC-XML Transcoder tests...\n");
    
    char *xml = hathitrust_solr_to_marcxml(
        "uc1.31970011037748",
        "L'art mérovingien et carolingien",
        "Coutil, Léon",
        "1930"
    );
    assert(xml != NULL);
    printf("[Test] Transcoded MARC-XML length: %zu bytes\n", strlen(xml));
    
    assert(strstr(xml, "<controlfield tag=\"001\">uc1.31970011037748</controlfield>") != NULL);
    assert(strstr(xml, "<subfield code=\"a\">L'art mérovingien et carolingien</subfield>") != NULL);
    assert(strstr(xml, "<subfield code=\"a\">Coutil, Léon</subfield>") != NULL);
    assert(strstr(xml, "<subfield code=\"c\">1930</subfield>") != NULL);
    
    free(xml);
    printf("[Test] Solr to MARC-XML Transcoder tests passed.\n");
}

static void test_cache_profiler(void) {
    printf("[Test] Running Resumption Cache Latency Profiler tests...\n");
    
    double avg_ns = hathitrust_profile_cache_latency();
    printf("[Test] Average Latency: %.2f ns/lookup\n", avg_ns);
    assert(avg_ns >= 0.0);
    
    printf("[Test] Resumption Cache Latency Profiler tests passed.\n");
}

static void test_cli_invocation(void) {
    printf("[Test] Running Unified CLI Execution tests...\n");
    
    int pipe_fd[2];
    assert(pipe(pipe_fd) == 0);
    
    pid_t pid = fork();
    assert(pid >= 0);
    
    if (pid == 0) {
        // Child: execute CLI --profile
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        
        execl("./tests/hathitrust_cli", "./tests/hathitrust_cli", "--profile", NULL);
        exit(1);
    }
    
    // Parent
    close(pipe_fd[1]);
    
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(pipe_fd[0], buffer, sizeof(buffer) - 1);
    close(pipe_fd[0]);
    
    printf("[Test] CLI stdout output:\n%s\n", buffer);
    assert(strstr(buffer, "[CLI] Average Latency:") != NULL);
    
    int status;
    waitpid(pid, &status, 0);
    printf("[Test] Unified CLI Execution tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust CLI, Solr Transcoder, and Profiler tests...\n");
    test_solr_transcoder();
    test_cache_profiler();
    test_cli_invocation();
    printf("[Test] All final integration tests completed successfully.\n");
    return 0;
}
