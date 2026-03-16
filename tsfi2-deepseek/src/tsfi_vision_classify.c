#include "tsfi_vision_internal.h"

static LauClassificationAsset g_classifiers[32];
static int g_classifier_count = 0;
static bool g_classifiers_loaded = false;

static void tsfi_vision_init_classifiers(void) {
    if (g_classifiers_loaded) return;
    DIR *d = opendir("assets/classifiers");
    if (!d) {
        LOG("[VISION] Warning: Could not open assets/classifiers\n");
        g_classifiers_loaded = true;
        return;
    }
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".class")) {
            char name[256];
            snprintf(name, sizeof(name), "%s", dir->d_name);
            char *ext = strstr(name, ".class");
            if (ext) *ext = '\0';
            if (g_classifier_count < 32) {
                if (tsfi_classification_load(name, &g_classifiers[g_classifier_count])) {
                    LOG("[VISION] Loaded classifier: %s (Class %d)\n", name, g_classifiers[g_classifier_count].target_class);
                    g_classifier_count++;
                }
            }
        }
    }
    closedir(d);
    g_classifiers_loaded = true;
}
void tsfi_vision_manifest_register(TSFiFloralManifest *m, const TSFiFlowerPhenotype *p) {
    if (!m || !p || m->count >= 16) return;
    for (int i = 0; i < m->count; i++) if (strcmp(m->entries[i].phenotype.name, p->name) == 0) return;
    m->entries[m->count].phenotype = *p; m->entries[m->count].pixels = NULL; m->count++;
}

void tsfi_vision_manifest_update(TSFiFloralManifest *m, const char *name, const TSFiResonanceAnalysis *a, const uint32_t *pixels, int w, int h) {
    if (!m || !name || !a) return;
    for (int i = 0; i < m->count; i++) {
        if (strcmp(m->entries[i].phenotype.name, name) == 0) {
            m->entries[i].last_analysis = *a;
            if (pixels) {
                if (m->entries[i].width != w || m->entries[i].height != h) {
                    if (m->entries[i].pixels) lau_free(m->entries[i].pixels);
                    m->entries[i].pixels = (uint32_t*)lau_malloc(w * h * sizeof(uint32_t));
                }
                memcpy(m->entries[i].pixels, pixels, w * h * sizeof(uint32_t));
                m->entries[i].width = w; m->entries[i].height = h;
            }
            return;
        }
    }
}

TSFiFlowerEntry* tsfi_vision_manifest_get(TSFiFloralManifest *m, const char *name) {
    if (!m || !name) return NULL;
    for (int i = 0; i < m->count; i++) if (strcmp(m->entries[i].phenotype.name, name) == 0) return &m->entries[i];
    return NULL;
}
bool tsfi_classification_load(const char *name, LauClassificationAsset *out) {
    char path[256]; snprintf(path, 255, "assets/classifiers/%s.class", name);
    FILE *f = fopen(path, "r"); if (!f) return false;
    memset(out, 0, sizeof(LauClassificationAsset));
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *saveptr;
        char *key = strtok_r(line, "=", &saveptr);
        char *val = strtok_r(NULL, "\n", &saveptr);
        if (!key || !val) continue;
        if (strcmp(key, "name") == 0) strncpy(out->name, val, 31);
        else if (strcmp(key, "target_class") == 0) out->target_class = (TSFiResonanceClass)atoi(val);
        else if (strcmp(key, "weight_radial") == 0) out->weight_radial = (float)atof(val);
        else if (strcmp(key, "weight_symmetry") == 0) out->weight_symmetry = (float)atof(val);
        else if (strcmp(key, "weight_holes") == 0) out->weight_holes = (float)atof(val);
        else if (strcmp(key, "weight_complexity") == 0) out->weight_complexity = (float)atof(val);
        else if (strcmp(key, "threshold_radial") == 0) out->threshold_radial = (float)atof(val);
        else if (strcmp(key, "threshold_symmetry") == 0) out->threshold_symmetry = (float)atof(val);
        else if (strcmp(key, "threshold_top_bot") == 0) out->threshold_top_bot = (float)atof(val);
        else if (strcmp(key, "required_holes") == 0) out->required_holes = atoi(val);
        else if (strcmp(key, "ideal_radial_profile") == 0) {
            sscanf(val, "%f,%f,%f,%f,%f,%f,%f,%f", 
                   &out->ideal_radial_profile[0], &out->ideal_radial_profile[1],
                   &out->ideal_radial_profile[2], &out->ideal_radial_profile[3],
                   &out->ideal_radial_profile[4], &out->ideal_radial_profile[5],
                   &out->ideal_radial_profile[6], &out->ideal_radial_profile[7]);
        }
    }
    fclose(f); return true;
}

TSFiClassification tsfi_vision_classify_asset(const TSFiResonanceAnalysis *a, const LauClassificationAsset *c) {
    TSFiClassification res = { .class_id = TSFI_CLASS_UNKNOWN, .confidence = 0.0f, .raw_score = 0.0f };
    float score = 0.0f;
    
    // High-fidelity profile matching
    float profile_mse = 0;
    for (int i=0; i<8; i++) {
        float d = a->captured_radial_profile[i] - c->ideal_radial_profile[i];
        profile_mse += d*d;
    }
    float profile_score = 1.0f - tsfi_sqrtf(profile_mse / 8.0f);
    if (profile_score < 0) profile_score = 0;
    
    score += profile_score * c->weight_radial;
    score += a->glyph_symmetry * c->weight_symmetry;
    score += a->complexity * c->weight_complexity;
    
    if (c->required_holes >= 0) {
        if ((int)a->topo_hole_count == c->required_holes) {
            score += c->weight_holes;
        } else {
            score *= 0.1f; // Heavy penalty for topological mismatch
        }
    }

    // Threshold Checks (Mandatory for specific classes)
    if (a->radial_score < c->threshold_radial) score *= 0.5f;
    if (a->glyph_symmetry < c->threshold_symmetry) score *= 0.5f;
    
    // Top/Bot ratio check (if specified)
    if (c->threshold_top_bot > 0) {
        if (a->specular_contrast > c->threshold_top_bot) score *= 0.2f; // 'A' should be bottom-heavy
    }

    res.raw_score = score;
    res.confidence = tsfi_fminf(1.0f, score / 4.0f);
    if (res.confidence > 0.5f) res.class_id = c->target_class;
    
    return res;
}

TSFiClassification tsfi_vision_classify(const TSFiResonanceAnalysis *a) {
    tsfi_vision_init_classifiers();
    TSFiClassification best = { .class_id = TSFI_CLASS_UNKNOWN, .confidence = 0.0f, .raw_score = -1.0f };

    for (int i = 0; i < g_classifier_count; i++) {
        TSFiClassification c = tsfi_vision_classify_asset(a, &g_classifiers[i]);
        
        // Selection Logic:
        // 1. Prefer higher confidence.
        // 2. If confidence matches, prefer non-FLORAL (ID 1) over FLORAL.
        // 3. Otherwise prefer higher class ID (more specific).
        bool better = false;
        if (c.confidence > best.confidence) better = true;
        else if (c.confidence == best.confidence && best.confidence > 0) {
            if (best.class_id == TSFI_CLASS_FLORAL && c.class_id != TSFI_CLASS_FLORAL) better = true;
            else if (c.class_id != TSFI_CLASS_FLORAL && c.class_id > best.class_id) better = true;
        }

        if (better) {
            best = c;
        }
    }

    return best;
}

int tsfi_vision_classify_thunk(void *obj) {
    if (!obj) return TSFI_CLASS_UNKNOWN;
    
    // 1. Check for explicit analysis tag: 'VISA' 
    uint32_t magic = *(uint32_t*)obj;
    if (magic == 0x41534956) { // 'VISA'
        TSFiResonanceAnalysis *a = (TSFiResonanceAnalysis*)((uint32_t*)obj + 1);
        TSFiClassification c = tsfi_vision_classify(a);
        return (int)c.class_id;
    }
    
    // 2. Check for K0RnStream (Genome)
    // A K0RnStream is a mapped struct, it doesn't have a VISA tag at secret 0.
    // We can identify it via its structural signature or header metadata.
    LauFooter *f = (LauFooter*)((char*)obj - sizeof(LauFooter));
    if (f->type == LAU_TYPE_WIRED) {
        K0RnStream *s = (K0RnStream*)obj;
        // Identify the Original Teddy Bear K0Rn via structural signature
        if (s->op_count == 8 && s->ops[0].type == K0RN_OP_CAPSULE && 
            s->ops[1].type == K0RN_OP_SPHERE && s->ops[1].p1 == 0.2f) {
            return TSFI_CLASS_TEDDY;
        }
        // Generic fallback: If it has ops, it's a GLYPH phenotype source
        if (s->op_count > 0) return TSFI_CLASS_GLYPH;
    }

    // 3. Fallback: Check if it looks like a TSFiFlowerEntry
    TSFiFlowerEntry *e = (TSFiFlowerEntry*)obj;
    if (e->phenotype.name[0] != '\0' && e->phenotype.name[31] == '\0') {
        TSFiClassification c = tsfi_vision_classify(&e->last_analysis);
        return (int)c.class_id;
    }

    return TSFI_CLASS_UNKNOWN;
}
