#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846

#endif
int tsfi_cobol_sort_merge(const tsfi_cobol_fd *fd, tsfi_ramac_card *cards, int card_count) {
    if (!fd || !cards || card_count <= 0) return -1;
    if (fd->key_start < 0 || fd->key_start + fd->key_len > 80) return -2;
    
    // Bubble sort records based on key substring comparison
    for (int i = 0; i < card_count - 1; i++) {
        for (int j = 0; j < card_count - i - 1; j++) {
            char key_a[80];
            char key_b[80];
            memcpy(key_a, &cards[j].columns[fd->key_start], fd->key_len);
            key_a[fd->key_len] = '\0';
            memcpy(key_b, &cards[j+1].columns[fd->key_start], fd->key_len);
            key_b[fd->key_len] = '\0';
            
            if (strcmp(key_a, key_b) > 0) {
                tsfi_ramac_card temp = cards[j];
                cards[j] = cards[j+1];
                cards[j+1] = temp;
            }
        }
    }
    return 0;
}

void tsfi_cobol_report_writer(const char *title, int total_acc, char *output_report, int max_len) {
    if (!title || !output_report || max_len <= 0) return;
    
    snprintf(output_report, max_len,
             "========================================\n"
             "           COBOL REPORT WRITER          \n"
             "Title: %s\n"
             "----------------------------------------\n"
             "ACCUMULATOR GRAND TOTAL: %-10d\n"
             "========================================\n",
             title, total_acc);
}

int tsfi_codasyl_dml_find(const tsfi_codasyl_subschema *subschema, const tsfi_codasyl_dbtg_set *sets, int set_count, int relation_id) {
    if (!subschema || !sets || set_count <= 0) return -1;
    
    int authorized = 0;
    for (int i = 0; i < 4; i++) {
        if (subschema->allowed_relation_ids[i] == relation_id) {
            authorized = 1;
            break;
        }
    }
    if (!authorized) return -2;
    
    for (int i = 0; i < set_count; i++) {
        if (sets[i].relation_id == relation_id) {
            return i;
        }
    }
    return -3;
}

void tsfi_codasyl_checkpoint_save(const tsfi_codasyl_dbtg_set *sets, int count, tsfi_codasyl_checkpoint *checkpoint) {
    if (!sets || count <= 0 || !checkpoint) return;
    checkpoint->active_relations_count = count > 10 ? 10 : count;
    for (int i = 0; i < checkpoint->active_relations_count; i++) {
        checkpoint->saved_relation_ids[i] = sets[i].relation_id;
    }
}

void tsfi_codasyl_checkpoint_rollback(tsfi_codasyl_dbtg_set *sets, int *count, const tsfi_codasyl_checkpoint *checkpoint) {
    if (!sets || !count || !checkpoint) return;
    *count = checkpoint->active_relations_count;
    for (int i = 0; i < *count; i++) {
        sets[i].relation_id = checkpoint->saved_relation_ids[i];
    }
}

int tsfi_cobol_compute_eval(const char *expression, const int reg_values[8]) {
    if (!expression || !reg_values) return 0;
    while (*expression == ' ' || *expression == '\t') expression++;
    int rx = -1, ry = -1, rz = -1;
    char op1 = '\0', op2 = '\0';
    
    int matched = sscanf(expression, "R%d %c R%d %c R%d", &rx, &op1, &ry, &op2, &rz);
    if (matched == 5) {
        int val1 = (rx >= 0 && rx < 8) ? reg_values[rx] : 0;
        int val2 = (ry >= 0 && ry < 8) ? reg_values[ry] : 0;
        int val3 = (rz >= 0 && rz < 8) ? reg_values[rz] : 0;
        
        if (op1 == '*' && op2 == '+') {
            return val1 * val2 + val3;
        }
    }
    
    matched = sscanf(expression, "R%d %c R%d", &rx, &op1, &ry);
    if (matched == 3) {
        int val1 = (rx >= 0 && rx < 8) ? reg_values[rx] : 0;
        int val2 = (ry >= 0 && ry < 8) ? reg_values[ry] : 0;
        if (op1 == '+') return val1 + val2;
        if (op1 == '-') return val1 - val2;
        if (op1 == '*') return val1 * val2;
    }
    
    return 0;
}

int tsfi_cobol_call_algol_proc(int proc_id, int param) {
    if (proc_id == 1) {
        int res = 1;
        for (int i = 1; i <= param; i++) res *= i;
        return res;
    }
    return 0;
}

void tsfi_ids_init(ids_currency_tracker *tracker) {
    if (!tracker) return;
    tracker->size = 0;
    tracker->current_run_unit = -1;
    for (int i = 0; i < 4; i++) {
        tracker->current_record_type[i] = -1;
        tracker->current_set[i] = -1;
    }
}

int tsfi_ids_insert(ids_currency_tracker *tracker, int record_id, int parent_id, const char *data) {
    if (!tracker || tracker->size >= 16) return -1;
    ids_record *r = &tracker->records[tracker->size];
    r->record_id = record_id;
    r->parent_record_id = parent_id;
    r->next_record_id = record_id;
    if (data) {
        strncpy(r->data, data, sizeof(r->data) - 1);
        r->data[sizeof(r->data) - 1] = '\0';
    }
    
    for (int i = 0; i < tracker->size; i++) {
        if (tracker->records[i].parent_record_id == parent_id) {
            int curr = i;
            while (tracker->records[curr].next_record_id != tracker->records[i].record_id) {
                int next_found = 0;
                for (int k = 0; k < tracker->size; k++) {
                    if (tracker->records[k].record_id == tracker->records[curr].next_record_id) {
                        curr = k;
                        next_found = 1;
                        break;
                    }
                }
                if (!next_found) break;
            }
            r->next_record_id = tracker->records[curr].next_record_id;
            tracker->records[curr].next_record_id = record_id;
            break;
        }
    }
    tracker->current_run_unit = record_id;
    tracker->size++;
    return 0;
}

int tsfi_ids_navigate_next(ids_currency_tracker *tracker, int set_id) {
    if (!tracker || set_id < 0 || set_id >= 4) return -1;
    int curr_id = tracker->current_set[set_id];
    if (curr_id == -1) {
        if (tracker->size > 0) {
            tracker->current_set[set_id] = tracker->records[0].record_id;
            return tracker->records[0].record_id;
        }
        return -1;
    }
    
    for (int i = 0; i < tracker->size; i++) {
        if (tracker->records[i].record_id == curr_id) {
            int next_id = tracker->records[i].next_record_id;
            tracker->current_set[set_id] = next_id;
            tracker->current_run_unit = next_id;
            return next_id;
        }
    }
    return -1;
}

void tsfi_cross_chain_init(cross_chain_tracker *tracker) {
    if (!tracker) return;
    tracker->count = 0;
    for (int i = 0; i < 4; i++) {
        tracker->current_chain_token[i] = -1;
        tracker->current_cross_token[i] = -1;
    }
}

int tsfi_cross_chain_insert(cross_chain_tracker *tracker, const char *addr, int chain_id, const char *symbol) {
    if (!tracker || tracker->count >= 16 || !addr || !symbol) return -1;
    cross_chain_token *t = &tracker->tokens[tracker->count];
    strncpy(t->token_address, addr, sizeof(t->token_address) - 1);
    t->token_address[sizeof(t->token_address) - 1] = '\0';
    t->chain_id = chain_id;
    strncpy(t->symbol, symbol, sizeof(t->symbol) - 1);
    t->symbol[sizeof(t->symbol) - 1] = '\0';
    
    t->next_token_on_chain = tracker->count;
    t->next_token_cross_chain = tracker->count;
    
    for (int i = 0; i < tracker->count; i++) {
        if (tracker->tokens[i].chain_id == chain_id) {
            int curr = i;
            while (tracker->tokens[curr].next_token_on_chain != i) {
                curr = tracker->tokens[curr].next_token_on_chain;
            }
            t->next_token_on_chain = tracker->tokens[curr].next_token_on_chain;
            tracker->tokens[curr].next_token_on_chain = tracker->count;
            break;
        }
    }
    
    for (int i = 0; i < tracker->count; i++) {
        if (strcmp(tracker->tokens[i].symbol, symbol) == 0) {
            int curr = i;
            while (tracker->tokens[curr].next_token_cross_chain != i) {
                curr = tracker->tokens[curr].next_token_cross_chain;
            }
            t->next_token_cross_chain = tracker->tokens[curr].next_token_cross_chain;
            tracker->tokens[curr].next_token_cross_chain = tracker->count;
            break;
        }
    }
    
    tracker->count++;
    return 0;
}

int tsfi_cross_chain_navigate_chain(cross_chain_tracker *tracker, int chain_idx) {
    if (!tracker || chain_idx < 0 || chain_idx >= 4) return -1;
    int curr = tracker->current_chain_token[chain_idx];
    if (curr == -1) {
        for (int i = 0; i < tracker->count; i++) {
            if (tracker->tokens[i].chain_id == chain_idx) {
                tracker->current_chain_token[chain_idx] = i;
                return i;
            }
        }
        return -1;
    }
    int next = tracker->tokens[curr].next_token_on_chain;
    tracker->current_chain_token[chain_idx] = next;
    return next;
}

int tsfi_cross_chain_navigate_symbol(cross_chain_tracker *tracker, int sym_idx) {
    if (!tracker || sym_idx < 0 || sym_idx >= 4) return -1;
    int curr = tracker->current_cross_token[sym_idx];
    if (curr == -1) {
        return -1;
    }
    int next = tracker->tokens[curr].next_token_cross_chain;
    tracker->current_cross_token[sym_idx] = next;
    return next;
}

void tsfi_dbtg_selection_init(dbtg_selection_table *table) {
    if (!table) return;
    table->count = 0;
    for (int i = 0; i < 16; i++) {
        table->rules[i].owner_id = 0;
        table->rules[i].match_criteria[0] = '\0';
    }
}

int tsfi_dbtg_selection_register(dbtg_selection_table *table, uint32_t owner_id, const char *criteria) {
    if (!table || table->count >= 16 || !criteria) return -1;
    dbtg_selection_rule *rule = &table->rules[table->count++];
    rule->owner_id = owner_id;
    strncpy(rule->match_criteria, criteria, sizeof(rule->match_criteria) - 1);
    rule->match_criteria[sizeof(rule->match_criteria) - 1] = '\0';
    return 0;
}

int tsfi_dbtg_selection_resolve(const dbtg_selection_table *table, const char *member_field) {
    if (!table || !member_field) return -1;
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->rules[i].match_criteria, member_field) == 0) {
            return (int)table->rules[i].owner_id;
        }
    }
    return -1;
}

void tsfi_dsdl_init(dsdl_mapping_table *table) {
    if (!table) return;
    table->count = 0;
    for (int i = 0; i < 16; i++) {
        table->rules[i].logical_record_id = 0;
        table->rules[i].physical_cylinder = 0;
        table->rules[i].page_offset = 0;
    }
}

int tsfi_dsdl_register(dsdl_mapping_table *table, uint32_t record_id, uint32_t cylinder, uint32_t page) {
    if (!table || table->count >= 16) return -1;
    dsdl_mapping_rule *rule = &table->rules[table->count++];
    rule->logical_record_id = record_id;
    rule->physical_cylinder = cylinder;
    rule->page_offset = page;
    return 0;
}

int tsfi_dsdl_resolve(const dsdl_mapping_table *table, uint32_t record_id, uint32_t *out_cylinder, uint32_t *out_page) {
    if (!table || !out_cylinder || !out_page) return -1;
    for (int i = 0; i < table->count; i++) {
        if (table->rules[i].logical_record_id == record_id) {
            *out_cylinder = table->rules[i].physical_cylinder;
            *out_page = table->rules[i].page_offset;
            return 0;
        }
    }
    return -1;
}

void tsfi_cobol_ledger_init(cobol_ledger *ledger) {
    if (!ledger) return;
    ledger->count = 0;
    for (int i = 0; i < 16; i++) {
        ledger->accounts[i].account_id = 0;
        ledger->accounts[i].balance = 0.0;
        ledger->accounts[i].account_holder[0] = '\0';
    }
}

int tsfi_cobol_ledger_add(cobol_ledger *ledger, uint32_t acc_id, const char *holder, double initial_balance) {
    if (!ledger || ledger->count >= 16 || !holder) return -1;
    cobol_account *acc = &ledger->accounts[ledger->count++];
    acc->account_id = acc_id;
    acc->balance = initial_balance;
    strncpy(acc->account_holder, holder, sizeof(acc->account_holder) - 1);
    acc->account_holder[sizeof(acc->account_holder) - 1] = '\0';
    return 0;
}

int tsfi_cobol_ledger_transaction(cobol_ledger *ledger, uint32_t acc_id, double amount, char *audit_out, size_t max_len) {
    if (!ledger || !audit_out || max_len == 0) return -1;
    for (int i = 0; i < ledger->count; i++) {
        cobol_account *acc = &ledger->accounts[i];
        if (acc->account_id == acc_id) {
            double old_bal = acc->balance;
            acc->balance += amount;
            int bytes = snprintf(audit_out, max_len, "ACC=%u; HOLDER=%s; OLD=%.2f; NEW=%.2f; DELTA=%.2f",
                                 acc->account_id, acc->account_holder, old_bal, acc->balance, amount);
            return (bytes > 0 && (size_t)bytes < max_len) ? 0 : -2;
        }
    }
    return -3;
}

double tsfi_cobol_round(double value, int decimals, int mode) {
    double factor = pow(10.0, decimals);
    double scaled = value * factor;
    if (mode == COBOL_ROUND_TRUNC) {
        return trunc(scaled) / factor;
    } else if (mode == COBOL_ROUND_STANDARD) {
        return round(scaled) / factor;
    } else if (mode == COBOL_ROUND_BANKERS) {
        double r = round(scaled);
        double diff = scaled - r;
        if (fabs(diff) == 0.5) {
            double integer_part;
            modf(scaled, &integer_part);
            if (((int)integer_part % 2) == 0) {
                return integer_part / factor;
            } else {
                return (integer_part + (scaled > 0 ? 1.0 : -1.0)) / factor;
            }
        }
        return r / factor;
    }
    return value;
}

double tsfi_cobol_calc_interest(double principal, double rate, int periods, int mode) {
    double interest = principal * pow(1.0 + rate, periods) - principal;
    return tsfi_cobol_round(interest, 2, mode);
}

void tsfi_ach_init(ach_batch *batch) {
    if (!batch) return;
    batch->count = 0;
    for (int i = 0; i < 16; i++) {
        batch->entries[i].routing_number[0] = '\0';
        batch->entries[i].amount = 0.0;
    }
}

int tsfi_ach_verify_routing(const char *routing) {
    if (!routing || strlen(routing) != 9) return -1;
    int d[9];
    for (int i = 0; i < 9; i++) {
        if (routing[i] < '0' || routing[i] > '9') return -2;
        d[i] = routing[i] - '0';
    }
    int sum = 3 * (d[0] + d[3] + d[6]) + 7 * (d[1] + d[4] + d[7]) + (d[2] + d[5] + d[8]);
    return (sum % 10 == 0) ? 0 : -3;
}

int tsfi_ach_add(ach_batch *batch, const char *routing, double amount) {
    if (!batch || batch->count >= 16 || !routing) return -1;
    if (tsfi_ach_verify_routing(routing) != 0) return -2;
    ach_entry *entry = &batch->entries[batch->count++];
    strncpy(entry->routing_number, routing, sizeof(entry->routing_number) - 1);
    entry->routing_number[sizeof(entry->routing_number) - 1] = '\0';
    entry->amount = amount;
    return 0;
}

uint64_t tsfi_ach_calc_hash_total(const ach_batch *batch) {
    if (!batch) return 0;
    uint64_t total = 0;
    for (int i = 0; i < batch->count; i++) {
        char first_eight[9];
        strncpy(first_eight, batch->entries[i].routing_number, 8);
        first_eight[8] = '\0';
        total += strtoull(first_eight, NULL, 10);
    }
    return total;
}

int tsfi_nacha_generate_entry(char *record_out, size_t max_len, uint8_t tx_code, const char *routing, const char *account, double amount) {
    if (!record_out || max_len < 95 || !routing || !account) return -1;
    if (tsfi_ach_verify_routing(routing) != 0) return -2;
    if (tx_code != 22 && tx_code != 27 && tx_code != 32 && tx_code != 37) return -3;
    
    char routing_first8[9];
    strncpy(routing_first8, routing, 8);
    routing_first8[8] = '\0';
    char check_digit = routing[8];
    
    char acc_padded[18];
    snprintf(acc_padded, sizeof(acc_padded), "%-17.17s", account);
    
    uint64_t amt_cents = (uint64_t)round(amount * 100.0);
    char amt_padded[11];
    snprintf(amt_padded, sizeof(amt_padded), "%010llu", (unsigned long long)amt_cents);
    
    int written = snprintf(record_out, max_len, "6%02u%8.8s%c%-17.17s%10.10s%-15.15s%-22.22s%-2.2s0%015d",
                           tx_code, routing_first8, check_digit, acc_padded, amt_padded,
                           "", "Individual Name", "", 12345);
    return (written == 94) ? 0 : -4;
}

int tsfi_nacha_validate_record(const char *record) {
    if (!record || strlen(record) != 94) return -1;
    if (record[0] == '6') {
        char routing[10];
        strncpy(routing, record + 3, 9);
        routing[9] = '\0';
        return tsfi_ach_verify_routing(routing);
    }
    return 0;
}

int tsfi_nacha_generate_file(char *file_out, size_t max_len, const ach_batch *batch, uint8_t tx_code, const char *origin_routing, const char *dest_routing) {
    if (!file_out || max_len < 1000 || !batch || !origin_routing || !dest_routing) return -1;
    if (tsfi_ach_verify_routing(origin_routing) != 0 || tsfi_ach_verify_routing(dest_routing) != 0) return -2;
    file_out[0] = '\0';
    size_t offset = 0;
    int line_count = 0;
    int written = snprintf(file_out + offset, max_len - offset,
                           "101 %-10.10s%-10.10s2607161944A094101%9.9s%-26.26s\n",
                           dest_routing, origin_routing, "FedReserve", "Origin Bank");
    offset += written;
    line_count++;
    written = snprintf(file_out + offset, max_len - offset,
                       "5220%-16.16s%-20.20s%-10.10sPPDDescription   260716260716   1%8.8s0000001\n",
                       "Company Name", "Discretionary Data", "CompanyID", "FedReserve");
    offset += written;
    line_count++;
    double total_amount = 0.0;
    for (int i = 0; i < batch->count; i++) {
        char entry_buf[96];
        char acc_num[18];
        snprintf(acc_num, sizeof(acc_num), "ACC%d", i);
        int entry_res = tsfi_nacha_generate_entry(entry_buf, sizeof(entry_buf), tx_code,
                                                   batch->entries[i].routing_number, acc_num, batch->entries[i].amount);
        if (entry_res != 0) return -3;
        total_amount += batch->entries[i].amount;
        written = snprintf(file_out + offset, max_len - offset, "%s\n", entry_buf);
        offset += written;
        line_count++;
    }
    uint64_t hash_total = tsfi_ach_calc_hash_total(batch);
    uint64_t total_cents = (uint64_t)round(total_amount * 100.0);
    written = snprintf(file_out + offset, max_len - offset,
                       "8220%06d%010llu%012llu%012llu%-10.10s%-25.25s%8.8s0000001\n",
                       batch->count, (unsigned long long)(hash_total % 10000000000ULL),
                       0ULL, (unsigned long long)total_cents, "CompanyID", "", "FedReserve");
    offset += written;
    line_count++;
    int total_records = line_count + 1;
    int block_count = (total_records + 9) / 10;
    written = snprintf(file_out + offset, max_len - offset,
                       "9000001%06d%08d%010llu%012llu%012llu%-39.39s\n",
                       block_count, total_records, (unsigned long long)(hash_total % 10000000000ULL),
                       0ULL, (unsigned long long)total_cents, "");
    offset += written;
    line_count++;
    int target_lines = block_count * 10;
    while (line_count < target_lines) {
        written = snprintf(file_out + offset, max_len - offset,
                           "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n");
        offset += written;
        line_count++;
    }
    return line_count;
}

int tsfi_cobol_pack_hex(const char *hex_in, uint8_t *comp3_out, size_t max_len) {
    if (!hex_in || !comp3_out) return -1;
    if (hex_in[0] == '0' && (hex_in[1] == 'x' || hex_in[1] == 'X')) {
        hex_in += 2;
    }
    size_t len = strlen(hex_in);
    size_t total_nibbles = len + 1;
    if (total_nibbles % 2 != 0) {
        total_nibbles++;
    }
    size_t needed_bytes = total_nibbles / 2;
    if (needed_bytes > max_len) return -2;
    
    size_t in_idx = 0;
    size_t out_idx = 0;
    
    if (len % 2 == 0) {
        char ch = hex_in[in_idx++];
        uint8_t val = 0;
        if (ch >= '0' && ch <= '9') val = ch - '0';
        else if (ch >= 'a' && ch <= 'f') val = ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') val = ch - 'A' + 10;
        comp3_out[out_idx++] = val;
    }
    
    while (in_idx < len) {
        char ch1 = hex_in[in_idx++];
        uint8_t val1 = 0;
        if (ch1 >= '0' && ch1 <= '9') val1 = ch1 - '0';
        else if (ch1 >= 'a' && ch1 <= 'f') val1 = ch1 - 'a' + 10;
        else if (ch1 >= 'A' && ch1 <= 'F') val1 = ch1 - 'A' + 10;
        
        if (in_idx < len) {
            char ch2 = hex_in[in_idx++];
            uint8_t val2 = 0;
            if (ch2 >= '0' && ch2 <= '9') val2 = ch2 - '0';
            else if (ch2 >= 'a' && ch2 <= 'f') val2 = ch2 - 'a' + 10;
            else if (ch2 >= 'A' && ch2 <= 'F') val2 = ch2 - 'A' + 10;
            comp3_out[out_idx++] = (val1 << 4) | val2;
        } else {
            comp3_out[out_idx++] = (val1 << 4) | 0x0F;
            return (int)out_idx;
        }
    }
    return -3;
}

int tsfi_cobol_unpack_hex(const uint8_t *comp3_in, size_t comp3_len, char *hex_out, size_t max_len) {
    if (!comp3_in || comp3_len == 0 || !hex_out) return -1;
    uint8_t last_byte = comp3_in[comp3_len - 1];
    if ((last_byte & 0x0F) != 0x0F) return -2;
    
    size_t out_idx = 0;
    const char hex_chars[] = "0123456789abcdef";
    size_t start_idx = 0;
    
    if ((comp3_in[0] >> 4) == 0) {
        if (out_idx + 1 >= max_len) return -3;
        hex_out[out_idx++] = hex_chars[comp3_in[0] & 0x0F];
        start_idx = 1;
    }
    
    for (size_t i = start_idx; i < comp3_len; i++) {
        uint8_t b = comp3_in[i];
        uint8_t n1 = b >> 4;
        uint8_t n2 = b & 0x0F;
        
        if (i == comp3_len - 1) {
            if (out_idx + 1 >= max_len) return -3;
            hex_out[out_idx++] = hex_chars[n1];
        } else {
            if (out_idx + 2 >= max_len) return -3;
            hex_out[out_idx++] = hex_chars[n1];
            hex_out[out_idx++] = hex_chars[n2];
        }
    }
    hex_out[out_idx] = '\0';
    return 0;
}

void tsfi_ac_filter_init(tsfi_ac_filter *filter) {
    if (!filter) return;
    memset(filter, 0, sizeof(tsfi_ac_filter));
    for (int i = 0; i < 10; i++) {
        filter->nodes[0].next_states[i] = -1;
    }
    filter->nodes[0].fail_state = 0;
    filter->nodes[0].match_pattern_idx = -1;
    filter->node_count = 1;
}

int tsfi_ac_filter_add_pattern(tsfi_ac_filter *filter, const char *pattern, int pattern_idx) {
    if (!filter || !pattern || filter->node_count >= 128) return -1;
    int current = 0;
    for (int i = 0; pattern[i] != '\0'; i++) {
        if (pattern[i] < '0' || pattern[i] > '9') continue;
        int digit = pattern[i] - '0';
        if (filter->nodes[current].next_states[digit] == -1) {
            if (filter->node_count >= 128) return -2;
            int next_node = filter->node_count++;
            for (int k = 0; k < 10; k++) {
                filter->nodes[next_node].next_states[k] = -1;
            }
            filter->nodes[next_node].fail_state = 0;
            filter->nodes[next_node].match_pattern_idx = -1;
            filter->nodes[current].next_states[digit] = next_node;
        }
        current = filter->nodes[current].next_states[digit];
    }
    filter->nodes[current].match_pattern_idx = pattern_idx;
    return 0;
}

void tsfi_ac_filter_build(tsfi_ac_filter *filter) {
    if (!filter) return;
    int queue[128];
    int head = 0, tail = 0;
    for (int i = 0; i < 10; i++) {
        int next = filter->nodes[0].next_states[i];
        if (next != -1) {
            filter->nodes[next].fail_state = 0;
            queue[tail++] = next;
        } else {
            filter->nodes[0].next_states[i] = 0;
        }
    }
    while (head < tail) {
        int r = queue[head++];
        for (int i = 0; i < 10; i++) {
            int u = filter->nodes[r].next_states[i];
            if (u != -1 && u != 0) {
                queue[tail++] = u;
                int fail = filter->nodes[r].fail_state;
                while (filter->nodes[fail].next_states[i] == -1) {
                    fail = filter->nodes[fail].fail_state;
                }
                filter->nodes[u].fail_state = filter->nodes[fail].next_states[i];
                if (filter->nodes[filter->nodes[u].fail_state].match_pattern_idx != -1) {
                    filter->nodes[u].match_pattern_idx = filter->nodes[filter->nodes[u].fail_state].match_pattern_idx;
                }
            } else {
                int fail = filter->nodes[r].fail_state;
                filter->nodes[r].next_states[i] = filter->nodes[fail].next_states[i];
            }
        }
    }
}

int tsfi_ac_filter_search(const tsfi_ac_filter *filter, const char *text) {
    if (!filter || !text) return -1;
    int current = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] < '0' || text[i] > '9') continue;
        int digit = text[i] - '0';
        current = filter->nodes[current].next_states[digit];
        if (filter->nodes[current].match_pattern_idx != -1) {
            return filter->nodes[current].match_pattern_idx;
        }
    }
    return -1;
}

void tsfi_rwcs_init(tsfi_rwcs_report *rep, int page_limit, int line_limit) {
    if (!rep) return;
    rep->page_limit = page_limit;
    rep->line_limit = line_limit;
    rep->current_page = 1;
    rep->current_line = 0;
    rep->group_total = 0.0;
    rep->final_total = 0.0;
}

int tsfi_rwcs_write_header(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 128) return -1;
    int written = snprintf(out, max_len,
                           "=========================================\n"
                           "COBOL FINANCIAL AUDIT REPORT    PAGE: %02d\n"
                           "=========================================\n"
                           "ITEM-ID    ITEM-NAME             AMOUNT\n"
                           "-----------------------------------------\n",
                           rep->current_page);
    rep->current_line = 5;
    return written;
}

int tsfi_rwcs_process_item(tsfi_rwcs_report *rep, char *out, size_t max_len, int item_id, const char *name, double amount) {
    if (!rep || !out || max_len < 64) return -1;
    if (rep->current_line >= rep->line_limit) {
        rep->current_page++;
        int header_len = tsfi_rwcs_write_header(rep, out, max_len);
        int item_len = snprintf(out + header_len, max_len - header_len,
                                "%06d     %-20.20s  %10.2f\n",
                                item_id, name, amount);
        rep->group_total += amount;
        rep->final_total += amount;
        rep->current_line++;
        return header_len + item_len;
    } else {
        int item_len = snprintf(out, max_len,
                                "%06d     %-20.20s  %10.2f\n",
                                item_id, name, amount);
        rep->group_total += amount;
        rep->final_total += amount;
        rep->current_line++;
        return item_len;
    }
}

int tsfi_rwcs_control_break(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 64) return -1;
    int written = snprintf(out, max_len,
                           "-----------------------------------------\n"
                           "SUBTOTAL:                         %10.2f\n"
                           "=========================================\n",
                           rep->group_total);
    rep->group_total = 0.0;
    rep->current_line += 3;
    return written;
}

int tsfi_rwcs_write_final(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 64) return -1;
    int written = snprintf(out, max_len,
                           "-----------------------------------------\n"
                           "GRAND TOTAL:                      %10.2f\n"
                           "=========================================\n",
                           rep->final_total);
    rep->current_line += 3;
    return written;
}

void tsfi_dbtg_currency_init(tsfi_dbtg_currency *cur) {
    if (!cur) return;
    cur->current_run_unit = -1;
    for (int i = 0; i < 8; i++) {
        cur->current_record_type[i] = -1;
        cur->current_set_type[i] = -1;
    }
}

void tsfi_dbtg_currency_update(tsfi_dbtg_currency *cur, int run_unit, int record_type, int set_type) {
    if (!cur) return;
    cur->current_run_unit = run_unit;
    if (record_type >= 0 && record_type < 8) {
        cur->current_record_type[record_type] = run_unit;
    }
    if (set_type >= 0 && set_type < 8) {
        cur->current_set_type[set_type] = run_unit;
    }
}

void tsfi_dbtg_realm_init(tsfi_dbtg_realm_registry *reg) {
    if (!reg) return;
    memset(reg, 0, sizeof(tsfi_dbtg_realm_registry));
}

int tsfi_dbtg_realm_register(tsfi_dbtg_realm_registry *reg, const char *name) {
    if (!reg || !name) return -1;
    if (reg->area_count >= 4) return -2;
    strncpy(reg->areas[reg->area_count].area_name, name, sizeof(reg->areas[reg->area_count].area_name) - 1);
    reg->areas[reg->area_count].lock_mode = DBTG_LOCK_NONE;
    reg->areas[reg->area_count].is_open = 0;
    reg->area_count++;
    return 0;
}

int tsfi_dbtg_realm_open(tsfi_dbtg_realm_registry *reg, const char *name, int lock_mode) {
    if (!reg || !name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, name) == 0) {
            if (reg->areas[i].is_open) return -3;
            reg->areas[i].lock_mode = lock_mode;
            reg->areas[i].is_open = 1;
            return 0;
        }
    }
    return -4;
}

int tsfi_dbtg_realm_close(tsfi_dbtg_realm_registry *reg, const char *name) {
    if (!reg || !name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, name) == 0) {
            if (!reg->areas[i].is_open) return -3;
            reg->areas[i].lock_mode = DBTG_LOCK_NONE;
            reg->areas[i].is_open = 0;
            return 0;
        }
    }
    return -4;
}

void tsfi_dbtg_exception_init(tsfi_dbtg_exception_context *ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(tsfi_dbtg_exception_context));
    ctx->db_status = DB_STATUS_OK;
}

int tsfi_dbtg_validate_action(tsfi_dbtg_exception_context *ctx, const tsfi_dbtg_realm_registry *reg, const char *realm_name, int is_write_action) {
    if (!ctx || !reg || !realm_name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, realm_name) == 0) {
            if (!reg->areas[i].is_open) {
                ctx->db_status = DB_STATUS_NOT_OPEN;
                strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
                ctx->exception_triggered = 1;
                return -2;
            }
            if (is_write_action) {
                if (reg->areas[i].lock_mode == DBTG_LOCK_RETRIEVAL ||
                    reg->areas[i].lock_mode == DBTG_LOCK_EXCLUSIVE_RETRIEVAL) {
                    ctx->db_status = DB_STATUS_LOCK_VIOLATION;
                    strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
                    ctx->exception_triggered = 1;
                    return -3;
                }
            }
            ctx->db_status = DB_STATUS_OK;
            ctx->exception_triggered = 0;
            return 0;
        }
    }
    ctx->db_status = DB_STATUS_NOT_OPEN;
    strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
    ctx->exception_triggered = 1;
    return -4;
}

void tsfi_dbtg_set_membership_init(tsfi_dbtg_set_membership *sm, const char *name, int ins_mode, int ret_mode) {
    if (!sm) return;
    memset(sm, 0, sizeof(tsfi_dbtg_set_membership));
    if (name) strncpy(sm->set_name, name, sizeof(sm->set_name) - 1);
    sm->insertion_mode = ins_mode;
    sm->retention_mode = ret_mode;
}

int tsfi_dbtg_validate_disconnect(const tsfi_dbtg_set_membership *sm, int *db_status_out) {
    if (!sm || !db_status_out) return -1;
    if (sm->retention_mode == DBTG_RETENTION_MANDATORY) {
        *db_status_out = DB_STATUS_MEMBER_ERR;
        return -2;
    }
    *db_status_out = DB_STATUS_OK;
    return 0;
}

int tsfi_dbtg_navigate_set(int current_idx, int record_count, int direction_mode) {
    if (record_count <= 0) return -1;
    switch (direction_mode) {
        case DBTG_NAV_FIRST:
            return 0;
        case DBTG_NAV_LAST:
            return record_count - 1;
        case DBTG_NAV_NEXT:
            return (current_idx + 1) % record_count;
        case DBTG_NAV_PRIOR:
            return (current_idx - 1 + record_count) % record_count;
        default:
            return -1;
    }
}

int tsfi_dbtg_insert_sorted(tsfi_dbtg_sorted_record *records, int current_count, int max_capacity, int record_id, int sort_key) {
    if (!records || current_count < 0 || current_count >= max_capacity) return -1;
    int insert_idx = 0;
    while (insert_idx < current_count && records[insert_idx].sort_key < sort_key) {
        insert_idx++;
    }
    for (int i = current_count; i > insert_idx; i--) {
        records[i] = records[i - 1];
    }
    records[insert_idx].record_id = record_id;
    records[insert_idx].sort_key = sort_key;
    return current_count + 1;
}

void tsfi_dbtg_resolve_calc(const char *key, uint32_t total_pages, uint32_t slots_per_page, tsfi_dbtg_calc_address *address_out) {
    if (!key || total_pages == 0 || slots_per_page == 0 || !address_out) return;
    uint32_t hash = 2166136261U;
    while (*key) {
        hash ^= (uint8_t)*key++;
        hash *= 16777619U;
    }
    address_out->target_page = hash % total_pages;
    address_out->target_slot = (hash / total_pages) % slots_per_page;
}

void tsfi_dbtg_dml_tracker_init(tsfi_dbtg_dml_tracker *tracker) {
    if (!tracker) return;
    memset(tracker, 0, sizeof(tsfi_dbtg_dml_tracker));
}

int tsfi_dbtg_execute_dml(tsfi_dbtg_dml_tracker *tracker, int verb_opcode) {
    if (!tracker) return -1;
    switch (verb_opcode) {
        case DBTG_VERB_STORE:
            tracker->store_count++;
            break;
        case DBTG_VERB_GET:
            tracker->get_count++;
            break;
        case DBTG_VERB_MODIFY:
            tracker->modify_count++;
            break;
        case DBTG_VERB_ERASE:
            tracker->erase_count++;
            break;
        case DBTG_VERB_CONNECT:
            tracker->connect_count++;
            break;
        case DBTG_VERB_DISCONNECT:
            tracker->disconnect_count++;
            break;
        default:
            return -2;
    }
    return 0;
}

