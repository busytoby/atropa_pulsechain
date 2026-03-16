#include <stdbool.h>
#include <stdio.h>

void master_logic_epoch(int *ver) { 
    printf("[PLUGIN_C] Doubling version.\n");
    *ver *= 2; 
}
void master_logic_state(bool *ftw) { *ftw = true; }
void master_logic_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }
void master_logic_scramble(void *ws) { (void)ws; }
void master_logic_provenance(void *ws) { (void)ws; }
