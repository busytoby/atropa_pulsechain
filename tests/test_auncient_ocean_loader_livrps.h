#ifndef TEST_AUNCIENT_OCEAN_LOADER_LIVRPS_H
#define TEST_AUNCIENT_OCEAN_LOADER_LIVRPS_H

#include <string.h>

typedef struct {
    char local_variant_opinion[32];
    char referenced_variant_opinion[32];
    char reference_opinion[32];
} usd_variant_ref_conflict_t;

static void resolve_variant_ref_conflict(const usd_variant_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_variant_opinion) > 0) {
        strcpy(resolved_val, conflict->local_variant_opinion);
    } else if (strlen(conflict->referenced_variant_opinion) > 0) {
        strcpy(resolved_val, conflict->referenced_variant_opinion);
    } else {
        strcpy(resolved_val, conflict->reference_opinion);
    }
}

typedef struct {
    char reference_opinion[32];
    char payload_opinion[32];
    char specializes_opinion[32];
} usd_payload_ref_conflict_t;

static void resolve_payload_ref_conflict(const usd_payload_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->reference_opinion) > 0) {
        strcpy(resolved_val, conflict->reference_opinion);
    } else if (strlen(conflict->payload_opinion) > 0) {
        strcpy(resolved_val, conflict->payload_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char variant_opinion[32];
} usd_local_variant_conflict_t;

static void resolve_local_variant_conflict(const usd_local_variant_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->variant_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char specializes_opinion[32];
} usd_spec_inh_conflict_t;

static void resolve_spec_inh_conflict(const usd_spec_inh_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char class_opinions[3][32];
    int total_classes;
} usd_multiple_inherits_t;

static void resolve_multiple_inherits(const usd_multiple_inherits_t *mi, char *resolved_val) {
    for (int i = 0; i < mi->total_classes; i++) {
        if (strlen(mi->class_opinions[i]) > 0) {
            strcpy(resolved_val, mi->class_opinions[i]);
            return;
        }
    }
}

typedef struct {
    char variant_opinion[32];
    char specializes_opinion[32];
} usd_variant_spec_conflict_t;

static void resolve_variant_spec_conflict(const usd_variant_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->variant_opinion) > 0) {
        strcpy(resolved_val, conflict->variant_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char variant_opinion[32];
} usd_inh_variant_conflict_t;

static void resolve_inh_variant_conflict(const usd_inh_variant_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->variant_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char payload_opinion[32];
} usd_inh_payload_conflict_t;

static void resolve_inh_payload_conflict(const usd_inh_payload_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->payload_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char reference_opinion[32];
} usd_inh_ref_conflict_t;

static void resolve_inh_ref_conflict(const usd_inh_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->reference_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char inherits_opinion[32];
} usd_local_inh_conflict_t;

static void resolve_local_inh_conflict(const usd_local_inh_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->inherits_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char reference_opinion[32];
} usd_local_ref_conflict_t;

static void resolve_local_ref_conflict(const usd_local_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->reference_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char payload_opinion[32];
} usd_local_payload_conflict_t;

static void resolve_local_payload_conflict(const usd_local_payload_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->payload_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char specializes_opinion[32];
} usd_local_spec_conflict_t;

static void resolve_local_spec_conflict(const usd_local_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char reference_opinion[32];
    char payload_opinion[32];
} usd_ref_payload_conflict_t;

static void resolve_ref_payload_conflict(const usd_ref_payload_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->reference_opinion) > 0) {
        strcpy(resolved_val, conflict->reference_opinion);
    } else {
        strcpy(resolved_val, conflict->payload_opinion);
    }
}

typedef struct {
    char reference_opinion[32];
    char specializes_opinion[32];
} usd_ref_spec_conflict_t;

static void resolve_ref_spec_conflict(const usd_ref_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->reference_opinion) > 0) {
        strcpy(resolved_val, conflict->reference_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char payload_opinion[32];
    char specializes_opinion[32];
} usd_payload_spec_conflict_t;

static void resolve_payload_spec_conflict(const usd_payload_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->payload_opinion) > 0) {
        strcpy(resolved_val, conflict->payload_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

#endif // TEST_AUNCIENT_OCEAN_LOADER_LIVRPS_H
