#ifndef TSFI_BAYESIAN_H
#define TSFI_BAYESIAN_H

#include "tsfi_dat.h"

// Maximum dimensions for the transition matrix operator
#define MAX_BAYESIAN_NODES 8

// Matrix structure for Markov transition operators
typedef struct {
    int size;
    float data[MAX_BAYESIAN_NODES][MAX_BAYESIAN_NODES];
} TSFiTransitionMatrix;

// Bayesian network node structure
typedef struct {
    char path[128];
    int parent_indices[MAX_BAYESIAN_NODES];
    int parent_count;
    float base_probability; // Prior P(H)
} TSFiBayesianNode;

// Multiply two transition matrices: Out = A * B
void tsfi_matrix_multiply(const TSFiTransitionMatrix *A, const TSFiTransitionMatrix *B, TSFiTransitionMatrix *Out);

// Compute matrix power: Out = In^power (nilpotent check)
void tsfi_matrix_power(const TSFiTransitionMatrix *In, int power, TSFiTransitionMatrix *Out);

// Perform Bayesian inference: returns P(Hypothesis | Evidence)
float tsfi_bayesian_infer(tsfi_dat *dat, const char *hypothesis_path, const char *evidence_path);

#endif // TSFI_BAYESIAN_H
