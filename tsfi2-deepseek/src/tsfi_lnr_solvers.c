#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_lnr_solvers.h"

// LU Decomposition Thunk
bool lnr_solve_lu(const double *A, const double *B, double *x, int n) {
    double L[4][4] = {{0}};
    double U[4][4] = {{0}};
    int i, j, k;
    for (i = 0; i < n; i++) {
        for (k = i; k < n; k++) {
            double sum = 0.0;
            for (j = 0; j < i; j++) {
                sum += L[i][j] * U[j][k];
            }
            U[i][k] = A[i * n + k] - sum;
        }
        for (k = i; k < n; k++) {
            if (i == k) {
                L[i][i] = 1.0;
            } else {
                double sum = 0.0;
                for (j = 0; j < i; j++) {
                    sum += L[k][j] * U[j][i];
                }
                if (fabs(U[i][i]) < 1e-15) return false;
                L[k][i] = (A[k * n + i] - sum) / U[i][i];
            }
        }
    }
    double y[4] = {0};
    for (i = 0; i < n; i++) {
        double sum = 0.0;
        for (j = 0; j < i; j++) {
            sum += L[i][j] * y[j];
        }
        y[i] = B[i] - sum;
    }
    for (i = n - 1; i >= 0; i--) {
        double sum = 0.0;
        for (j = i + 1; j < n; j++) {
            sum += U[i][j] * x[j];
        }
        if (fabs(U[i][i]) < 1e-15) return false;
        x[i] = (y[i] - sum) / U[i][i];
    }
    return true;
}

// QR Decomposition Thunk via modified Gram-Schmidt
bool lnr_solve_qr(const double *A, const double *B, double *x, int n) {
    double Q[16] = {0};
    double R[16] = {0};
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            Q[i * n + j] = A[i * n + j];
        }
        for (int k = 0; k < j; k++) {
            double r_kj = 0.0;
            for (int i = 0; i < n; i++) {
                r_kj += Q[i * n + k] * A[i * n + j];
            }
            R[k * n + j] = r_kj;
            for (int i = 0; i < n; i++) {
                Q[i * n + j] -= r_kj * Q[i * n + k];
            }
        }
        double norm = 0.0;
        for (int i = 0; i < n; i++) {
            norm += Q[i * n + j] * Q[i * n + j];
        }
        norm = sqrt(norm);
        if (norm < 1e-15) return false;
        R[j * n + j] = norm;
        for (int i = 0; i < n; i++) {
            Q[i * n + j] /= norm;
        }
    }
    double QtB[4] = {0};
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            QtB[i] += Q[j * n + i] * B[j];
        }
    }
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0.0;
        for (int j = i + 1; j < n; j++) {
            sum += R[i * n + j] * x[j];
        }
        if (fabs(R[i * n + i]) < 1e-15) return false;
        x[i] = (QtB[i] - sum) / R[i * n + i];
    }
    return true;
}

// SVD Decomposition Thunk via QR iteration
bool lnr_solve_svd(const double *A, const double *B, double *x, int n) {
    double C[16] = {0};
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[k * n + i] * A[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }

    double V[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    const int max_iters = 100;
    for (int iter = 0; iter < max_iters; iter++) {
        double Q[16] = {0};
        double R[16] = {0};

        for (int j = 0; j < n; j++) {
            for (int i = 0; i < n; i++) {
                Q[i * n + j] = C[i * n + j];
            }
            for (int k = 0; k < j; k++) {
                double r_kj = 0.0;
                for (int i = 0; i < n; i++) {
                    r_kj += Q[i * n + k] * C[i * n + j];
                }
                R[k * n + j] = r_kj;
                for (int i = 0; i < n; i++) {
                    Q[i * n + j] -= r_kj * Q[i * n + k];
                }
            }
            double norm = 0.0;
            for (int i = 0; i < n; i++) {
                norm += Q[i * n + j] * Q[i * n + j];
            }
            norm = sqrt(norm);
            if (norm < 1e-15) {
                R[j * n + j] = 0.0;
            } else {
                R[j * n + j] = norm;
                for (int i = 0; i < n; i++) {
                    Q[i * n + j] /= norm;
                }
            }
        }

        double C_new[16] = {0};
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int k = 0; k < n; k++) {
                    sum += R[i * n + k] * Q[k * n + j];
                }
                C_new[i * n + j] = sum;
            }
        }

        double off_diag = 0.0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    off_diag += fabs(C_new[i * n + j]);
                }
            }
        }

        double V_new[16] = {0};
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double sum = 0.0;
                for (int k = 0; k < n; k++) {
                    sum += V[i * n + k] * Q[k * n + j];
                }
                V_new[i * n + j] = sum;
            }
        }
        memcpy(V, V_new, sizeof(V));
        memcpy(C, C_new, sizeof(C));

        if (off_diag < 1e-12) break;
    }

    double S[4] = {0};
    for (int i = 0; i < n; i++) {
        S[i] = C[i * n + i] > 0.0 ? sqrt(C[i * n + i]) : 0.0;
    }

    double U[16] = {0};
    for (int j = 0; j < n; j++) {
        double AV[4] = {0};
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                AV[i] += A[i * n + k] * V[k * n + j];
            }
        }
        if (S[j] > 1e-12) {
            for (int i = 0; i < n; i++) {
                U[i * n + j] = AV[i] / S[j];
            }
        } else {
            for (int i = 0; i < n; i++) {
                U[i * n + j] = 0.0;
            }
        }
    }

    double UtB[4] = {0};
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            UtB[j] += U[i * n + j] * B[i];
        }
    }
    double SinvUtB[4] = {0};
    for (int j = 0; j < n; j++) {
        if (S[j] > 1e-12) {
            SinvUtB[j] = UtB[j] / S[j];
        } else {
            SinvUtB[j] = 0.0;
        }
    }
    for (int i = 0; i < n; i++) {
        x[i] = 0.0;
        for (int j = 0; j < n; j++) {
            x[i] += V[i * n + j] * SinvUtB[j];
        }
    }
    return true;
}

// CG (Conjugate Gradient) Thunk
bool lnr_solve_cg(const double *A, const double *B, double *x, int n) {
    double r[4], p[4], Ap[4];
    for (int i = 0; i < n; i++) {
        x[i] = 0.0;
        r[i] = B[i];
        p[i] = r[i];
    }
    double rsold = 0.0;
    for (int i = 0; i < n; i++) rsold += r[i] * r[i];

    for (int step = 0; step < 50; step++) {
        for (int i = 0; i < n; i++) {
            Ap[i] = 0.0;
            for (int j = 0; j < n; j++) {
                Ap[i] += A[i * n + j] * p[j];
            }
        }
        double pAp = 0.0;
        for (int i = 0; i < n; i++) pAp += p[i] * Ap[i];
        if (fabs(pAp) < 1e-20) return false;

        double alpha = rsold / pAp;
        for (int i = 0; i < n; i++) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }
        double rsnew = 0.0;
        for (int i = 0; i < n; i++) rsnew += r[i] * r[i];
        if (sqrt(rsnew) < 1e-12) return true;

        double beta = rsnew / rsold;
        for (int i = 0; i < n; i++) {
            p[i] = r[i] + beta * p[i];
        }
        rsold = rsnew;
    }
    return true;
}

// GMRES Thunk
bool lnr_solve_gmres(const double *A, const double *B, double *x, int n) {
    double r[4];
    for (int i = 0; i < n; i++) x[i] = 0.0;

    double V[5][4] = {{0}};
    double H[5][4] = {{0}};

    double beta = 0.0;
    for (int i = 0; i < n; i++) {
        r[i] = B[i];
        beta += r[i] * r[i];
    }
    beta = sqrt(beta);
    if (beta < 1e-15) return true;

    for (int i = 0; i < n; i++) {
        V[0][i] = r[i] / beta;
    }

    for (int j = 0; j < n; j++) {
        double w[4] = {0};
        for (int r_idx = 0; r_idx < n; r_idx++) {
            for (int c_idx = 0; c_idx < n; c_idx++) {
                w[r_idx] += A[r_idx * n + c_idx] * V[j][c_idx];
            }
        }
        for (int i = 0; i <= j; i++) {
            double h_ij = 0.0;
            for (int k = 0; k < n; k++) h_ij += w[k] * V[i][k];
            H[i][j] = h_ij;
            for (int k = 0; k < n; k++) w[k] -= h_ij * V[i][k];
        }
        double h_j1j = 0.0;
        for (int k = 0; k < n; k++) h_j1j += w[k] * w[k];
        h_j1j = sqrt(h_j1j);
        H[j+1][j] = h_j1j;
        if (h_j1j > 1e-15 && j + 1 < n) {
            for (int k = 0; k < n; k++) V[j+1][k] = w[k] / h_j1j;
        }
    }

    double cs[4] = {0};
    double sn[4] = {0};
    double g[5] = {beta, 0, 0, 0, 0};

    for (int i = 0; i < n; i++) {
        for (int k = 0; k < i; k++) {
            double temp = cs[k] * H[k][i] + sn[k] * H[k+1][i];
            H[k+1][i] = -sn[k] * H[k][i] + cs[k] * H[k+1][i];
            H[k][i] = temp;
        }
        double h1 = H[i][i];
        double h2 = H[i+1][i];
        if (fabs(h2) < 1e-30) {
            cs[i] = 1.0;
            sn[i] = 0.0;
        } else {
            double t = sqrt(h1 * h1 + h2 * h2);
            cs[i] = h1 / t;
            sn[i] = h2 / t;
        }
        H[i][i] = cs[i] * h1 + sn[i] * h2;
        H[i+1][i] = 0.0;

        double temp_g = cs[i] * g[i] + sn[i] * g[i+1];
        g[i+1] = -sn[i] * g[i] + cs[i] * g[i+1];
        g[i] = temp_g;
    }

    double y[4] = {0};
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0.0;
        for (int j = i + 1; j < n; j++) {
            sum += H[i][j] * y[j];
        }
        if (fabs(H[i][i]) < 1e-15) return false;
        y[i] = (g[i] - sum) / H[i][i];
    }

    for (int i = 0; i < n; i++) {
        x[i] = 0.0;
        for (int j = 0; j < n; j++) {
            x[i] += V[j][i] * y[j];
        }
    }
    return true;
}

// Main LNR dynamic selection denoiser pass driven from the Auncient Vaesen guidance map
bool tsfi_montecarlo_regression_denoise_lnr(
    const float *noisy_input,
    const TSFiMCAuxFeatures *features,
    const float *guidance_map,
    float *clean_output,
    int width,
    int height,
    int window_radius,
    float spatial_sigma,
    float feature_sigma
) {
    if (!noisy_input || !features || !clean_output || !guidance_map || width <= 0 || height <= 0) {
        return false;
    }
    if (window_radius < 0 || spatial_sigma <= 0.0f || feature_sigma <= 0.0f) {
        return false;
    }

    float spatial_sig_sq = spatial_sigma * spatial_sigma;
    float feature_sig_sq = feature_sigma * feature_sigma;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            double A[16] = {0};
            double B[4] = {0};

            TSFiMCAuxFeatures c_feat = features[idx];
            float c_albedo = (c_feat.albedo.x + c_feat.albedo.y + c_feat.albedo.z) * 0.3333f;

            // Pass 1: Build standard regression matrices
            for (int wy = -window_radius; wy <= window_radius; wy++) {
                int ny = y + wy;
                if (ny < 0 || ny >= height) continue;
                for (int wx = -window_radius; wx <= window_radius; wx++) {
                    int nx = x + wx;
                    if (nx < 0 || nx >= width) continue;

                    int n_idx = ny * width + nx;
                    TSFiMCAuxFeatures n_feat = features[n_idx];

                    float s_dist_sq = (float)(wx * wx + wy * wy);
                    float d_depth = c_feat.depth - n_feat.depth;
                    float d_emot = c_feat.emotional_weight - n_feat.emotional_weight;
                    float dot_norm = c_feat.normal.x * n_feat.normal.x +
                                     c_feat.normal.y * n_feat.normal.y +
                                     c_feat.normal.z * n_feat.normal.z;
                    float d_norm_sq = fmaxf(0.0f, 1.0f - dot_norm);

                    float n_albedo = (n_feat.albedo.x + n_feat.albedo.y + n_feat.albedo.z) * 0.3333f;
                    float d_albedo = c_albedo - n_albedo;

                    float f_dist_sq = d_depth * d_depth + d_emot * d_emot + d_norm_sq + d_albedo * d_albedo;
                    double w = exp((double)(-s_dist_sq / spatial_sig_sq - f_dist_sq / feature_sig_sq));

                    double v[4];
                    v[0] = 1.0;
                    v[1] = (double)(n_feat.depth - ((n_idx > 0) ? features[n_idx - 1].depth : n_feat.depth));
                    v[2] = (double)(n_feat.emotional_weight - ((n_idx > 0) ? features[n_idx - 1].emotional_weight : n_feat.emotional_weight));
                    float prev_albedo = (n_idx > 0) ? (features[n_idx - 1].albedo.x + features[n_idx - 1].albedo.y + features[n_idx - 1].albedo.z) * 0.3333f : n_albedo;
                    v[3] = (double)(n_albedo - prev_albedo);

                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            A[i * 4 + j] += w * v[i] * v[j];
                        }
                        B[i] += w * v[i] * (double)noisy_input[n_idx];
                    }
                }
            }

            double coeffs[4] = {0};
            bool solved = false;
            float g_val = guidance_map[idx];

            if (g_val < 0.1f) {
                solved = lnr_solve_svd(A, B, coeffs, 4);
            } else if (g_val < 0.4f) {
                solved = lnr_solve_cg(A, B, coeffs, 4);
            } else if (g_val < 0.7f) {
                solved = lnr_solve_qr(A, B, coeffs, 4);
            } else if (g_val < 0.9f) {
                solved = lnr_solve_gmres(A, B, coeffs, 4);
            } else {
                solved = lnr_solve_lu(A, B, coeffs, 4);
            }

            // Pass 2: Iteratively Reweighted Least Squares (IRLS) for outlier robustness
            if (solved) {
                double A_rob[16] = {0};
                double B_rob[4] = {0};

                for (int wy = -window_radius; wy <= window_radius; wy++) {
                    int ny = y + wy;
                    if (ny < 0 || ny >= height) continue;
                    for (int wx = -window_radius; wx <= window_radius; wx++) {
                        int nx = x + wx;
                        if (nx < 0 || nx >= width) continue;

                        int n_idx = ny * width + nx;
                        TSFiMCAuxFeatures n_feat = features[n_idx];

                        float s_dist_sq = (float)(wx * wx + wy * wy);
                        float d_depth = c_feat.depth - n_feat.depth;
                        float d_emot = c_feat.emotional_weight - n_feat.emotional_weight;
                        float dot_norm = c_feat.normal.x * n_feat.normal.x +
                                         c_feat.normal.y * n_feat.normal.y +
                                         c_feat.normal.z * n_feat.normal.z;
                        float d_norm_sq = fmaxf(0.0f, 1.0f - dot_norm);

                        float n_albedo = (n_feat.albedo.x + n_feat.albedo.y + n_feat.albedo.z) * 0.3333f;
                        float d_albedo = c_albedo - n_albedo;

                        float f_dist_sq = d_depth * d_depth + d_emot * d_emot + d_norm_sq + d_albedo * d_albedo;
                        double w = exp((double)(-s_dist_sq / spatial_sig_sq - f_dist_sq / feature_sig_sq));

                        double v[4];
                        v[0] = 1.0;
                        v[1] = (double)(n_feat.depth - ((n_idx > 0) ? features[n_idx - 1].depth : n_feat.depth));
                        v[2] = (double)(n_feat.emotional_weight - ((n_idx > 0) ? features[n_idx - 1].emotional_weight : n_feat.emotional_weight));
                        float prev_albedo = (n_idx > 0) ? (features[n_idx - 1].albedo.x + features[n_idx - 1].albedo.y + features[n_idx - 1].albedo.z) * 0.3333f : n_albedo;
                        v[3] = (double)(n_albedo - prev_albedo);

                        double est = coeffs[0] * v[0] + coeffs[1] * v[1] + coeffs[2] * v[2] + coeffs[3] * v[3];
                        double res = (double)noisy_input[n_idx] - est;
                        double w_robust = exp(-fabs(res) / 0.15); // Suppression of outliers

                        double w_total = w * w_robust;

                        for (int i = 0; i < 4; i++) {
                            for (int j = 0; j < 4; j++) {
                                A_rob[i * 4 + j] += w_total * v[i] * v[j];
                            }
                            B_rob[i] += w_total * v[i] * (double)noisy_input[n_idx];
                        }
                    }
                }

                double coeffs_rob[4] = {0};
                bool solved_rob = false;
                if (g_val < 0.1f) {
                    solved_rob = lnr_solve_svd(A_rob, B_rob, coeffs_rob, 4);
                } else if (g_val < 0.4f) {
                    solved_rob = lnr_solve_cg(A_rob, B_rob, coeffs_rob, 4);
                } else if (g_val < 0.7f) {
                    solved_rob = lnr_solve_qr(A_rob, B_rob, coeffs_rob, 4);
                } else if (g_val < 0.9f) {
                    solved_rob = lnr_solve_gmres(A_rob, B_rob, coeffs_rob, 4);
                } else {
                    solved_rob = lnr_solve_lu(A_rob, B_rob, coeffs_rob, 4);
                }

                if (solved_rob) {
                    clean_output[idx] = (float)coeffs_rob[0];
                } else {
                    clean_output[idx] = (float)coeffs[0];
                }
            } else {
                clean_output[idx] = noisy_input[idx];
            }
        }
    }
    return true;
}
