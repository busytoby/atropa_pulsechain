#include "tsfi_oregon_trail.h"
#include <stdlib.h>

void tsfi_oregon_trail_init(TsfiOregonTrail *game) {
    if (!game) return;
    game->miles_traveled = 0;
    game->food_lbs = 200;
    game->oxen = 4;
    game->bullets = 100;
    game->cash = 400;
    game->health = 100;
    game->day = 1;
    game->weather = WEATHER_GOOD;
    game->has_dysentery = false;
    game->reached_destination = false;
    game->game_over = false;
}

void tsfi_oregon_trail_buy_supplies(TsfiOregonTrail *game, int oxen_to_buy, int food_to_buy, int bullets_to_buy) {
    if (!game) return;
    int cost = (oxen_to_buy * 20) + (int)((float)food_to_buy * 0.2f) + (int)((float)bullets_to_buy * 0.1f);
    if (game->cash >= cost) {
        game->cash -= cost;
        game->oxen += oxen_to_buy;
        game->food_lbs += food_to_buy;
        game->bullets += bullets_to_buy;
    }
}

void tsfi_oregon_trail_step(TsfiOregonTrail *game, int pace_mode, int rations_mode) {
    if (!game || game->game_over) return;

    // 1. Food Consumption
    int food_consumed = 0;
    if (rations_mode == 0) food_consumed = 3;      // Filling
    else if (rations_mode == 1) food_consumed = 2; // Meager
    else food_consumed = 1;                        // Bare Bones

    game->food_lbs -= food_consumed;
    if (game->food_lbs < 0) {
        game->food_lbs = 0;
        game->health -= 8; // Starving
    }

    // 2. Pace & Distance
    int base_miles = 0;
    if (pace_mode == 0) base_miles = 8;        // Steady
    else if (pace_mode == 1) base_miles = 15;  // Strenuous
    else base_miles = 22;                      // Gruelling

    if (game->oxen > 0) {
        float ox_factor = (float)game->oxen / 4.0f;
        if (ox_factor > 1.5f) ox_factor = 1.5f; // Diminishing returns
        game->miles_traveled += (int)((float)base_miles * ox_factor);
    }

    // 3. Health Updates
    if (pace_mode == 2) game->health -= 3; // Gruelling drains health
    if (rations_mode == 2) game->health -= 2; // Bare bones drains health
    if (rations_mode == 0 && pace_mode == 0) game->health += 1; // Good conditions restore health

    if (game->has_dysentery) {
        game->health -= 4; // Sickness drain
    } else {
        // Chance to contract dysentery if health is low
        if (game->health < 50 && (rand() % 100) < 15) {
            game->has_dysentery = true;
        }
    }

    // Constraints
    if (game->health > 100) game->health = 100;
    if (game->health <= 0) {
        game->health = 0;
        game->game_over = true;
    }

    // Win condition
    if (game->miles_traveled >= TOTAL_DISTANCE) {
        game->miles_traveled = TOTAL_DISTANCE;
        game->reached_destination = true;
        game->game_over = true;
    }

    // 4. Update weather & days
    game->day++;
    game->weather = (TsfiWeather)(rand() % 4);
}

bool tsfi_oregon_trail_cross_river(TsfiOregonTrail *game, bool caulk_and_float) {
    if (!game || game->game_over) return false;

    bool success = false;
    if (caulk_and_float) {
        // Caulk and float: 80% success
        success = (rand() % 100) < 80;
    } else {
        // Ford the river: depth dependent
        // Assume river depth is simulated randomly (1 to 6 feet)
        int depth = (rand() % 6) + 1;
        if (depth <= 3) {
            success = true; // Shallow ford
        } else {
            success = (rand() % 100) < 40; // Deep ford is risky
        }
    }

    if (!success) {
        // Lose supplies on disaster
        game->food_lbs -= 50;
        if (game->food_lbs < 0) game->food_lbs = 0;
        if (game->oxen > 1) game->oxen--;
        game->health -= 15;
        if (game->health <= 0) {
            game->health = 0;
            game->game_over = true;
        }
    }

    return success;
}

void tsfi_oregon_trail_hunt(TsfiOregonTrail *game) {
    if (!game || game->game_over || game->bullets < 10) return;

    game->bullets -= 10;
    // Weather affects hunting success
    int food_gain = 0;
    if (game->weather == WEATHER_GOOD) food_gain = 120;
    else if (game->weather == WEATHER_FAIR) food_gain = 80;
    else food_gain = 40;

    game->food_lbs += food_gain;
}
