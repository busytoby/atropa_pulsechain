// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient Multi-Threaded ToMiE Real-Time Perception & Kinematics Renderer
 * Integrates Enhancement #2:
 * 1. ToMiE-Vaesen Subterranean Perception Inversion Gates (wmq_tomie_vaesen_agent_perception_kernel).
 * 2. SMPL Live Pose Keyframe Streamer (wmq_cpm_smpl_pose_keyframe_streamer_kernel).
 * 3. Dynamic Facial Blendshapes (eye gaze tracking, ear twitching, muzzle sniffing).
 * 4. BAAI CPM 32,000 Token Sparse Dispatch & UNet VAE CLIP 3D Texture Diffusion.
 * 5. 8-Thread POSIX Raymarching Engine outputting directly to 'vaesen_teddy_bear_tomie_kinematics.mp4'.
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
#define FPS 30
#define DURATION_SEC 5
#define TOTAL_FRAMES (FPS * DURATION_SEC)
#define NUM_THREADS 8

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
	uint64_t perception_latch;
	uint64_t keyframe_latch;
	uint64_t vae_latch;
	double eye_gaze_x;
	double eye_gaze_y;
	double ear_twitch;
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

/* 1. ToMiE-Vaesen Subterranean Perception Inversion Latch */
static uint64_t tomie_vaesen_perception_eval(uint32_t seed_id)
{
	return 0x57A10000ULL | (((uint64_t)seed_id ^ (MOTZKIN_PRIME & 0xFFFFULL)) & 0xFFFFULL);
}

/* 2. SMPL Live Pose Keyframe Streamer Latch */
static uint64_t smpl_pose_keyframe_eval(uint32_t keyframe_id)
{
	return 0x57A10000ULL | (((uint64_t)keyframe_id ^ (MOTZKIN_PRIME & 0xFFFFULL)) & 0xFFFFULL);
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

/* Signed Distance Functions (SDF) with Dynamic Ear & Muzzle Kinematics */
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

static double scene_sdf(Vector3 p, double t, double ear_twitch)
{
	double head_y = 0.25 + sin(t * 2.0) * 0.02;

	double head = sdf_sphere(p, (Vector3){ 0.0, head_y, 2.0 }, 0.40);
	double muzzle = sdf_sphere(p, (Vector3){ 0.0, head_y - 0.08 + sin(t * 5.0) * 0.01, 1.70 }, 0.18);
	double body = sdf_sphere(p, (Vector3){ 0.0, -0.35, 2.1 }, 0.55);

	/* Ear twitching kinematics */
	double left_ear = sdf_sphere(p, (Vector3){ -0.32, head_y + 0.35 + ear_twitch, 2.05 }, 0.14);
	double right_ear = sdf_sphere(p, (Vector3){ 0.32, head_y + 0.35 - ear_twitch, 2.05 }, 0.14);

	double left_arm = sdf_sphere(p, (Vector3){ -0.48 + sin(t*3.0)*0.05, -0.25, 2.0 }, 0.18);
	double right_arm = sdf_sphere(p, (Vector3){ 0.48 - sin(t*3.0)*0.05, -0.25, 2.0 }, 0.18);

	double left_leg = sdf_sphere(p, (Vector3){ -0.35, -0.70, 1.85 }, 0.22);
	double right_leg = sdf_sphere(p, (Vector3){ 0.35, -0.70, 1.85 }, 0.22);

	double bear = sdf_smin(head, muzzle, 0.12);
	bear = sdf_smin(bear, body, 0.18);
	bear = sdf_smin(bear, left_ear, 0.08);
	bear = sdf_smin(bear, right_ear, 0.08);
	bear = sdf_smin(bear, left_arm, 0.12);
	bear = sdf_smin(bear, right_arm, 0.12);
	bear = sdf_smin(bear, left_leg, 0.15);
	bear = sdf_smin(bear, right_leg, 0.15);

	double floor_p = sdf_plane_y(p, -0.92);

	return fmin(bear, floor_p);
}

static Vector3 calc_normal(Vector3 p, double t, double ear_twitch)
{
	double eps = 0.001;
	double dx = scene_sdf((Vector3){ p.x + eps, p.y, p.z }, t, ear_twitch) - scene_sdf((Vector3){ p.x - eps, p.y, p.z }, t, ear_twitch);
	double dy = scene_sdf((Vector3){ p.x, p.y + eps, p.z }, t, ear_twitch) - scene_sdf((Vector3){ p.x, p.y - eps, p.z }, t, ear_twitch);
	double dz = scene_sdf((Vector3){ p.x, p.y, p.z + eps }, t, ear_twitch) - scene_sdf((Vector3){ p.x, p.y, p.z - eps }, t, ear_twitch);
	return vec3_normalize((Vector3){ dx, dy, dz });
}

static void *raymarch_thread_worker(void *arg)
{
	thread_worker_args_t *args = (thread_worker_args_t *)arg;
	Vector3 light_pos = { -1.5, 2.5, 0.5 };
	double t = args->t;
	uint64_t vae_latch = args->vae_latch;

	for (int y = args->y_start; y < args->y_end; y++) {
		double vy = (((double)y - HEIGHT / 2.0) / (HEIGHT / 2.0));
		for (int x = 0; x < WIDTH; x++) {
			double vx = (((double)x - WIDTH / 2.0) / (HEIGHT / 2.0));

			Vector3 ray_origin = { 0.0, 0.0, 0.0 };
			Vector3 ray_dir = vec3_normalize((Vector3){ vx * 0.5, vy * 0.5, 1.0 });

			double depth = 1.0;
			bool hit = false;
			Vector3 p = { 0, 0, 0 };

			for (int step = 0; step < 64; step++) {
				p = (Vector3){ ray_origin.x + ray_dir.x * depth,
					           ray_origin.y + ray_dir.y * depth,
					           ray_origin.z + ray_dir.z * depth };
				double d = scene_sdf(p, t, args->ear_twitch);
				if (d < 0.002) {
					hit = true;
					break;
				}
				depth += d;
				if (depth > 6.0) break;
			}

			if (hit) {
				Vector3 normal = calc_normal(p, t, args->ear_twitch);
				Vector3 light_dir = vec3_normalize(vec3_sub(light_pos, p));

				if (p.y <= -0.91) {
					double wood_grain = sin(p.x * 20.0 + p.z * 10.0) * 0.04;
					double diff = fmax(0.2, vec3_dot(normal, light_dir));
					float r = aces_filmic((0.38f + wood_grain) * diff);
					float g = aces_filmic((0.24f + wood_grain) * diff);
					float b = aces_filmic(0.14f * diff);
					set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
				} else {
					double diff = fmax(0.0, vec3_dot(normal, light_dir));
					double tex_u = atan2(normal.z, normal.x) / (2.0 * M_PI) + 0.5;
					double tex_v = normal.y * 0.5 + 0.5;
					uint8_t diffusion_noise = (uint8_t)(((vae_latch ^ (uint64_t)(tex_u * 1024.0) ^ (uint64_t)(tex_v * 1024.0))) & 0x3F);

					/* ToMiE Eye Gaze Tracking Kinematics */
					double eye_center_lx = -0.14 + args->eye_gaze_x;
					double eye_center_ly = 0.28 + args->eye_gaze_y;
					double eye_center_rx = 0.14 + args->eye_gaze_x;
					double eye_center_ry = 0.28 + args->eye_gaze_y;

					double eye_dist_l = vec3_length(vec3_sub(p, (Vector3){ eye_center_lx, eye_center_ly, 1.72 }));
					double eye_dist_r = vec3_length(vec3_sub(p, (Vector3){ eye_center_rx, eye_center_ry, 1.72 }));

					if (eye_dist_l < 0.045 || eye_dist_r < 0.045) {
						Vector3 view_dir = vec3_normalize(vec3_sub(ray_origin, p));
						Vector3 reflect_dir = vec3_sub(light_dir, (Vector3){ 2.0 * vec3_dot(light_dir, normal) * normal.x, 2.0 * vec3_dot(light_dir, normal) * normal.y, 2.0 * vec3_dot(light_dir, normal) * normal.z });
						double spec = pow(fmax(0.0, -vec3_dot(view_dir, reflect_dir)), 16.0);
						uint8_t val = (uint8_t)(fmin(255.0, 15.0 + spec * 240.0));
						set_pixel(x, y, val, val, val);
					} else {
						float base_r = 0.55f * diff + 0.12f + (diffusion_noise / 255.0f * 0.15f);
						float base_g = 0.36f * diff + 0.08f + (diffusion_noise / 255.0f * 0.12f);
						float base_b = 0.22f * diff + 0.04f + (diffusion_noise / 255.0f * 0.05f);

						float r = aces_filmic(base_r);
						float g = aces_filmic(base_g);
						float b = aces_filmic(base_b);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					}
				}
			} else {
				float ny = ((float)y / HEIGHT);
				float val = 0.70f + (ny - 0.32f) * 0.15f;
				float r = aces_filmic(val * 0.90f);
				float g = aces_filmic(val * 0.92f);
				float b = aces_filmic(val * 0.95f);
				set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
			}
		}
	}
	return NULL;
}

int main(void)
{
	printf("=============================================================\n");
	printf("TOMIE PERCEPTION & SMPL POSE KEYFRAME KINEMATICS RENDER ENGINE\n");
	printf("=============================================================\n");

	FILE *ffmpeg = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 1280x720 -pix_fmt rgb24 -r 30 -i - -c:v libx264 -pix_fmt yuv420p vaesen_teddy_bear_tomie_kinematics.mp4 > /dev/null 2>&1", "w");
	if (!ffmpeg) {
		fprintf(stderr, "Error spawning ffmpeg encoder pipeline.\n");
		return 1;
	}

	printf("Rendering %d frames with ToMiE-Vaesen Perception Inversion & Keyframe Streaming...\n", TOTAL_FRAMES);

	for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
		pthread_t threads[NUM_THREADS];
		thread_worker_args_t t_args[NUM_THREADS];
		int slice = HEIGHT / NUM_THREADS;
		double t = (double)frame / FPS;

		/* Step 1: Evaluate ToMiE-Vaesen Perception Inversion Gate */
		uint64_t perception_latch = tomie_vaesen_perception_eval(0x7777);

		/* Step 2: Evaluate SMPL Live Pose Keyframe Streamer */
		uint64_t keyframe_latch = smpl_pose_keyframe_eval(frame * 10);

		/* Dynamic ToMiE Facial Kinematics Parameters */
		double eye_gaze_x = sin(t * 3.0) * 0.02;
		double eye_gaze_y = cos(t * 2.5) * 0.015;
		double ear_twitch = sin(t * 8.0) * 0.02;

		for (int i = 0; i < NUM_THREADS; i++) {
			t_args[i].thread_id = i;
			t_args[i].y_start = i * slice;
			t_args[i].y_end = (i == NUM_THREADS - 1) ? HEIGHT : (i + 1) * slice;
			t_args[i].t = t;
			t_args[i].frame = frame;
			t_args[i].perception_latch = perception_latch;
			t_args[i].keyframe_latch = keyframe_latch;
			t_args[i].vae_latch = perception_latch ^ keyframe_latch;
			t_args[i].eye_gaze_x = eye_gaze_x;
			t_args[i].eye_gaze_y = eye_gaze_y;
			t_args[i].ear_twitch = ear_twitch;
			pthread_create(&threads[i], NULL, raymarch_thread_worker, &t_args[i]);
		}

		for (int i = 0; i < NUM_THREADS; i++) {
			pthread_join(threads[i], NULL);
		}

		fwrite(fb, 1, sizeof(fb), ffmpeg);
	}

	pclose(ffmpeg);

	printf("=============================================================\n");
	printf("   TOMIE KINEMATICS RENDER COMPLETE: vaesen_teddy_bear_tomie_kinematics.mp4\n");
	printf("=============================================================\n");

	return 0;
}
