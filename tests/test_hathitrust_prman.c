#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_mets.h"
#include "inc/hathitrust_rib.h"
#include "inc/hathitrust_osl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void test_rib_scene_compiler(void) {
    printf("[Test] Running RenderMan RIB Scene Compiler tests...\n");
    
    HtrcMetsStructure mets;
    mets.volume_id = strdup("nyp.33433082257217");
    mets.page_count = 1;
    mets.pages = malloc(1 * sizeof(HtrcMetsPage));
    mets.pages[0].seq = 1;
    mets.pages[0].label = strdup("Page 1");
    mets.pages[0].file_id = strdup("FID1");
    
    char *rib = hathitrust_mets_to_rib(&mets);
    assert(rib != NULL);
    printf("[Test] Generated RIB length: %zu bytes\n", strlen(rib));
    
    assert(strstr(rib, "##RenderMan RIB-Structure 1.1") != NULL);
    assert(strstr(rib, "Display \"book_render.tiff\"") != NULL);
    assert(strstr(rib, "SubdivisionMesh \"catmull-clark\"") != NULL);
    assert(strstr(rib, "page_tex_1") != NULL);
    
    free(rib);
    hathitrust_mets_free(&mets);
    printf("[Test] RIB Scene Compiler tests passed.\n");
}

static void test_osl_generator(void) {
    printf("[Test] Running OSL Shader Generator tests...\n");
    
    char *osl = hathitrust_generate_aging_osl(1930, 2030); // 100 years old
    assert(osl != NULL);
    printf("[Test] Generated OSL:\n%s\n", osl);
    
    assert(strstr(osl, "shader HtrcPaperAging") != NULL);
    assert(strstr(osl, "agingFactor = 1.000000") != NULL);
    assert(strstr(osl, "agedColor = color(0.85, 0.76, 0.58)") != NULL);
    
    free(osl);
    printf("[Test] OSL Shader Generator tests passed.\n");
}

static void test_runprogram_procedural(void) {
    printf("[Test] Running PRMan RunProgram Procedural tests...\n");
    
    // We execute the compiled procedural binary and feed coordinates over a pipe
    int in_pipe[2];
    int out_pipe[2];
    
    assert(pipe(in_pipe) == 0);
    assert(pipe(out_pipe) == 0);
    
    pid_t pid = fork();
    assert(pid >= 0);
    
    if (pid == 0) {
        // Child: exec procedural
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        
        close(in_pipe[1]);
        close(out_pipe[0]);
        
        execl("./tests/hathitrust_prman_procedural", "./tests/hathitrust_prman_procedural", NULL);
        exit(1);
    }
    
    // Parent
    close(in_pipe[0]);
    close(out_pipe[1]);
    
    // Feed mock ALTO coordinates to stdin: HPOS VPOS WIDTH HEIGHT
    const char *mock_input = "100 200 150 40\n";
    write(in_pipe[1], mock_input, strlen(mock_input));
    close(in_pipe[1]);
    
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    read(out_pipe[0], buffer, sizeof(buffer) - 1);
    close(out_pipe[0]);
    
    printf("[Test] Procedural stdout output:\n%s\n", buffer);
    assert(strstr(buffer, "SubdivisionMesh \"catmull-clark\"") != NULL);
    assert(strstr(buffer, "[-0.1000 -0.2400 0]") != NULL || strstr(buffer, "0.1000") != NULL);
    
    int status;
    waitpid(pid, &status, 0);
    printf("[Test] PRMan RunProgram Procedural tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust RenderMan integration tests...\n");
    test_rib_scene_compiler();
    test_osl_generator();
    test_runprogram_procedural();
    printf("[Test] All RenderMan integration tests completed successfully.\n");
    return 0;
}
