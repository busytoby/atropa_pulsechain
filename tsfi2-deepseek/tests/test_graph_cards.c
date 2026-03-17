#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "tsfi_graph_cards.h"

void print_atom(const char *label, const TsfiTasteAtom *atom) {
    printf("[%s] Taste Atom: ", label);
    for (int i = 0; i < 4; i++) printf("%.3f ", atom->secrets[i]);
    printf("...\n");
}

int main() {
    printf("=== TSFi ICPC Graph Card Isomorphism Test ===\n");

    static uint8_t buf1[1024*1024], buf2[1024*1024], buf3[1024*1024];
    memset(buf1, 0, sizeof(buf1)); memset(buf2, 0, sizeof(buf2)); memset(buf3, 0, sizeof(buf3));

    TsfiGraphCard *card1 = (TsfiGraphCard*)buf1;
    card1->node_count = 4; card1->edge_count = 4;
    card1->edges[0] = (TsfiEdge){1, 2}; card1->edges[1] = (TsfiEdge){2, 3}; 
    card1->edges[2] = (TsfiEdge){3, 1}; card1->edges[3] = (TsfiEdge){1, 4};

    TsfiGraphCard *card2 = (TsfiGraphCard*)buf2;
    card2->node_count = 4; card2->edge_count = 4;
    card2->edges[0] = (TsfiEdge){2, 1}; card2->edges[1] = (TsfiEdge){2, 3}; 
    card2->edges[2] = (TsfiEdge){3, 4}; card2->edges[3] = (TsfiEdge){4, 2};

    tsfi_thunk_solve_graph_isomorphism(card1);
    tsfi_thunk_solve_graph_isomorphism(card2);

    print_atom("Skeleton A", &card1->canonical_atom);
    print_atom("Skeleton B", &card2->canonical_atom);

    for (int i = 0; i < 16; i++) {
        if (card1->canonical_atom.secrets[i] != card2->canonical_atom.secrets[i]) {
            printf("[FAIL] Mismatch at index %d\n", i);
            return 1;
        }
    }
    printf("[PASS] Isomorphic Skeletons correctly identified.\n");

    // --- TEST 2: Non-Isomorphic ---
    TsfiGraphCard *card3 = (TsfiGraphCard*)buf3;
    card3->node_count = 4; card3->edge_count = 4;
    card3->edges[0] = (TsfiEdge){1, 2}; card3->edges[1] = (TsfiEdge){2, 3}; 
    card3->edges[2] = (TsfiEdge){3, 4}; card3->edges[3] = (TsfiEdge){4, 1};

    tsfi_thunk_solve_graph_isomorphism(card3);
    print_atom("Skeleton C", &card3->canonical_atom);

    if (card1->canonical_atom.secrets[0] == card3->canonical_atom.secrets[0]) {
        bool identical = true;
        for(int i=0; i<16; i++) if(card1->canonical_atom.secrets[i] != card3->canonical_atom.secrets[i]) identical = false;
        if (identical) { printf("[FAIL] Non-isomorphic graphs produced same atom!\n"); return 1; }
    }
    printf("[PASS] Non-Isomorphic Skeletons correctly identified.\n");

    printf("=== ALL GRAPH CARD TESTS PASSED ===\n");
    return 0;
}
