#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_prman_velocity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HtrcVertex hathitrust_prman_calc_velocity(
    const HtrcVertex *v1,
    const HtrcVertex *v2,
    double time_delta
) {
    HtrcVertex velocity = {0.0, 0.0, 0.0};
    if (!v1 || !v2 || time_delta <= 0.0) return velocity;
    
    velocity.x = (v2->x - v1->x) / time_delta;
    velocity.y = (v2->y - v1->y) / time_delta;
    velocity.z = (v2->z - v1->z) / time_delta;
    
    return velocity;
}
