#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../src/auncient_timeline_autodin.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CICS BLOCK-MODE 3270 TERMINAL EMULATION DEMO\n");
    printf("=============================================================\n");

    // Display CICS block-mode terminal grid layout
    printf(" ╔═════════════════════════════════════════════════════════╗ \n");
    printf(" ║                 IBM 3270 CICS ONLINE SESSION            ║ \n");
    printf(" ╠═════════════════════════════════════════════════════════╣ \n");
    printf(" ║  ENTER TRANSACTION PARAMETERS:                          ║ \n");
    printf(" ║                                                         ║ \n");
    printf(" ║  TRANS ID  ===> [       ]  (e.g., 0x1001)               ║ \n");
    printf(" ║  REC KEY   ===> [       ]  (e.g., REC_01)                ║ \n");
    printf(" ║  ACTION    ===> [ ]        (R=Read, W=Write)             ║ \n");
    printf(" ║                                                         ║ \n");
    printf(" ║  PRESS [ENTER] TO TRANSMIT TRANSACTION BLOCK            ║ \n");
    printf(" ╚═════════════════════════════════════════════════════════╝ \n");
    printf("\n");

    // Transaction 1: Read operation
    printf("[CICS] Simulating offline form filling: TransID=0x1001, Key='REC_01', Action='R'\n");
    auncient_autodin_audit_edit("REC_01", 6, 0, 'R');
    auncient_cics_process_transaction(0x1001, "REC_01", 'R');
    printf("\n");

    // Transaction 2: Write operation
    printf("[CICS] Simulating offline form filling: TransID=0x1002, Key='REC_02', Action='W'\n");
    auncient_autodin_audit_edit("REC_02", 6, 0, 'W');
    auncient_cics_process_transaction(0x1002, "REC_02", 'W');

    printf("=============================================================\n");
    printf("CICS TERMINAL DEMO RUN COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
