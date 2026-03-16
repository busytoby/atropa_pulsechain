#include <stdbool.h>
#include <stdio.h>

void master_logic_epoch(int *ver) { 
    printf("[PLUGIN_A] Setting version to 100.\n");
    *ver = 100; 
}
void master_logic_state(bool *ftw) { *ftw = true; }
void master_logic_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }
void master_logic_scramble(void *ws) { (void)ws; }
void master_logic_provenance(void *ws) { (void)ws; }

