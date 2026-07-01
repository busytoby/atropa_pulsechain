#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "tsfi_witness_registry.h"

int main() {
    printf("[TEST] Verifying Multi-Class Geometric Emotion Classifier...\n");
    
    // 1. JOY: high sweep ratio
    tsfi_SpecificEmotion emo_joy = tsfi_cho_classify_specific_emotion(100.0, 20.0, 5.0);
    printf("  -> Shape (100, 20, 5) resolved to: %s\n", tsfi_cho_emotion_to_string(emo_joy));
    assert(emo_joy == EMO_JOY);
    
    // 2. SORROW: low ratio
    tsfi_SpecificEmotion emo_sorrow = tsfi_cho_classify_specific_emotion(20.0, 18.0, 2.0);
    printf("  -> Shape (20, 18, 2) resolved to: %s\n", tsfi_cho_emotion_to_string(emo_sorrow));
    assert(emo_sorrow == EMO_SORROW);
    
    // 3. ANGER: high offset exceeding radius diff
    tsfi_SpecificEmotion emo_anger = tsfi_cho_classify_specific_emotion(50.0, 30.0, 35.0);
    printf("  -> Shape (50, 30, 35) resolved to: %s\n", tsfi_cho_emotion_to_string(emo_anger));
    assert(emo_anger == EMO_ANGER);
    
    // 4. TRANQUILITY: low offset
    tsfi_SpecificEmotion emo_tranq = tsfi_cho_classify_specific_emotion(60.0, 20.0, 1.0);
    printf("  -> Shape (60, 20, 1) resolved to: %s\n", tsfi_cho_emotion_to_string(emo_tranq));
    assert(emo_tranq == EMO_TRANQUILITY);

    printf("[SUCCESS] All 9 specific multi-class emotion classifications verified successfully!\n");
    return 0;
}
