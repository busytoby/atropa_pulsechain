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

    // 171. Verify IRS IMF Standard Mileage Rate Business Expense Matcher
    printf("[TEST] Validating IRS IMF Mileage Deduction Matcher...\n");
    int mileage_valid = -1;
    int mileage_res = tsfi_mf_imf_verify_mileage_deduction(100.0, 67.00, 0.67, &mileage_valid);
    assert(mileage_res == 0);
    assert(mileage_valid == 1);
    printf("  [PASS] IRS IMF Mileage Deduction Matcher verified.\n");

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

    printf("[SUCCESS] Micro Focus COBOL standard compatibility checks part 2 completed successfully!\n");
    return 0;
}
