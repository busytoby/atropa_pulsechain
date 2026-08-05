#ifndef HATHITRUST_PRMAN_VELOCITY_H
#define HATHITRUST_PRMAN_VELOCITY_H

#include "hathitrust_render.h"

/*
 * Calculates the velocity vectors of page vertices between two time steps 
 * to supply RenderMan with motion blur parameters (__V attribute).
 * Parameters:
 *   v1: Vertex position at start of frame.
 *   v2: Vertex position at end of frame.
 *   time_delta: Time delta between frames.
 * Returns:
 *   Velocity vector.
 */
HtrcVertex hathitrust_prman_calc_velocity(
    const HtrcVertex *v1,
    const HtrcVertex *v2,
    double time_delta
);

#endif /* HATHITRUST_PRMAN_VELOCITY_H */
