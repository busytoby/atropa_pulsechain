#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_bayesian.h"

void tsfi_matrix_multiply(const TSFiTransitionMatrix *A, const TSFiTransitionMatrix *B, TSFiTransitionMatrix *Out) {
    if (!A || !B || !Out) return;
    int size = A->size;
    Out->size = size;
    
    // Temp matrix to support in-place multiplication
    TSFiTransitionMatrix temp;
    temp.size = size;
    memset(temp.data, 0, sizeof(temp.data));

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float sum = 0.0f;
            for (int k = 0; k < size; k++) {
                sum += A->data[i][k] * B->data[k][j];
            }
            temp.data[i][j] = sum;
        }
    }

    memcpy(Out->data, temp.data, sizeof(temp.data));
}

void tsfi_matrix_power(const TSFiTransitionMatrix *In, int power, TSFiTransitionMatrix *Out) {
    if (!In || !Out || power < 0) return;
    int size = In->size;
    Out->size = size;
    memset(Out->data, 0, sizeof(Out->data));

    // Initialize Out as Identity matrix
    for (int i = 0; i < size; i++) {
        Out->data[i][i] = 1.0f;
    }

    if (power == 0) return;

    TSFiTransitionMatrix base = *In;
    int p = power;
    while (p > 0) {
        if (p & 1) {
            tsfi_matrix_multiply(Out, &base, Out);
        }
        tsfi_matrix_multiply(&base, &base, &base);
        p >>= 1;
    }
}

float tsfi_bayesian_infer(tsfi_dat *dat, const char *hypothesis_path, const char *evidence_path) {
    if (!dat || !hypothesis_path || !evidence_path) return 0.0f;

    // Search priors and joint probabilities
    const char *hyp_str = tsfi_dat_search(dat, hypothesis_path);
    const char *ev_str = tsfi_dat_search(dat, evidence_path);

    if (!hyp_str || !ev_str) return 0.0f;

    float p_hyp = atof(hyp_str) / 100.0f;
    float p_ev = atof(ev_str) / 100.0f;

    if (p_ev <= 0.0f) return 0.0f;

    // Query joint probability: P(Evidence AND Hypothesis)
    // We construct a query path query/joint/<hyp>/<ev>
    char joint_path[256];
    snprintf(joint_path, sizeof(joint_path), "query/joint/%s/%s", hypothesis_path, evidence_path);
    const char *joint_str = tsfi_dat_search(dat, joint_path);

    float p_joint = 0.0f;
    if (joint_str) {
        p_joint = atof(joint_str) / 100.0f;
    } else {
        // Fallback assuming independent events if no explicit joint value is recorded
        p_joint = p_hyp * p_ev;
    }

    // Bayes' Theorem: P(H | E) = P(H AND E) / P(E)
    float posterior = p_joint / p_ev;
    if (posterior > 1.0f) posterior = 1.0f;
    return posterior;
}
