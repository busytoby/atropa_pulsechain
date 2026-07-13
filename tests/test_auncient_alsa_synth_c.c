#include "../firmware/tsfi_lib/tsfi_alsa_synth.c"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

int main(void) {
    printf("=============================================================\n");
    printf("Auncient ALSA Sound Synthesis Modulator C Verification Suite\n");
    printf("=============================================================\n");

    ALSASynth synth;
    
    printf("1. Initializing ALSA Sound Synthesizer at 440.0 Hz (Note A4)...\n");
    bool init_ok = alsa_synth_init(&synth, 440.0);
    assert(init_ok == true);
    printf("   ✓ Sound thread launched successfully.\n");

    // Sleep to simulate playing tone
    usleep(500000); // Play A4 for 0.5s

    printf("2. Shifting frequency to 880.0 Hz (Note A5)...\n");
    alsa_synth_set_frequency(&synth, 880.0);
    assert(synth.current_frequency == 880.0);
    printf("   ✓ Frequency shifted in runtime.\n");

    usleep(500000); // Play A5 for 0.5s

    printf("3. Shutting down sound thread...\n");
    alsa_synth_stop(&synth);
    printf("   ✓ Sound engine stopped cleanly.\n");

    printf("\n=============================================================\n");
    printf("AUNCIENT ALSA SOUND SYNTHESIS TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
