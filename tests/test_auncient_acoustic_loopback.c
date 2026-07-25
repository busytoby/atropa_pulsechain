#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef struct {
    double frequency;
    double amplitude;
} audio_sample_t;

typedef struct {
    bool scsi_locked;
    uint32_t last_executed_op;
    double feedback_frequency; // Sound feedback returned to agent
} os_receiver_t;

// Process incoming audio sample through the OS Acoustic Decoder
void process_os_acoustic_receiver(os_receiver_t *os, const audio_sample_t *sample) {
    // 1. Wheeler Jump Detector (detects attack transient spike)
    if (sample->amplitude > 4.5) {
        printf("   [OS RECEIVER] Wheeler Jump detected! Latching command bus.\n");
    }

    // 2. Formant Command Decoder (maps frequency bands to ABI operations)
    if (sample->amplitude > 0.5) {
        if (sample->frequency >= 400.0 && sample->frequency <= 500.0) {
            // Lock SCSI command band (e.g. 440Hz)
            os->scsi_locked = true;
            os->last_executed_op = 0x10;
            os->feedback_frequency = 523.25; // C5 Success Tone
            printf("   [OS EXECUTION] Frequency %.1fHz decoded to Lock SCSI. Feedback: %.1fHz\n",
                   sample->frequency, os->feedback_frequency);
        } else if (sample->frequency >= 800.0 && sample->frequency <= 900.0) {
            // Unlock SCSI command band (e.g. 880Hz)
            os->scsi_locked = false;
            os->last_executed_op = 0x11;
            os->feedback_frequency = 523.25; // C5 Success Tone
            printf("   [OS EXECUTION] Frequency %.1fHz decoded to Unlock SCSI. Feedback: %.1fHz\n",
                   sample->frequency, os->feedback_frequency);
        } else {
            // Unrecognized band
            os->feedback_frequency = 110.0; // A2 Failure Drone
            printf("   [OS EXECUTION] Unrecognized frequency %.1fHz. Feedback: %.1fHz\n",
                   sample->frequency, os->feedback_frequency);
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ACOUSTIC COMMAND LOOPBACK SUITE\n");
    printf("=============================================================\n");

    os_receiver_t os = { .scsi_locked = false, .last_executed_op = 0, .feedback_frequency = 0.0 };

    // 1. Simulate Agent producing a "Lock SCSI" word
    // Word starts with a Wheeler Jump (high amplitude transient spike)
    printf("[TEST] Agent sending Wheeler Jump attack transient...\n");
    audio_sample_t jump_sample = { .frequency = 440.0, .amplitude = 5.0 };
    process_os_acoustic_receiver(&os, &jump_sample);

    // Followed by body frequency modulating target command (440Hz formant carrier)
    printf("[TEST] Agent sending body formant command wave...\n");
    audio_sample_t body_sample = { .frequency = 440.0, .amplitude = 1.0 };
    process_os_acoustic_receiver(&os, &body_sample);

    assert(os.scsi_locked == true);
    assert(os.last_executed_op == 0x10);
    assert(os.feedback_frequency == 523.25); // Success tone
    printf("   ✓ OS successfully executed command and generated success feedback.\n");

    // 2. Simulate Agent producing an unrecognized word (triggering alarm)
    printf("[TEST] Agent sending invalid formant command wave...\n");
    audio_sample_t invalid_sample = { .frequency = 1200.0, .amplitude = 1.0 };
    process_os_acoustic_receiver(&os, &invalid_sample);
    
    assert(os.feedback_frequency == 110.0); // Failure drone
    printf("   ✓ OS successfully rejected command and generated failure drone.\n");

    printf("=============================================================\n");
    printf("ACOUSTIC COMMAND LOOPBACK TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
