#ifndef TSFI_OREGON_TRAIL_H
#define TSFI_OREGON_TRAIL_H

#include <stdint.h>
#include <stdbool.h>

#define TOTAL_DISTANCE 2000 // Miles to Oregon

typedef enum {
    WEATHER_GOOD = 0,
    WEATHER_FAIR,
    WEATHER_POOR,
    WEATHER_VERY_POOR
} TsfiWeather;

typedef struct {
    int miles_traveled;
    int food_lbs;
    int oxen;
    int bullets;
    int cash;
    int health;              // Health from 0 (dead) to 100 (perfect)
    int day;
    TsfiWeather weather;
    bool has_dysentery;
    bool reached_destination;
    bool game_over;
} TsfiOregonTrail;

// --- API Declarations ---

void tsfi_oregon_trail_init(TsfiOregonTrail *game);
void tsfi_oregon_trail_buy_supplies(TsfiOregonTrail *game, int oxen_to_buy, int food_to_buy, int bullets_to_buy);
void tsfi_oregon_trail_step(TsfiOregonTrail *game, int pace_mode, int rations_mode);
bool tsfi_oregon_trail_cross_river(TsfiOregonTrail *game, bool caulk_and_float);
void tsfi_oregon_trail_hunt(TsfiOregonTrail *game);

#endif // TSFI_OREGON_TRAIL_H
