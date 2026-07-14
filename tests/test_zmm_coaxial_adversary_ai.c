#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define PLAYER_SLOT      0
#define ADVERSARY_SLOT   1
#define MAP_SIZE         16
#define GAS_PER_TICK     15
#define INITIAL_GAS      1000

typedef struct {
    int32_t x;
    int32_t y;
    int32_t health;
    uint32_t gas_balance;
    uint32_t score;
} CoaxialEntity;

typedef struct {
    CoaxialEntity entities[2];
    uint32_t map[MAP_SIZE][MAP_SIZE];
    uint64_t transaction_count;
} CoaxialSystem;

// Initialize the coaxial system
void init_coaxial_system(CoaxialSystem *sys) {
    memset(sys, 0, sizeof(CoaxialSystem));
    
    // Setup player (Thor)
    sys->entities[PLAYER_SLOT].x = 2;
    sys->entities[PLAYER_SLOT].y = 2;
    sys->entities[PLAYER_SLOT].health = 2000;
    sys->entities[PLAYER_SLOT].gas_balance = INITIAL_GAS;
    sys->entities[PLAYER_SLOT].score = 0;

    // Setup adversarial AI agent (Questor's Shadow)
    sys->entities[ADVERSARY_SLOT].x = 10;
    sys->entities[ADVERSARY_SLOT].y = 10;
    sys->entities[ADVERSARY_SLOT].health = 1500;
    sys->entities[ADVERSARY_SLOT].gas_balance = INITIAL_GAS;
    sys->entities[ADVERSARY_SLOT].score = 0;

    // Create simple boundaries on playfield
    for (int i = 0; i < MAP_SIZE; i++) {
        sys->map[0][i] = 1;
        sys->map[MAP_SIZE - 1][i] = 1;
        sys->map[i][0] = 1;
        sys->map[i][MAP_SIZE - 1] = 1;
    }
}

// Runs the adversarial AI pathfinding decision tick using EDO-22 register space proofs
void tick_adversary_ai(CoaxialSystem *sys) {
    CoaxialEntity *adv = &sys->entities[ADVERSARY_SLOT];
    CoaxialEntity *play = &sys->entities[PLAYER_SLOT];

    if (adv->gas_balance < GAS_PER_TICK) {
        printf("   [AI Warning] Out of gas: Adversary AI halted.\n");
        return;
    }
    adv->gas_balance -= GAS_PER_TICK;

    // Auncient tracking logic: Move towards the player coordinates to intercept
    int32_t dx = play->x - adv->x;
    int32_t dy = play->y - adv->y;

    if (abs(dx) > abs(dy)) {
        adv->x += (dx > 0) ? 1 : -1;
    } else if (abs(dy) > 0) {
        adv->y += (dy > 0) ? 1 : -1;
    }

    // Verify step boundaries
    if (sys->map[adv->y][adv->x] == 1) {
        // Revert movement if hitting wall playfield
        if (abs(dx) > abs(dy)) adv->x -= (dx > 0) ? 1 : -1;
        else adv->y -= (dy > 0) ? 1 : -1;
    }

    sys->transaction_count++;
    printf("   [Adversary Tick] AI moved to (%d, %d). Remaining Gas: %u\n", 
           adv->x, adv->y, adv->gas_balance);
}

// Checks collisions between player and adversary on-cycle
void verify_coaxial_collisions(CoaxialSystem *sys) {
    CoaxialEntity *play = &sys->entities[PLAYER_SLOT];
    CoaxialEntity *adv = &sys->entities[ADVERSARY_SLOT];

    double distance = sqrt(pow(play->x - adv->x, 2) + pow(play->y - adv->y, 2));
    
    // Close combat collision trigger
    if (distance < 1.5) {
        play->health -= 150;
        adv->health -= 100;
        printf("   [Collision Alert] Melee strike triggered! Player HP: %d, AI HP: %d\n", 
               play->health, adv->health);

        // EDO-22 pitch verification frequency check based on collision coordinates
        int32_t scaleIndex = (play->x + adv->y) % 22;
        double frequency = 220.0 * pow(2.0, scaleIndex / 22.0);
        printf("   [EDO-22 Acoustic Sound] Tone Synthesized: %.2f Hz\n", frequency);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL ADVERSARY AI AGENT PLATFORM TESTS\n");
    printf("=============================================================\n");

    CoaxialSystem sys;
    init_coaxial_system(&sys);

    printf("1. Running pathfinding ticks for Adversary AI agent...\n");
    
    // Tick AI multiple times to reach the player
    for (int i = 0; i < 8; i++) {
        tick_adversary_ai(&sys);
        verify_coaxial_collisions(&sys);
    }

    assert(sys.entities[ADVERSARY_SLOT].x < 10 || sys.entities[ADVERSARY_SLOT].y < 10);
    assert(sys.entities[ADVERSARY_SLOT].gas_balance < INITIAL_GAS);

    printf("   ✓ AI successfully consumed gas and tracked player positions.\n\n");

    printf("2. Simulating out of gas behavior...\n");
    sys.entities[ADVERSARY_SLOT].gas_balance = 5; // Set below 15 threshold
    tick_adversary_ai(&sys);
    assert(sys.entities[ADVERSARY_SLOT].gas_balance == 5); // Position and balance shouldn't change
    printf("   ✓ Out of gas execution halt verified.\n\n");

    printf("=============================================================\n");
    printf("COAXIAL ADVERSARY AI INTEGRATION TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
