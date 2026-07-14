#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// Simulated GTIA PMG Collision Register Offsets
typedef struct {
    uint8_t P0PF; // Player 0 to Playfield (Thor to Wall/Enemy)
    uint8_t M0PF; // Missile 0 to Playfield (Axe to Wall/Enemy)
    uint8_t P0PL; // Player 0 to Player (Thor to Thyra/Merlin)
} GtiaCollisionRegisters;

typedef struct {
    uint8_t hpos_p0; // Thor horizontal position
    uint8_t hpos_m0; // Throwing Axe horizontal position
    uint8_t wall_x;  // Wall playfield coordinate
    uint8_t enemy_x; // Enemy playfield coordinate
} PMGHardwareState;

// Resolves cycle-level collisions and updates registers (PMK mapping)
void resolve_pmg_collisions(PMGHardwareState *hw, GtiaCollisionRegisters *regs, uint32_t *enemy_health) {
    // Reset collision registers for this cycle evaluation
    regs->P0PF = 0;
    regs->M0PF = 0;
    regs->P0PL = 0;

    // 1. Check Player 0 (Thor) to Playfield (Wall) collision
    if (hw->hpos_p0 == hw->wall_x) {
        regs->P0PF |= (1 << 0); // Set P0PF collision bit
        printf("   [GTIA Collision] P0PF active: Player 0 (Thor) collided with Wall at x=%u\n", hw->hpos_p0);
    }

    // 2. Check Missile 0 (Axe) to Playfield (Enemy) collision
    if (hw->hpos_m0 == hw->enemy_x) {
        regs->M0PF |= (1 << 0); // Set M0PF collision bit
        if (*enemy_health > 0) {
            (*enemy_health)--;
            printf("   [GTIA Collision] M0PF active: Missile 0 (Axe) struck Enemy at x=%u. Enemy Health: %u\n", 
                   hw->hpos_m0, *enemy_health);
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PLAYER-MISSILE GRAPHICS (PMG) COLLISION KEY TESTS\n");
    printf("=============================================================\n");

    PMGHardwareState hw = {
        .hpos_p0 = 120, // Thor position
        .hpos_m0 = 150, // Axe position
        .wall_x = 120,  // Wall positioned at x=120
        .enemy_x = 180  // Enemy positioned at x=180
    };

    GtiaCollisionRegisters regs;
    uint32_t enemy_health = 3;

    // 1. Test Player-to-Playfield (P0PF) wall collision
    printf("1. Simulating Player 0 (Thor) moving into wall...\n");
    resolve_pmg_collisions(&hw, &regs, &enemy_health);
    assert(regs.P0PF & (1 << 0));
    assert(!(regs.M0PF & (1 << 0)));
    printf("   ✓ Player collision register (P0PF) verified.\n\n");

    // 2. Test Missile-to-Playfield (M0PF) enemy collision
    printf("2. Simulating Missile 0 (Axe) traveling and striking enemy at x=180...\n");
    hw.hpos_p0 = 100; // Move Thor away from wall
    hw.hpos_m0 = 180; // Move Axe to enemy position
    
    resolve_pmg_collisions(&hw, &regs, &enemy_health);
    assert(!(regs.P0PF & (1 << 0)));
    assert(regs.M0PF & (1 << 0));
    assert(enemy_health == 2); // Took 1 point of damage
    printf("   ✓ Missile strike register (M0PF) and damage verified.\n\n");

    printf("=============================================================\n");
    printf("PMG COLLISION REGISTER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");

    return 0;
}
