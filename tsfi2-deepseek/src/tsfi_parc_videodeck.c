#include <stdlib.h>
#include <string.h>
#include "tsfi_parc_videodeck.h"

#define ALTO_FRAME_WORDS (38 * 808)

int tsfi_parc_deck_init(tsfi_parc_video_deck_t *deck, int total_frames, int total_tracks) {
    if (!deck || total_frames <= 0 || total_tracks <= 0) return -1;

    deck->front_buffer = calloc(ALTO_FRAME_WORDS, sizeof(uint16_t));
    deck->back_buffer = calloc(ALTO_FRAME_WORDS, sizeof(uint16_t));
    if (!deck->front_buffer || !deck->back_buffer) {
        free(deck->front_buffer);
        free(deck->back_buffer);
        return -2;
    }

    deck->play_state = DECK_STATE_STOPPED;
    deck->current_frame = 0;
    deck->total_frames = total_frames;
    deck->audio_clock = 0.0f;
    deck->video_clock = 0.0f;
    deck->active_track_idx = 0;
    deck->total_tracks = total_tracks;

    return 0;
}

void tsfi_parc_deck_free(tsfi_parc_video_deck_t *deck) {
    if (!deck) return;
    free(deck->front_buffer);
    free(deck->back_buffer);
    deck->front_buffer = NULL;
    deck->back_buffer = NULL;
}

int tsfi_parc_deck_step(tsfi_parc_video_deck_t *deck, float delta_time, float external_audio_time) {
    if (!deck) return -1;

    if (deck->play_state != DECK_STATE_PLAYING) return 0;

    // Advance clocks
    deck->video_clock += delta_time;
    deck->audio_clock = external_audio_time;

    // Enforce AV sync threshold (clamping drift to 0.05 seconds)
    float drift = deck->video_clock - deck->audio_clock;
    if (drift > 0.05f) {
        // Video is running too fast, stall video frames
        return 1; // stalled
    } else if (drift < -0.05f) {
        // Video is running too slow, catch up by skipping
        deck->video_clock = deck->audio_clock;
    }

    // Determine current frame based on 30 FPS timing
    int target_frame = (int)(deck->video_clock * 30.0f);
    if (target_frame >= deck->total_frames) {
        deck->current_frame = 0;
        deck->video_clock = 0.0f;
    } else {
        deck->current_frame = target_frame;
    }

    return 0;
}

int tsfi_parc_deck_flip(tsfi_parc_video_deck_t *deck) {
    if (!deck) return -1;
    // Swap front and back display memory buffer pointers
    uint16_t *temp = deck->front_buffer;
    deck->front_buffer = deck->back_buffer;
    deck->back_buffer = temp;
    return 0;
}

int tsfi_parc_deck_control(tsfi_parc_video_deck_t *deck, int command, int arg) {
    if (!deck) return -1;

    switch (command) {
        case DECK_CMD_PLAY:
            deck->play_state = DECK_STATE_PLAYING;
            break;
        case DECK_CMD_PAUSE:
            deck->play_state = DECK_STATE_PAUSED;
            break;
        case DECK_CMD_STOP:
            deck->play_state = DECK_STATE_STOPPED;
            deck->current_frame = 0;
            deck->video_clock = 0.0f;
            break;
        case DECK_CMD_SEEK:
            if (arg >= 0 && arg < deck->total_frames) {
                deck->current_frame = arg;
                deck->video_clock = (float)arg / 30.0f;
            }
            break;
        case DECK_CMD_TRACK:
            if (arg >= 0 && arg < deck->total_tracks) {
                deck->active_track_idx = arg;
                deck->current_frame = 0;
                deck->video_clock = 0.0f;
            }
            break;
        default:
            return -2; // unknown command
    }
    return 0;
}
