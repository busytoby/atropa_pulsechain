#ifndef TSFI_RIINTERFACE_H
#define TSFI_RIINTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "tsfi_displacementshader.h"
#include "tsfi_depthoffield.h"

typedef struct {
    uint16_t hudson_vce_color_reg[16];
    int active_sprite_id;
    double clip_min_x, clip_min_y;
    double clip_max_x, clip_max_y;
    bool is_world_active;
    uint16_t psg_channel_freq[6]; // huC6280 PSG voice channels
    uint8_t psg_channel_vol[6];
    uint8_t psg_channel_pan[6]; // huC6280 PSG panning control
    uint8_t psg_channel_wavetable[6][32]; // huC6280 PSG 5-bit custom wavetable
    uint8_t psg_lfo_ctrl; // huC6280 PSG LFO control
    uint8_t psg_lfo_freq; // huC6280 PSG LFO frequency
    uint8_t psg_lfo_phase; // huC6280 PSG LFO phase displacement
    uint8_t psg_noise_ctrl[2]; // huC6280 PSG noise control for channels 5-6
    uint8_t psg_channel_dda[6]; // huC6280 PSG Direct D/A data registers
    uint8_t psg_master_volume_l; // huC6280 PSG Master Left volume
    uint8_t psg_master_volume_r; // huC6280 PSG Master Right volume
    uint16_t vdc_scroll_x; // huC6270 VDC background scroll X
    uint16_t vdc_scroll_y; // huC6270 VDC background scroll Y
    uint16_t vdc_raster_interrupt_line; // huC6270 VDC scanline target for H-Blank interrupt
    bool vdc_collision_flag; // huC6270 VDC sprite collision flag
    bool hblank_active; // H-Blank state indicator
    uint8_t frame_buffer[256 * 256]; // Simulated VDC screen memory (256x256)
    uint32_t irq_counter; // hardware interrupt counter
    bool irq_active;
    
    // Joint Co-Design pipeline sub-contexts
    TSFiDisplacementShader shader;
    TSFiDepthOfField dof;
    
    // Multiframe temporal-spatial accumulator
    uint8_t multiframe_buffer[4][256 * 256];
    int current_frame_idx;
} TSFiRiInterface;


// Initialize the RiInterface context
void tsfi_riinterface_init(TSFiRiInterface *ri);

// Accumulate current frame into multiframe buffer
void tsfi_riinterface_accumulate_frame(TSFiRiInterface *ri);

// RiWorldBegin callback: cycles/resets the mirrored Hudson VCE color palette registers
void tsfi_riinterface_world_begin(TSFiRiInterface *ri);

// RiWorldEnd callback
void tsfi_riinterface_world_end(TSFiRiInterface *ri);

// RiSphere callback: maps 3D sphere coordinate boundaries to the active Hudson VDC sprite layer
bool tsfi_riinterface_sphere(TSFiRiInterface *ri, int sprite_id, double radius);

// Emulates Tom Hudson's clipLine boundary checks for coordinate vectors
bool tsfi_riinterface_clip_check(const TSFiRiInterface *ri, double x, double y);

// Dynamic sound modulation based on RenderMan camera panning speed
void tsfi_riinterface_modulate_psg(TSFiRiInterface *ri, double velocity);

// Hardware-level VDC DMA block transfer simulation
void tsfi_riinterface_vdc_dma_copy(TSFiRiInterface *ri, uint16_t src_idx, uint16_t dest_idx, uint16_t length);

// Simulates the discharge cycles of field-effect transistors (FETs) using Verlet integration
void tsfi_riinterface_discharge_verlet(TSFiRiInterface *ri, double *pos_x, double *prev_pos_x, int count, double dt, double decay);

// Ticks the hardware timer to trigger periodic interrupts (IRQs)
void tsfi_riinterface_tick_irq(TSFiRiInterface *ri, double dt);

// Executes the complete 8-step RenderMan sequence loop mapped to Hudson hardware
void tsfi_riinterface_run_8step_loop(TSFiRiInterface *ri, double camera_velocity, double *pos_x, double *prev_pos_x, int count);

// Writes custom 5-bit wavetable arrays to the active PSG channel buffer
void tsfi_riinterface_write_wavetable(TSFiRiInterface *ri, int channel, const uint8_t *wave_data);

// Enables low-frequency oscillator (LFO) registers for channels 1 and 2
void tsfi_riinterface_enable_lfo(TSFiRiInterface *ri, uint8_t ctrl, uint8_t freq);

// Sets the phase displacement parameters for the PSG LFO modulation
void tsfi_riinterface_set_lfo_phase(TSFiRiInterface *ri, uint8_t phase);

// Enables white noise generation on channels 5 and 6
void tsfi_riinterface_enable_noise(TSFiRiInterface *ri, int channel, bool enable, uint8_t frequency);

// Updates background scroll position registers to map camera coordinates
void tsfi_riinterface_scroll_background(TSFiRiInterface *ri, uint16_t x, uint16_t y);

// Writes direct digital-to-analog sound samples to specified audio channels
void tsfi_riinterface_write_dda(TSFiRiInterface *ri, int channel, uint8_t sample);

// Configures the global stereo master volume registers of the PSG mixer
void tsfi_riinterface_set_master_volume(TSFiRiInterface *ri, uint8_t volume_l, uint8_t volume_r);

// Emulates VDC hardware collision flags updates based on coordinate distance checks
void tsfi_riinterface_check_sprite_collision(TSFiRiInterface *ri, double s1_x, double s1_y, double s2_x, double s2_y, double threshold);

// Sets the scanline target parameter to trigger horizontal blanking (H-Blank) interrupts
void tsfi_riinterface_set_raster_line(TSFiRiInterface *ri, uint16_t line);

// Synchronizes the displacement shader amplitude with keyboard keycodes from the WinchesterMQ REU RAM
void tsfi_riinterface_sync_winchester(TSFiRiInterface *ri, const void *vm_state);

// Executes the unified co-design RenderMan loop driven by WinchesterMQ and Yul thunks
void tsfi_riinterface_run_co_design_loop(TSFiRiInterface *ri, void *vm_state, double *pos_x, double *prev_pos_x, int count);

// Resolves PMG sprite collision coordinates using EDoF deconvolution within the CICS transaction frame
void tsfi_riinterface_resolve_pmg_cics_collision(TSFiRiInterface *ri, uint32_t player_id, int *health);

// Adjusts the Hudson focal actuator parameters based on co-design RMSE metrics
void tsfi_riinterface_adjust_hudson_focus(TSFiRiInterface *ri, const double *ideal_image, double target_rmse_threshold);

#endif // TSFI_RIINTERFACE_H



