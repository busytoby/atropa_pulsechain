#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "tsfi_oregon_trail.h"

int main() {
    printf("=== TSFi2 & Oregon Trail Simulation Test ===\n");

    // 1. Initial State & Store Purchase
    printf("[OREGON] Initializing game state & buying supplies...\n");
    TsfiOregonTrail game;
    tsfi_oregon_trail_init(&game);
    assert(game.miles_traveled == 0);
    assert(game.cash == 400);

    // Buy 2 oxen (40 cash), 200 lbs food (40 cash), 100 bullets (10 cash) -> total cost 90 cash
    tsfi_oregon_trail_buy_supplies(&game, 2, 200, 100);
    printf("[OREGON]   After Store: Cash: $%d, Oxen: %d, Food: %d lbs, Bullets: %d\n",
           game.cash, game.oxen, game.food_lbs, game.bullets);
    assert(game.cash == 310);
    assert(game.oxen == 6);
    assert(game.food_lbs == 400);
    assert(game.bullets == 200);

    // 2. Normal Step Updates
    printf("[OREGON] Testing pace and food rations consumption...\n");
    // Pace: Steady (8), Rations: Filling (3)
    tsfi_oregon_trail_step(&game, 0, 0);
    // distance = 8 * (6/4) = 12 miles. food = 400 - 3 = 397 lbs.
    assert(game.miles_traveled == 12);
    assert(game.food_lbs == 397);
    assert(game.day == 2);

    // 3. River Crossing Simulation
    printf("[OREGON] Testing river crossing logic...\n");
    // Seed random to force a failure or test probability loop
    srand(42);
    int initial_food = game.food_lbs;
    int initial_oxen = game.oxen;
    
    // Cross river (caulk & float)
    bool crossed = tsfi_oregon_trail_cross_river(&game, true);
    if (!crossed) {
        printf("[OREGON]   River crossing failed! Food: %d lbs, Oxen: %d, HP: %d\n",
               game.food_lbs, game.oxen, game.health);
        assert(game.food_lbs < initial_food);
        assert(game.oxen < initial_oxen);
    } else {
        printf("[OREGON]   River crossing succeeded!\n");
    }

    // 4. Hunting
    printf("[OREGON] Testing hunting mechanics...\n");
    int bullets_before = game.bullets;
    int food_before = game.food_lbs;
    game.weather = WEATHER_GOOD;
    tsfi_oregon_trail_hunt(&game);
    printf("[OREGON]   Hunted! Bullets: %d (before %d), Food: %d lbs (before %d)\n",
           game.bullets, bullets_before, game.food_lbs, food_before);
    assert(game.bullets == bullets_before - 10);
    assert(game.food_lbs == food_before + 120);

    // 5. Winning Condition
    printf("[OREGON] Testing destination victory condition...\n");
    game.miles_traveled = 1990;
    game.game_over = false;
    game.reached_destination = false;
    // Step with strenous pace (15 miles) to cross 2000
    tsfi_oregon_trail_step(&game, 1, 0);
    assert(game.miles_traveled == TOTAL_DISTANCE);
    assert(game.reached_destination == true);
    assert(game.game_over == true);
    printf("[OREGON]   Victory check passed.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_oregon_trail", NULL);

    printf("[PASS] Oregon Trail simulation verified.\n");
    return 0;
}
