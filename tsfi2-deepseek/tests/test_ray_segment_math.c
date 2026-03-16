#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

// Vector Types
typedef struct { float x, y; } vec2;
typedef struct { float x, y, z; } vec3;

// Math Helpers
vec2 sub(vec2 a, vec2 b) { return (vec2){a.x - b.x, a.y - b.y}; }
vec2 add(vec2 a, vec2 b) { return (vec2){a.x + b.x, a.y + b.y}; }
float dot(vec2 a, vec2 b) { return a.x*b.x + a.y*b.y; }
float len(vec2 a) { return sqrtf(dot(a, a)); }

// The Intersection Logic (Simulating RDNA4 Intersection Shader)
// Ray: Origin(rox, roy), Dir(rdx, rdy) - 2D slice of the 3D ray
// Segment: P0, P1, P2 (Quadratic Bezier)
// Thickness: w
bool intersect_segment(vec2 ro, vec2 rd, vec2 p0, vec2 p1, vec2 p2, float thickness, float *t_out) {
    // 1. We are in 2D glyph space. The 3D ray hits the Z=0 plane.
    // Assuming the Caller already transformed the 3D ray to the Plane Hit Point (P_hit).
    // So "ro" here is actually the Point on the plane we are testing.
    // "rd" is not needed if we just test distance to point.
    // Wait, an Intersection Shader takes a Ray.
    // But for 2D text in 3D, the text is flat.
    // The Ray Accelerator hits the AABB (Z-bounded).
    // The shader computes exact intersection.
    
    // Simplification for 2D Text:
    // We treat the glyph as a tube in 3D? Or a flat cutout?
    // "Hologram" usually implies flat cutout.
    // So we calculate the distance from P_hit (ro) to the Bezier Curve.
    
    vec2 pos = ro;
    
    // Distance to Quadratic Bezier (Iterative Approx or Analytic)
    // Analytic is solvable (Cubic roots) but complex for C test.
    // Let's use the Iterative approach (10 steps).
    // Q(t) = (1-t)^2 P0 + 2(1-t)t P1 + t^2 P2
    
    float min_d = 1e9;
    
    // Brute force 20 samples for verification
    for(int i=0; i<=20; i++) {
        float t = (float)i / 20.0f;
        float u = 1.0f - t;
        
        float x = u*u*p0.x + 2*u*t*p1.x + t*t*p2.x;
        float y = u*u*p0.y + 2*u*t*p1.y + t*t*p2.y;
        
        vec2 p = {x, y};
        float d = len(sub(pos, p));
        if (d < min_d) min_d = d;
    }
    
    *t_out = min_d;
    return min_d <= thickness;
}

int main() {
    printf("[TEST] Ray-Segment Intersection Math (Simulation)...\n");

    // Define a Curve (The top of an 'A')
    // Coords from our engine: (-6000, -12000) to (0, 12000)
    // Left Leg. P0=(-6000, -12000), P1=mid?, P2=(0, 12000).
    // It's a line, so P1 is midpoint.
    vec2 p0 = {-6000, -12000};
    vec2 p2 = {0, 12000};
    vec2 p1 = {-3000, 0}; // Midpoint for line
    
    float thickness = 400.0f;

    // Test 1: Hit exactly on the line
    // Midpoint is (-3000, 0).
    float dist;
    bool hit = intersect_segment((vec2){-3000, 0}, (vec2){0,0}, p0, p1, p2, thickness, &dist);
    printf("[MATH] Probe (-3000, 0): Dist=%.2f, Hit=%s\n", dist, hit ? "YES" : "NO");
    assert(hit);
    assert(dist < 10.0f);

    // Test 2: Hit Near (Inside Thickness)
    // -3000 + 200 = -2800.
    hit = intersect_segment((vec2){-2800, 0}, (vec2){0,0}, p0, p1, p2, thickness, &dist);
    printf("[MATH] Probe (-2800, 0): Dist=%.2f, Hit=%s\n", dist, hit ? "YES" : "NO");
    assert(hit);

    // Test 3: Miss (Outside Thickness)
    // -3000 + 500 = -2500.
    hit = intersect_segment((vec2){-2500, 0}, (vec2){0,0}, p0, p1, p2, thickness, &dist);
    printf("[MATH] Probe (-2500, 0): Dist=%.2f, Hit=%s\n", dist, hit ? "YES" : "NO");
    assert(!hit);
    assert(dist > 400.0f);

    printf("[TEST] Passed. Intersection Logic Valid.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}