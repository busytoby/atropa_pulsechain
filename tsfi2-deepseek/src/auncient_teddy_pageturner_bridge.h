#ifndef AUNCIENT_TEDDY_PAGETURNER_BRIDGE_H
#define AUNCIENT_TEDDY_PAGETURNER_BRIDGE_H

#include "auncient_teddy_bear_heart_usda.h"
#include "cpm_tomie_pageturner_renderman.h"
#include "cpm_tomie_renderman_pageturner_presenter.h"
#include <stdbool.h>

typedef struct {
    TeddyBearHeartUsdaAsset teddy;
    PageTurnerRenderManEngine engine;
    RenderManFrameBuffer framebuffer;
    float live_cardiac_phase_rad;
    uint32_t bridge_rule18_checksum;
    bool bridge_verified;
} AuncientTeddyPageTurnerBridge;

void auncient_teddy_pageturner_bridge_init(AuncientTeddyPageTurnerBridge *bridge, uint32_t dna_seed);
bool auncient_teddy_pageturner_bridge_step(AuncientTeddyPageTurnerBridge *bridge, float dt_sec, float temp_c, float rh_pct, float co2_ppm, float turn_progress);
uint32_t auncient_teddy_pageturner_bridge_compute_rule18(const AuncientTeddyPageTurnerBridge *bridge);

#endif /* AUNCIENT_TEDDY_PAGETURNER_BRIDGE_H */
