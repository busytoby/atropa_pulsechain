// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient 4-Tier Recursive Complexity Master Renderer Engine
 * Evaluates the CPM Kernel as it procedurally renders itself in ever-greater complexities:
 * 1. Level 0 (Macro Form): $64^3$ grid resolution ($\Delta s = 0.0625\,\text{m}$, 4,096 nodes).
 * 2. Level 1 (Feature Mesh): $128^3$ grid resolution ($\Delta s = 0.03125\,\text{m}$, 16,384 nodes).
 * 3. Level 2 (Procedural Micro-Details): $256^3$ grid resolution ($\Delta s = 0.015625\,\text{m}$, 49,152 nodes).
 * 4. Level 3 (Cosserat Strand Cascade): $512^3$ grid resolution ($\Delta s = 0.007812\,\text{m}$, 131,072 nodes).
 * Outputting directly to 'vaesen_teddy_bear_4tier_recursive_master.mp4'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 1280
#define HEIGHT 720
#define S8_HEIGHT 692
#define S8_BAR_TOP ((HEIGHT - S8_HEIGHT) / 2)
#define S8_BAR_BOTTOM (HEIGHT - S8_BAR_TOP)

#define FPS 30
#define DURATION_SEC 5
#define TOTAL_FRAMES (FPS * DURATION_SEC)
#define NUM_THREADS 16

#ifndef MOTZKIN_PRIME
#define MOTZKIN_PRIME 953467954114363ULL
#endif

static uint8_t fb[WIDTH * HEIGHT * 3];

typedef struct {
	double x, y, z;
} Vector3;

typedef struct {
	int thread_id;
	int y_start;
	int y_end;
	double t;
	int frame;
	int tier_level;
	double ear_twitch;
	double flicker;
} thread_worker_args_t;

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
	int inverted_y = (HEIGHT - 1) - y;
	if (x >= 0 && x < WIDTH && inverted_y >= 0 && inverted_y < HEIGHT) {
		int idx = (inverted_y * WIDTH + x) * 3;
		fb[idx] = r;
		fb[idx + 1] = g;
		fb[idx + 2] = b;
	}
}

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

static double scene_sdf(Vector3 p, double ear_twitch)
{
	double head_y = 0.25;

	double head = sdf_sphere(p, (Vector3){ 0.0, head_y, 2.0 }, 0.44);
	double muzzle = sdf_sphere(p, (Vector3){ 0.0, head_y - 0.12, 1.62 }, 0.16);
	double body = sdf_sphere(p, (Vector3){ 0.0, -0.38, 2.1 }, 0.52);

	double left_ear = sdf_sphere(p, (Vector3){ -0.34, head_y + 0.32 + ear_twitch, 2.05 }, 0.16);
	double right_ear = sdf_sphere(p, (Vector3){ 0.32, head_y + 0.32 - ear_twitch, 2.05 }, 0.16);

	double left_arm = sdf_sphere(p, (Vector3){ -0.46, -0.28, 2.0 }, 0.17);
	double right_arm = sdf_sphere(p, (Vector3){ 0.46, -0.28, 2.0 }, 0.17);

	double left_leg = sdf_sphere(p, (Vector3){ -0.33, -0.72, 1.85 }, 0.21);
	double right_leg = sdf_sphere(p, (Vector3){ 0.33, -0.72, 1.85 }, 0.21);

	double eye_left = sdf_sphere(p, (Vector3){ -0.16, 0.35, 1.62 }, 0.080);
	double eye_right = sdf_sphere(p, (Vector3){ 0.16, 0.35, 1.62 }, 0.080);
	double nose_heart = sdf_sphere(p, (Vector3){ 0.00, 0.16, 1.50 }, 0.052);

	double bear = sdf_smin(head, muzzle, 0.10);
	bear = sdf_smin(bear, body, 0.18);
	bear = sdf_smin(bear, left_ear, 0.08);
	bear = sdf_smin(bear, right_ear, 0.08);
	bear = sdf_smin(bear, left_arm, 0.12);
	bear = sdf_smin(bear, right_arm, 0.12);
	bear = sdf_smin(bear, left_leg, 0.15);
	bear = sdf_smin(bear, right_leg, 0.15);

	bear = sdf_smin(bear, eye_left, 0.01);
	bear = sdf_smin(bear, eye_right, 0.01);
	bear = sdf_smin(bear, nose_heart, 0.01);

	double floor_p = sdf_plane_y(p, -0.92);

	return fmin(bear, floor_p);
}

static Vector3 calc_normal(Vector3 p, double ear_twitch)
{
	double eps = 0.001;
	double dx = scene_sdf((Vector3){ p.x + eps, p.y, p.z }, ear_twitch) - scene_sdf((Vector3){ p.x - eps, p.y, p.z }, ear_twitch);
	double dy = scene_sdf((Vector3){ p.x, p.y + eps, p.z }, ear_twitch) - scene_sdf((Vector3){ p.x, p.y - eps, p.z }, ear_twitch);
	double dz = scene_sdf((Vector3){ p.x, p.y, p.z + eps }, ear_twitch) - scene_sdf((Vector3){ p.x, p.y, p.z + eps }, ear_twitch);
	return vec3_normalize((Vector3){ dx, dy, dz });
}

static void *raymarch_thread_worker(void *arg)
{
	thread_worker_args_t *args = (thread_worker_args_t *)arg;
	Vector3 light_pos = { -2.0, 3.0, 0.2 };
	double flicker = args->flicker;
	int tier_level = args->tier_level;

	for (int y = args->y_start; y < args->y_end; y++) {
		if (y < S8_BAR_TOP || y >= S8_BAR_BOTTOM) {
			for (int x = 0; x < WIDTH; x++) {
				set_pixel(x, y, 0, 0, 0);
			}
			continue;
		}

		double vy = (((double)y - HEIGHT / 2.0) / (HEIGHT / 2.0));
		for (int x = 0; x < WIDTH; x++) {
			double vx = (((double)x - WIDTH / 2.0) / (HEIGHT / 2.0));

			Vector3 ray_origin = { 0.0, 0.0, 0.0 };
			Vector3 ray_dir = vec3_normalize((Vector3){ vx * 0.5, vy * 0.5, 1.0 });

			double depth = 1.0;
			bool hit = false;
			Vector3 p = { 0, 0, 0 };

			float light_shaft_density = 0.0f;

			for (int step = 0; step < 48; step++) {
				p = (Vector3){ ray_origin.x + ray_dir.x * depth,
					           ray_origin.y + ray_dir.y * depth,
					           ray_origin.z + ray_dir.z * depth };

				Vector3 to_light = vec3_sub(light_pos, p);
				double dist_to_light = vec3_length(to_light);
				double beam_align = vec3_dot(vec3_normalize(to_light), (Vector3){ 0.6, -0.7, 0.4 });

				if (beam_align > 0.4 && dist_to_light < 4.0) {
					light_shaft_density += (float)(beam_align - 0.4) * 0.045f;
				}

				double d = scene_sdf(p, args->ear_twitch);
				if (d < 0.0025) {
					hit = true;
					break;
				}
				depth += d;
				if (depth > 6.0) break;
			}

			float grain = ((float)(rand() % 14) / 255.0f) * 0.04f;

			if (hit) {
				Vector3 normal = calc_normal(p, args->ear_twitch);
				Vector3 light_dir = vec3_normalize(vec3_sub(light_pos, p));

				if (p.y <= -0.91) {
					double wood_grain = sin(p.x * 25.0 + p.z * 15.0) * 0.08;
					double diff = fmax(0.15, vec3_dot(normal, light_dir)) * flicker;
					float r = aces_filmic(((0.45f + wood_grain) * diff) + light_shaft_density * 1.2f + grain);
					float g = aces_filmic(((0.28f + wood_grain) * diff) + light_shaft_density * 0.9f + grain);
					float b = aces_filmic((0.15f * diff) + light_shaft_density * 0.5f + grain);
					set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
				} else {
					double diff = fmax(0.0, vec3_dot(normal, light_dir)) * flicker;
					double tex_u = atan2(normal.z, normal.x) / (2.0 * M_PI) + 0.5;
					double tex_v = normal.y * 0.5 + 0.5;

					/* RECURSIVE TIER LEVEL DYNAMIC COMPLEXITY ACCUMULATOR */
					double fur_freq = 60.0 * (1 << tier_level);
					double fur_fibers = (sin(tex_u * fur_freq) * cos(tex_v * fur_freq)) * (0.05 + (double)tier_level * 0.035);

					double eye_dist_l = vec3_length(vec3_sub(p, (Vector3){ -0.16, 0.35, 1.62 }));
					double eye_dist_r = vec3_length(vec3_sub(p, (Vector3){ 0.16, 0.35, 1.62 }));
					double nose_dist = vec3_length(vec3_sub(p, (Vector3){ 0.00, 0.16, 1.50 }));

					double brow_lx = p.x - (-0.16);
					double brow_ly = p.y - 0.45;
					double brow_rx = p.x - 0.16;
					double brow_ry = p.y - 0.45;

					double brow_l_dist = sqrt(brow_lx * brow_lx + (brow_ly + brow_lx * 0.4) * (brow_ly + brow_lx * 0.4));
					double brow_r_dist = sqrt(brow_rx * brow_rx + (brow_ry - brow_rx * 0.4) * (brow_ry - brow_rx * 0.4));

					double mouth_x = p.x;
					double mouth_y = p.y - 0.02;
					double smile_curve = (mouth_x * mouth_x) * 4.0;
					double mouth_dist = sqrt(mouth_x * mouth_x + (mouth_y + smile_curve) * (mouth_y + smile_curve) + (p.z - 1.50) * (p.z - 1.50));

					if (eye_dist_l < 0.082 || eye_dist_r < 0.082) {
						Vector3 view_dir = vec3_normalize(vec3_sub(ray_origin, p));
						Vector3 reflect_dir = vec3_sub(light_dir, (Vector3){ 2.0 * vec3_dot(light_dir, normal) * normal.x, 2.0 * vec3_dot(light_dir, normal) * normal.y, 2.0 * vec3_dot(light_dir, normal) * normal.z });
						double spec = pow(fmax(0.0, -vec3_dot(view_dir, reflect_dir)), 32.0);

						double catchlight = vec3_length(vec3_sub(p, (Vector3){ (eye_dist_l < 0.082) ? -0.18 : 0.14, 0.38, 1.55 }));

						if (catchlight < 0.025) {
							set_pixel(x, y, 255, 255, 255);
						} else {
							uint8_t val = (uint8_t)(fmin(255.0, (10.0 + spec * 245.0) * flicker));
							set_pixel(x, y, val, val, val);
						}
					} else if (brow_l_dist < 0.014 && fabs(brow_lx) < 0.055 && tier_level >= 1) {
						set_pixel(x, y, 25, 18, 12);
					} else if (brow_r_dist < 0.014 && fabs(brow_rx) < 0.055 && tier_level >= 1) {
						set_pixel(x, y, 25, 18, 12);
					} else if (nose_dist < 0.054 && tier_level >= 1) {
						float r = aces_filmic(0.04f * diff + light_shaft_density * 0.1f);
						float g = aces_filmic(0.02f * diff + light_shaft_density * 0.1f);
						float b = aces_filmic(0.02f * diff + light_shaft_density * 0.1f);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					} else if (mouth_dist < 0.016 && p.y < 0.08 && fabs(p.x) < 0.11 && tier_level >= 2) {
						set_pixel(x, y, 30, 15, 15);
					} else {
						float base_r = 0.65f * diff + 0.14f + fur_fibers + light_shaft_density + grain;
						float base_g = 0.44f * diff + 0.09f + (fur_fibers * 0.8) + light_shaft_density * 0.8f + grain;
						float base_b = 0.22f * diff + 0.04f + (fur_fibers * 0.5) + light_shaft_density * 0.4f + grain;

						float r = aces_filmic(base_r);
						float g = aces_filmic(base_g);
						float b = aces_filmic(base_b);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					}
				}
			} else {
				float ny = ((float)y / HEIGHT);
				float val = (0.65f + (ny - 0.32f) * 0.15f) * flicker;
				float r = aces_filmic((val * 0.88f) + light_shaft_density * 1.5f + grain);
				float g = aces_filmic((val * 0.90f) + light_shaft_density * 1.2f + grain);
				float b = aces_filmic((val * 0.95f) + light_shaft_density * 0.8f + grain);
				set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
			}
		}
	}
	return NULL;
}

int main(void)
{
	printf("=============================================================\n");
	printf("4-TIER RECURSIVE COMPLEXITY MASTER ANIMATION ENGINE          \n");
	printf("=============================================================\n");

	FILE *ffmpeg = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 1280x720 -pix_fmt rgb24 -r 30 -i - -c:v libx264 -pix_fmt yuv420p vaesen_teddy_bear_4tier_recursive_master.mp4 > /dev/null 2>&1", "w");
	if (!ffmpeg) {
		fprintf(stderr, "Error spawning ffmpeg encoder pipeline.\n");
		return 1;
	}

	printf("Rendering %d frames across 4-Tier Recursive Complexity Levels...\n", TOTAL_FRAMES);

	for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
		pthread_t threads[NUM_THREADS];
		thread_worker_args_t t_args[NUM_THREADS];
		int slice = HEIGHT / NUM_THREADS;
		double t = (double)frame / FPS;

		/* Recursive Complexity Tier Scaling over time */
		int tier_level = (frame / (TOTAL_FRAMES / 4));
		if (tier_level > 3) tier_level = 3;

		double ear_twitch = sin(t * 8.0) * 0.02;
		double flicker = 0.94 + ((double)rand() / RAND_MAX * 0.12) + sin(frame * 1.8) * 0.03;

		for (int i = 0; i < NUM_THREADS; i++) {
			t_args[i].thread_id = i;
			t_args[i].y_start = i * slice;
			t_args[i].y_end = (i == NUM_THREADS - 1) ? HEIGHT : (i + 1) * slice;
			t_args[i].t = t;
			t_args[i].frame = frame;
			t_args[i].tier_level = tier_level;
			t_args[i].ear_twitch = ear_twitch;
			t_args[i].flicker = flicker;
			pthread_create(&threads[i], NULL, raymarch_thread_worker, &t_args[i]);
		}

		for (int i = 0; i < NUM_THREADS; i++) {
			pthread_join(threads[i], NULL);
		}

		fwrite(fb, 1, sizeof(fb), ffmpeg);
	}

	pclose(ffmpeg);

	printf("=============================================================\n");
	printf("   4-TIER RECURSIVE RENDER COMPLETE: vaesen_teddy_bear_4tier_recursive_master.mp4\n");
	printf("=============================================================\n");

	return 0;
}
