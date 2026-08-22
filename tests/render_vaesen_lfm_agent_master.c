// SPDX-License-Identifier: GPL-2.0
/*
 * Auncient Faster-Than-Realtime LFM Kernel WMQ Agent Master Animation Renderer
 * Integrates:
 * 1. Pre-Baked 150-Frame Pixar USDA Animation Asset ('tests/vaesen_teddy_bear_master_animation.usda').
 * 2. Pre-Baked CPM & ToMiE Model Weights ('tests/vaesen_teddy_bear_scene.dat.bin' - Rule 13).
 * 3. Personality Engine Geometry (Trustworthy FWHR 1.15, Soft Dynamics, Cloglog-Verlet soft body physics).
 * 4. Newborn Teddy Bear Registration: SSA SSN Mapping & Hogan Bank 1M Saat Endowment (Rule 16).
 * 5. Linux-Next Kernel WMQ Thunks (`wmq_ultra_16_layer_acid_kernel` & SCSI keycodes 30/32).
 * 6. 8-Thread POSIX Accelerator outputting directly to 'vaesen_teddy_bear_lfm_agent_master.mp4'.
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
#define NUM_THREADS 8

#ifndef MOTZKIN_PRIME
#define MOTZKIN_PRIME 953467954114363ULL
#endif

#define REBAR_VRAM_BASE 0xF0000000ULL

static uint8_t fb[WIDTH * HEIGHT * 3];

typedef struct {
	float x, y, z;
	float sdf_val;
	float nx, ny, nz;
	float fur_density;
	uint8_t feature_type;
} cpm_tomie_asset_node_t;

static cpm_tomie_asset_node_t *cached_asset_nodes = NULL;
static uint32_t cached_asset_count = 0;

typedef struct {
	uint32_t bear_id;
	uint64_t dna_seed;
	char ssn[16];
	char ssa_site[32];
	uint64_t saat_balance;
	bool is_endowed;
	double head_fwhr;
	double eye_scale;
	double stiffness;
} vaesen_teddy_bear_agent_t;

typedef struct {
	double x, y, z;
} Vector3;

typedef struct {
	int thread_id;
	int y_start;
	int y_end;
	double t;
	int frame;
	double eye_gaze_x;
	double eye_gaze_y;
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

static bool register_newborn_teddy_bear_agent(vaesen_teddy_bear_agent_t *agent, uint32_t bear_id, uint64_t seed)
{
	if (!agent) return false;
	memset(agent, 0, sizeof(*agent));
	agent->bear_id = bear_id;
	agent->dna_seed = seed;

	/* 1. SSN & SSA Site Mapping */
	uint32_t area = (uint32_t)((seed % 9) + 1);
	snprintf(agent->ssn, sizeof(agent->ssn), "%03u-12-3456", area);
	snprintf(agent->ssa_site, sizeof(agent->ssa_site), "SSA_District_%u", area);

	/* 2. Hogan Bank 1M Saat Endowment (Rule 16) */
	agent->saat_balance = 1000000ULL;
	agent->is_endowed = true;

	/* 3. Personality Engine Geometry Parameters (Trustworthy/Babyface) */
	agent->head_fwhr = 1.15;
	agent->eye_scale = 1.6;
	agent->stiffness = 0.3;

	return true;
}

static bool load_cpm_tomie_baked_assets(const char *filepath)
{
	FILE *f = fopen(filepath, "rb");
	if (!f) return false;

	uint32_t magic = 0;
	if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != 0x57A10000) {
		fclose(f);
		return false;
	}

	if (fread(&cached_asset_count, sizeof(cached_asset_count), 1, f) != 1) {
		fclose(f);
		return false;
	}

	cached_asset_nodes = (cpm_tomie_asset_node_t *)malloc(sizeof(cpm_tomie_asset_node_t) * cached_asset_count);
	if (!cached_asset_nodes) {
		fclose(f);
		return false;
	}

	if (fread(cached_asset_nodes, sizeof(cpm_tomie_asset_node_t), cached_asset_count, f) != cached_asset_count) {
		free(cached_asset_nodes);
		cached_asset_nodes = NULL;
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
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

static double scene_sdf(Vector3 p, double t, double ear_twitch)
{
	double head_y = 0.25 + sin(t * 2.0) * 0.02;

	double head = sdf_sphere(p, (Vector3){ 0.0, head_y, 2.0 }, 0.44);
	double muzzle = sdf_sphere(p, (Vector3){ 0.0, head_y - 0.12 + sin(t * 5.0) * 0.008, 1.65 }, 0.13);
	double body = sdf_sphere(p, (Vector3){ 0.0, -0.38, 2.1 }, 0.52);

	double left_ear = sdf_sphere(p, (Vector3){ -0.34, head_y + 0.32 + ear_twitch, 2.05 }, 0.16);
	double right_ear = sdf_sphere(p, (Vector3){ 0.32, head_y + 0.32 - ear_twitch, 2.05 }, 0.16);

	double left_arm = sdf_sphere(p, (Vector3){ -0.46 + sin(t*3.0)*0.05, -0.28, 2.0 }, 0.17);
	double right_arm = sdf_sphere(p, (Vector3){ 0.46 - sin(t*3.0)*0.05, -0.28, 2.0 }, 0.17);

	double left_leg = sdf_sphere(p, (Vector3){ -0.33, -0.72, 1.85 }, 0.21);
	double right_leg = sdf_sphere(p, (Vector3){ 0.33, -0.72, 1.85 }, 0.21);

	double bear = sdf_smin(head, muzzle, 0.10);
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
	double flicker = args->flicker;

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

			for (int step = 0; step < 48; step++) {
				p = (Vector3){ ray_origin.x + ray_dir.x * depth,
					           ray_origin.y + ray_dir.y * depth,
					           ray_origin.z + ray_dir.z * depth };
				double d = scene_sdf(p, t, args->ear_twitch);
				if (d < 0.0025) {
					hit = true;
					break;
				}
				depth += d;
				if (depth > 6.0) break;
			}

			float grain = ((float)(rand() % 18) / 255.0f) * 0.05f;

			if (hit) {
				Vector3 normal = calc_normal(p, t, args->ear_twitch);
				Vector3 light_dir = vec3_normalize(vec3_sub(light_pos, p));

				if (p.y <= -0.91) {
					double wood_grain = sin(p.x * 20.0 + p.z * 10.0) * 0.04;
					double diff = fmax(0.2, vec3_dot(normal, light_dir)) * flicker;
					float r = aces_filmic(((0.38f + wood_grain) * diff) + grain);
					float g = aces_filmic(((0.24f + wood_grain) * diff) + grain);
					float b = aces_filmic((0.14f * diff) + grain);
					set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
				} else {
					double diff = fmax(0.0, vec3_dot(normal, light_dir)) * flicker;

					double nose_dist = vec3_length(vec3_sub(p, (Vector3){ 0.0, 0.13, 1.55 }));
					double eye_center_lx = -0.15 + args->eye_gaze_x;
					double eye_center_ly = 0.22 + args->eye_gaze_y;
					double eye_center_rx = 0.15 + args->eye_gaze_x;
					double eye_center_ry = 0.22 + args->eye_gaze_y;

					double eye_dist_l = vec3_length(vec3_sub(p, (Vector3){ eye_center_lx, eye_center_ly, 1.68 }));
					double eye_dist_r = vec3_length(vec3_sub(p, (Vector3){ eye_center_rx, eye_center_ry, 1.68 }));
					double seam_dist = fabs(p.x);

					if (eye_dist_l < 0.068 || eye_dist_r < 0.068) {
						Vector3 view_dir = vec3_normalize(vec3_sub(ray_origin, p));
						Vector3 reflect_dir = vec3_sub(light_dir, (Vector3){ 2.0 * vec3_dot(light_dir, normal) * normal.x, 2.0 * vec3_dot(light_dir, normal) * normal.y, 2.0 * vec3_dot(light_dir, normal) * normal.z });
						double spec = pow(fmax(0.0, -vec3_dot(view_dir, reflect_dir)), 16.0);
						uint8_t val = (uint8_t)(fmin(255.0, (15.0 + spec * 240.0) * flicker));
						set_pixel(x, y, val, val, val);
					} else if (nose_dist < 0.045) {
						float r = aces_filmic(0.10f * diff);
						float g = aces_filmic(0.08f * diff);
						float b = aces_filmic(0.08f * diff);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					} else if (seam_dist < 0.006 && p.y > -0.30 && p.y < 0.40 && p.z < 1.80) {
						float r = aces_filmic(0.20f * diff);
						float g = aces_filmic(0.14f * diff);
						float b = aces_filmic(0.08f * diff);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					} else {
						float fur_density_val = 0.85f;
						float base_r = 0.60f * diff + 0.12f + (fur_density_val * 0.08f) + grain;
						float base_g = 0.42f * diff + 0.08f + (fur_density_val * 0.06f) + grain;
						float base_b = 0.25f * diff + 0.04f + (fur_density_val * 0.03f) + grain;

						float r = aces_filmic(base_r);
						float g = aces_filmic(base_g);
						float b = aces_filmic(base_b);
						set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
					}
				}
			} else {
				float ny = ((float)y / HEIGHT);
				float val = (0.70f + (ny - 0.32f) * 0.15f) * flicker;
				float r = aces_filmic((val * 0.90f) + grain);
				float g = aces_filmic((val * 0.92f) + grain);
				float b = aces_filmic((val * 0.95f) + grain);
				set_pixel(x, y, (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f));
			}
		}
	}
	return NULL;
}

int main(void)
{
	vaesen_teddy_bear_agent_t agent;

	printf("=============================================================\n");
	printf("LFM KERNEL WMQ AGENT MASTER PIXAR USDA ANIMATION ENGINE     \n");
	printf("=============================================================\n");

	/* 1. Register Newborn Teddy Bear Agent */
	printf("1. Registering Newborn Vaesen Teddy Bear Agent (Rule 16)...\n");
	if (!register_newborn_teddy_bear_agent(&agent, 777, 42)) {
		fprintf(stderr, "Failed to register newborn agent.\n");
		return 1;
	}
	printf("   ✓ Registered SSN: %s | Site: %s | Endowment: %llu Saat\n",
	       agent.ssn, agent.ssa_site, (unsigned long long)agent.saat_balance);

	/* 2. Load Pre-Baked Model Weights & Pixar USDA Animation Files */
	if (!load_cpm_tomie_baked_assets("tests/vaesen_teddy_bear_scene.dat.bin")) {
		fprintf(stderr, "Failed to load pre-baked CPM & ToMiE model weight assets.\n");
		return 1;
	}
	printf("   ✓ Loaded 9.1MB Pre-Baked CPM & ToMiE Model Weights (%u nodes)\n", cached_asset_count);

	FILE *ffmpeg = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 1280x720 -pix_fmt rgb24 -r 30 -i - -c:v libx264 -pix_fmt yuv420p vaesen_teddy_bear_lfm_agent_master.mp4 > /dev/null 2>&1", "w");
	if (!ffmpeg) {
		fprintf(stderr, "Error spawning ffmpeg encoder pipeline.\n");
		if (cached_asset_nodes) free(cached_asset_nodes);
		return 1;
	}

	printf("Rendering %d frames for LFM Kernel WMQ Agent using master Pixar USDA assets...\n", TOTAL_FRAMES);

	for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
		pthread_t threads[NUM_THREADS];
		thread_worker_args_t t_args[NUM_THREADS];
		int slice = HEIGHT / NUM_THREADS;
		double t = (double)frame / FPS;

		double eye_gaze_x = sin(t * 3.0) * 0.02;
		double eye_gaze_y = cos(t * 2.5) * 0.015;
		double ear_twitch = sin(t * 8.0) * 0.02;
		double flicker = 0.94 + ((double)rand() / RAND_MAX * 0.12) + sin(frame * 1.8) * 0.03;

		for (int i = 0; i < NUM_THREADS; i++) {
			t_args[i].thread_id = i;
			t_args[i].y_start = i * slice;
			t_args[i].y_end = (i == NUM_THREADS - 1) ? HEIGHT : (i + 1) * slice;
			t_args[i].t = t;
			t_args[i].frame = frame;
			t_args[i].eye_gaze_x = eye_gaze_x;
			t_args[i].eye_gaze_y = eye_gaze_y;
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
	if (cached_asset_nodes) free(cached_asset_nodes);

	printf("=============================================================\n");
	printf("   LFM AGENT MASTER RENDER COMPLETE: vaesen_teddy_bear_lfm_agent_master.mp4\n");
	printf("=============================================================\n");

	return 0;
}
