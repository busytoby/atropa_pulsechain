#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int tsfi_cw_icp_register_product(tsfi_cw_icp_product *catalog, int *catalog_size, const tsfi_cw_icp_product *new_prod) {
    if (!catalog || !catalog_size || !new_prod) return -1;
    
    // Check if exists
    for (int i = 0; i < *catalog_size; i++) {
        if (strcmp(catalog[i].product_id, new_prod->product_id) == 0) {
            catalog[i] = *new_prod;
            return 0;
        }
    }
    
    // Append new
    if (*catalog_size >= 32) return -3;
    catalog[*catalog_size] = *new_prod;
    (*catalog_size)++;
    return 0;
}

int tsfi_cw_icp_check_award(const tsfi_cw_icp_product *prod, tsfi_cw_icp_award_status *status_out) {
    if (!prod || !status_out) return -1;
    
    status_out->total_revenue = prod->unit_price * prod->install_count;
    status_out->qualifies_for_million_dollar_award = (status_out->total_revenue >= 1000000.0) ? 1 : 0;
    
    return 0;
}

int tsfi_cw_icp_process_agent_sale(tsfi_cw_icp_agent *agent, const char *sale_card_line) {
    if (!agent || !sale_card_line) return -1;
    if (strlen(sale_card_line) < 13) return -2;
    
    char agent_id[7];
    memcpy(agent_id, sale_card_line, 6);
    agent_id[6] = '\0';
    if (strcmp(agent->agent_id, agent_id) != 0) return -3;
    
    char amt_str[7];
    memcpy(amt_str, sale_card_line + 7, 6);
    amt_str[6] = '\0';
    double sale_amt = atof(amt_str);
    
    agent->total_sales += sale_amt;
    
    // Tiered commission: 5% up to 50,000, 10% on remainder
    if (agent->total_sales <= 50000.0) {
        agent->commission_earned = agent->total_sales * 0.05;
    } else {
        agent->commission_earned = (50000.0 * 0.05) + ((agent->total_sales - 50000.0) * 0.10);
    }
    
    agent->total_payout = agent->base_salary + agent->commission_earned;
    return 0;
}

int tsfi_cw_icp_audit_contract(const tsfi_cw_icp_contract *contract, double *total_value_out, double *remaining_value_out) {
    if (!contract || !total_value_out || !remaining_value_out) return -1;
    
    *total_value_out = contract->monthly_fee * contract->contract_months;
    
    int rem_months = contract->contract_months - contract->months_elapsed;
    if (rem_months < 0) rem_months = 0;
    *remaining_value_out = contract->monthly_fee * rem_months;
    
    return 0;
}

int tsfi_cw_icp_audit_compatibility(const tsfi_cw_icp_product *prod, const char *target_hardware, int *is_compatible_out) {
    if (!prod || !target_hardware || !is_compatible_out) return -1;
    
    if (strstr(target_hardware, prod->hardware_platform) || strstr(prod->hardware_platform, target_hardware)) {
        *is_compatible_out = 1;
    } else {
        *is_compatible_out = 0;
    }
    return 0;
}

int tsfi_cw_icp_distribute_royalties(const tsfi_cw_icp_product *prod, double rate, double *royalty_out) {
    if (!prod || !royalty_out || rate < 0.0 || rate > 1.0) return -1;
    
    double revenue = prod->unit_price * prod->install_count;
    *royalty_out = revenue * rate;
    return 0;
}

int tsfi_cw_icp_search_directory(const tsfi_cw_icp_product *catalog, int catalog_size, const char *hardware_filter, tsfi_cw_icp_product *results_out, int *results_count_out) {
    if (!catalog || catalog_size < 0 || !results_out || !results_count_out) return -1;
    
    *results_count_out = 0;
    for (int i = 0; i < catalog_size; i++) {
        if (!hardware_filter || strlen(hardware_filter) == 0 ||
            strstr(catalog[i].hardware_platform, hardware_filter) ||
            strstr(hardware_filter, catalog[i].hardware_platform)) {
            results_out[*results_count_out] = catalog[i];
            (*results_count_out)++;
        }
    }
    return 0;
}

int tsfi_cw_icp_migration_audit(const tsfi_cw_icp_product *old_prod, const tsfi_cw_icp_product *new_prod, int *upgrade_allowed_out) {
    if (!old_prod || !new_prod || !upgrade_allowed_out) return -1;
    
    if (strcmp(old_prod->vendor, new_prod->vendor) == 0 && new_prod->unit_price >= old_prod->unit_price) {
        *upgrade_allowed_out = 1;
    } else {
        *upgrade_allowed_out = 0;
    }
    return 0;
}

int tsfi_cw_icp_audit_license_compliance(int purchased, int active, int *warning_out) {
    if (!warning_out) return -1;
    *warning_out = (active > purchased) ? 1 : 0;
    return 0;
}

int tsfi_cw_icp_track_quota(double target, double actual, double *bonus_payout_out) {
    if (!bonus_payout_out || target <= 0.0) return -1;
    
    if (actual < target) {
        *bonus_payout_out = 0.0;
    } else if (actual < target * 1.20) {
        *bonus_payout_out = actual * 0.02;
    } else {
        *bonus_payout_out = actual * 0.05;
    }
    return 0;
}

int tsfi_cw_icp_calculate_bundle_price(const tsfi_cw_icp_product *products, int count, double discount_rate, double *total_price_out) {
    if (!products || count <= 0 || !total_price_out || discount_rate < 0.0 || discount_rate > 1.0) return -1;
    
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += products[i].unit_price;
    }
    *total_price_out = sum * (1.0 - discount_rate);
    return 0;
}

int tsfi_cw_icp_subscription_status(const tsfi_cw_icp_subscription *sub, double *total_paid_out, double *remaining_obligations_out) {
    if (!sub || !total_paid_out || !remaining_obligations_out) return -1;
    
    *total_paid_out = sub->monthly_rate * sub->cycles_paid;
    
    int rem = sub->billing_cycles_total - sub->cycles_paid;
    if (rem < 0) rem = 0;
    *remaining_obligations_out = sub->monthly_rate * rem;
    return 0;
}

int tsfi_cw_icp_analyze_vendor(const tsfi_cw_icp_product *catalog, int catalog_size, const char *vendor_name, double *total_revenue_out, int *award_count_out) {
    if (!catalog || catalog_size < 0 || !vendor_name || !total_revenue_out || !award_count_out) return -1;
    
    *total_revenue_out = 0.0;
    *award_count_out = 0;
    for (int i = 0; i < catalog_size; i++) {
        if (strcmp(catalog[i].vendor, vendor_name) == 0) {
            double rev = catalog[i].unit_price * catalog[i].install_count;
            *total_revenue_out += rev;
            if (rev >= 1000000.0) {
                (*award_count_out)++;
            }
        }
    }
    return 0;
}

int tsfi_cw_icp_calculate_transfer_tax(double license_amount, double tax_rate, double *tax_amount_out) {
    if (license_amount < 0.0 || tax_rate < 0.0 || !tax_amount_out) return -1;
    *tax_amount_out = license_amount * tax_rate;
    return 0;
}

int tsfi_cw_icp_calculate_software_depreciation(double cost, double salvage_value, int lifespan_years, int target_year, double *depreciated_value_out) {
    if (cost < 0.0 || salvage_value < 0.0 || lifespan_years <= 0 || target_year < 0 || !depreciated_value_out) return -1;
    
    if (target_year >= lifespan_years) {
        *depreciated_value_out = salvage_value;
    } else {
        double annual_dep = (cost - salvage_value) / lifespan_years;
        *depreciated_value_out = cost - (annual_dep * target_year);
    }
    return 0;
}

int tsfi_cw_icp_calculate_renewal_penalty(double renewal_fee, int days_late, double penalty_rate_per_day, double *total_due_out) {
    if (renewal_fee < 0.0 || penalty_rate_per_day < 0.0 || !total_due_out) return -1;
    
    if (days_late <= 0) {
        *total_due_out = renewal_fee;
    } else {
        *total_due_out = renewal_fee * (1.0 + ((double)days_late * penalty_rate_per_day));
    }
    return 0;
}

int tsfi_cw_icp_audit_transfer(tsfi_cw_icp_product *prod, const char *new_vendor, double transfer_fee_rate, double *transfer_fee_out) {
    if (!prod || !new_vendor || transfer_fee_rate < 0.0 || !transfer_fee_out) return -1;
    
    *transfer_fee_out = prod->unit_price * transfer_fee_rate;
    strncpy(prod->vendor, new_vendor, sizeof(prod->vendor) - 1);
    prod->vendor[sizeof(prod->vendor) - 1] = '\0';
    return 0;
}

int tsfi_cw_icp_consolidate_royalties(const tsfi_cw_icp_product *catalog, int catalog_size, const char *vendor, double royalty_rate, double *consolidated_payment_out) {
    if (!catalog || catalog_size < 0 || !vendor || royalty_rate < 0.0 || !consolidated_payment_out) return -1;
    
    *consolidated_payment_out = 0.0;
    for (int i = 0; i < catalog_size; i++) {
        if (strcmp(catalog[i].vendor, vendor) == 0) {
            *consolidated_payment_out += (catalog[i].unit_price * catalog[i].install_count * royalty_rate);
        }
    }
    return 0;
}

int tsfi_cw_icp_check_expiration(int current_year, int current_month, int current_day, int exp_year, int exp_month, int exp_day, int *expired_out, int *days_remaining_out) {
    if (!expired_out || !days_remaining_out) return -1;
    
    long long days_cur = (long long)current_year * 365 + (long long)current_month * 30 + current_day;
    long long days_exp = (long long)exp_year * 365 + (long long)exp_month * 30 + exp_day;
    
    *days_remaining_out = (int)(days_exp - days_cur);
    *expired_out = (*days_remaining_out < 0) ? 1 : 0;
    return 0;
}

int tsfi_cw_icp_calculate_payback(double purchase_cost, double annual_savings, double *payback_years_out) {
    if (purchase_cost < 0.0 || annual_savings <= 0.0 || !payback_years_out) return -1;
    *payback_years_out = purchase_cost / annual_savings;
    return 0;
}

int tsfi_cw_icp_register_release(tsfi_cw_icp_release_record *releases, int *release_count, const tsfi_cw_icp_release_record *new_release) {
    if (!releases || !release_count || !new_release) return -1;
    
    // Check if exists
    for (int i = 0; i < *release_count; i++) {
        if (strcmp(releases[i].product_id, new_release->product_id) == 0 &&
            strcmp(releases[i].version_string, new_release->version_string) == 0) {
            releases[i] = *new_release;
            return 0;
        }
    }
    
    // Append new
    if (*release_count >= 32) return -3;
    releases[*release_count] = *new_release;
    (*release_count)++;
    return 0;
}

int tsfi_cw_icp_audit_vendor(const tsfi_cw_icp_vendor_record *vendor, int *is_compliant_out) {
    if (!vendor || !is_compliant_out) return -1;
    
    if (vendor->is_active && (strcmp(vendor->country_code, "US") == 0 || strcmp(vendor->country_code, "UK") == 0)) {
        *is_compliant_out = 1;
    } else {
        *is_compliant_out = 0;
    }
    return 0;
}

int tsfi_cw_icp_assign_category(tsfi_cw_icp_category_map *maps, int *map_count, const char *prod_id, const char *category) {
    if (!maps || !map_count || !prod_id || !category) return -1;
    
    for (int i = 0; i < *map_count; i++) {
        if (strcmp(maps[i].product_id, prod_id) == 0) {
            strncpy(maps[i].category_name, category, sizeof(maps[i].category_name) - 1);
            maps[i].category_name[sizeof(maps[i].category_name) - 1] = '\0';
            return 0;
        }
    }
    
    if (*map_count >= 32) return -3;
    strncpy(maps[*map_count].product_id, prod_id, sizeof(maps[*map_count].product_id) - 1);
    maps[*map_count].product_id[sizeof(maps[*map_count].product_id) - 1] = '\0';
    strncpy(maps[*map_count].category_name, category, sizeof(maps[*map_count].category_name) - 1);
    maps[*map_count].category_name[sizeof(maps[*map_count].category_name) - 1] = '\0';
    (*map_count)++;
    return 0;
}

int tsfi_cw_icp_query_category(const tsfi_cw_icp_category_map *maps, int map_count, const char *category, char ids_out[][8], int *ids_count_out) {
    if (!maps || map_count < 0 || !category || !ids_out || !ids_count_out) return -1;
    
    *ids_count_out = 0;
    for (int i = 0; i < map_count; i++) {
        if (strcmp(maps[i].category_name, category) == 0) {
            strncpy(ids_out[*ids_count_out], maps[i].product_id, 7);
            ids_out[*ids_count_out][7] = '\0';
            (*ids_count_out)++;
        }
    }
    return 0;
}

int tsfi_cw_icp_audit_grace_period(int days_late, int allowed_grace_days, int *suspended_out) {
    if (!suspended_out) return -1;
    *suspended_out = (days_late > allowed_grace_days) ? 1 : 0;
    return 0;
}

int tsfi_cw_icp_calculate_upgrade_price(double current_version_price, double new_version_price, double loyalty_discount_rate, double *upgrade_price_out) {
    if (current_version_price < 0.0 || new_version_price < 0.0 || loyalty_discount_rate < 0.0 || loyalty_discount_rate > 1.0 || !upgrade_price_out) return -1;
    
    double diff = new_version_price - current_version_price;
    if (diff < 0.0) diff = 0.0;
    *upgrade_price_out = diff * (1.0 - loyalty_discount_rate);
    return 0;
}

int tsfi_cw_icp_audit_support_sla(int response_time_mins, int target_sla_mins, double contract_monthly_fee, double *rebate_out) {
    if (response_time_mins < 0 || target_sla_mins <= 0 || contract_monthly_fee < 0.0 || !rebate_out) return -1;
    
    if (response_time_mins > target_sla_mins) {
        *rebate_out = contract_monthly_fee * 0.05;
    } else {
        *rebate_out = 0.0;
    }
    return 0;
}
