#include "tsfi_witness_registry.h"
#include <string.h>
#include <math.h>

void tsfi_cho_registry_init(tsfi_ChoRegistry* registry) {
    if (!registry) return;
    memset(registry, 0, sizeof(tsfi_ChoRegistry));
    registry->Count = 0;
}

bool tsfi_cho_swear_in(tsfi_ChoRegistry* registry, const char* target_address, uint64_t target_wavefunc, uint64_t exp, uint64_t mod) {
    if (!registry || !target_address || strlen(target_address) != 42) {
        return false;
    }
    if (registry->Count >= MAX_DELEGATES) {
        return false;
    }

    for (uint32_t i = 0; i < registry->Count; i++) {
        if (strcmp(registry->Delegates[i].Address, target_address) == 0) {
            return false;
        }
    }

    uint64_t hash_val = 5381;
    for (int i = 0; i < 42; i++) {
        hash_val = ((hash_val << 5) + hash_val) + target_address[i];
    }
    uint64_t soul = hash_val ^ 0xFEEDfaceULL;

    tsfi_DelegateRecord* delegate = &registry->Delegates[registry->Count];
    memset(delegate, 0, sizeof(tsfi_DelegateRecord));
    strncpy(delegate->Address, target_address, 42);
    delegate->Address[42] = '\0';
    delegate->Soul = soul;
    delegate->VerifiedWavefunc = target_wavefunc;

    if (target_wavefunc > 0) {
        delegate->Competent = true;
    } else {
        delegate->Competent = false;
    }

    delegate->R_hyp = (double)(soul % 97) + 20.0;
    delegate->r_hyp = (double)(soul % 53) + 10.0;
    delegate->d_hyp = (double)(soul % 41) + 5.0;

    delegate->LocalExp = exp;
    delegate->LocalMod = mod;
    delegate->ClassifiedState = STATE_NORMATIVE;
    delegate->SelfDbCount = 0;
    delegate->HistoryCount = 0;
    delegate->BlowUpFactor = 0.0;
    delegate->ConstraintEigenvalue = 1.0;
    
    // Default opinions start neutral at 0.5
    delegate->SelfOpinion = 0.5;
    for (int i = 0; i < MAX_DELEGATES; i++) {
        delegate->PeerOpinions[i] = 0.5;
    }

    registry->Count++;
    return true;
}

bool tsfi_cho_learn_rules(tsfi_DelegateRecord* local_bear, const tsfi_DelegateRecord* foreign_bear) {
    if (!local_bear || !foreign_bear) return false;

    for (int i = 0; i < MAX_RULE_CACHE; i++) {
        if (local_bear->LearnedRules[i].Known && strcmp(local_bear->LearnedRules[i].ForeignAddress, foreign_bear->Address) == 0) {
            return true;
        }
    }

    for (int i = 0; i < MAX_RULE_CACHE; i++) {
        if (!local_bear->LearnedRules[i].Known) {
            strncpy(local_bear->LearnedRules[i].ForeignAddress, foreign_bear->Address, 42);
            local_bear->LearnedRules[i].ForeignAddress[42] = '\0';
            local_bear->LearnedRules[i].Exp = foreign_bear->LocalExp;
            local_bear->LearnedRules[i].Mod = foreign_bear->LocalMod;
            local_bear->LearnedRules[i].Known = true;
            return true;
        }
    }

    return false;
}

bool tsfi_cho_witness_overlap(const tsfi_DelegateRecord* a, const tsfi_DelegateRecord* b) {
    if (!a || !b) return false;

    bool rules_known = false;
    for (int i = 0; i < MAX_RULE_CACHE; i++) {
        if (a->LearnedRules[i].Known && strcmp(a->LearnedRules[i].ForeignAddress, b->Address) == 0) {
            rules_known = true;
            break;
        }
    }

    if (!rules_known) {
        return false;
    }

    double max_a = fabs(a->R_hyp - a->r_hyp) + a->d_hyp;
    double min_a = fabs(fabs(a->R_hyp - a->r_hyp) - a->d_hyp);

    double max_b = fabs(b->R_hyp - b->r_hyp) + b->d_hyp;
    double min_b = fabs(fabs(b->R_hyp - b->r_hyp) - b->d_hyp);

    if (max_a < min_b || max_b < min_a) {
        return false;
    }
    return true;
}

tsfi_BearEmotionState tsfi_cho_classify_emotion(const tsfi_DelegateRecord* bear, double dynamic_d_l, double dynamic_d_r) {
    if (!bear) return STATE_UNKNOWN;

    double delta_l = fabs(dynamic_d_l - bear->d_hyp);
    double delta_r = fabs(dynamic_d_r - bear->d_hyp);
    double max_delta = delta_l > delta_r ? delta_l : delta_r;

    if (dynamic_d_l > 1.6 * bear->d_hyp || dynamic_d_l < 0.3 * bear->d_hyp) {
        return STATE_STRESSED;
    }
    
    if (max_delta > 5.0) {
        return STATE_EXCITED;
    }

    return STATE_NORMATIVE;
}

void tsfi_cho_log_self_observation(tsfi_DelegateRecord* bear, double R, double r, double d, tsfi_AestheticClass classification) {
    if (!bear || bear->SelfDbCount >= MAX_SELF_OBSERVATIONS) return;

    tsfi_SelfObservationRecord* rec = &bear->SelfDatabase[bear->SelfDbCount];
    rec->R_hyp = R;
    rec->r_hyp = r;
    rec->d_hyp = d;
    rec->Classification = classification;

    bear->SelfDbCount++;
}

tsfi_AestheticClass tsfi_cho_classify_foreign_aesthetic(const tsfi_DelegateRecord* local_bear, const tsfi_DelegateRecord* foreign_bear) {
    if (!local_bear || !foreign_bear) return AESTHETIC_NEUTRAL;

    double foreign_ratio = foreign_bear->R_hyp / (foreign_bear->r_hyp > 0.0 ? foreign_bear->r_hyp : 1.0);

    for (uint32_t i = 0; i < local_bear->SelfDbCount; i++) {
        const tsfi_SelfObservationRecord* rec = &local_bear->SelfDatabase[i];
        double local_ratio = rec->R_hyp / (rec->r_hyp > 0.0 ? rec->r_hyp : 1.0);
        
        if (fabs(local_ratio - foreign_ratio) < 0.1 && fabs(rec->d_hyp - foreign_bear->d_hyp) < 2.0) {
            return rec->Classification;
        }
    }

    double ratio_fract = foreign_ratio - floor(foreign_ratio);
    if (ratio_fract < 0.05 || ratio_fract > 0.95) {
        return AESTHETIC_HARMONIOUS;
    }
    return AESTHETIC_DISHARMONIOUS;
}

void tsfi_cho_record_attestation(tsfi_DelegateRecord* target_bear, tsfi_BearEmotionState state) {
    if (!target_bear) return;

    if (target_bear->HistoryCount < MAX_SEQUENCE_LEN) {
        target_bear->ObservedHistory[target_bear->HistoryCount] = state;
        target_bear->HistoryCount++;
    } else {
        for (int i = 1; i < MAX_SEQUENCE_LEN; i++) {
            target_bear->ObservedHistory[i - 1] = target_bear->ObservedHistory[i];
        }
        target_bear->ObservedHistory[MAX_SEQUENCE_LEN - 1] = state;
    }
}

tsfi_AttestationPattern tsfi_cho_classify_attestation_pattern(const tsfi_DelegateRecord* target_bear) {
    if (!target_bear || target_bear->HistoryCount < 3) return PATTERN_CHAOTIC;

    uint32_t idx = target_bear->HistoryCount;
    tsfi_BearEmotionState s1 = target_bear->ObservedHistory[idx - 3];
    tsfi_BearEmotionState s2 = target_bear->ObservedHistory[idx - 2];
    tsfi_BearEmotionState s3 = target_bear->ObservedHistory[idx - 1];

    if (s1 == STATE_STRESSED && s2 == STATE_STRESSED && s3 == STATE_EXCITED) {
        return PATTERN_THREATENING;
    }

    if (s1 == STATE_NORMATIVE && s2 == STATE_EXCITED && s3 == STATE_NORMATIVE) {
        return PATTERN_REASSURING;
    }

    if (s1 == STATE_NORMATIVE && s2 == STATE_NORMATIVE && s3 == STATE_NORMATIVE) {
        return PATTERN_STABLE;
    }

    return PATTERN_CHAOTIC;
}

void tsfi_cho_restrict_eigenvector_constraints(tsfi_DelegateRecord* bear, double instability) {
    if (!bear) return;

    bear->BlowUpFactor += instability;
    bear->ConstraintEigenvalue = 1.0 / (1.0 + bear->BlowUpFactor);
}

void tsfi_cho_update_opinion(tsfi_DelegateRecord* local_bear, int peer_idx, bool is_harmonious) {
    if (!local_bear) return;

    double target_val = is_harmonious ? 1.0 : 0.0;
    double alpha = 0.15; // Learning/Drift rate

    if (peer_idx == -1) {
        local_bear->SelfOpinion += alpha * (target_val - local_bear->SelfOpinion);
    } else if (peer_idx >= 0 && peer_idx < MAX_DELEGATES) {
        local_bear->PeerOpinions[peer_idx] += alpha * (target_val - local_bear->PeerOpinions[peer_idx]);
    }
}
