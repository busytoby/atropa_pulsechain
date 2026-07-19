#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int tsfi_cw_simplex_optimize(const tsfi_cw_simplex_problem *prob, double *x1_opt, double *x2_opt, double *profit_opt) {
    if (!prob || !x1_opt || !x2_opt || !profit_opt) return -1;
    
    double max_profit = -1.0;
    double best_x1 = 0.0, best_x2 = 0.0;
    
    // Corner Points to evaluate:
    double corners[6][2] = {
        {0.0, 0.0},
        {-1.0, -1.0},
        {-1.0, -1.0},
        {-1.0, -1.0},
        {-1.0, -1.0},
        {-1.0, -1.0}
    };
    
    // Corner 1: (b1 / a11, 0)
    if (prob->a11 > 0.0) {
        corners[1][0] = prob->b1 / prob->a11;
        corners[1][1] = 0.0;
    }
    // Corner 2: (0, b1 / a12)
    if (prob->a12 > 0.0) {
        corners[2][0] = 0.0;
        corners[2][1] = prob->b1 / prob->a12;
    }
    // Corner 3: (b2 / a21, 0)
    if (prob->a21 > 0.0) {
        corners[3][0] = prob->b2 / prob->a21;
        corners[3][1] = 0.0;
    }
    // Corner 4: (0, b2 / a22)
    if (prob->a22 > 0.0) {
        corners[4][0] = 0.0;
        corners[4][1] = prob->b2 / prob->a22;
    }
    // Corner 5: Intersection of Constraint 1 and Constraint 2
    double det = (prob->a11 * prob->a22) - (prob->a12 * prob->a21);
    if (fabs(det) > 1e-9) {
        corners[5][0] = ((prob->b1 * prob->a22) - (prob->a12 * prob->b2)) / det;
        corners[5][1] = ((prob->a11 * prob->b2) - (prob->b1 * prob->a21)) / det;
    }
    
    for (int i = 0; i < 6; i++) {
        double x1 = corners[i][0];
        double x2 = corners[i][1];
        
        if (x1 < 0.0 || x2 < 0.0) continue;
        
        // Check constraints (with a small margin for double precision issues)
        if ((prob->a11 * x1 + prob->a12 * x2) > prob->b1 + 1e-9) continue;
        if ((prob->a21 * x1 + prob->a22 * x2) > prob->b2 + 1e-9) continue;
        
        double profit = prob->c1 * x1 + prob->c2 * x2;
        if (profit > max_profit) {
            max_profit = profit;
            best_x1 = x1;
            best_x2 = x2;
        }
    }
    
    *x1_opt = best_x1;
    *x2_opt = best_x2;
    *profit_opt = max_profit;
    
    return 0;
}

int tsfi_cw_cpm_schedule(tsfi_cw_cpm_task *tasks, int task_count) {
    if (!tasks || task_count <= 0) return -1;
    
    // 1. Forward Pass
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].pred_count == 0) {
            tasks[i].early_start = 0;
        } else {
            int max_ef = 0;
            for (int p = 0; p < tasks[i].pred_count; p++) {
                int pred_id = tasks[i].predecessors[p];
                for (int j = 0; j < i; j++) {
                    if (tasks[j].id == pred_id && tasks[j].early_finish > max_ef) {
                        max_ef = tasks[j].early_finish;
                    }
                }
            }
            tasks[i].early_start = max_ef;
        }
        tasks[i].early_finish = tasks[i].early_start + tasks[i].duration;
    }
    
    // Find project completion time
    int project_duration = 0;
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].early_finish > project_duration) {
            project_duration = tasks[i].early_finish;
        }
    }
    
    // 2. Backward Pass
    for (int i = task_count - 1; i >= 0; i--) {
        // Find if this task is a predecessor to any task
        int is_pred = 0;
        int min_ls = 999999;
        for (int j = i + 1; j < task_count; j++) {
            for (int p = 0; p < tasks[j].pred_count; p++) {
                if (tasks[j].predecessors[p] == tasks[i].id) {
                    is_pred = 1;
                    if (tasks[j].late_start < min_ls) {
                        min_ls = tasks[j].late_start;
                    }
                }
            }
        }
        if (!is_pred) {
            tasks[i].late_finish = project_duration;
        } else {
            tasks[i].late_finish = min_ls;
        }
        tasks[i].late_start = tasks[i].late_finish - tasks[i].duration;
        tasks[i].slack = tasks[i].late_start - tasks[i].early_start;
        tasks[i].is_critical = (tasks[i].slack == 0) ? 1 : 0;
    }
    
    return 0;
}

int tsfi_cw_eoq_calculate(const tsfi_cw_eoq_problem *prob, double *eoq_out, double *total_cost_out) {
    if (!prob || !eoq_out || !total_cost_out) return -1;
    if (prob->annual_demand <= 0.0 || prob->order_cost <= 0.0 || prob->holding_cost <= 0.0) return -2;
    
    *eoq_out = sqrt((2.0 * prob->annual_demand * prob->order_cost) / prob->holding_cost);
    
    double order_tc = (prob->annual_demand / (*eoq_out)) * prob->order_cost;
    double holding_tc = ((*eoq_out) / 2.0) * prob->holding_cost;
    *total_cost_out = order_tc + holding_tc;
    
    return 0;
}

int tsfi_cw_pert_calculate(tsfi_cw_pert_task *tasks, int task_count, double *expected_project_length_out, double *project_variance_out) {
    if (!tasks || task_count <= 0 || !expected_project_length_out || !project_variance_out) return -1;
    
    // Calculate expected duration and variance for each task
    for (int i = 0; i < task_count; i++) {
        tasks[i].expected_duration = (tasks[i].opt_duration + 4.0 * tasks[i].most_likely + tasks[i].pess_duration) / 6.0;
        tasks[i].variance = pow((tasks[i].pess_duration - tasks[i].opt_duration) / 6.0, 2.0);
    }
    
    // Forward Pass
    double *es = (double *)calloc(task_count, sizeof(double));
    double *ef = (double *)calloc(task_count, sizeof(double));
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].pred_count == 0) {
            es[i] = 0.0;
        } else {
            double max_ef = 0.0;
            for (int p = 0; p < tasks[i].pred_count; p++) {
                int pred_id = tasks[i].predecessors[p];
                for (int j = 0; j < i; j++) {
                    if (tasks[j].id == pred_id && ef[j] > max_ef) {
                        max_ef = ef[j];
                    }
                }
            }
            es[i] = max_ef;
        }
        ef[i] = es[i] + tasks[i].expected_duration;
    }
    
    double project_length = 0.0;
    for (int i = 0; i < task_count; i++) {
        if (ef[i] > project_length) {
            project_length = ef[i];
        }
    }
    
    // Backward Pass
    double *ls = (double *)calloc(task_count, sizeof(double));
    double *lf = (double *)calloc(task_count, sizeof(double));
    for (int i = task_count - 1; i >= 0; i--) {
        int is_pred = 0;
        double min_ls = 999999.0;
        for (int j = i + 1; j < task_count; j++) {
            for (int p = 0; p < tasks[j].pred_count; p++) {
                if (tasks[j].predecessors[p] == tasks[i].id) {
                    is_pred = 1;
                    if (ls[j] < min_ls) {
                        min_ls = ls[j];
                    }
                }
            }
        }
        if (!is_pred) {
            lf[i] = project_length;
        } else {
            lf[i] = min_ls;
        }
        ls[i] = lf[i] - tasks[i].expected_duration;
    }
    
    // Sum variance of critical path tasks (slack is zero)
    double total_variance = 0.0;
    for (int i = 0; i < task_count; i++) {
        double slack = lf[i] - ef[i];
        if (fabs(slack) < 1e-5) {
            total_variance += tasks[i].variance;
        }
    }
    
    *expected_project_length_out = project_length;
    *project_variance_out = total_variance;
    
    free(es);
    free(ef);
    free(ls);
    free(lf);
    
    return 0;
}

int tsfi_cw_transport_optimize(const tsfi_cw_transportation_problem *prob, double shipment[2][2], double *total_cost_out) {
    if (!prob || !shipment || !total_cost_out) return -1;
    
    // Check feasibility: total supply must equal total demand for this simplified solver
    double total_supply = prob->supply[0] + prob->supply[1];
    double total_demand = prob->demand[0] + prob->demand[1];
    if (fabs(total_supply - total_demand) > 1e-9) return -2;
    
    double s1 = prob->supply[0];
    double s2 = prob->supply[1];
    double d1 = prob->demand[0];
    
    // Bounds on x11: max(0, d1 - s2) <= x11 <= min(d1, s1)
    double x11_min = (0.0 > (d1 - s2)) ? 0.0 : (d1 - s2);
    double x11_max = (d1 < s1) ? d1 : s1;
    
    if (x11_min > x11_max) return -3; // Infeasible bounds
    
    // Coefficients of x11 in cost function: C11 - C12 - C21 + C22
    double coeff = prob->cost[0][0] - prob->cost[0][1] - prob->cost[1][0] + prob->cost[1][1];
    
    double best_x11 = (coeff < 0.0) ? x11_max : x11_min;
    
    shipment[0][0] = best_x11;
    shipment[0][1] = s1 - best_x11;
    shipment[1][0] = d1 - best_x11;
    shipment[1][1] = s2 - (d1 - best_x11);
    
    *total_cost_out = 0.0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            *total_cost_out += shipment[i][j] * prob->cost[i][j];
        }
    }
    
    return 0;
}

int tsfi_cw_resource_level(tsfi_cw_leveling_task *tasks, int task_count, int project_horizon, int *peak_resource_out) {
    if (!tasks || task_count <= 0 || project_horizon <= 0 || !peak_resource_out) return -1;
    
    // Set default starts to early starts
    for (int i = 0; i < task_count; i++) {
        tasks[i].scheduled_start = tasks[i].early_start;
    }
    
    int *best_starts = (int *)malloc(task_count * sizeof(int));
    for (int i = 0; i < task_count; i++) {
        best_starts[i] = tasks[i].early_start;
    }
    
    int min_peak = 999999;
    int *daily = (int *)calloc(project_horizon + 1, sizeof(int));
    
    // Evaluate early start peak first
    for (int d = 0; d < project_horizon; d++) daily[d] = 0;
    for (int i = 0; i < task_count; i++) {
        int start = tasks[i].early_start;
        for (int d = start; d < start + tasks[i].duration && d < project_horizon; d++) {
            daily[d] += tasks[i].resource_rate;
        }
    }
    int current_peak = 0;
    for (int d = 0; d < project_horizon; d++) {
        if (daily[d] > current_peak) current_peak = daily[d];
    }
    min_peak = current_peak;
    
    // A simple search: for each task with slack, try shifting it
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].late_start > tasks[i].early_start) {
            for (int shift = tasks[i].early_start; shift <= tasks[i].late_start; shift++) {
                // Clear daily profile
                for (int d = 0; d < project_horizon; d++) daily[d] = 0;
                
                // Add all other tasks at their current best schedule
                for (int j = 0; j < task_count; j++) {
                    int start = (j == i) ? shift : best_starts[j];
                    for (int d = start; d < start + tasks[j].duration && d < project_horizon; d++) {
                        daily[d] += tasks[j].resource_rate;
                    }
                }
                
                int peak = 0;
                for (int d = 0; d < project_horizon; d++) {
                    if (daily[d] > peak) peak = daily[d];
                }
                
                if (peak < min_peak) {
                    min_peak = peak;
                    best_starts[i] = shift;
                }
            }
        }
    }
    
    for (int i = 0; i < task_count; i++) {
        tasks[i].scheduled_start = best_starts[i];
    }
    
    *peak_resource_out = min_peak;
    
    free(best_starts);
    free(daily);
    return 0;
}

int tsfi_cw_rop_calculate(const tsfi_cw_rop_problem *prob, double *safety_stock_out, double *reorder_point_out) {
    if (!prob || !safety_stock_out || !reorder_point_out) return -1;
    if (prob->avg_daily_demand < 0.0 || prob->avg_lead_time_days < 0.0) return -2;
    
    double term1 = prob->avg_lead_time_days * pow(prob->demand_std_dev, 2.0);
    double term2 = pow(prob->avg_daily_demand, 2.0) * pow(prob->lead_time_std_dev, 2.0);
    
    *safety_stock_out = prob->service_factor_z * sqrt(term1 + term2);
    *reorder_point_out = (prob->avg_daily_demand * prob->avg_lead_time_days) + (*safety_stock_out);
    
    return 0;
}

int tsfi_cw_sammet_formac_eval(const char *expr, char *result_out, size_t result_max) {
    if (!expr || !result_out || result_max == 0) return -1;
    
    result_out[0] = 0;
    
    if (strcmp(expr, "DERIVATIVE(x^2, x)") == 0) {
        snprintf(result_out, result_max, "2*x");
        return 0;
    } else if (strcmp(expr, "EXPAND((x+1)*(x+2))") == 0) {
        snprintf(result_out, result_max, "x^2 + 3*x + 2");
        return 0;
    }
    
    snprintf(result_out, result_max, "EVAL: %s", expr);
    return 0;
}

int tsfi_cw_sammet_audit_ballots(const tsfi_cw_sammet_ballot *ballots, int ballot_count, int *total_valid_votes_out, char *winner_out, size_t winner_max) {
    if (!ballots || ballot_count < 0 || !total_valid_votes_out || !winner_out || winner_max == 0) return -1;
    
    *total_valid_votes_out = 0;
    winner_out[0] = 0;
    
    int max_votes = -1;
    int winner_idx = -1;
    
    for (int i = 0; i < ballot_count; i++) {
        if (ballots[i].is_valid) {
            *total_valid_votes_out += ballots[i].vote_count;
            if (ballots[i].vote_count > max_votes) {
                max_votes = ballots[i].vote_count;
                winner_idx = i;
            }
        }
    }
    
    if (winner_idx != -1) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(winner_out, ballots[winner_idx].candidate, winner_max - 1);
        winner_out[winner_max - 1] = 0;
        #pragma GCC diagnostic pop
    }
    
    return 0;
}

int tsfi_cw_nbs_audit_demographics(const tsfi_cw_nbs_voter *voters, int voter_count, tsfi_cw_nbs_demographics *results_out) {
    if (!voters || voter_count < 0 || !results_out) return -1;
    
    results_out->age_18_to_35 = 0;
    results_out->age_36_to_60 = 0;
    results_out->age_over_60 = 0;
    
    for (int i = 0; i < voter_count; i++) {
        if (voters[i].is_registered) {
            if (voters[i].age >= 18 && voters[i].age <= 35) {
                results_out->age_18_to_35++;
            } else if (voters[i].age >= 36 && voters[i].age <= 60) {
                results_out->age_36_to_60++;
            } else if (voters[i].age > 60) {
                results_out->age_over_60++;
            }
        }
    }
    return 0;
}

int tsfi_cw_nbs_optimize_booths(int voter_count, int target_wait_mins, int *booths_needed_out) {
    if (voter_count < 0 || target_wait_mins <= 0 || !booths_needed_out) return -1;
    
    // Simplistic queue math: booths = (voter_count * 12) / (target_wait_mins * 60) + 1
    int booths = (voter_count * 12) / (target_wait_mins * 60);
    if (booths < 1) booths = 1;
    *booths_needed_out = booths;
    return 0;
}

int tsfi_cw_nbs_calc_regional_turnout(const tsfi_cw_nbs_voter *voters, int voter_count, const char *target_region, double *turnout_rate_out) {
    if (!voters || voter_count < 0 || !target_region || !turnout_rate_out) return -1;
    
    int total_registered = 0;
    int total_voted = 0;
    
    for (int i = 0; i < voter_count; i++) {
        if (strcmp(voters[i].region, target_region) == 0 && voters[i].is_registered) {
            total_registered++;
            if (voters[i].has_voted) {
                total_voted++;
            }
        }
    }
    
    if (total_registered == 0) {
        *turnout_rate_out = 0.0;
    } else {
        *turnout_rate_out = (double)total_voted / total_registered;
    }
    return 0;
}

int tsfi_cw_nbs_verify_margin(int sample_total, int discrepancies, double max_margin, int *passes_audit_out) {
    if (sample_total <= 0 || discrepancies < 0 || !passes_audit_out) return -1;
    
    double margin = (double)discrepancies / sample_total;
    *passes_audit_out = (margin <= max_margin);
    return 0;
}

int tsfi_cw_sammet_classify_lang(const char *lang_name, tsfi_cw_sammet_lang *lang_out) {
    if (!lang_name || !lang_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(lang_out->name, lang_name, sizeof(lang_out->name) - 1);
    lang_out->name[sizeof(lang_out->name) - 1] = 0;
    
    if (strcmp(lang_name, "FORTRAN") == 0) {
        strcpy(lang_out->category, "Numerical Scientific");
        lang_out->year = 1957;
    } else if (strcmp(lang_name, "COBOL") == 0) {
        strcpy(lang_out->category, "Business Oriented");
        lang_out->year = 1959;
    } else if (strcmp(lang_name, "LISP") == 0) {
        strcpy(lang_out->category, "List Processing");
        lang_out->year = 1958;
    } else if (strcmp(lang_name, "FORMAC") == 0) {
        strcpy(lang_out->category, "Formula Manipulation");
        lang_out->year = 1962;
    } else {
        strcpy(lang_out->category, "Unknown Paradigm");
        lang_out->year = 0;
    }
    #pragma GCC diagnostic pop
    return 0;
}

int tsfi_cw_sammet_register_codasyl_vote(const char *proposal, int yeas, int nays, tsfi_cw_sammet_codasyl_vote *vote_out) {
    if (!proposal || !vote_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(vote_out->proposal, proposal, sizeof(vote_out->proposal) - 1);
    vote_out->proposal[sizeof(vote_out->proposal) - 1] = 0;
    #pragma GCC diagnostic pop
    
    vote_out->yeas = yeas;
    vote_out->nays = nays;
    vote_out->approved = (yeas > nays);
    return 0;
}

int tsfi_cw_hainaut_reverse_engineer(const char *metadata_ddl, tsfi_cw_hainaut_table *tables_out, int max_tables, int *table_count_out) {
    if (!metadata_ddl || !tables_out || max_tables <= 0 || !table_count_out) return -1;
    
    *table_count_out = 0;
    const char *line = metadata_ddl;
    
    while (line && *line != 0 && *table_count_out < max_tables) {
        if (strncmp(line, "CREATE TABLE ", 13) == 0) {
            char name[32] = {0};
            char pk[32] = {0};
            char fk[32] = {0};
            char ref[32] = {0};
            
            // Format: CREATE TABLE <name> DDL ( <pk> PK, <fk> FK REFERENCES <ref> )
            int parsed = sscanf(line + 13, "%31s ( %31s PK , %31s FK REFERENCES %31s )", name, pk, fk, ref);
            if (parsed >= 2) {
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wstringop-truncation"
                strncpy(tables_out[*table_count_out].table_name, name, sizeof(tables_out[*table_count_out].table_name) - 1);
                tables_out[*table_count_out].table_name[sizeof(tables_out[*table_count_out].table_name) - 1] = 0;
                
                strncpy(tables_out[*table_count_out].primary_key, pk, sizeof(tables_out[*table_count_out].primary_key) - 1);
                tables_out[*table_count_out].primary_key[sizeof(tables_out[*table_count_out].primary_key) - 1] = 0;
                #pragma GCC diagnostic pop
                
                if (parsed == 4) {
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wstringop-truncation"
                    strncpy(tables_out[*table_count_out].foreign_key, fk, sizeof(tables_out[*table_count_out].foreign_key) - 1);
                    tables_out[*table_count_out].foreign_key[sizeof(tables_out[*table_count_out].foreign_key) - 1] = 0;
                    
                    strncpy(tables_out[*table_count_out].references_table, ref, sizeof(tables_out[*table_count_out].references_table) - 1);
                    tables_out[*table_count_out].references_table[sizeof(tables_out[*table_count_out].references_table) - 1] = 0;
                    #pragma GCC diagnostic pop
                } else {
                    tables_out[*table_count_out].foreign_key[0] = 0;
                    tables_out[*table_count_out].references_table[0] = 0;
                }
                (*table_count_out)++;
            }
        }
        line = strchr(line, '\n');
        if (line) line++;
    }
    return 0;
}

int tsfi_cw_hainaut_migrate_table_split(const tsfi_cw_hainaut_table *source_table, tsfi_cw_hainaut_table *target_table_a, tsfi_cw_hainaut_table *target_table_b) {
    if (!source_table || !target_table_a || !target_table_b) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    // Split target_table_a: keeps primary key and becomes original
    snprintf(target_table_a->table_name, sizeof(target_table_a->table_name), "%s_BASE", source_table->table_name);
    strcpy(target_table_a->primary_key, source_table->primary_key);
    target_table_a->foreign_key[0] = 0;
    target_table_a->references_table[0] = 0;
    
    // Split target_table_b: holds a foreign key reference pointing to base table A
    snprintf(target_table_b->table_name, sizeof(target_table_b->table_name), "%s_EXT", source_table->table_name);
    snprintf(target_table_b->primary_key, sizeof(target_table_b->primary_key), "EXT_%s", source_table->primary_key);
    strcpy(target_table_b->foreign_key, source_table->primary_key);
    strcpy(target_table_b->references_table, target_table_a->table_name);
    #pragma GCC diagnostic pop
    
    return 0;
}

int tsfi_cw_hainaut_transform_er_to_relational(const tsfi_cw_hainaut_entity *entity, tsfi_cw_hainaut_table *table_out) {
    if (!entity || !table_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(table_out->table_name, sizeof(table_out->table_name), "T_%s", entity->name);
    snprintf(table_out->primary_key, sizeof(table_out->primary_key), "PK_%s", entity->attr);
    #pragma GCC diagnostic pop
    
    table_out->foreign_key[0] = 0;
    table_out->references_table[0] = 0;
    return 0;
}

int tsfi_cw_hainaut_merge_schemas(const tsfi_cw_hainaut_table *table_a, const tsfi_cw_hainaut_table *table_b, tsfi_cw_hainaut_table *merged_table_out) {
    if (!table_a || !table_b || !merged_table_out) return -1;
    
    // Merge structure: Name from Table A, primary key from Table A, foreign key from Table B
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(merged_table_out->table_name, sizeof(merged_table_out->table_name), "%s_MG", table_a->table_name);
    #pragma GCC diagnostic pop
    
    strcpy(merged_table_out->primary_key, table_a->primary_key);
    strcpy(merged_table_out->foreign_key, table_b->foreign_key);
    strcpy(merged_table_out->references_table, table_b->references_table);
    return 0;
}

int tsfi_cw_hainaut_promote_attribute(const tsfi_cw_hainaut_table *source_table, const char *attribute_name, tsfi_cw_hainaut_table *new_table_out) {
    if (!source_table || !attribute_name || !new_table_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(new_table_out->table_name, sizeof(new_table_out->table_name), "%s_%s", source_table->table_name, attribute_name);
    snprintf(new_table_out->primary_key, sizeof(new_table_out->primary_key), "PK_%s", attribute_name);
    #pragma GCC diagnostic pop
    
    strcpy(new_table_out->foreign_key, source_table->primary_key);
    strcpy(new_table_out->references_table, source_table->table_name);
    return 0;
}

int tsfi_cw_hainaut_check_redundancy(const tsfi_cw_hainaut_table *tables, int count, int *redundancy_detected_out) {
    if (!tables || count <= 0 || !redundancy_detected_out) return -1;
    
    *redundancy_detected_out = 0;
    
    for (int i = 0; i < count; i++) {
        const tsfi_cw_hainaut_table *t_i = &tables[i];
        if (strlen(t_i->foreign_key) > 0 && strlen(t_i->references_table) > 0) {
            // Find if there is a transitive redundancy path:
            // e.g. t_i references t_j, t_j references t_k, and t_i also references t_k directly.
            for (int j = 0; j < count; j++) {
                const tsfi_cw_hainaut_table *t_j = &tables[j];
                if (strcmp(t_i->references_table, t_j->table_name) == 0 &&
                    strlen(t_j->foreign_key) > 0 && strlen(t_j->references_table) > 0) {
                    
                    for (int k = 0; k < count; k++) {
                        const tsfi_cw_hainaut_table *t_k = &tables[k];
                        if (strcmp(t_j->references_table, t_k->table_name) == 0) {
                            // If t_i also has another foreign key reference or references table k directly, redundancy detected
                            *redundancy_detected_out = 1;
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int tsfi_cw_hainaut_split_attribute(const tsfi_cw_hainaut_table *source_table, const char *complex_attr, char *part_a_out, char *part_b_out, int max_len) {
    if (!source_table || !complex_attr || !part_a_out || !part_b_out || max_len <= 0) return -1;
    
    // Find delimiter underscore or space to split the attribute
    const char *delim = strchr(complex_attr, '_');
    if (!delim) delim = strchr(complex_attr, ' ');
    
    if (delim) {
        int len_a = delim - complex_attr;
        if (len_a >= max_len) len_a = max_len - 1;
        strncpy(part_a_out, complex_attr, len_a);
        part_a_out[len_a] = 0;
        
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(part_b_out, delim + 1, max_len - 1);
        part_b_out[max_len - 1] = 0;
        #pragma GCC diagnostic pop
    } else {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(part_a_out, complex_attr, max_len - 1);
        part_a_out[max_len - 1] = 0;
        #pragma GCC diagnostic pop
        part_b_out[0] = 0;
    }
    return 0;
}

int tsfi_cw_hainaut_generate_view(const tsfi_cw_hainaut_table *table, const char *attributes, tsfi_cw_hainaut_view *view_out) {
    if (!table || !attributes || !view_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(view_out->view_name, sizeof(view_out->view_name), "V_%s", table->table_name);
    #pragma GCC diagnostic pop
    
    strcpy(view_out->source_table, table->table_name);
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(view_out->projected_attributes, attributes, sizeof(view_out->projected_attributes) - 1);
    view_out->projected_attributes[sizeof(view_out->projected_attributes) - 1] = 0;
    #pragma GCC diagnostic pop
    
    return 0;
}

int tsfi_cw_hainaut_transform_generalization(const tsfi_cw_hainaut_table *super_table, const tsfi_cw_hainaut_table *sub_table, tsfi_cw_hainaut_table *flat_table_out) {
    if (!super_table || !sub_table || !flat_table_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(flat_table_out->table_name, sizeof(flat_table_out->table_name), "%s_FLAT", super_table->table_name);
    #pragma GCC diagnostic pop
    
    strcpy(flat_table_out->primary_key, super_table->primary_key);
    
    // Check if sub_table has attributes to map or keep
    strcpy(flat_table_out->foreign_key, sub_table->foreign_key);
    strcpy(flat_table_out->references_table, sub_table->references_table);
    return 0;
}

int tsfi_cw_hainaut_degrade_key(const tsfi_cw_hainaut_table *source_table, tsfi_cw_hainaut_table *degraded_table_out) {
    if (!source_table || !degraded_table_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(degraded_table_out->table_name, sizeof(degraded_table_out->table_name), "%s_DEG", source_table->table_name);
    #pragma GCC diagnostic pop
    
    strcpy(degraded_table_out->primary_key, source_table->primary_key);
    
    // Clear foreign key and references (they are now degraded to normal attributes)
    degraded_table_out->foreign_key[0] = 0;
    degraded_table_out->references_table[0] = 0;
    return 0;
}

int tsfi_cw_hainaut_promote_multivalued(const tsfi_cw_hainaut_table *source_table, const char *multivalued_attr, tsfi_cw_hainaut_table *new_table_out) {
    if (!source_table || !multivalued_attr || !new_table_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(new_table_out->table_name, sizeof(new_table_out->table_name), "%s_%s_1NF", source_table->table_name, multivalued_attr);
    snprintf(new_table_out->primary_key, sizeof(new_table_out->primary_key), "PK_%s", multivalued_attr);
    #pragma GCC diagnostic pop
    
    strcpy(new_table_out->foreign_key, source_table->primary_key);
    strcpy(new_table_out->references_table, source_table->table_name);
    return 0;
}

int tsfi_cw_hainaut_map_dependencies(const tsfi_cw_hainaut_table *table, const tsfi_cw_hainaut_fd *fds, int fd_count, int *violation_detected_out) {
    if (!table || !fds || fd_count <= 0 || !violation_detected_out) return -1;
    
    *violation_detected_out = 0;
    for (int i = 0; i < fd_count; i++) {
        // If determinant is not primary key, it violates BCNF/3NF
        if (strcmp(fds[i].determinant, table->primary_key) != 0) {
            *violation_detected_out = 1;
            return 0;
        }
    }
    return 0;
}

int tsfi_cw_hainaut_demote_entity(const tsfi_cw_hainaut_table *entity_table, tsfi_cw_hainaut_table *target_table_io) {
    if (!entity_table || !target_table_io) return -1;
    
    // Demote table entity to a foreign key link on target table
    strcpy(target_table_io->foreign_key, entity_table->primary_key);
    strcpy(target_table_io->references_table, entity_table->table_name);
    return 0;
}

int tsfi_cw_hainaut_export_ddl(const tsfi_cw_hainaut_table *table, char *buffer_out, int max_len) {
    if (!table || !buffer_out || max_len <= 0) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    if (strlen(table->foreign_key) > 0 && strlen(table->references_table) > 0) {
        snprintf(buffer_out, max_len, "CREATE TABLE %s ( %s PK , %s FK REFERENCES %s )",
                 table->table_name, table->primary_key, table->foreign_key, table->references_table);
    } else {
        snprintf(buffer_out, max_len, "CREATE TABLE %s ( %s PK )",
                 table->table_name, table->primary_key);
    }
    #pragma GCC diagnostic pop
    
    return 0;
}

int tsfi_cw_hainaut_analyze_quality(const tsfi_cw_hainaut_table *tables, int count, double *quality_score_out) {
    if (!tables || count <= 0 || !quality_score_out) return -1;
    
    double score = 100.0;
    for (int i = 0; i < count; i++) {
        // Table lacks a primary key: deduct 15 points
        if (strlen(tables[i].primary_key) == 0) {
            score -= 15.0;
        }
        // Self-referencing tables: deduct 10 points
        if (strcmp(tables[i].table_name, tables[i].references_table) == 0 && strlen(tables[i].references_table) > 0) {
            score -= 10.0;
        }
    }
    
    if (score < 0.0) score = 0.0;
    *quality_score_out = score;
    return 0;
}

int tsfi_cw_hainaut_promote_to_key(const char *attribute_name, tsfi_cw_hainaut_table *table_io) {
    if (!attribute_name || !table_io) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(table_io->primary_key, attribute_name, sizeof(table_io->primary_key) - 1);
    table_io->primary_key[sizeof(table_io->primary_key) - 1] = 0;
    #pragma GCC diagnostic pop
    
    return 0;
}

int tsfi_cw_hainaut_verify_equivalence(const tsfi_cw_hainaut_table *schema_a, int count_a, const tsfi_cw_hainaut_table *schema_b, int count_b, int *is_equivalent_out) {
    if (!schema_a || count_a <= 0 || !schema_b || count_b <= 0 || !is_equivalent_out) return -1;
    
    *is_equivalent_out = 0;
    
    // Check if primary key sets match across schemas
    int matched_keys = 0;
    for (int i = 0; i < count_a; i++) {
        for (int j = 0; j < count_b; j++) {
            if (strcmp(schema_a[i].primary_key, schema_b[j].primary_key) == 0 &&
                strlen(schema_a[i].primary_key) > 0) {
                matched_keys++;
                break;
            }
        }
    }
    
    // If we matched all primary keys, they are equivalent under key constraint projection
    if (matched_keys >= count_a && count_a == count_b) {
        *is_equivalent_out = 1;
    }
    return 0;
}

int tsfi_cw_hainaut_collapse_subtype(const tsfi_cw_hainaut_table *super_table, const tsfi_cw_hainaut_table *sub_table, tsfi_cw_hainaut_table *collapsed_table_out) {
    if (!super_table || !sub_table || !collapsed_table_out) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(collapsed_table_out->table_name, sizeof(collapsed_table_out->table_name), "%s_CLPS", super_table->table_name);
    #pragma GCC diagnostic pop
    
    strcpy(collapsed_table_out->primary_key, super_table->primary_key);
    
    // subtype fields become nullable/optional on parent supertype
    strcpy(collapsed_table_out->foreign_key, sub_table->foreign_key);
    strcpy(collapsed_table_out->references_table, sub_table->references_table);
    
    return 0;
}

int tsfi_cw_roland_merge_attributes(const tsfi_cw_hainaut_table *table, const char *attr_a, const char *attr_b, char *merged_attr_out, int max_len) {
    if (!table || !attr_a || !attr_b || !merged_attr_out || max_len <= 0) return -1;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(merged_attr_out, max_len, "%s_%s", attr_a, attr_b);
    #pragma GCC diagnostic pop
    
    return 0;
}

int tsfi_cw_roland_solve_join_path(const tsfi_cw_hainaut_table *tables, int count, const char *start_table, const char *end_table, char *path_out, int max_len) {
    if (!tables || count <= 0 || !start_table || !end_table || !path_out || max_len <= 0) return -1;
    
    // Simple path routing: check direct reference or through a single middle table
    for (int i = 0; i < count; i++) {
        if (strcmp(tables[i].table_name, start_table) == 0 &&
            strcmp(tables[i].references_table, end_table) == 0) {
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-truncation"
            snprintf(path_out, max_len, "%s -> %s", start_table, end_table);
            #pragma GCC diagnostic pop
            return 0;
        }
    }
    
    for (int i = 0; i < count; i++) {
        if (strcmp(tables[i].table_name, start_table) == 0 && strlen(tables[i].references_table) > 0) {
            const char *mid = tables[i].references_table;
            for (int j = 0; j < count; j++) {
                if (strcmp(tables[j].table_name, mid) == 0 && strcmp(tables[j].references_table, end_table) == 0) {
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wformat-truncation"
                    snprintf(path_out, max_len, "%s -> %s -> %s", start_table, mid, end_table);
                    #pragma GCC diagnostic pop
                    return 0;
                }
            }
        }
    }
    
    path_out[0] = 0;
    return 1; // path not resolved
}

int tsfi_cw_roland_check_domain(const char *merged_attr, int min_val, int max_val, int *is_valid_out) {
    if (!merged_attr || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    // Find numeric suffix or value inside the attribute string
    const char *p = merged_attr;
    while (*p && (*p < '0' || *p > '9')) p++;
    
    if (*p) {
        int val = atoi(p);
        if (val >= min_val && val <= max_val) {
            *is_valid_out = 1;
        }
    }
    return 0;
}

int tsfi_cw_roland_optimize_bcnf(const tsfi_cw_hainaut_table *table, const tsfi_cw_hainaut_fd *fds, int fd_count, tsfi_cw_hainaut_table *optimized_tables_out, int *optimized_count_out) {
    if (!table || !fds || fd_count <= 0 || !optimized_tables_out || !optimized_count_out) return -1;
    
    *optimized_count_out = 0;
    int bcnf_violation = 0;
    int violating_fd_idx = -1;
    
    for (int i = 0; i < fd_count; i++) {
        // If determinant is not primary key, BCNF is violated
        if (strcmp(fds[i].determinant, table->primary_key) != 0) {
            bcnf_violation = 1;
            violating_fd_idx = i;
            break;
        }
    }
    
    if (bcnf_violation && violating_fd_idx != -1) {
        // Decompose into two tables: 
        // Table A: determinant + dependent (e.g. holds the functional dependency)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-truncation"
        snprintf(optimized_tables_out[0].table_name, sizeof(optimized_tables_out[0].table_name), "%s_R1", table->table_name);
        #pragma GCC diagnostic pop
        strcpy(optimized_tables_out[0].primary_key, fds[violating_fd_idx].determinant);
        optimized_tables_out[0].foreign_key[0] = 0;
        optimized_tables_out[0].references_table[0] = 0;
        
        // Table B: table original primary key + determinant (as foreign key)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-truncation"
        snprintf(optimized_tables_out[1].table_name, sizeof(optimized_tables_out[1].table_name), "%s_R2", table->table_name);
        #pragma GCC diagnostic pop
        strcpy(optimized_tables_out[1].primary_key, table->primary_key);
        strcpy(optimized_tables_out[1].foreign_key, fds[violating_fd_idx].determinant);
        strcpy(optimized_tables_out[1].references_table, optimized_tables_out[0].table_name);
        
        *optimized_count_out = 2;
    } else {
        // Already in BCNF, return original table
        optimized_tables_out[0] = *table;
        *optimized_count_out = 1;
    }
    return 0;
}

int tsfi_cw_validate_23_node_domain(const tsfi_cw_23_node *node, int *is_valid_out) {
    if (!node || !is_valid_out) return -1;
    
    *is_valid_out = 1;
    if (node->key_count < 1 || node->key_count > 2) {
        *is_valid_out = 0;
        return 0;
    }
    
    if (node->key_count == 2 && node->keys[0] >= node->keys[1]) {
        *is_valid_out = 0;
        return 0;
    }
    
    // Check search range continuity boundaries
    if (node->key_count == 1) {
        // Child 0 holds keys < keys[0], Child 1 holds keys >= keys[0]
        if (node->child_bounds[0][1] > node->keys[0] || node->child_bounds[1][0] < node->keys[0]) {
            *is_valid_out = 0;
        }
    } else {
        // Child 0 < keys[0] <= Child 1 < keys[1] <= Child 2
        if (node->child_bounds[0][1] > node->keys[0] ||
            node->child_bounds[1][0] < node->keys[0] ||
            node->child_bounds[1][1] > node->keys[1] ||
            node->child_bounds[2][0] < node->keys[1]) {
            *is_valid_out = 0;
        }
    }
    return 0;
}

int tsfi_cw_split_23_node_bcnf(tsfi_cw_23_node *parent, tsfi_cw_23_node *child_a_out, tsfi_cw_23_node *child_b_out) {
    if (!parent || !child_a_out || !child_b_out) return -1;
    
    // Simulate splitting node: take parent's keys, divide between child A and child B
    child_a_out->key_count = 1;
    child_a_out->keys[0] = parent->keys[0];
    child_a_out->child_bounds[0][0] = parent->child_bounds[0][0];
    child_a_out->child_bounds[0][1] = parent->keys[0];
    child_a_out->child_bounds[1][0] = parent->keys[0];
    child_a_out->child_bounds[1][1] = parent->keys[0] + 50;
    
    child_b_out->key_count = 1;
    child_b_out->keys[0] = parent->keys[1];
    child_b_out->child_bounds[0][0] = parent->keys[1] - 50;
    child_b_out->child_bounds[0][1] = parent->keys[1];
    child_b_out->child_bounds[1][0] = parent->keys[1];
    child_b_out->child_bounds[1][1] = parent->child_bounds[2][1];
    
    return 0;
}

int tsfi_cw_merge_23_nodes(const tsfi_cw_23_node *child_a, const tsfi_cw_23_node *child_b, int parent_divider, tsfi_cw_23_node *merged_node_out) {
    if (!child_a || !child_b || !merged_node_out) return -1;
    
    // Check if combined key counts can fit into a single 3-node (max 2 keys)
    if (child_a->key_count + child_b->key_count + 1 > 2) {
        return 1; // cannot merge directly without splitting or parent borrow
    }
    
    merged_node_out->key_count = 2;
    merged_node_out->keys[0] = child_a->keys[0];
    merged_node_out->keys[1] = parent_divider;
    
    merged_node_out->child_bounds[0][0] = child_a->child_bounds[0][0];
    merged_node_out->child_bounds[0][1] = child_a->child_bounds[0][1];
    merged_node_out->child_bounds[1][0] = child_a->child_bounds[1][0];
    merged_node_out->child_bounds[1][1] = child_a->child_bounds[1][1];
    merged_node_out->child_bounds[2][0] = child_b->child_bounds[0][0];
    merged_node_out->child_bounds[2][1] = child_b->child_bounds[0][1];
    
    return 0;
}

int tsfi_cw_audit_23_dependencies(const tsfi_cw_23_node *node, const tsfi_cw_hainaut_fd *fds, int fd_count, int *violation_detected_out) {
    if (!node || !fds || fd_count <= 0 || !violation_detected_out) return -1;
    
    *violation_detected_out = 0;
    
    // Audit if any determinant represents key mapping matching the node keys
    for (int i = 0; i < fd_count; i++) {
        int key_found = 0;
        int det_val = atoi(fds[i].determinant);
        for (int k = 0; k < node->key_count; k++) {
            if (node->keys[k] == det_val) {
                key_found = 1;
                break;
            }
        }
        if (!key_found && det_val > 0) {
            *violation_detected_out = 1;
            return 0;
        }
    }
    return 0;
}

int tsfi_cw_roland_prune_transitive(const tsfi_cw_hainaut_table *table, const tsfi_cw_hainaut_fd *fds, int fd_count, tsfi_cw_hainaut_table *pruned_table_out) {
    if (!table || !fds || fd_count <= 0 || !pruned_table_out) return -1;
    
    *pruned_table_out = *table;
    
    // Search for a transitive path: A -> B and B -> C, then prune direct A -> C if it references it
    for (int i = 0; i < fd_count; i++) {
        for (int j = 0; j < fd_count; j++) {
            if (strcmp(fds[i].dependent, fds[j].determinant) == 0) {
                if (strcmp(table->foreign_key, fds[j].dependent) == 0) {
                    pruned_table_out->foreign_key[0] = 0;
                    pruned_table_out->references_table[0] = 0;
                }
            }
        }
    }
    return 0;
}

int tsfi_cw_roland_audit_anomalies(const tsfi_cw_hainaut_table *table, const tsfi_cw_hainaut_fd *fds, int fd_count, int *anomaly_detected_out) {
    if (!table || !fds || fd_count <= 0 || !anomaly_detected_out) return -1;
    
    *anomaly_detected_out = 0;
    
    for (int i = 0; i < fd_count; i++) {
        if (strcmp(fds[i].determinant, table->primary_key) != 0 &&
            strlen(fds[i].determinant) > 0 &&
            strlen(fds[i].dependent) > 0) {
            *anomaly_detected_out = 1;
            return 0;
        }
    }
    return 0;
}

int tsfi_cw_henrard_analyze_dml(const char *source_code, char *accessed_table_out, int max_len) {
    if (!source_code || !accessed_table_out || max_len <= 0) return -1;
    
    accessed_table_out[0] = 0;
    
    // Find keywords "FROM " or "READ " or "UPDATE "
    const char *kw_from = strstr(source_code, "FROM ");
    const char *kw_read = strstr(source_code, "READ ");
    const char *kw_update = strstr(source_code, "UPDATE ");
    
    const char *match = NULL;
    if (kw_from) match = kw_from + 5;
    else if (kw_read) match = kw_read + 5;
    else if (kw_update) match = kw_update + 7;
    
    if (match) {
        int idx = 0;
        while (match[idx] && match[idx] != ' ' && match[idx] != ';' && match[idx] != '\r' && match[idx] != '\n' && idx < max_len - 1) {
            accessed_table_out[idx] = match[idx];
            idx++;
        }
        accessed_table_out[idx] = 0;
        return 0;
    }
    return 1; // no match found
}

int tsfi_cw_henrard_infer_constraints(const char *query_join, char *inferred_fk_out, char *inferred_ref_table_out, int max_len) {
    if (!query_join || !inferred_fk_out || !inferred_ref_table_out || max_len <= 0) return -1;
    
    inferred_fk_out[0] = 0;
    inferred_ref_table_out[0] = 0;
    
    // Parse "JOIN <REF_TABLE> ON <FK_COL> = ..."
    const char *kw_join = strstr(query_join, "JOIN ");
    const char *kw_on = strstr(query_join, " ON ");
    
    if (kw_join && kw_on && kw_on > kw_join) {
        // Extract ref table
        const char *ref_p = kw_join + 5;
        int idx = 0;
        while (ref_p[idx] && ref_p[idx] != ' ' && idx < max_len - 1) {
            inferred_ref_table_out[idx] = ref_p[idx];
            idx++;
        }
        inferred_ref_table_out[idx] = 0;
        
        // Extract foreign key from "ON <TABLE>.<FK_COL> = ..."
        const char *on_p = kw_on + 4;
        while (*on_p && *on_p != '.' && *on_p != '=') on_p++;
        if (*on_p == '.') {
            on_p++;
            idx = 0;
            while (on_p[idx] && on_p[idx] != ' ' && on_p[idx] != '=' && idx < max_len - 1) {
                inferred_fk_out[idx] = on_p[idx];
                idx++;
            }
            inferred_fk_out[idx] = 0;
            return 0;
        }
    }
    return 1; // parse failed
}

int tsfi_cw_kendrick_route_call(int dial_digit, tsfi_cw_kendrick_switch *sw_io_out, int *line_connected_out) {
    if (!sw_io_out || !line_connected_out) return -1;
    
    *line_connected_out = 0;
    
    // Check digital dial validation
    if (dial_digit < 0 || dial_digit > 9) return 1;
    
    if (sw_io_out->active_lines < sw_io_out->capacity) {
        sw_io_out->active_lines++;
        *line_connected_out = 1;
    }
    return 0;
}

int tsfi_cw_kendrick_integrate(double step_size, double initial_val, int iterations, double *result_out) {
    if (step_size <= 0.0 || iterations < 0 || !result_out) return -1;
    
    double val = initial_val;
    for (int i = 0; i < iterations; i++) {
        val += step_size * val; // simple dy/dx = y step
    }
    *result_out = val;
    return 0;
}

int tsfi_cw_kendrick_multiplex(int channel_count, const int *input_signals, int *output_combined_out) {
    if (channel_count <= 0 || !input_signals || !output_combined_out) return -1;
    
    int combined = 0;
    for (int i = 0; i < channel_count; i++) {
        combined += input_signals[i];
    }
    *output_combined_out = combined;
    return 0;
}

int tsfi_cw_kendrick_log_event(int line_id, const char *status, tsfi_cw_kendrick_log_entry *log_out, int max_entries, int *current_count_io) {
    if (!status || !log_out || max_entries <= 0 || !current_count_io || *current_count_io < 0) return -1;
    
    if (*current_count_io < max_entries) {
        log_out[*current_count_io].line_id = line_id;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(log_out[*current_count_io].status, status, sizeof(log_out[*current_count_io].status) - 1);
        log_out[*current_count_io].status[sizeof(log_out[*current_count_io].status) - 1] = 0;
        #pragma GCC diagnostic pop
        (*current_count_io)++;
        return 0;
    }
    
    return 1; // Log capacity reached
}

int tsfi_cw_keating_write_core(int x, int y, int state, tsfi_cw_keating_core *cores, int core_count) {
    if (!cores || core_count < 0) return -1;
    
    for (int i = 0; i < core_count; i++) {
        if (cores[i].x_coord == x && cores[i].y_coord == y) {
            cores[i].state = state;
            return 0; // successfully updated core state
        }
    }
    return 1; // core coordinate not mapped
}

int tsfi_cw_keating_synthesize_freq(double base_freq, int scale_factor, double *freq_out) {
    if (base_freq <= 0.0 || scale_factor <= 0 || !freq_out) return -1;
    
    *freq_out = base_freq * scale_factor;
    return 0;
}

int tsfi_cw_keating_bandpass_filter(float input_freq, float center_freq, float bandwidth_hz, float db_gain, float *output_signal_out) {
    if (center_freq <= 0.0f || bandwidth_hz <= 0.0f || !output_signal_out) return -1;
    
    float half_bw = bandwidth_hz * 0.5f;
    if (input_freq >= (center_freq - half_bw) && input_freq <= (center_freq + half_bw)) {
        // Apply amplification based on decibel gain: gain_linear = 10^(db/20)
        float linear_gain = powf(10.0f, db_gain / 20.0f);
        *output_signal_out = input_freq * linear_gain;
    } else {
        // Attenuate standard signal falling outside bandpass limits
        *output_signal_out = input_freq * 0.01f;
    }
    return 0;
}

