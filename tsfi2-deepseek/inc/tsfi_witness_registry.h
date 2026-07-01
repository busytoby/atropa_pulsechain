#ifndef TSFI_WITNESS_REGISTRY_H
#define TSFI_WITNESS_REGISTRY_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_DELEGATES 16
#define MAX_RULE_CACHE 8
#define MAX_SELF_OBSERVATIONS 32
#define MAX_SEQUENCE_LEN 8

typedef enum {
    STATE_NORMATIVE = 0,
    STATE_EXCITED,
    STATE_STRESSED,
    STATE_UNKNOWN
} tsfi_BearEmotionState;

typedef enum {
    AESTHETIC_HARMONIOUS = 0,
    AESTHETIC_DISHARMONIOUS,
    AESTHETIC_NEUTRAL
} tsfi_AestheticClass;

typedef enum {
    PATTERN_STABLE = 0,
    PATTERN_REASSURING,
    PATTERN_THREATENING,
    PATTERN_CHAOTIC
} tsfi_AttestationPattern;

typedef struct {
    double R_hyp;
    double r_hyp;
    double d_hyp;
    tsfi_AestheticClass Classification;
} tsfi_SelfObservationRecord;

typedef struct {
    char ForeignAddress[43];
    uint64_t Exp;
    uint64_t Mod;
    bool Known;
} tsfi_LutTranslationRule;

typedef struct {
    char Address[43];
    uint64_t Soul;
    uint64_t VerifiedWavefunc;
    bool Competent;
    double R_hyp;
    double r_hyp;
    double d_hyp;
    
    uint64_t LocalExp;
    uint64_t LocalMod;

    tsfi_LutTranslationRule LearnedRules[MAX_RULE_CACHE];
    tsfi_BearEmotionState ClassifiedState;

    tsfi_SelfObservationRecord SelfDatabase[MAX_SELF_OBSERVATIONS];
    uint32_t SelfDbCount;

    tsfi_BearEmotionState ObservedHistory[MAX_SEQUENCE_LEN];
    uint32_t HistoryCount;

    // Eigenvector constraints and dynamic parameters
    double BlowUpFactor;          /* Cumulative instability metrics */
    double ConstraintEigenvalue;   /* Scaling boundary limit */
} tsfi_DelegateRecord;

typedef struct {
    tsfi_DelegateRecord Delegates[MAX_DELEGATES];
    uint32_t Count;
} tsfi_ChoRegistry;

void tsfi_cho_registry_init(tsfi_ChoRegistry* registry);

bool tsfi_cho_swear_in(tsfi_ChoRegistry* registry, const char* target_address, uint64_t target_wavefunc, uint64_t exp, uint64_t mod);

bool tsfi_cho_learn_rules(tsfi_DelegateRecord* local_bear, const tsfi_DelegateRecord* foreign_bear);

bool tsfi_cho_witness_overlap(const tsfi_DelegateRecord* a, const tsfi_DelegateRecord* b);

tsfi_BearEmotionState tsfi_cho_classify_emotion(const tsfi_DelegateRecord* bear, double dynamic_d_l, double dynamic_d_r);

void tsfi_cho_log_self_observation(tsfi_DelegateRecord* bear, double R, double r, double d, tsfi_AestheticClass classification);

tsfi_AestheticClass tsfi_cho_classify_foreign_aesthetic(const tsfi_DelegateRecord* local_bear, const tsfi_DelegateRecord* foreign_bear);

void tsfi_cho_record_attestation(tsfi_DelegateRecord* target_bear, tsfi_BearEmotionState state);

tsfi_AttestationPattern tsfi_cho_classify_attestation_pattern(const tsfi_DelegateRecord* target_bear);

/*
 * Dynamic constraint scaling: high BlowUpFactor collapses target eigenvalues,
 * narrowing parameter boundaries.
 */
void tsfi_cho_restrict_eigenvector_constraints(tsfi_DelegateRecord* bear, double instability);

#endif /* TSFI_WITNESS_REGISTRY_H */
