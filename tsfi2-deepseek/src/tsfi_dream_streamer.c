#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_dream_streamer.h"
#include "tsfi_c_math.h"
#include "lau_registry.h"

void tsfi_dream_streamer_init(TsfiDreamStreamerSystem *system) {
    if (!system) return;
    pthread_mutex_init(&system->manifold_lock, NULL);
    system->is_active = false;
    system->sov_melanin = 0.5f;
    system->sov_roughness = 0.4f;
    system->sov_irid = 0.2f;
}

void* tsfi_dream_streamer_loop(void *system_ptr) {
    TsfiDreamStreamerSystem *sys = (TsfiDreamStreamerSystem*)system_ptr;
    LauTelemetryState *telem = lau_telemetry_get_state();
    
    printf("[STREAMER] Deep Moon Dream Matrix Active.\n");
    int frame_count = 0;
    
    while (sys->is_active) {
        pthread_mutex_lock(&sys->manifold_lock);
        
        // 1. DEEP: Stream Geometric Mutations
        if (sys->deep_module && sys->deep_module->stream_mutations) {
            sys->deep_module->stream_mutations(sys->live_dag_flower, sys->deep_module->deep_data);
            sys->deep_module->stream_mutations(sys->live_dag_bear, sys->deep_module->deep_data);
        }
        
        // 2. CORE: Path-Trace the Current Manifold State
        // (W=512, H=512, Depth disabled for this background telem loop)
        float t_sim = (float)(frame_count % 600) / 599.0f;
        tsfi_svdag_path_trace(sys->live_frame_buffer, NULL, sys->live_dag_flower, sys->live_dag_bear, 
                              512, 512, t_sim, sys->sov_melanin, sys->sov_roughness, sys->sov_irid);
        
        // 3. MOON: Verify and Adjust Physical Parameters
        if (sys->moon_module && sys->moon_module->verify_output) {
            sys->moon_module->verify_output(sys->live_frame_buffer, 512, 512, 
                                            &sys->sov_melanin, &sys->sov_roughness, &sys->sov_irid);
        }

        // 4. TELEMETRY: Emit Forensic Metadata
        if (telem) {
            telem->render_telemetry.blue_secret_jitter = (float)(rand() % 100) / 1000.0f;
            telem->render_telemetry.magnetic_yi_rigidity = 0.95f + ((float)(rand() % 50) / 1000.0f);
            telem->render_telemetry.petal_irid_shift = sys->sov_irid;
            telem->render_telemetry.current_stage = (uint32_t)((frame_count % 600) / 85);
            telem->render_telemetry.path_trace_m_evals = 5500.0f;
        }
        
        pthread_mutex_unlock(&sys->manifold_lock);
        
        frame_count++;
        usleep(33000); 
    }
    
    printf("[STREAMER] Deep Moon Dream Matrix Offline.\n");
    return NULL;
}

void tsfi_dream_streamer_start(TsfiDreamStreamerSystem *system) {
    if (!system || system->is_active) return;
    system->is_active = true;
    pthread_create(&system->streamer_thread, NULL, tsfi_dream_streamer_loop, system);
}

void tsfi_dream_streamer_stop(TsfiDreamStreamerSystem *system) {
    if (!system || !system->is_active) return;
    system->is_active = false;
    pthread_join(system->streamer_thread, NULL);
    pthread_mutex_destroy(&system->manifold_lock);
}
