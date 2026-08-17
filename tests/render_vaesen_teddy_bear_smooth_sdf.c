// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient Full-Frame Photorealistic 3D Vaesen Teddy Bear Renderer Engine
 * Removes the hardcoded floor bar overlay and allows the raymarcher to render
 * the teddy bear and continuous 3D studio environment seamlessly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 1280
#define HEIGHT 720
#define FPS 30
#define DURATION_SEC 5
#define TOTAL_FRAMES (FPS * DURATION_SEC)

#ifndef MOTZKIN_PRIME
#define MOTZKIN_PRIME 953467954114363ULL
#endif

static uint8_t fb[WIDTH * HEIGHT * 3];

typedef struct {
	double x, y, z;
} Vector3;

static inline double vec3_length(Vector3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline Vector3 vec3_normalize(Vector3 v)
{
	double len = vec3_length(v);
	if (len < 1e-6) return (Vector3){ 0, 0, 0 };
	return (Vector3){ v.x / len, v.y / len, v.z / len };
}

static inline double vec3_dot(Vector3 a, Vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vector3 vec3_sub(Vector3 a, Vector3 b)
{
	return (Vector3){ a.x - b.x, a.y - b.y, a.z - b.z };
}

/* ACES Filmic Tone-Mapping Curve */
static inline float aces_filmic(float x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

static void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	/* Y-Up correction: invert Y coordinate so Y=0 is bottom */
	int inverted_y = (HEIGHT - 1) - y;
	if (x >= 0 && x < WIDTH && inverted_y >= 0 && inverted_y < HEIGHT) {
		int idx = (inverted_y * WIDTH + x) * 3;
		fb[idx] = r;
		fb[idx + 1] = g;
		fb[idx + 2] = b;
	}
}

/* Signed Distance Functions (SDF) for Smooth Teddy Bear & Floor */
static double sdf_sphere(Vector3 p, Vector3 center, double radius)
{
	return vec3_length(vec3_sub(p, center)) - radius;
}

static double sdf_plane_y(Vector3 p, double height)
{
	return p.y - height;
}

static double sdf_smin(double a, double b, double k)
{
	double h = fmax(k - fabs(a - b), 0.0) / k;
	return fmin(a, b) - h * h * k * (1.0 / 4.0);
}

/* Complete Scene Distance Field including Floor Plane */
static double scene_sdf(Vector3 p, double t)
{
	double head_y = 0.25 + sin(t * 2.0) * 0.02;

	/* Smooth blended head, muzzle, body, ears, and limbs */
	double head = sdf_sphere(p, (Vector3){ 0.0, head_y, 2.0 }, 0.40);
	double muzzle = sdf_sphere(p, (Vector3){ 0.0, head_y - 0.08, 1.70 }, 0.18);
	double body = sdf_sphere(p, (Vector3){ 0.0, -0.35, 2.1 }, 0.55);

	double left_ear = sdf_sphere(p, (Vector3){ -0.32, head_y + 0.35, 2.05 }, 0.14);
	double right_ear = sdf_sphere(p, (Vector3){ 0.32, head_y + 0.35, 2.05 }, 0.14);

	double left_arm = sdf_sphere(p, (Vector3){ -0.48 + sin(t*3.0)*0.05, -0.25, 2.0 }, 0.18);
	double right_arm = sdf_sphere(p, (Vector3){ 0.48 - sin(t*3.0)*0.05, -0.25, 2.0 }, 0.18);

	double left_leg = sdf_sphere(p, (Vector3){ -0.35, -0.70, 1.85 }, 0.22);
	double right_leg = sdf_sphere(p, (Vector3){ 0.35, -0.70, 1.85 }, 0.22);

	/* Smooth blending across body parts */
	double bear = sdf_smin(head, muzzle, 0.12);
	bear = sdf_smin(bear, body, 0.18);
	bear = sdf_smin(bear, left_ear, 0.08);
	bear = sdf_smin(bear, right_ear, 0.08);
	bear = sdf_smin(bear, left_arm, 0.12);
	bear = sdf_smin(bear, right_arm, 0.12);
	bear = sdf_smin(bear, left_leg, 0.15);
	bear = sdf_smin(bear, right_leg, 0.15);

	/* Studio Floor Plane at y = -0.92 */
	double floor_p = sdf_plane_y(p, -0.92);

	return fmin(bear, floor_p);
}

/* Calculate Normal Vector via Finite Differences */
static Vector3 calc_normal(Vector3 p, double t)
{
	double eps = 0.001;
	double dx = scene_sdf((Vector3){ p.x + eps, p.y, p.z }, t) - scene_sdf((Vector3){ p.x - eps, p.y, p.z }, t);
	double dy = scene_sdf((Vector3){ p.x, p.y + eps, p.z }, t) - scene_sdf((Vector3){ p.x, p.y - eps, p.z }, t);
	double dz = scene_sdf((Vector3){ p.x, p.y, p.z + eps }, t) - scene_sdf((Vector3){ p.x, p.y, p.z - eps }, t);
	return vec3_normalize((Vector3){ dx, dy, dz });
}

int main(void)
{
	printf("=============================================================\n");
	printf("AUNCIENT FULL-FRAME PHOTOREALISTIC TEDDY BEAR RENDER ENGINE  \n");
	printf("=============================================================\n");

	FILE *ffmpeg = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 1280x720 -pix_fmt rgb24 -r 30 -i - -c:v libx264 -pix_fmt yuv420p vaesen_teddy_bear_photorealistic.mp4 > /dev/null 2>&1", "w");
	if (!ffmpeg) {
		fprintf(stderr, "Error spawning ffmpeg encoder pipeline.\n");
		return 1;
	}

	printf("Raymarching full-frame 3D scene to 'vaesen_teddy_bear_photorealistic.mp4'...\n");

	Vector3 light_pos = { -1.5, 2.5, 0.5 };

	for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
		double t = (double)frame / FPS;

		for (int y = 0; y < HEIGHT; y++) {
			double vy = (((double)y - HEIGHT / 2.0) / (HEIGHT / 2.0));
			for (int x = 0; x < WIDTH; x++) {
				double vx = (((double)x - WIDTH / 2.0) / (HEIGHT / 2.0));

				/* Ray marching full viewport */
				Vector3 ray_origin = { 0.0, 0.0, 0.0 };
				Vector3 ray_dir = vec3_normalize((Vector3){ vx * 0.5, vy * 0.5, 1.0 });

				double depth = 1.0;
				bool hit = false;
				Vector3 p = { 0, 0, 0 };

				for (int step = 0; step < 80; step++) {
					p = (Vector3){ ray_origin.x + ray_dir.x * depth,
						           ray_origin.y + ray_dir.y * depth,
						           ray_origin.z + ray_dir.z * depth };
					double d = scene_sdf(p, t);
					if (d < 0.0015) {
						hit = true;
						break;
					}
					depth += d;
					if (depth > 6.0) break;
				}

				if (hit) {
					Vector3 normal = calc_normal(p, t);
					Vector3 light_dir = vec3_normalize(vec3_sub(light_pos, p));

					/* Check if hit Floor Plane (p.y <= -0.91) */
					if (p.y <= -0.91) {
						double wood_grain = sin(p.x * 20.0 + p.z * 10.0) * 0.04;
						double diff = fmax(0.2, vec3_dot(normal, light_dir));
						float r = aces_filmic((0.38f + wood_grain) * diff);
						float g = aces_filmic((0.24f + wood_grain) * diff);
						float b = aces_filmic(0.14f * diff);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					} else {
						/* Teddy Bear Shading */
						double diff = fmax(0.0, vec3_dot(normal, light_dir));
						uint8_t fur_noise = (uint8_t)(((x * 13 + y * 29 + frame * 7) ^ (uint64_t)(p.x * 100)) & 0x1F);

						/* Glass Eye Highlights */
						double eye_dist_l = vec3_length(vec3_sub(p, (Vector3){ -0.14, 0.28, 1.72 }));
						double eye_dist_r = vec3_length(vec3_sub(p, (Vector3){ 0.14, 0.28, 1.72 }));

						if (eye_dist_l < 0.045 || eye_dist_r < 0.045) {
							Vector3 view_dir = vec3_normalize(vec3_sub(ray_origin, p));
							Vector3 reflect_dir = vec3_sub(light_dir, (Vector3){ 2.0 * vec3_dot(light_dir, normal) * normal.x, 2.0 * vec3_dot(light_dir, normal) * normal.y, 2.0 * vec3_dot(light_dir, normal) * normal.z });
							double spec = pow(fmax(0.0, -vec3_dot(view_dir, reflect_dir)), 16.0);
							uint8_t val = (uint8_t)(fmin(255.0, 15.0 + spec * 240.0));
							set_pixel(x, y, val, val, val);
						} else {
							float base_r = 0.55f * diff + 0.15f + (fur_noise / 255.0f * 0.1f);
							float base_g = 0.36f * diff + 0.10f + (fur_noise / 255.0f * 0.1f);
							float base_b = 0.22f * diff + 0.05f;

							float r = aces_filmic(base_r);
							float g = aces_filmic(base_g);
							float b = aces_filmic(base_b);
							set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
						}
					}
				} else {
					/* Neutral Studio Wall Backdrop */
					float ny = ((float)y / HEIGHT);
					float val = 0.70f + (ny - 0.32f) * 0.15f;
					float r = aces_filmic(val * 0.90f);
					float g = aces_filmic(val * 0.92f);
					float b = aces_filmic(val * 0.95f);
					set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
				}
			}
		}

		fwrite(fb, 1, sizeof(fb), ffmpeg);
	}

	pclose(ffmpeg);

	printf("=============================================================\n");
	printf("   FULL-FRAME RENDER COMPLETE: vaesen_teddy_bear_photorealistic.mp4\n");
	printf("=============================================================\n");

	return 0;
}
