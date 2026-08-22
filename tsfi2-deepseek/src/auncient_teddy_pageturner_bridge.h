#ifndef AUNCIENT_TEDDY_BEAR_PAGETURNER_BRIDGE_H
#define AUNCIENT_TEDDY_BEAR_PAGETURNER_BRIDGE_H

#include "auncient_teddy_bear_heart_usda.h"
#include "cpm_tomie_pageturner_renderman.h"
#include "cpm_tomie_renderman_pageturner_presenter.h"
#include <stdbool.h>

typedef struct {
    TeddyBearHeartUsdaAsset teddy_bear;
    PageTurnerRenderManEngine engine;
    RenderManFrameBuffer framebuffer;
    float live_cardiac_phase_rad;
    uint32_t bridge_rule18_checksum;
    bool bridge_verified;
} AuncientTeddyBearPageTurnerBridge;

void auncient_teddy_bear_pageturner_bridge_init(AuncientTeddyBearPageTurnerBridge *bridge, uint32_t dna_seed);
bool auncient_teddy_bear_pageturner_bridge_step(AuncientTeddyBearPageTurnerBridge *bridge, float dt_sec, float temp_c, float rh_pct, float co2_ppm, float turn_progress);
uint32_t auncient_teddy_bear_pageturner_bridge_compute_rule18(const AuncientTeddyBearPageTurnerBridge *bridge);

#endif /* AUNCIENT_TEDDY_BEAR_PAGETURNER_BRIDGE_H */
