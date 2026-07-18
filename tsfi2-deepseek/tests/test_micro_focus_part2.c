#include "tsfi_micro_focus.h"
#include "tsfi_majormud.h"
#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_priority_queue.h"
#include "tsfi_cade_imf.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    printf("[TEST] Running Micro Focus COBOL standard compatibility checks part 2...\n");

    // 113. Verify MajorMUD Mainframe Z-machine Init
    printf("[TEST] Validating MajorMUD Mainframe Init...\n");
    char mud_state[256] = {0};
    int mud_init_res = tsfi_mf_zmachine_majormud_init(mud_state, sizeof(mud_state));
    assert(mud_init_res == 0);
    assert(strstr(mud_state, "CLASS:Warrior") != NULL);
    printf("  [PASS] MajorMUD Mainframe Init verified.\n");

    // 114. Verify MajorMUD Mainframe Z-machine Look Command
    printf("[TEST] Validating MajorMUD Mainframe Look Command...\n");
    char mud_resp[256] = {0};
    int mud_cmd_res = tsfi_mf_zmachine_majormud_command("look", mud_state, mud_resp, sizeof(mud_resp));
    assert(mud_cmd_res == 0);
    assert(strstr(mud_resp, "Room 1") != NULL);
    assert(strstr(mud_resp, "Goblin") != NULL);
    printf("  [PASS] MajorMUD Mainframe Look Command verified.\n");

    // 115. Verify MajorMUD Mainframe Z-machine Bridge Action
    printf("[TEST] Validating MajorMUD Mainframe Bridge Action...\n");
    char bridge_resp[256] = {0};
    int mud_bridge_res = tsfi_mf_zmachine_majormud_bridge_action(4, "look", mud_state, bridge_resp, sizeof(bridge_resp));
    assert(mud_bridge_res == 0);
    assert(strstr(bridge_resp, "Room 4") != NULL);
    printf("  [PASS] MajorMUD Mainframe Bridge Action verified.\n");

    // 116. Verify IRS CADE Taxpayer Account Registry
    printf("[TEST] Validating IRS CADE Taxpayer Account Registry...\n");
    char taxpayer_registry[128] = {0};
    int cade_reg_res = tsfi_mf_cade_register_taxpayer("999-12-3456", 1250.00, 1, taxpayer_registry, sizeof(taxpayer_registry));
    assert(cade_reg_res == 0);
    assert(strstr(taxpayer_registry, "BAL:1250.00") != NULL);
    printf("  [PASS] IRS CADE Taxpayer Account Registry verified.\n");

    // 117. Verify IRS IMF Transaction Processor
    printf("[TEST] Validating IRS IMF Transaction Processor...\n");
    double tax_balance = 1250.00;
    char imf_log[128] = {0};
    int imf_res = tsfi_mf_imf_process_transaction("999-12-3456", 846, 250.00, &tax_balance, imf_log, sizeof(imf_log));
    assert(imf_res == 0);
    assert(tax_balance == 1000.00);
    assert(strstr(imf_log, "TC846") != NULL);
    printf("  [PASS] IRS IMF Transaction Processor verified.\n");

    // 118. Verify IRS CADE Daily Batch Processing Cycle
    printf("[TEST] Validating IRS CADE Daily Batch Processor...\n");
    const char *ssns[] = {"999-12-3456", "999-12-7890"};
    const int tcs[] = {150, 846};
    const double amounts[] = {300.00, 100.00};
    double balances[] = {1000.00, 500.00};
    char batch_report[256] = {0};
    int batch_res = tsfi_mf_cade_process_daily_batch(ssns, tcs, amounts, 2, balances, batch_report, sizeof(batch_report));
    assert(batch_res == 0);
    assert(balances[0] == 1300.00);
    assert(balances[1] == 400.00);
    assert(strstr(batch_report, "ASSESSMENTS: 300.00") != NULL);
    assert(strstr(batch_report, "REFUNDS: 100.00") != NULL);
    printf("  [PASS] IRS CADE Daily Batch Processor verified.\n");

    // 119. Verify IRS IMF Transaction Code Validator
    printf("[TEST] Validating IRS IMF Transaction Code Validator...\n");
    int code_valid = -1;
    int val_code_res = tsfi_mf_imf_validate_transaction_code(290, 50.00, &code_valid);
    assert(val_code_res == 0);
    assert(code_valid == 1);
    printf("  [PASS] IRS IMF Transaction Code Validator verified.\n");

    // 120. Verify IRS IMF Freeze Code handlers
    printf("[TEST] Validating IRS IMF Freeze Handlers...\n");
    char account_state[64] = {0};
    int set_fr_res = tsfi_mf_imf_set_freeze_code("999-12-3456", 'G', account_state, sizeof(account_state));
    assert(set_fr_res == 0);
    int is_frozen = -1;
    int is_fr_res = tsfi_mf_imf_is_frozen(account_state, &is_frozen);
    assert(is_fr_res == 0);
    assert(is_frozen == 1);
    printf("  [PASS] IRS IMF Freeze Handlers verified.\n");

    // 121. Verify IRS CADE Taxpayer File Lookup
    printf("[TEST] Validating IRS CADE Taxpayer Lookup...\n");
    double lookup_bal = 0.0;
    int lookup_status = -1;
    int lookup_res = tsfi_mf_cade_lookup_taxpayer("999-12-3456", taxpayer_registry, &lookup_bal, &lookup_status);
    assert(lookup_res == 0);
    assert(lookup_bal == 1250.00);
    assert(lookup_status == 1);
    printf("  [PASS] IRS CADE Taxpayer Lookup verified.\n");

    // 122. Verify IRS IMF Cycle Code Conversions
    printf("[TEST] Validating IRS IMF Cycle Code Conversions...\n");
    char cycle_buf[16] = {0};
    int enc_cyc_res = tsfi_mf_imf_encode_cycle_code(2026, 28, 4, cycle_buf, sizeof(cycle_buf));
    assert(enc_cyc_res == 0);
    assert(strcmp(cycle_buf, "20262804") == 0);
    int dec_y = 0, dec_w = 0, dec_d = 0;
    int dec_cyc_res = tsfi_mf_imf_decode_cycle_code(cycle_buf, &dec_y, &dec_w, &dec_d);
    assert(dec_cyc_res == 0);
    assert(dec_y == 2026);
    assert(dec_w == 28);
    assert(dec_d == 4);
    printf("  [PASS] IRS IMF Cycle Code Conversions verified.\n");

    // 123. Verify IRS CADE Taxpayer Status Updater
    printf("[TEST] Validating IRS CADE Taxpayer Status Updater...\n");
    int upd_status_res = tsfi_mf_cade_update_taxpayer_status(taxpayer_registry, 3);
    assert(upd_status_res == 0);
    assert(strstr(taxpayer_registry, "STATUS:3") != NULL);
    printf("  [PASS] IRS CADE Taxpayer Status Updater verified.\n");

    // 124. Verify IRS IMF Document Locator Number (DLN) Parser
    printf("[TEST] Validating IRS IMF DLN Parser...\n");
    int site = 0, tax_class = 0, doc_code = 0, julian = 0, serial = 0;
    int parse_dln_res = tsfi_mf_imf_parse_dln("29110214400123", &site, &tax_class, &doc_code, &julian, &serial);
    assert(parse_dln_res == 0);
    assert(site == 29);
    assert(tax_class == 1);
    assert(doc_code == 10);
    assert(julian == 214);
    assert(serial == 400123);
    printf("  [PASS] IRS IMF DLN Parser verified.\n");

    // 125. Verify IRS CADE Taxpayer Balance Adjuster
    printf("[TEST] Validating IRS CADE Balance Adjuster...\n");
    int adj_bal_res = tsfi_mf_cade_adjust_balance(taxpayer_registry, -150.00);
    assert(adj_bal_res == 0);
    assert(strstr(taxpayer_registry, "BAL:1100.00") != NULL);
    printf("  [PASS] IRS CADE Balance Adjuster verified.\n");

    // 126. Verify IRS IMF Document Locator Number (DLN) Generator
    printf("[TEST] Validating IRS IMF DLN Generator...\n");
    char gen_dln[16] = {0};
    int gen_dln_res = tsfi_mf_imf_generate_dln(29, 1, 10, 214, 400123, gen_dln, sizeof(gen_dln));
    assert(gen_dln_res == 0);
    assert(strcmp(gen_dln, "29110214400123") == 0);
    printf("  [PASS] IRS IMF DLN Generator verified.\n");

    // 127. Verify IRS IMF Tax Class Inquirer
    printf("[TEST] Validating IRS IMF Tax Class Inquirer...\n");
    char class_name[32] = {0};
    int get_class_res = tsfi_mf_imf_get_tax_class_name(3, class_name, sizeof(class_name));
    assert(get_class_res == 0);
    assert(strcmp(class_name, "Corporation Tax") == 0);
    printf("  [PASS] IRS IMF Tax Class Inquirer verified.\n");

    // 128. Verify IRS IMF Transaction Code Sign Resolver
    printf("[TEST] Validating IRS IMF Transaction Sign Resolver...\n");
    int sign_val = 0;
    int sign_res = tsfi_mf_imf_get_transaction_sign(846, &sign_val);
    assert(sign_res == 0);
    assert(sign_val == -1);
    printf("  [PASS] IRS IMF Transaction Sign Resolver verified.\n");

    // 129. Verify IRS IMF Account Balance Reconciler
    printf("[TEST] Validating IRS IMF Account Reconciler...\n");
    const int recon_tcs[] = {150, 610, 290};
    const double recon_amounts[] = {500.00, 200.00, 100.00};
    int reconciled = -1;
    int recon_res = tsfi_mf_imf_reconcile_balance(1000.00, recon_tcs, recon_amounts, 3, 1400.00, &reconciled);
    assert(recon_res == 0);
    assert(reconciled == 1);
    printf("  [PASS] IRS IMF Account Reconciler verified.\n");

    // 130. Verify IRS IMF Document Locator Number (DLN) Validator
    printf("[TEST] Validating IRS IMF DLN Validator...\n");
    int dln_valid = -1;
    int val_dln_res = tsfi_mf_imf_validate_dln("29110214400123", &dln_valid);
    assert(val_dln_res == 0);
    assert(dln_valid == 1);
    printf("  [PASS] IRS IMF DLN Validator verified.\n");

    // 131. Verify IRS CADE Batch Report Formatter
    printf("[TEST] Validating IRS CADE Batch Report Formatter...\n");
    char batch_summary[128] = {0};
    int fmt_batch_res = tsfi_mf_cade_format_batch_summary(1050, 2, 500000.00, 150000.00, batch_summary, sizeof(batch_summary));
    assert(fmt_batch_res == 0);
    assert(strstr(batch_summary, "PROC:001050") != NULL);
    assert(strstr(batch_summary, "ERR:0002") != NULL);
    assert(strstr(batch_summary, "DEBIT:500000.00") != NULL);
    assert(strstr(batch_summary, "CREDIT:150000.00") != NULL);
    printf("  [PASS] IRS CADE Batch Report Formatter verified.\n");

    // 132. Verify IRS IMF Document Code name resolver
    printf("[TEST] Validating IRS IMF Doc Code Resolver...\n");
    char doc_name[32] = {0};
    int get_doc_res = tsfi_mf_imf_get_doc_code_name(20, doc_name, sizeof(doc_name));
    assert(get_doc_res == 0);
    assert(strcmp(doc_name, "Form 1120") == 0);
    printf("  [PASS] IRS IMF Doc Code Resolver verified.\n");

    // 133. Verify IRS CADE Taxpayer Registry Exporter
    printf("[TEST] Validating IRS CADE Registry Exporter...\n");
    char exported_reg[256] = {0};
    int exp_res = tsfi_mf_cade_export_registry(taxpayer_registry, exported_reg, sizeof(exported_reg));
    assert(exp_res == 0);
    assert(strstr(exported_reg, "BAL:1100.00") != NULL);
    printf("  [PASS] IRS CADE Registry Exporter verified.\n");

    // 134. Verify IRS IMF Document Code Validator
    printf("[TEST] Validating IRS IMF Doc Code Validator...\n");
    int doc_valid = -1;
    int val_doc_res = tsfi_mf_imf_validate_doc_code(10, &doc_valid);
    assert(val_doc_res == 0);
    assert(doc_valid == 1);
    printf("  [PASS] IRS IMF Doc Code Validator verified.\n");

    // 135. Verify IRS CADE Taxpayer Registry Compactor
    printf("[TEST] Validating IRS CADE Registry Compactor...\n");
    char registry_spacey[64] = "SSN:999-12-3456 | BAL:100.00";
    int compact_res = tsfi_mf_cade_compact_registry(registry_spacey);
    assert(compact_res == 0);
    assert(strcmp(registry_spacey, "SSN:999-12-3456|BAL:100.00") == 0);
    printf("  [PASS] IRS CADE Registry Compactor verified.\n");

    // 136. Verify IRS IMF Document Code locator
    printf("[TEST] Validating IRS IMF Doc Code Locator...\n");
    int look_doc = 0;
    int look_doc_res = tsfi_mf_imf_get_doc_code_by_name("Form 1040", &look_doc);
    assert(look_doc_res == 0);
    assert(look_doc == 10);
    printf("  [PASS] IRS IMF Doc Code Locator verified.\n");

    // 137. Verify IRS CADE Taxpayer Registry Offset Locator
    printf("[TEST] Validating IRS CADE Registry Offset Locator...\n");
    int offset_loc = -1;
    int loc_res = tsfi_mf_cade_locate_record_offset("999-12-3456", registry_spacey, &offset_loc);
    assert(loc_res == 0);
    assert(offset_loc == 4);
    printf("  [PASS] IRS CADE Registry Offset Locator verified.\n");

    // 138. Verify IRS IMF Excise/Misc Form Verifier
    printf("[TEST] Validating IRS IMF Excise Form Verifier...\n");
    int is_excise = -1;
    int excise_chk_res = tsfi_mf_imf_verify_document_form(40, 40, &is_excise);
    assert(excise_chk_res == 0);
    assert(is_excise == 1);
    printf("  [PASS] IRS IMF Excise Form Verifier verified.\n");

    // 139. Verify IRS CADE Taxpayer Status Name Resolver
    printf("[TEST] Validating IRS CADE Taxpayer Status Name Resolver...\n");
    char status_name[32] = {0};
    int get_stat_res = tsfi_mf_cade_get_status_name(2, status_name, sizeof(status_name));
    assert(get_stat_res == 0);
    assert(strcmp(status_name, "Under Audit") == 0);
    printf("  [PASS] IRS CADE Taxpayer Status Name Resolver verified.\n");

    // 140. Verify IRS IMF Generic Document Form Verifier
    printf("[TEST] Validating IRS IMF Generic Document Form Verifier...\n");
    int form_match = -1;
    int form_chk = tsfi_mf_imf_verify_document_form(10, 10, &form_match);
    assert(form_chk == 0);
    assert(form_match == 1);
    printf("  [PASS] IRS IMF Generic Document Form Verifier verified.\n");

    // 141. Verify IRS CADE Generic Taxpayer Status Mask Checker
    printf("[TEST] Validating IRS CADE Generic Taxpayer Status Mask Checker...\n");
    int status_match = -1;
    int status_chk = tsfi_mf_cade_check_status_mask(2, 2, &status_match);
    assert(status_chk == 0);
    assert(status_match == 1);
    printf("  [PASS] IRS CADE Generic Taxpayer Status Mask Checker verified.\n");

    // 142. Verify IRS CADE Taxpayer SSN Partition Routing
    printf("[TEST] Validating IRS CADE Taxpayer SSN Partition Routing...\n");
    int partition = -1;
    int route_res = tsfi_mf_cade_route_partition("999-12-3456", &partition);
    assert(route_res == 0);
    assert(partition == 5);
    printf("  [PASS] IRS CADE Taxpayer SSN Partition Routing verified.\n");

    // 143. Verify IRS IMF SSN Check Digit Validator
    printf("[TEST] Validating IRS IMF SSN Check Digit Validator...\n");
    int ssn_valid = -1;
    int ssn_res = tsfi_mf_imf_verify_ssn_check_digit("000-00-0000", &ssn_valid);
    assert(ssn_res == 0);
    assert(ssn_valid == 1);
    printf("  [PASS] IRS IMF SSN Check Digit Validator verified.\n");

    // 144. Verify IRS IMF Audit Selection Discrepancy Classifier
    printf("[TEST] Validating IRS IMF Audit Selection Discrepancy Classifier...\n");
    int audit_flag = -1;
    int discrepancy_res = tsfi_mf_imf_evaluate_audit_discrepancy(50000.00, 60000.00, 0, &audit_flag);
    assert(discrepancy_res == 0);
    assert(audit_flag == 1);
    printf("  [PASS] IRS IMF Audit Selection Discrepancy Classifier verified.\n");

    // 145. Verify IRS IMF Refund Scheduler
    printf("[TEST] Validating IRS IMF Refund Scheduler...\n");
    char refund_date[32] = {0};
    int schedule_res = tsfi_mf_imf_schedule_refund("20262804", 846, refund_date, sizeof(refund_date));
    assert(schedule_res == 0);
    assert(strcmp(refund_date, "2026-W29") == 0);
    printf("  [PASS] IRS IMF Refund Scheduler verified.\n");

    // 146. Verify IRS IMF Taxpayer Account Balance Offset Rebuilder
    printf("[TEST] Validating IRS IMF Liability Offset Processor...\n");
    double net_refund = -1.0;
    double remaining_liability = -1.0;
    int offset_res = tsfi_mf_imf_apply_liability_offset(1000.00, 300.00, &net_refund, &remaining_liability);
    assert(offset_res == 0);
    assert(net_refund == 700.00);
    assert(remaining_liability == 0.0);
    printf("  [PASS] IRS IMF Liability Offset Processor verified.\n");

    // 147. Verify IRS IMF Filing Status Restriction checks
    printf("[TEST] Validating IRS IMF Filing Status Restriction...\n");
    int allowed = -1;
    int status_res = tsfi_mf_imf_verify_filing_status(3, 88, &allowed);
    assert(status_res == 0);
    assert(allowed == 0);
    printf("  [PASS] IRS IMF Filing Status Restriction verified.\n");

    // 148. Verify IRS CADE Daily Batch Error Accounting
    printf("[TEST] Validating IRS CADE Daily Batch Error Accounting...\n");
    char severity[16] = {0};
    int err_res = tsfi_mf_cade_classify_batch_error(503, severity, sizeof(severity));
    assert(err_res == 0);
    assert(strcmp(severity, "CRITICAL") == 0);
    printf("  [PASS] IRS CADE Daily Batch Error Accounting verified.\n");

    // 149. Verify IRS IMF Adjusted Gross Income (AGI) Matcher
    printf("[TEST] Validating IRS IMF AGI Matcher...\n");
    int agi_match = -1;
    int agi_res = tsfi_mf_imf_verify_agi_calculation(75000.00, 5000.00, 70000.00, &agi_match);
    assert(agi_res == 0);
    assert(agi_match == 1);
    printf("  [PASS] IRS IMF AGI Matcher verified.\n");

    // 150. Verify IRS IMF Standard vs Itemized Deduction Matcher
    printf("[TEST] Validating IRS IMF Deduction Matcher...\n");
    int ded_valid = -1;
    int ded_res = tsfi_mf_imf_validate_deductions(1, 12000.00, 0, &ded_valid);
    assert(ded_res == 0);
    assert(ded_valid == 1);
    printf("  [PASS] IRS IMF Deduction Matcher verified.\n");

    // 151. Verify IRS CADE Refund Offset Delay Flag
    printf("[TEST] Validating IRS CADE Refund Delay Checker...\n");
    int has_hold = -1;
    int hold_res = tsfi_mf_cade_check_refund_hold(1, 0, &has_hold);
    assert(hold_res == 0);
    assert(has_hold == 1);
    printf("  [PASS] IRS CADE Refund Delay Checker verified.\n");

    // 152. Verify IRS IMF Required Signature presence verifier
    printf("[TEST] Validating IRS IMF Signature Verifier...\n");
    int sig_valid = -1;
    int sig_res = tsfi_mf_imf_verify_signatures(2, 1, 0, &sig_valid);
    assert(sig_res == 0);
    assert(sig_valid == 0);
    printf("  [PASS] IRS IMF Signature Verifier verified.\n");

    // 153. Verify IRS IMF Dependents SSN Duplication Checker
    printf("[TEST] Validating IRS IMF Dependent Duplicate Checker...\n");
    int dep_dups = -1;
    const char *deps[] = {"999-12-3456", "999-12-0000"};
    int dep_res = tsfi_mf_imf_check_dependent_duplicates("999-12-3456", "999-12-7890", deps, 2, &dep_dups);
    assert(dep_res == 0);
    assert(dep_dups == 1);
    printf("  [PASS] IRS IMF Dependent Duplicate Checker verified.\n");

    // 154. Verify IRS IMF Filing Extension Date Matcher
    printf("[TEST] Validating IRS IMF Filing Deadline Matcher...\n");
    int timely = -1;
    int deadline_res = tsfi_mf_imf_verify_filing_deadline(120, 1, &timely);
    assert(deadline_res == 0);
    assert(timely == 1);
    printf("  [PASS] IRS IMF Filing Deadline Matcher verified.\n");

    // 155. Verify IRS CADE Refund Direct Deposit vs Paper Check Router
    printf("[TEST] Validating IRS CADE Refund Disbursement Router...\n");
    int method = -1;
    int route_disb_res = tsfi_mf_cade_route_refund_disbursement("123456789", "987654321", &method);
    assert(route_disb_res == 0);
    assert(method == 1);
    printf("  [PASS] IRS CADE Refund Disbursement Router verified.\n");

    // 156. Verify IRS IMF Refund Amount Ceiling Check
    printf("[TEST] Validating IRS IMF Refund Ceiling Checker...\n");
    int requires_review = -1;
    int limit_res = tsfi_mf_imf_verify_refund_threshold(12500.00, 10000.00, &requires_review);
    assert(limit_res == 0);
    assert(requires_review == 1);
    printf("  [PASS] IRS IMF Refund Ceiling Checker verified.\n");

    // 157. Verify IRS CADE Prior Year AGI Identity Verifier
    printf("[TEST] Validating IRS CADE Prior Year AGI Matcher...\n");
    int prior_match = -1;
    int prior_res = tsfi_mf_cade_verify_prior_year_agi(45000.00, 45000.00, &prior_match);
    assert(prior_res == 0);
    assert(prior_match == 1);
    printf("  [PASS] IRS CADE Prior Year AGI Matcher verified.\n");

    // 158. Verify IRS IMF AGI Bracket Classifier
    printf("[TEST] Validating IRS IMF AGI Bracket Classifier...\n");
    int bracket = -1;
    int bracket_res = tsfi_mf_imf_classify_agi_bracket(75000.00, &bracket);
    assert(bracket_res == 0);
    assert(bracket == 2);
    printf("  [PASS] IRS IMF AGI Bracket Classifier verified.\n");

    // 159. Verify IRS CADE Third-Party Designee Presence Check
    printf("[TEST] Validating IRS CADE Designee PIN Verifier...\n");
    int designee_valid = -1;
    int designee_res = tsfi_mf_cade_verify_designee_record(1, "12345", &designee_valid);
    assert(designee_res == 0);
    assert(designee_valid == 1);
    printf("  [PASS] IRS CADE Designee PIN Verifier verified.\n");

    // 160. Verify IRS CADE Filing Method Indicator Validator
    printf("[TEST] Validating IRS CADE Filing Method Validator...\n");
    int method_valid = -1;
    int method_chk_res = tsfi_mf_cade_verify_filing_method(2, &method_valid);
    assert(method_chk_res == 0);
    assert(method_valid == 1);
    printf("  [PASS] IRS CADE Filing Method Validator verified.\n");

    // 161. Verify IRS IMF First-Time Filer Flag Selector
    printf("[TEST] Validating IRS IMF First-Time Filer Checker...\n");
    int first_time = -1;
    int first_time_res = tsfi_mf_imf_check_first_time_filer("999-12-9999", "SSN:999-12-3456 | SSN:999-12-7890", &first_time);
    assert(first_time_res == 0);
    assert(first_time == 1);
    printf("  [PASS] IRS IMF First-Time Filer Checker verified.\n");

    // 162. Verify IRS CADE Taxpayer Phone Number Area Code Matcher
    printf("[TEST] Validating IRS CADE Phone Area Code Matcher...\n");
    int phone_match = -1;
    int phone_res = tsfi_mf_cade_verify_phone_area_code("213-555-0199", "CA", &phone_match);
    assert(phone_res == 0);
    assert(phone_match == 1);
    printf("  [PASS] IRS CADE Phone Area Code Matcher verified.\n");

    // 163. Verify IRS IMF Dependents Age Eligibility Check
    printf("[TEST] Validating IRS IMF Dependent Age Checker...\n");
    int age_eligible = -1;
    int age_res = tsfi_mf_imf_verify_dependent_age(2015, 2026, &age_eligible);
    assert(age_res == 0);
    assert(age_eligible == 1);
    printf("  [PASS] IRS IMF Dependent Age Checker verified.\n");

    // 164. Verify DDL Schema Executions
    printf("[TEST] Validating DDL Schema Builder...\n");
    char ddl_out[128] = {0};
    int set_ddl_res = tsfi_mf_ddl_set_mode(0);
    assert(set_ddl_res == 0);
    int ddl_res = tsfi_mf_ddl_execute("CREATE TABLE taxpayers", ddl_out, sizeof(ddl_out));
    assert(ddl_res == 0);
    assert(strstr(ddl_out, "Created table") != NULL);
    printf("  [PASS] DDL Schema Builder verified.\n");

    // 165. Verify DML Operations
    printf("[TEST] Validating DML Record Operations...\n");
    char dml_out[128] = {0};
    int set_dml_res = tsfi_mf_dml_set_mode(0);
    assert(set_dml_res == 0);
    int dml_res1 = tsfi_mf_dml_execute("INSERT INTO taxpayers VALUES ('999-12-3456', 1500.00, 1)", dml_out, sizeof(dml_out));
    assert(dml_res1 == 0);
    assert(strstr(dml_out, "Inserted record") != NULL);

    memset(dml_out, 0, sizeof(dml_out));
    int dml_res2 = tsfi_mf_dml_execute("SELECT balance FROM taxpayers WHERE ssn = '999-12-3456'", dml_out, sizeof(dml_out));
    assert(dml_res2 == 0);
    assert(strcmp(dml_out, "BALANCE:1500.00") == 0);
    printf("  [PASS] DML Record Operations verified.\n");

    // 166. Verify IRS CADE Taxpayer Zip Code State Matcher
    printf("[TEST] Validating IRS CADE Zip Code State Matcher...\n");
    int zip_match = -1;
    int zip_res = tsfi_mf_cade_verify_zip_state("90210", "CA", &zip_match);
    assert(zip_res == 0);
    assert(zip_match == 1);
    printf("  [PASS] IRS CADE Zip Code State Matcher verified.\n");

    // 167. Verify IRS IMF EITC Qualifying Child Matcher
    printf("[TEST] Validating IRS IMF EITC Child Matcher...\n");
    int eitc_eligible = -1;
    int eitc_res = tsfi_mf_imf_verify_eitc_qualifying_child("daughter", 200, &eitc_eligible);
    assert(eitc_res == 0);
    assert(eitc_eligible == 1);
    printf("  [PASS] IRS IMF EITC Child Matcher verified.\n");

    // 168. Verify IRS IMF Homebuyer Recapture Validator
    printf("[TEST] Validating IRS IMF Homebuyer Recapture Validator...\n");
    double recap = -1.0;
    int recap_res = tsfi_mf_imf_verify_homebuyer_recapture(2008, 7500.00, &recap);
    assert(recap_res == 0);
    assert(recap == 500.00);
    printf("  [PASS] IRS IMF Homebuyer Recapture Validator verified.\n");

    // 169. Verify IRS CADE Prior Refund Offset Query
    printf("[TEST] Validating IRS CADE Prior Refund Offset Query...\n");
    double offset = -1.0;
    int offset_chk_res = tsfi_mf_cade_query_prior_refund_offset("999-12-3456", "SSN:999-12-3456=250.00 | SSN:999-12-7890=0.00", &offset);
    assert(offset_chk_res == 0);
    assert(offset == 250.00);
    printf("  [PASS] IRS CADE Prior Refund Offset Query verified.\n");

    // 170. Verify IRS CADE Taxpayer IP PIN Length Matcher
    printf("[TEST] Validating IRS CADE IP PIN Length Matcher...\n");
    int pin_valid = -1;
    int pin_res = tsfi_mf_cade_verify_ip_pin("123456", &pin_valid);
    assert(pin_res == 0);
    assert(pin_valid == 1);
    printf("  [PASS] IRS CADE IP PIN Length Matcher verified.\n");

    // 171. Verify IRS IMF Single Standard Deduction Amount Matcher
    printf("[TEST] Validating IRS IMF Single Standard Deduction Matcher...\n");
    int single_ded_valid = -1;
    int single_ded_res = tsfi_mf_imf_verify_single_standard_deduction(13850.00, &single_ded_valid);
    assert(single_ded_res == 0);
    assert(single_ded_valid == 1);
    printf("  [PASS] IRS IMF Single Standard Deduction Matcher verified.\n");

    // 172. Verify IRS CADE Taxpayer Address State Abbreviation Validator
    printf("[TEST] Validating IRS CADE State Code Validator...\n");
    int state_valid = -1;
    int state_res = tsfi_mf_cade_verify_state_code("CA", &state_valid);
    assert(state_res == 0);
    assert(state_valid == 1);
    printf("  [PASS] IRS CADE State Code Validator verified.\n");

    // 173. Verify IRS IMF Child and Dependent Care Credit Ceiling Matcher
    printf("[TEST] Validating IRS IMF Dependent Care Ceiling Matcher...\n");
    int care_valid = -1;
    int care_res = tsfi_mf_imf_verify_dependent_care_ceiling(2, 5000.00, &care_valid);
    assert(care_res == 0);
    assert(care_valid == 1);
    printf("  [PASS] IRS IMF Dependent Care Ceiling Matcher verified.\n");

    // 174. Verify IRS CADE Taxpayer Address Street Number Parity Checker
    printf("[TEST] Validating IRS CADE Street Parity Checker...\n");
    int parity = -1;
    int parity_res = tsfi_mf_cade_verify_street_parity("123 Maple Street", &parity);
    assert(parity_res == 0);
    assert(parity == 1);
    printf("  [PASS] IRS CADE Street Parity Checker verified.\n");

    // 175. Verify IRS IMF Student Loan Interest Deduction Ceiling Validator
    printf("[TEST] Validating IRS IMF Student Loan Interest Validator...\n");
    int interest_valid = -1;
    int interest_res = tsfi_mf_imf_verify_student_loan_interest(1800.00, &interest_valid);
    assert(interest_res == 0);
    assert(interest_valid == 1);
    printf("  [PASS] IRS IMF Student Loan Interest Validator verified.\n");

    // 176. Verify IRS IMF Taxpayer Age / Blindness Standard Deduction Bonus Matcher
    printf("[TEST] Validating IRS IMF Deduction Bonus Matcher...\n");
    int bonus_valid = -1;
    int bonus_res = tsfi_mf_imf_verify_deduction_bonus(1, 0, 1, 1950.00, &bonus_valid);
    assert(bonus_res == 0);
    assert(bonus_valid == 1);
    printf("  [PASS] IRS IMF Deduction Bonus Matcher verified.\n");

    // 177. Verify IRS CADE Social Security Benefit Taxability Range Checker
    printf("[TEST] Validating IRS CADE SS Benefits Taxability Range Checker...\n");
    int ss_valid = -1;
    int ss_res = tsfi_mf_cade_verify_ss_benefits(18000.00, 3000.00, 0.00, 1, &ss_valid);
    assert(ss_res == 0);
    assert(ss_valid == 1);
    printf("  [PASS] IRS CADE SS Benefits Taxability Range Checker verified.\n");

    // 178. Verify IRS CADE Taxpayer Signature Date Chronological Validator
    printf("[TEST] Validating IRS CADE Signature Date Chronological Validator...\n");
    int date_valid = -1;
    int date_res = tsfi_mf_cade_verify_signature_date(2027, 2026, &date_valid);
    assert(date_res == 0);
    assert(date_valid == 1);
    printf("  [PASS] IRS CADE Signature Date Chronological Validator verified.\n");

    // 179. Verify IRS IMF Qualified Business Income (QBI) Deduction Threshold Checker
    printf("[TEST] Validating IRS IMF QBI Threshold Checker...\n");
    int qbi_below = -1;
    int qbi_res = tsfi_mf_imf_verify_qbi_threshold(150000.00, 1, &qbi_below);
    assert(qbi_res == 0);
    assert(qbi_below == 1);
    printf("  [PASS] IRS IMF QBI Threshold Checker verified.\n");

    // 180. Verify IRS CADE State Tax Withholding Matcher
    printf("[TEST] Validating IRS CADE State Withholding Matcher...\n");
    int withholding_valid = -1;
    int withholding_res = tsfi_mf_cade_verify_state_withholding("CA", 50000.00, 2500.00, &withholding_valid);
    assert(withholding_res == 0);
    assert(withholding_valid == 1);
    printf("  [PASS] IRS CADE State Withholding Matcher verified.\n");

    // 181. Verify IRS IMF Distributive Share K-1 Partnership Validator
    printf("[TEST] Validating IRS IMF K-1 Distributive Share Validator...\n");
    int k1_valid = -1;
    int k1_res = tsfi_mf_imf_verify_k1_share(10.0, 100000.00, 10000.00, &k1_valid);
    assert(k1_res == 0);
    assert(k1_valid == 1);
    printf("  [PASS] IRS IMF K-1 Distributive Share Validator verified.\n");

    // 182. Verify IRS CADE Taxpayer Signature Witness Presence Indicator Checker
    printf("[TEST] Validating IRS CADE Witness Indicator Checker...\n");
    int witness_valid = -1;
    int witness_res = tsfi_mf_cade_verify_witness_indicator(8283, 1, &witness_valid);
    assert(witness_res == 0);
    assert(witness_valid == 1);
    printf("  [PASS] IRS CADE Witness Indicator Checker verified.\n");

    // 183. Verify IRS IMF Additional Child Tax Credit (ACTC) Refund Limit Matcher
    printf("[TEST] Validating IRS IMF ACTC Limit Matcher...\n");
    int actc_valid = -1;
    int actc_res = tsfi_mf_imf_verify_actc_limit(2, 3400.00, &actc_valid);
    assert(actc_res == 0);
    assert(actc_valid == 1);
    printf("  [PASS] IRS IMF ACTC Limit Matcher verified.\n");

    // 184. Verify IRS CADE State Filing Volume Throttler Check
    printf("[TEST] Validating IRS CADE Filing Volume Throttler...\n");
    int vol_allowed = -1;
    int volume_res = tsfi_mf_cade_verify_filing_volume("CA", 5000, 10000, &vol_allowed);
    assert(volume_res == 0);
    assert(vol_allowed == 1);
    printf("  [PASS] IRS CADE Filing Volume Throttler verified.\n");

    // 185. Verify IRS IMF First-Time Filer Address Validation Check
    printf("[TEST] Validating IRS IMF First-Time Filer Address Checker...\n");
    int addr_valid = -1;
    int addr_res = tsfi_mf_imf_verify_first_time_address("123 Maple Street", &addr_valid);
    assert(addr_res == 0);
    assert(addr_valid == 1);
    printf("  [PASS] IRS IMF First-Time Filer Address Checker verified.\n");

    // 186. Verify IRS IMF Federal Tax Withholding Cap Validator
    printf("[TEST] Validating IRS IMF Federal Withholding Cap Validator...\n");
    int fed_valid = -1;
    int fed_res = tsfi_mf_imf_verify_federal_withholding_cap(100000.00, 25000.00, &fed_valid);
    assert(fed_res == 0);
    assert(fed_valid == 1);
    printf("  [PASS] IRS IMF Federal Withholding Cap Validator verified.\n");

    // 187. Verify IRS IMF Alternative Minimum Tax (AMT) Simple Liability Checker
    printf("[TEST] Validating IRS IMF AMT Threshold Checker...\n");
    int requires_amt = -1;
    int amt_res = tsfi_mf_imf_check_amt_threshold(95000.00, 1, &requires_amt);
    assert(amt_res == 0);
    assert(requires_amt == 1);
    printf("  [PASS] IRS IMF AMT Threshold Checker verified.\n");

    // 188. Verify IRS IMF Federal Social Security Tax Cap Matcher
    printf("[TEST] Validating IRS IMF SS Tax Cap Matcher...\n");
    int ss_cap_valid = -1;
    int ss_cap_res = tsfi_mf_imf_verify_social_security_tax_cap(50000.00, 3100.00, &ss_cap_valid);
    assert(ss_cap_res == 0);
    assert(ss_cap_valid == 1);
    printf("  [PASS] IRS IMF SS Tax Cap Matcher verified.\n");

    // 189. Verify IRS IMF Federal Qualified Dividend Tax Rate Matcher
    printf("[TEST] Validating IRS IMF Qualified Dividend Rate Matcher...\n");
    int div_rate_valid = -1;
    int div_rate_res = tsfi_mf_imf_verify_qualified_dividend_rate(100000.00, 1000.00, 150.00, &div_rate_valid);
    assert(div_rate_res == 0);
    assert(div_rate_valid == 1);
    printf("  [PASS] IRS IMF Qualified Dividend Rate Matcher verified.\n");

    // 190. Verify IRS IMF Schedule C Self-Employment Tax Threshold Matcher
    printf("[TEST] Validating IRS IMF Schedule SE Tax Threshold Matcher...\n");
    int se_required = -1;
    int se_res = tsfi_mf_imf_check_se_tax_threshold(600.00, &se_required);
    assert(se_res == 0);
    assert(se_required == 1);
    printf("  [PASS] IRS IMF Schedule SE Tax Threshold Matcher verified.\n");

    // 191. Verify IRS IMF Federal Net Investment Income Tax (NIIT) Threshold Checker
    printf("[TEST] Validating IRS IMF NIIT Threshold Checker...\n");
    int niit_required = -1;
    int niit_res = tsfi_mf_imf_check_niit_threshold(220000.00, 1, &niit_required);
    assert(niit_res == 0);
    assert(niit_required == 1);
    printf("  [PASS] IRS IMF NIIT Threshold Checker verified.\n");

    // 192. Verify IRS IMF Schedule A Medical Expense Floor Matcher
    printf("[TEST] Validating IRS IMF Medical Expense Floor Matcher...\n");
    int medical_valid = -1;
    int medical_res = tsfi_mf_imf_verify_medical_floor(100000.00, 10000.00, 2500.00, &medical_valid);
    assert(medical_res == 0);
    assert(medical_valid == 1);
    printf("  [PASS] IRS IMF Medical Expense Floor Matcher verified.\n");

    // 193. Verify IRS IMF Federal Qualified Business Income (QBI) Deduction Rate Matcher
    printf("[TEST] Validating IRS IMF QBI Rate Matcher...\n");
    int qbi_valid = -1;
    int qbi_ded_res = tsfi_mf_imf_verify_qbi_deduction(50000.00, 100000.00, 10000.00, &qbi_valid);
    assert(qbi_ded_res == 0);
    assert(qbi_valid == 1);
    printf("  [PASS] IRS IMF QBI Rate Matcher verified.\n");

    // 194. Verify IRS IMF Standard vs Itemized Deduction Selector
    printf("[TEST] Validating IRS IMF Deduction Selector...\n");
    int select_valid = -1;
    int select_res = tsfi_mf_imf_select_deduction(13850.00, 15000.00, 15000.00, &select_valid);
    assert(select_res == 0);
    assert(select_valid == 1);
    printf("  [PASS] IRS IMF Deduction Selector verified.\n");

    // 195. Verify IRS IMF Charitable Cash Contribution Cap Validator
    printf("[TEST] Validating IRS IMF Charitable Cash Cap Checker...\n");
    int charity_valid = -1;
    int charity_res = tsfi_mf_imf_verify_charity_cap(100000.00, 50000.00, &charity_valid);
    assert(charity_res == 0);
    assert(charity_valid == 1);
    printf("  [PASS] IRS IMF Charitable Cash Cap Checker verified.\n");

    // 196. Verify IRS IMF Premium Tax Credit (PTC) Simple Eligibility Checker
    printf("[TEST] Validating IRS IMF PTC Eligibility Checker...\n");
    int ptc_eligible = -1;
    int ptc_res = tsfi_mf_imf_verify_ptc_eligibility(45000.00, 15000.00, &ptc_eligible);
    assert(ptc_res == 0);
    assert(ptc_eligible == 1);
    printf("  [PASS] IRS IMF PTC Eligibility Checker verified.\n");

    // 197. Verify IRS IMF Federal Foreign Tax Credit Limit Matcher
    printf("[TEST] Validating IRS IMF Foreign Tax Credit Limit Matcher...\n");
    int ftc_valid = -1;
    int ftc_res = tsfi_mf_imf_verify_foreign_tax_credit(2000.00, 5000.00, 1500.00, &ftc_valid);
    assert(ftc_res == 0);
    assert(ftc_valid == 1);
    printf("  [PASS] IRS IMF Foreign Tax Credit Limit Matcher verified.\n");

    // 198. Verify IRS IMF Federal Simple Interest Income Taxability Range Checker
    printf("[TEST] Validating IRS IMF EITC Interest Limit Matcher...\n");
    int eitc_interest_eligible = -1;
    int eitc_interest_res = tsfi_mf_imf_check_interest_limit_for_eitc(5000.00, &eitc_interest_eligible);
    assert(eitc_interest_res == 0);
    assert(eitc_interest_eligible == 1);
    printf("  [PASS] IRS IMF EITC Interest Limit Matcher verified.\n");

    // 199. Verify IRS IMF Federal Earned Income Bracket Matcher
    printf("[TEST] Validating IRS IMF EITC Income Limit Matcher...\n");
    int eitc_limit_eligible = -1;
    int eitc_limit_res = tsfi_mf_imf_verify_eitc_income_limit(35000.00, 3, 2, &eitc_limit_eligible);
    assert(eitc_limit_res == 0);
    assert(eitc_limit_eligible == 1);
    printf("  [PASS] IRS IMF EITC Income Limit Matcher verified.\n");

    // 200. Verify IRS IMF Federal Capital Loss Deduction Cap Checker
    printf("[TEST] Validating IRS IMF Capital Loss Cap Validator...\n");
    int loss_valid = -1;
    int loss_res = tsfi_mf_imf_verify_capital_loss_cap(2000.00, 1, &loss_valid);
    assert(loss_res == 0);
    assert(loss_valid == 1);
    printf("  [PASS] IRS IMF Capital Loss Cap Validator verified.\n");

    // 201. Verify IRS IMF Federal Simple Student Loan Interest AGI Phase-Out Checker
    printf("[TEST] Validating IRS IMF Student Loan Phase-Out Checker...\n");
    int below_phaseout = -1;
    int phaseout_res = tsfi_mf_imf_check_student_loan_phaseout(75000.00, 1, &below_phaseout);
    assert(phaseout_res == 0);
    assert(below_phaseout == 1);
    printf("  [PASS] IRS IMF Student Loan Phase-Out Checker verified.\n");

    // 202. Verify IRS IMF Earned Income Credit Investment Income Limit Matcher
    printf("[TEST] Validating IRS IMF EITC Investment Income Matcher...\n");
    int eitc_inv_eligible = -1;
    int eitc_inv_res = tsfi_mf_imf_verify_eitc_investment_income(3000.00, 2000.00, &eitc_inv_eligible);
    assert(eitc_inv_res == 0);
    assert(eitc_inv_eligible == 1);
    printf("  [PASS] IRS IMF EITC Investment Income Matcher verified.\n");

    // 203. Verify IRS IMF SIMPLE IRA Elective Contribution Limit Matcher
    printf("[TEST] Validating IRS IMF SIMPLE IRA Contribution Matcher...\n");
    int ira_contrib_valid = -1;
    int ira_contrib_res = tsfi_mf_imf_verify_simple_ira_contribution(12000.00, &ira_contrib_valid);
    assert(ira_contrib_res == 0);
    assert(ira_contrib_valid == 1);
    printf("  [PASS] IRS IMF SIMPLE IRA Contribution Matcher verified.\n");

    // 204. Verify IRS IMF Additional Child Tax Credit (ACTC) Income Threshold Checker
    printf("[TEST] Validating IRS IMF ACTC Income Floor Checker...\n");
    int actc_above_floor = -1;
    int actc_floor_res = tsfi_mf_imf_check_actc_income_floor(5000.00, &actc_above_floor);
    assert(actc_floor_res == 0);
    assert(actc_above_floor == 1);
    printf("  [PASS] IRS IMF ACTC Income Floor Checker verified.\n");

    // 205. Verify IRS IMF SIMPLE IRA Employer Match Matcher
    printf("[TEST] Validating IRS IMF SIMPLE IRA Match Matcher...\n");
    int ira_match_valid = -1;
    int ira_match_res = tsfi_mf_imf_verify_simple_ira_match(50000.00, 1500.00, &ira_match_valid);
    assert(ira_match_res == 0);
    assert(ira_match_valid == 1);
    printf("  [PASS] IRS IMF SIMPLE IRA Match Matcher verified.\n");

    // 206. Verify IRS IMF Adoption Tax Credit Limit Matcher
    printf("[TEST] Validating IRS IMF Adoption Credit Limit Matcher...\n");
    int adopt_valid = -1;
    int adopt_res = tsfi_mf_imf_verify_adoption_credit(10000.00, &adopt_valid);
    assert(adopt_res == 0);
    assert(adopt_valid == 1);
    printf("  [PASS] IRS IMF Adoption Credit Limit Matcher verified.\n");

    // 207. Verify IRS IMF Educator Expense Deduction Cap Checker
    printf("[TEST] Validating IRS IMF Educator Expense Cap Checker...\n");
    int educator_valid = -1;
    int educator_res = tsfi_mf_imf_verify_educator_expense(250.00, 0, &educator_valid);
    assert(educator_res == 0);
    assert(educator_valid == 1);
    printf("  [PASS] IRS IMF Educator Expense Cap Checker verified.\n");

    // 208. Verify IRS IMF Lifetime Learning Credit Limit Matcher
    printf("[TEST] Validating IRS IMF Lifetime Learning Credit Matcher...\n");
    int llc_valid = -1;
    int llc_res = tsfi_mf_imf_verify_lifetime_learning_credit(1500.00, &llc_valid);
    assert(llc_res == 0);
    assert(llc_valid == 1);
    printf("  [PASS] IRS IMF Lifetime Learning Credit Matcher verified.\n");

    // 209. Verify IRS IMF Child Tax Credit (CTC) Phase-Out Start Threshold Matcher
    printf("[TEST] Validating IRS IMF CTC Phase-Out Threshold Matcher...\n");
    int ctc_below_threshold = -1;
    int ctc_threshold_res = tsfi_mf_imf_check_ctc_phaseout_threshold(150000.00, 1, &ctc_below_threshold);
    assert(ctc_threshold_res == 0);
    assert(ctc_below_threshold == 1);
    printf("  [PASS] IRS IMF CTC Phase-Out Threshold Matcher verified.\n");

    // 210. Verify IRS IMF Net Capital Gains 0% Preferential Rate Bracket Matcher
    printf("[TEST] Validating IRS IMF Capital Gains Zero Rate Matcher...\n");
    int qualifies_zero = -1;
    int cg_zero_res = tsfi_mf_imf_verify_zero_rate_capital_gains(40000.00, 1, &qualifies_zero);
    assert(cg_zero_res == 0);
    assert(qualifies_zero == 1);
    printf("  [PASS] IRS IMF Capital Gains Zero Rate Matcher verified.\n");

    // 211. Verify IRS CADE Filing Status Head of Household Dependent Matcher
    printf("[TEST] Validating IRS CADE HoH Dependent Validator...\n");
    int hoh_valid = -1;
    int hoh_res = tsfi_mf_cade_verify_hoh_dependent(4, 2, &hoh_valid);
    assert(hoh_res == 0);
    assert(hoh_valid == 1);
    printf("  [PASS] IRS CADE HoH Dependent Validator verified.\n");

    // 212. Verify IRS IMF Taxable Social Security Formula Checker
    printf("[TEST] Validating IRS IMF Social Security High Taxability Checker...\n");
    int subject_85 = -1;
    int ss_85_res = tsfi_mf_imf_check_ss_high_taxability(40000.00, 10000.00, 1, &subject_85);
    assert(ss_85_res == 0);
    assert(subject_85 == 1);
    printf("  [PASS] IRS IMF Social Security High Taxability Checker verified.\n");

    // 213. Verify IRS IMF Tax-Exempt Interest Reporting Matcher
    printf("[TEST] Validating IRS IMF Tax-Exempt Interest Matcher...\n");
    int interest_match = -1;
    int int_match_res = tsfi_mf_imf_match_tax_exempt_interest(1200.00, 1200.00, &interest_match);
    assert(int_match_res == 0);
    assert(interest_match == 1);
    printf("  [PASS] IRS IMF Tax-Exempt Interest Matcher verified.\n");

    // 214. Verify IRS IMF Net Capital Gains 15% Preferential Rate Bracket Matcher
    printf("[TEST] Validating IRS IMF Capital Gains Fifteen Rate Matcher...\n");
    int qualifies_fifteen = -1;
    int cg_fifteen_res = tsfi_mf_imf_verify_fifteen_rate_capital_gains(150000.00, 1, &qualifies_fifteen);
    assert(cg_fifteen_res == 0);
    assert(qualifies_fifteen == 1);
    printf("  [PASS] IRS IMF Capital Gains Fifteen Rate Matcher verified.\n");

    // 215. Verify IRS CADE Married Filing Jointly (MFJ) Spousal Signature Matcher
    printf("[TEST] Validating IRS CADE MFJ Spousal Signature Matcher...\n");
    int mfj_sig_valid = -1;
    int mfj_sig_res = tsfi_mf_cade_verify_mfj_signatures(2, 1, 1, &mfj_sig_valid);
    assert(mfj_sig_res == 0);
    assert(mfj_sig_valid == 1);
    printf("  [PASS] IRS CADE MFJ Spousal Signature Matcher verified.\n");

    // 216. Verify IRS IMF Net Capital Gains 20% Preferential Rate Bracket Matcher
    printf("[TEST] Validating IRS IMF Capital Gains Twenty Rate Matcher...\n");
    int qualifies_twenty = -1;
    int cg_twenty_res = tsfi_mf_imf_verify_twenty_rate_capital_gains(600000.00, 1, &qualifies_twenty);
    assert(cg_twenty_res == 0);
    assert(qualifies_twenty == 1);
    printf("  [PASS] IRS IMF Capital Gains Twenty Rate Matcher verified.\n");

    // 217. Verify IRS CADE Qualifying Widow(er) Dependent Child Matcher
    printf("[TEST] Validating IRS CADE QW Dependent Matcher...\n");
    int qw_valid = -1;
    int qw_res = tsfi_mf_cade_verify_qw_dependent(5, 1, &qw_valid);
    assert(qw_res == 0);
    assert(qw_valid == 1);
    printf("  [PASS] IRS CADE QW Dependent Matcher verified.\n");

    // 218. Verify IRS IMF Section 179 Property Expense Limit Checker
    printf("[TEST] Validating IRS IMF Section 179 Limit Matcher...\n");
    int s179_valid = -1;
    int s179_res = tsfi_mf_imf_verify_section179_limit(500000.00, &s179_valid);
    assert(s179_res == 0);
    assert(s179_valid == 1);
    printf("  [PASS] IRS IMF Section 179 Limit Matcher verified.\n");

    // 219. Verify IRS CADE Married Filing Separately (MFS) Spousal Name Validator
    printf("[TEST] Validating IRS CADE MFS Spousal Name Validator...\n");
    int mfs_name_valid = -1;
    int mfs_name_res = tsfi_mf_cade_verify_mfs_spouse_name(3, "Jane", "Doe", &mfs_name_valid);
    assert(mfs_name_res == 0);
    assert(mfs_name_valid == 1);
    printf("  [PASS] IRS CADE MFS Spousal Name Validator verified.\n");

    // 220. Verify IRS IMF Section 179 Threshold Phase-Out Matcher
    printf("[TEST] Validating IRS IMF Section 179 Phase-Out Matcher...\n");
    int s179_phaseout = -1;
    int s179_po_res = tsfi_mf_imf_check_section179_phaseout(3500000.00, &s179_phaseout);
    assert(s179_po_res == 0);
    assert(s179_phaseout == 1);
    printf("  [PASS] IRS IMF Section 179 Phase-Out Matcher verified.\n");

    // 221. Verify IRS CADE Schedule B Foreign Account Reporting Indicator Checker
    printf("[TEST] Validating IRS CADE Schedule B Foreign Account Indicator Checker...\n");
    int foreign_indicator_valid = -1;
    int foreign_ind_res = tsfi_mf_cade_check_foreign_account_indicator(2000.00, 1, &foreign_indicator_valid);
    assert(foreign_ind_res == 0);
    assert(foreign_indicator_valid == 1);
    printf("  [PASS] IRS CADE Schedule B Foreign Account Indicator Checker verified.\n");

    // 222. Verify IRS IMF Section 179 Phase-Out Deduction Reducer
    printf("[TEST] Validating IRS IMF Section 179 Reduced Limit Calculator...\n");
    double reduced_limit = -1.0;
    int limit_calc_res = tsfi_mf_imf_calculate_section179_reduced_limit(3150000.00, &reduced_limit);
    assert(limit_calc_res == 0);
    assert(reduced_limit == 1120000.00);
    printf("  [PASS] IRS IMF Section 179 Reduced Limit Calculator verified.\n");

    // 223. Verify IRS CADE Schedule B Part I Interest Source Matcher
    printf("[TEST] Validating IRS CADE Schedule B Interest Matcher...\n");
    double interest_sources[] = {500.00, 300.50, 200.00};
    int interest_sources_match = -1;
    int interest_match_res = tsfi_mf_cade_match_schedule_b_interest(interest_sources, 3, 1000.50, &interest_sources_match);
    assert(interest_match_res == 0);
    assert(interest_sources_match == 1);
    printf("  [PASS] IRS CADE Schedule B Interest Matcher verified.\n");

    // 224. Verify IRS CADE Schedule B Part II Dividend Source Matcher
    printf("[TEST] Validating IRS CADE Schedule B Dividend Matcher...\n");
    double dividend_sources[] = {1000.00, 1500.00, 500.00};
    int dividend_sources_match = -1;
    int dividend_match_res = tsfi_mf_cade_match_schedule_b_dividends(dividend_sources, 3, 3000.00, &dividend_sources_match);
    assert(dividend_match_res == 0);
    assert(dividend_sources_match == 1);
    printf("  [PASS] IRS CADE Schedule B Dividend Matcher verified.\n");

    // 225. Verify IRS IMF Qualifying Surviving Spouse Filing Year Matcher
    printf("[TEST] Validating IRS IMF QSS Filing Year Matcher...\n");
    int qss_year_valid = -1;
    int qss_year_res = tsfi_mf_imf_verify_qss_filing_year(2021, 2023, &qss_year_valid);
    assert(qss_year_res == 0);
    assert(qss_year_valid == 1);
    printf("  [PASS] IRS IMF QSS Filing Year Matcher verified.\n");

    // 226. Verify IRS IMF Section 1244 Small Business Stock Loss Matcher
    printf("[TEST] Validating IRS IMF Section 1244 Loss Matcher...\n");
    int s1244_valid = -1;
    int s1244_res = tsfi_mf_imf_verify_section1244_loss(75000.00, 2, &s1244_valid);
    assert(s1244_res == 0);
    assert(s1244_valid == 1);
    printf("  [PASS] IRS IMF Section 1244 Loss Matcher verified.\n");

    // 227. Verify IRS CADE Schedule B Part I Seller-Financed Interest Disclosure Matcher
    printf("[TEST] Validating IRS CADE Seller-Financed SSN Checker...\n");
    int sfn_valid = -1;
    int sfn_res = tsfi_mf_cade_verify_seller_financed_ssn(1, "123456789", &sfn_valid);
    assert(sfn_res == 0);
    assert(sfn_valid == 1);
    printf("  [PASS] IRS CADE Seller-Financed SSN Checker verified.\n");

    // 228. Verify IRS CADE Schedule B Part I Seller-Financed Interest Buyer Address Matcher
    printf("[TEST] Validating IRS CADE Seller-Financed Address Checker...\n");
    int sfn_addr_valid = -1;
    int sfn_addr_res = tsfi_mf_cade_verify_seller_financed_address(1, "123 Main St", &sfn_addr_valid);
    assert(sfn_addr_res == 0);
    assert(sfn_addr_valid == 1);
    printf("  [PASS] IRS CADE Seller-Financed Address Checker verified.\n");

    // 229. Verify IRS CADE Schedule B Part II Foreign Dividend Source Matcher
    printf("[TEST] Validating IRS CADE Schedule B Foreign Dividend Matcher...\n");
    double foreign_div_sources[] = {250.00, 350.00};
    int foreign_div_match = -1;
    int foreign_div_res = tsfi_mf_cade_match_foreign_dividends(foreign_div_sources, 2, 600.00, &foreign_div_match);
    assert(foreign_div_res == 0);
    assert(foreign_div_match == 1);
    printf("  [PASS] IRS CADE Schedule B Foreign Dividend Matcher verified.\n");

    // 230. Verify IRS CADE Schedule B Part III Foreign Trust Reporting Indicator Checker
    printf("[TEST] Validating IRS CADE Schedule B Foreign Trust Indicator Checker...\n");
    int trust_indicator_valid = -1;
    int trust_ind_res = tsfi_mf_cade_verify_foreign_trust_indicator(1, 1, &trust_indicator_valid);
    assert(trust_ind_res == 0);
    assert(trust_indicator_valid == 1);
    printf("  [PASS] IRS CADE Schedule B Foreign Trust Indicator Checker verified.\n");

    // 231. Verify IRS IMF Section 1244 Small Business Stock Loss Excess Capital Loss Redirection Matcher
    printf("[TEST] Validating IRS IMF Section 1244 Redirection Matcher...\n");
    int redirection_valid = -1;
    int redir_res = tsfi_mf_imf_verify_section1244_excess_redirection(120000.00, 2, 100000.00, 20000.00, &redirection_valid);
    assert(redir_res == 0);
    assert(redirection_valid == 1);
    printf("  [PASS] IRS IMF Section 1244 Redirection Matcher verified.\n");

    // 232. Verify IRS CADE Schedule B Part I Seller-Financed Interest Buyer SSN Formatting Matcher
    printf("[TEST] Validating IRS CADE Seller-Financed SSN Format Validator...\n");
    int sfn_fmt_valid = -1;
    int sfn_fmt_res = tsfi_mf_cade_verify_seller_financed_ssn_format("555123456", &sfn_fmt_valid);
    assert(sfn_fmt_res == 0);
    assert(sfn_fmt_valid == 1);
    printf("  [PASS] IRS CADE Seller-Financed SSN Format Validator verified.\n");

    // 233. Verify IRS IMF Section 179 Property Eligible Expense Allocation Validator
    printf("[TEST] Validating IRS IMF Section 179 Income Limit Matcher...\n");
    int income_limit_valid = -1;
    int income_lim_res = tsfi_mf_imf_verify_section179_income_limit(5000.00, 10000.00, &income_limit_valid);
    assert(income_lim_res == 0);
    assert(income_limit_valid == 1);
    printf("  [PASS] IRS IMF Section 179 Income Limit Matcher verified.\n");

    // 234. Verify IRS CADE Schedule B Part III Foreign Account Country Code Matcher
    printf("[TEST] Validating IRS CADE Foreign Country Code Matcher...\n");
    int cc_valid = -1;
    int cc_res = tsfi_mf_cade_verify_foreign_country_code(1, "UK", &cc_valid);
    assert(cc_res == 0);
    assert(cc_valid == 1);
    printf("  [PASS] IRS CADE Foreign Country Code Matcher verified.\n");

    // 235. Verify IRS IMF Schedule K-1 Distributive Dividend Share Matcher
    printf("[TEST] Validating IRS IMF K-1 Dividend Share Matcher...\n");
    int k1_div_match = -1;
    int k1_div_res = tsfi_mf_imf_match_k1_dividends(1500.00, 1500.00, &k1_div_match);
    assert(k1_div_res == 0);
    assert(k1_div_match == 1);
    printf("  [PASS] IRS IMF K-1 Dividend Share Matcher verified.\n");

    // 236. Verify IRS CADE Schedule B Foreign Account Yes/No and Country Code Consistency Matcher
    printf("[TEST] Validating IRS CADE Foreign Account Consistency Matcher...\n");
    int consistency_valid = -1;
    int consistency_res = tsfi_mf_cade_verify_foreign_account_consistency(1, "FR", &consistency_valid);
    assert(consistency_res == 0);
    assert(consistency_valid == 1);
    printf("  [PASS] IRS CADE Foreign Account Consistency Matcher verified.\n");

    // 237. Verify IRS IMF Schedule K-1 Partnership Distributive Capital Gains Share Matcher
    printf("[TEST] Validating IRS IMF K-1 Capital Gains Share Matcher...\n");
    int k1_cg_match = -1;
    int k1_cg_res = tsfi_mf_imf_match_k1_capital_gains(2500.00, 2500.00, &k1_cg_match);
    assert(k1_cg_res == 0);
    assert(k1_cg_match == 1);
    printf("  [PASS] IRS IMF K-1 Capital Gains Share Matcher verified.\n");

    // 238. Verify IRS CADE Form 1099-DIV Payer TIN Format Validator
    printf("[TEST] Validating IRS CADE DIV Payer TIN Validator...\n");
    int tin_valid = -1;
    int tin_res = tsfi_mf_cade_verify_div_payer_tin("12-3456780", &tin_valid);
    assert(tin_res == 0);
    assert(tin_valid == 1);
    printf("  [PASS] IRS CADE DIV Payer TIN Validator verified.\n");

    // 239. Verify IRS IMF Schedule K-1 Partnership Distributive Interest Share Matcher
    printf("[TEST] Validating IRS IMF K-1 Interest Share Matcher...\n");
    int k1_int_match = -1;
    int k1_int_res = tsfi_mf_imf_match_k1_interest(800.00, 800.00, &k1_int_match);
    assert(k1_int_res == 0);
    assert(k1_int_match == 1);
    printf("  [PASS] IRS IMF K-1 Interest Share Matcher verified.\n");

    // 240. Verify IRS CADE Form 1099-DIV Recipient TIN Format Validator
    printf("[TEST] Validating IRS CADE DIV Recipient TIN Validator...\n");
    int rec_tin_valid = -1;
    int rec_tin_res = tsfi_mf_cade_verify_div_recipient_tin("555-12-3456", &rec_tin_valid);
    assert(rec_tin_res == 0);
    assert(rec_tin_valid == 1);
    printf("  [PASS] IRS CADE DIV Recipient TIN Validator verified.\n");

    // 241. Verify IRS CADE Form 1040 Primary and Secondary SSN Matcher
    printf("[TEST] Validating IRS CADE Distinct SSNs Matcher...\n");
    int distinct_ssns_valid = -1;
    int distinct_res = tsfi_mf_cade_verify_distinct_ssns("111223333", "444556666", 2, &distinct_ssns_valid);
    assert(distinct_res == 0);
    assert(distinct_ssns_valid == 1);
    printf("  [PASS] IRS CADE Distinct SSNs Matcher verified.\n");

    // 242. Verify IRS CADE Form 1099-INT Payer TIN Format Validator
    printf("[TEST] Validating IRS CADE INT Payer TIN Validator...\n");
    int int_payer_valid = -1;
    int int_payer_res = tsfi_mf_cade_verify_int_payer_tin("12-3456780", &int_payer_valid);
    assert(int_payer_res == 0);
    assert(int_payer_valid == 1);
    printf("  [PASS] IRS CADE INT Payer TIN Validator verified.\n");

    // 243. Verify IRS CADE Form 1099-INT Recipient TIN Format Validator
    printf("[TEST] Validating IRS CADE INT Recipient TIN Validator...\n");
    int int_rec_valid = -1;
    int int_rec_res = tsfi_mf_cade_verify_int_recipient_tin("555-12-3456", &int_rec_valid);
    assert(int_rec_res == 0);
    assert(int_rec_valid == 1);
    printf("  [PASS] IRS CADE INT Recipient TIN Validator verified.\n");

    // 244. Verify IRS IMF Schedule B Dividend/Interest Reclassification Matcher
    printf("[TEST] Validating IRS IMF Dividend/Interest Reclassification Matcher...\n");
    double final_int = 0.0, final_div = 0.0;
    int reclass_res = tsfi_mf_imf_verify_reclassified_dividends(1000.00, 500.00, 200.00, &final_int, &final_div);
    assert(reclass_res == 0);
    assert(final_int == 800.00);
    assert(final_div == 700.00);
    printf("  [PASS] IRS IMF Dividend/Interest Reclassification Matcher verified.\n");

    // 245. Verify IRS CADE Form 1040 Joint Return Secondary Spouse Signature Validator
    printf("[TEST] Validating IRS CADE Joint Secondary Signature Validator...\n");
    int joint_sig_valid = -1;
    int joint_sig_res = tsfi_mf_cade_verify_joint_secondary_signature(2, 1, &joint_sig_valid);
    assert(joint_sig_res == 0);
    assert(joint_sig_valid == 1);
    printf("  [PASS] IRS CADE Joint Secondary Signature Validator verified.\n");

    // 246. Verify IRS CADE Form 1099-DIV Payer Address Matcher
    printf("[TEST] Validating IRS CADE DIV Payer Address Matcher...\n");
    int div_addr_valid = -1;
    int div_addr_res = tsfi_mf_cade_verify_div_payer_address("456 Financial Way", &div_addr_valid);
    assert(div_addr_res == 0);
    assert(div_addr_valid == 1);
    printf("  [PASS] IRS CADE DIV Payer Address Matcher verified.\n");

    // 247. Verify IRS CADE Form 1099-INT Payer Address Matcher
    printf("[TEST] Validating IRS CADE INT Payer Address Matcher...\n");
    int int_addr_valid = -1;
    int int_addr_res = tsfi_mf_cade_verify_int_payer_address("789 Banking Rd", &int_addr_valid);
    assert(int_addr_res == 0);
    assert(int_addr_valid == 1);
    printf("  [PASS] IRS CADE INT Payer Address Matcher verified.\n");

    // 248. Verify IRS CADE Form 1099-DIV Recipient Name Matcher
    printf("[TEST] Validating IRS CADE DIV Recipient Name Matcher...\n");
    int div_rec_name_valid = -1;
    int div_rec_name_res = tsfi_mf_cade_verify_div_recipient_name("Alice Smith", &div_rec_name_valid);
    assert(div_rec_name_res == 0);
    assert(div_rec_name_valid == 1);
    printf("  [PASS] IRS CADE DIV Recipient Name Matcher verified.\n");

    // 249. Verify IRS CADE Form 1099-INT Recipient Name Matcher
    printf("[TEST] Validating IRS CADE INT Recipient Name Matcher...\n");
    int int_rec_name_valid = -1;
    int int_rec_name_res = tsfi_mf_cade_verify_int_recipient_name("Bob Jones", &int_rec_name_valid);
    assert(int_rec_name_res == 0);
    assert(int_rec_name_valid == 1);
    printf("  [PASS] IRS CADE INT Recipient Name Matcher verified.\n");

    // 250. Verify NATO STANAG 5066 Header Validator
    printf("[TEST] Validating NATO STANAG 5066 Header Validator...\n");
    const unsigned char s5066_hdr[] = {0x90, 0xEB, 0x05, 0x00, 0x10};
    int s5066_valid = -1;
    int s5066_res = tsfi_mf_nato_verify_stanag5066_header(s5066_hdr, sizeof(s5066_hdr), &s5066_valid);
    assert(s5066_res == 0);
    assert(s5066_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Header Validator verified.\n");

    // 251. Verify NATO STANAG 5066 Kermit Payload Validator
    printf("[TEST] Validating NATO STANAG 5066 Kermit Payload Validator...\n");
    const unsigned char kermit_pkt[] = {0x01, 35, 35, 35, 74};
    int kermit_valid = -1;
    int kermit_res = tsfi_mf_nato_verify_kermit_payload(kermit_pkt, sizeof(kermit_pkt), &kermit_valid);
    assert(kermit_res == 0);
    assert(kermit_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Kermit Payload Validator verified.\n");

    // 252. Verify NATO STANAG 5066 Client Protocol Data Unit (C_PDU) Type Matcher
    printf("[TEST] Validating NATO STANAG 5066 C_PDU Type Matcher...\n");
    int cpdu_type_valid = -1;
    int cpdu_type_res = tsfi_mf_nato_verify_cpdu_type(3, &cpdu_type_valid);
    assert(cpdu_type_res == 0);
    assert(cpdu_type_valid == 1);
    printf("  [PASS] NATO STANAG 5066 C_PDU Type Matcher verified.\n");

    // 253. Verify NATO STANAG 5066 HF MTU Segment Size Validator
    printf("[TEST] Validating NATO STANAG 5066 HF MTU Segment Size Validator...\n");
    int seg_size_valid = -1;
    int seg_size_res = tsfi_mf_nato_verify_segment_size(1024, &seg_size_valid);
    assert(seg_size_res == 0);
    assert(seg_size_valid == 1);
    printf("  [PASS] NATO STANAG 5066 HF MTU Segment Size Validator verified.\n");

    // 254. Verify NATO STANAG 5066 Service Access Point (SAP) Number Matcher
    printf("[TEST] Validating NATO STANAG 5066 SAP Matcher...\n");
    int sap_valid = -1;
    int sap_res = tsfi_mf_nato_verify_sap_number(12, &sap_valid);
    assert(sap_res == 0);
    assert(sap_valid == 1);
    printf("  [PASS] NATO STANAG 5066 SAP Matcher verified.\n");

    // 255. Verify NATO STANAG 5066 ARQ Window Size Validator
    printf("[TEST] Validating NATO STANAG 5066 ARQ Window Size Validator...\n");
    int window_valid = -1;
    int window_res = tsfi_mf_nato_verify_arq_window(64, &window_valid);
    assert(window_res == 0);
    assert(window_valid == 1);
    printf("  [PASS] NATO STANAG 5066 ARQ Window Size Validator verified.\n");

    // 256. Verify NATO STANAG 5066 Priority Level Checker
    printf("[TEST] Validating NATO STANAG 5066 Priority Level Checker...\n");
    int priority_valid = -1;
    int priority_res = tsfi_mf_nato_verify_priority_level(8, &priority_valid);
    assert(priority_res == 0);
    assert(priority_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Priority Level Checker verified.\n");

    // 257. Verify NATO STANAG 5066 Sync Sequence Type Matcher
    printf("[TEST] Validating NATO STANAG 5066 Sync Sequence Matcher...\n");
    int sync_seq_valid = -1;
    int sync_seq_res = tsfi_mf_nato_match_sync_sequence(0x90EB, &sync_seq_valid);
    assert(sync_seq_res == 0);
    assert(sync_seq_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Sync Sequence Matcher verified.\n");

    // 258. Verify NATO STANAG 5066 Client Protocol Data Unit (C_PDU) Length Checker
    printf("[TEST] Validating NATO STANAG 5066 C_PDU Length Checker...\n");
    int cpdu_len_valid = -1;
    int cpdu_len_res = tsfi_mf_nato_verify_cpdu_length(2048, &cpdu_len_valid);
    assert(cpdu_len_res == 0);
    assert(cpdu_len_valid == 1);
    printf("  [PASS] NATO STANAG 5066 C_PDU Length Checker verified.\n");

    // 259. Verify NATO STANAG 5066 Transmission Mode Selector
    printf("[TEST] Validating NATO STANAG 5066 Tx Mode Validator...\n");
    int tx_mode_valid = -1;
    int tx_mode_res = tsfi_mf_nato_verify_tx_mode(1, &tx_mode_valid);
    assert(tx_mode_res == 0);
    assert(tx_mode_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Tx Mode Validator verified.\n");

    // 260. Verify NATO STANAG 5066 Short-TIN Node Address Validator
    printf("[TEST] Validating NATO STANAG 5066 Short-TIN Address Matcher...\n");
    int short_tin_valid = -1;
    int short_tin_res = tsfi_mf_nato_verify_short_tin(50000, &short_tin_valid);
    assert(short_tin_res == 0);
    assert(short_tin_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Short-TIN Address Matcher verified.\n");

    // 261. Verify NATO STANAG 5066 Segmented Transfer Offset Matcher
    printf("[TEST] Validating NATO STANAG 5066 Segmented Offset Matcher...\n");
    int seg_offset_valid = -1;
    int seg_offset_res = tsfi_mf_nato_verify_segment_offset(1000, 500, 2000, &seg_offset_valid);
    assert(seg_offset_res == 0);
    assert(seg_offset_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Segmented Offset Matcher verified.\n");

    // 262. Verify NATO Short-TIN IRS Exclusivity Matcher
    printf("[TEST] Validating NATO Short-TIN IRS Exclusivity Matcher...\n");
    int tin_isolated = -1;
    int tin_isol_res = tsfi_mf_cross_verify_tin_isolation("777665555", &tin_isolated);
    assert(tin_isol_res == 0);
    assert(tin_isolated == 1);
    printf("  [PASS] NATO Short-TIN IRS Exclusivity Matcher verified.\n");

    printf("[SUCCESS] Micro Focus COBOL standard compatibility checks part 2 completed successfully!\n");
    return 0;
}
