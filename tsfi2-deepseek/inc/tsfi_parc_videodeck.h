#ifndef TSFI_PARC_VIDEODECK_H
#define TSFI_PARC_VIDEODECK_H

#include <stdint.h>

#define DECK_STATE_STOPPED 0
#define DECK_STATE_PLAYING 1
#define DECK_STATE_PAUSED  2

#define DECK_CMD_PLAY    1
#define DECK_CMD_PAUSE   2
#define DECK_CMD_STOP    3
#define DECK_CMD_SEEK    4
#define DECK_CMD_TRACK   5

typedef struct {
    uint16_t *front_buffer; // 38 * 808 words
    uint16_t *back_buffer;  // 38 * 808 words
    int play_state;
    int current_frame;
    int total_frames;
    float audio_clock;
    float video_clock;
    int active_track_idx;
    int total_tracks;
} tsfi_parc_video_deck_t;

/* Initialize video deck, buffer descriptors, and playlist tracks */
int tsfi_parc_deck_init(tsfi_parc_video_deck_t *deck, int total_frames, int total_tracks);

/* Free allocated deck display buffer resources */
void tsfi_parc_deck_free(tsfi_parc_video_deck_t *deck);

/* Step playback clocks, enforce AV sync thresholds, and advance active frames */
int tsfi_parc_deck_step(tsfi_parc_video_deck_t *deck, float delta_time, float external_audio_time);

/* Trigger VBlank hardware-level page flip swapping front/back buffers */
int tsfi_parc_deck_flip(tsfi_parc_video_deck_t *deck);

/* Dispatch user commands (Play, Pause, Stop, Seek, Switch Track) */
int tsfi_parc_deck_control(tsfi_parc_video_deck_t *deck, int command, int arg);

#endif // TSFI_PARC_VIDEODECK_H
