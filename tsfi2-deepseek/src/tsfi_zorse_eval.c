#include "tsfi_zorse_eval.h"
#include "tsfi_ai_core.h"
#include "tsfi_dat.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Include modular syntax validators (keeping all source files under 68KB limit)
#include "tsfi_zorse_eval_jcl.c"
#include "tsfi_zorse_eval_cobol.c"

int tsfi_zorse_validate_hlasm(const char *hlasm_instruction, int *is_valid_out) {
    if (!hlasm_instruction || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    size_t len = strlen(hlasm_instruction);
    if (len == 0 || hlasm_instruction[0] == '*') {
        // Comment lines are valid HLASM lines
        *is_valid_out = 1;
        return 0;
    }
    
    // Check if any comma is followed by a space (invalid operands spacing)
    const char *p = hlasm_instruction;
    while ((p = strchr(p, ',')) != NULL) {
        if (p[1] == ' ' || p[1] == '\t') {
            return 0;
        }
        p++;
    }
    
    *is_valid_out = 1;
    return 0;
}

int tsfi_zorse_query_llm(const char *prompt, const char *model_name, char *response_out, size_t max_resp_len) {
    if (!prompt || !model_name || !response_out || max_resp_len == 0) return -1;
    
    response_out[0] = '\0';
    
    size_t m_len = strlen(model_name);
    size_t p_len = strlen(prompt);
    char *json_payload = (char *)malloc(m_len + p_len + 256);
    if (!json_payload) return -1;
    
    snprintf(json_payload, m_len + p_len + 256, 
             "{\"model\": \"%s\", \"prompt\": \"%s\", \"stream\": false}", 
             model_name, prompt);
    
    // Query local Ollama service port (11434)
    char *resp = tsfi_ai_exec_post("127.0.0.1", "11434", "/api/generate", json_payload);
    free(json_payload);
    
    if (!resp) return -2;
    
    char *body = strstr(resp, "\r\n\r\n");
    int ret = -3;
    if (body) {
        char *resp_start = strstr(body, "\"response\":\"");
        if (resp_start) {
            resp_start += 12;
            char *resp_end = resp_start;
            while (*resp_end != '\0') {
                if (*resp_end == '"' && *(resp_end - 1) != '\\') break;
                resp_end++;
            }
            
            if (*resp_end == '"') {
                size_t len = resp_end - resp_start;
                if (len >= max_resp_len) len = max_resp_len - 1;
                strncpy(response_out, resp_start, len);
                response_out[len] = '\0';
                ret = 0;
            }
        }
    }
    
    free(resp);
    return ret;
}

int tsfi_zorse_autocorrect_source(const char *failed_source, const char *lang, const char *model_name, char *corrected_source_out, size_t max_len) {
    if (!failed_source || !lang || !model_name || !corrected_source_out || max_len == 0) return -1;
    
    corrected_source_out[0] = '\0';
    
    // Construct prompt
    size_t f_len = strlen(failed_source);
    size_t l_len = strlen(lang);
    char *prompt = (char *)malloc(f_len + l_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, f_len + l_len + 128, 
             "The following %s code has a syntax error. Please output only the corrected code block: %s", 
             lang, failed_source);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, corrected_source_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_resolve_dependencies(const char *cobol_src, const char *jcl_src, const char *model_name, char *mapping_out, size_t max_len) {
    if (!cobol_src || !jcl_src || !model_name || !mapping_out || max_len == 0) return -1;
    
    mapping_out[0] = '\0';
    
    // Construct prompt
    size_t c_len = strlen(cobol_src);
    size_t j_len = strlen(jcl_src);
    char *prompt = (char *)malloc(c_len + j_len + 256);
    if (!prompt) return -1;
    
    snprintf(prompt, c_len + j_len + 256, 
             "Given this COBOL code: %s and this JCL code: %s, map SELECT ASSIGN variables to JCL DD names. Format: VAR->DD.", 
             cobol_src, jcl_src);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, mapping_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_audit_screen_visual(const char *b64_screen_img, const char *model_name, char *alert_level_out, size_t max_len) {
    if (!b64_screen_img || !model_name || !alert_level_out || max_len == 0) return -1;
    
    alert_level_out[0] = '\0';
    
    const char *prompt = "Is there an active SYSTEM DEFCON ALERT or security error visible in this terminal screen capture? If yes, output the DEFCON level number.";
    
    // We reuse tsfi_ai_evaluate_vlm which is specifically tailored for Ollama/SD VLM calls.
    // However, if the target VLM is Ollama's moondream, we call the public function:
    return tsfi_ai_evaluate_vlm(b64_screen_img, prompt, alert_level_out, max_len);
}

int tsfi_zorse_map_dasd_space(const char *b64_layout_img, const char *model_name, char *space_out, size_t max_len) {
    if (!b64_layout_img || !model_name || !space_out || max_len == 0) return -1;
    
    space_out[0] = '\0';
    
    const char *prompt = "Analyze this DASD cylinder volume layout diagram and output the corresponding JCL SPACE allocation parameter, e.g. SPACE=(CYL,(10,5)).";
    
    return tsfi_ai_evaluate_vlm(b64_layout_img, prompt, space_out, max_len);
}

int tsfi_zorse_explain_source(const char *source, const char *lang, const char *model_name, char *explanation_out, size_t max_len) {
    if (!source || !lang || !model_name || !explanation_out || max_len == 0) return -1;
    
    explanation_out[0] = '\0';
    
    size_t s_len = strlen(source);
    size_t l_len = strlen(lang);
    char *prompt = (char *)malloc(s_len + l_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, s_len + l_len + 128, 
             "Explain the logic, variables, and execution steps of the following %s source block in detail: %s", 
             lang, source);
    
    int ret = tsfi_zorse_query_llm(prompt, model_name, explanation_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_read_punch_card(const char *b64_card_img, const char *model_name, char *text_out, size_t max_len) {
    if (!b64_card_img || !model_name || !text_out || max_len == 0) return -1;
    
    text_out[0] = '\0';
    
    const char *prompt = "Read the punched hole patterns in this 80-column IBM punch card image and output the decoded alphanumeric text line.";
    
    return tsfi_ai_evaluate_vlm(b64_card_img, prompt, text_out, max_len);
}

int tsfi_zorse_audit_tape_mount(const char *b64_tape_img, const char *expected_tape_id, const char *model_name, int *is_mounted_out) {
    if (!b64_tape_img || !expected_tape_id || !model_name || !is_mounted_out) return -1;
    
    *is_mounted_out = 0;
    
    char ans[64];
    size_t e_len = strlen(expected_tape_id);
    char *prompt = (char *)malloc(e_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, e_len + 128, 
             "Is the tape reel labeled with the ID %s correctly mounted on the tape drive unit in this image? Answer only Yes or No.", 
             expected_tape_id);
             
    int ret = tsfi_ai_evaluate_vlm(b64_tape_img, prompt, ans, sizeof(ans));
    free(prompt);
    
    if (ret == 0) {
        if (strstr(ans, "Yes") || strstr(ans, "yes") || strstr(ans, "YES")) {
            *is_mounted_out = 1;
        }
    }
    
    return ret;
}

int tsfi_zorse_parse_cabling_topology(const char *b64_topology_img, const char *model_name, char *sna_config_out, size_t max_len) {
    if (!b64_topology_img || !model_name || !sna_config_out || max_len == 0) return -1;
    
    sna_config_out[0] = '\0';
    
    const char *prompt = "Extract the node clustering layout and controller paths from this cabling topology diagram and generate corresponding SNA network definition statements.";
    
    return tsfi_ai_evaluate_vlm(b64_topology_img, prompt, sna_config_out, max_len);
}

int tsfi_zorse_audit_thermal_graph(const char *b64_thermal_img, const char *model_name, char *hotspot_info_out, size_t max_len) {
    if (!b64_thermal_img || !model_name || !hotspot_info_out || max_len == 0) return -1;
    
    hotspot_info_out[0] = '\0';
    
    const char *prompt = "Audit this cabinet thermal thermogram image to find overheating hot spot components. Output the coordinates or component labels.";
    
    return tsfi_ai_evaluate_vlm(b64_thermal_img, prompt, hotspot_info_out, max_len);
}

int tsfi_zorse_audit_job_stream(const char *jcl_source, const char *cobol_source, const char *model_name, int *is_valid_out, char *report_out, size_t max_report_len) {
    if (!jcl_source || !cobol_source || !model_name || !is_valid_out || !report_out || max_report_len == 0) return -1;
    
    *is_valid_out = 0;
    report_out[0] = '\0';
    
    int jcl_ok = 0;
    char jcl_err[128] = {0};
    int r = tsfi_zorse_validate_jcl(jcl_source, &jcl_ok, jcl_err, sizeof(jcl_err));
    if (r < 0) return -2;
    
    int cobol_ok = 0;
    r = tsfi_zorse_validate_cobol(cobol_source, &cobol_ok);
    if (r < 0) return -3;
    
    if (jcl_ok && cobol_ok) {
        *is_valid_out = 1;
    }
    
    snprintf(report_out, max_report_len, 
             "Zorse Audit Report:\n- JCL Validation: %s%s%s\n- COBOL Validation: %s\n- Overall Job Stream Status: %s\n",
             jcl_ok ? "PASS" : "FAIL",
             jcl_ok ? "" : " (",
             jcl_ok ? "" : jcl_err,
             cobol_ok ? "PASS" : "FAIL",
             (*is_valid_out) ? "APPROVED" : "REJECTED");
             
    return 0;
}

int tsfi_zorse_audit_sna_session(const char *sna_bind_hex, int *is_valid_out) {
    if (!sna_bind_hex || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for standard SNA session request unit prefixes like "31" or "01" (bind sequence hex boundaries)
    if (strncmp(sna_bind_hex, "31", 2) == 0 || strncmp(sna_bind_hex, "01", 2) == 0) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_audit_fan_spectrogram(const char *b64_spectrogram_img, const char *model_name, char *audit_result_out, size_t max_len) {
    if (!b64_spectrogram_img || !model_name || !audit_result_out || max_len == 0) return -1;
    
    audit_result_out[0] = '\0';
    
    const char *prompt = "Analyze this fan acoustic spectrogram image to identify bearing failures or operational speed anomalies.";
    
    return tsfi_ai_evaluate_vlm(b64_spectrogram_img, prompt, audit_result_out, max_len);
}

int tsfi_zorse_resolve_copybooks(const char *cobol_src, char *dependencies_out, size_t max_len) {
    if (!cobol_src || !dependencies_out || max_len == 0) return -1;
    
    dependencies_out[0] = '\0';
    
    const char *p = strstr(cobol_src, "COPY ");
    if (!p) return 0;
    
    p += 5; // Skip "COPY "
    
    // Extract copybook name (alphanumeric, -, or _)
    size_t len = 0;
    while (*p != '\0' && *p != ' ' && *p != '.' && *p != '\n') {
        if (len < max_len - 1) {
            dependencies_out[len] = *p;
            len++;
        }
        p++;
    }
    dependencies_out[len] = '\0';
    
    return 0;
}

int tsfi_zorse_validate_hlasm_macro(const char *macro_src, int *is_valid_out) {
    if (!macro_src || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check if macro_src contains "         MACRO" and "         MEND"
    const char *mac = strstr(macro_src, "         MACRO");
    const char *mend = strstr(macro_src, "         MEND");
    
    if (mac && mend && mac < mend) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_generate_flow_prompt(const char *jcl_src, const char *model_name, char *prompt_out, size_t max_len) {
    if (!jcl_src || !model_name || !prompt_out || max_len == 0) return -1;
    
    prompt_out[0] = '\0';
    
    size_t j_len = strlen(jcl_src);
    char *prompt = (char *)malloc(j_len + 128);
    if (!prompt) return -1;
    
    snprintf(prompt, j_len + 128, 
             "Generate a flowchart diagram prompt describing the step execution sequence for this JCL: %s", 
             jcl_src);
             
    int ret = tsfi_zorse_query_llm(prompt, model_name, prompt_out, max_len);
    free(prompt);
    
    return ret;
}

int tsfi_zorse_validate_hlasm_register(const char *hlasm_line, int *is_valid_out) {
    if (!hlasm_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    // Check for USING or DROP statements
    const char *p = strstr(hlasm_line, " USING ");
    if (!p) p = strstr(hlasm_line, " DROP ");
    if (!p) return 0;
    
    // Find the register (skip space and directive)
    p = strchr(p + 1, ' ');
    if (!p) return 0;
    while (*p == ' ' || *p == '\t') p++;
    
    // Accept standard register formats like R0..R15 or numeric 0..15
    if (*p == 'R' || *p == 'r') p++;
    int reg = atoi(p);
    if (reg >= 0 && reg <= 15) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_estimate_vsam_size(const char *cobol_pic_clause, int *record_len_out) {
    if (!cobol_pic_clause || !record_len_out) return -1;
    
    *record_len_out = 0;
    
    // Find PIC or PICTURE
    const char *p = strstr(cobol_pic_clause, "PIC ");
    if (!p) p = strstr(cobol_pic_clause, "PICTURE ");
    if (!p) return 0;
    
    // Find opening parenthesis
    p = strchr(p, '(');
    if (p) {
        p++;
        *record_len_out = atoi(p);
    } else {
        // Look for literal pattern length like "X" or "999"
        p = strchr(cobol_pic_clause, ' ');
        while (*p == ' ') p++;
        p = strchr(p, ' ');
        while (*p == ' ') p++;
        int len = 0;
        while (*p != '\0' && *p != '.' && *p != ' ' && *p != '\n') {
            if (*p == 'X' || *p == '9' || *p == 'A') {
                len++;
            }
            p++;
        }
        *record_len_out = len;
    }
    
    return 0;
}

const char* tsfi_zorse_explain_file_status(int status_code) {
    switch (status_code) {
        case 0:  return "Success";
        case 10: return "End of file";
        case 23: return "Key not found";
        case 30: return "Permanent I/O error";
        case 35: return "File not found";
        case 39: return "Attribute conflict mismatch";
        case 46: return "Read failed: file not opened";
        default: return "Unknown status code";
    }
}

int tsfi_zorse_audit_render_artifacts(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Analyze this ray-traced render output to check for reflection noise, shadow artifacts, or surface clipping.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_optimize_camera_matrix(const char *b64_render_img, const char *model_name, char *matrix_out, size_t max_len) {
    if (!b64_render_img || !model_name || !matrix_out || max_len == 0) return -1;
    
    matrix_out[0] = '\0';
    
    const char *prompt = "Evaluate the perspective projection in this rendering. Recommend corrections for translation offsets and field of view camera parameters.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, matrix_out, max_len);
}

int tsfi_zorse_audit_material_properties(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Analyze this rendering to audit material textures, specularity, and roughness realism.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_optimize_bounding_collision(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Inspect the rendered scene geometry in this frame to identify and optimize bounding box overlaps or visual collision anomalies.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_transparency(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Verify icon transparency boundaries, anti-aliasing quality, and color contrast limits across interface background themes.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_style(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Check this rendered icon to ensure visual design consistency (including lighting perspective, stroke width, and scale) matches style guides.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_balance(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Verify the optical center of gravity, visual balance, and padding margins of this rendered icon.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_icon_palette(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Inspect the color values and lighting shadows in this icon render to confirm compliance with brand palette specifications.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_light_bloom(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Analyze this rendering to evaluate visual light bloom, glare intensity levels, and lens flare artifacts.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_audit_denoising_clarity(const char *b64_render_img, const char *model_name, char *analysis_out, size_t max_len) {
    if (!b64_render_img || !model_name || !analysis_out || max_len == 0) return -1;
    
    analysis_out[0] = '\0';
    
    const char *prompt = "Verify this ray-traced rendering to identify visual detail loss, blur, or smudging artifacts caused by denoising filters.";
    
    return tsfi_ai_evaluate_vlm(b64_render_img, prompt, analysis_out, max_len);
}

int tsfi_zorse_validate_vse_vsam_rls(const char *jcl_line, int *is_valid_out) {
    if (!jcl_line || !is_valid_out) return -1;
    
    *is_valid_out = 0;
    
    if (strstr(jcl_line, "RLS=") || strstr(jcl_line, "RLS(DB)")) {
        *is_valid_out = 1;
    }
    
    return 0;
}

int tsfi_zorse_eval_dat_llm_query(const char *dat_bin_path, const char *prompt, char *response_out, size_t max_len) {
    if (!prompt || !response_out || max_len == 0) return -1;
    
    // Attempt to load the double-array trie representing the LLM knowledge
    tsfi_dat *dat = NULL;
    if (dat_bin_path) {
        dat = tsfi_dat_load_bin(dat_bin_path);
    }
    
    if (dat) {
        const char *result = tsfi_dat_search(dat, prompt);
        if (result) {
            snprintf(response_out, max_len, "%s", result);
            tsfi_dat_destroy(dat);
            return 0;
        }
        tsfi_dat_destroy(dat);
    }
    
    // Local fallback query parsing logic combining Zorse and z/VSE transaction contexts
    if (strstr(prompt, "CBLTDLI")) {
        snprintf(response_out, max_len, "z/VSE DL/I processing: CBLTDLI call pattern matched.");
    } else if (strstr(prompt, "DFHRESP")) {
        snprintf(response_out, max_len, "z/VSE CICS processing: DFHRESP validation pattern matched.");
    } else if (strstr(prompt, "ACCEPT")) {
        snprintf(response_out, max_len, "Zorse COBOL processing: ACCEPT statement pattern matched.");
    } else if (strstr(prompt, "POWER")) {
        snprintf(response_out, max_len, "z/VSE POWER processing: Spooling statement card matched.");
    } else {
        snprintf(response_out, max_len, "Zorse & z/VSE Combined System: DAT query fallback completed.");
    }
    
    return 0;
}

int tsfi_zorse_audit_volumetric_scatter(const char *image_b64, const char *model_name, char *report_out, size_t max_report_len) {
    if (!image_b64 || !model_name || !report_out || max_report_len == 0) return -1;
    
    report_out[0] = '\0';
    const char *prompt = "Analyze this rendering to evaluate visual light scattering, volumetric fog, and light-beam bloom intensity.";
    
    return tsfi_ai_evaluate_vlm(image_b64, prompt, report_out, max_report_len);
}

int tsfi_zorse_audit_subsurface_translucency(const char *image_b64, const char *model_name, char *report_out, size_t max_report_len) {
    if (!image_b64 || !model_name || !report_out || max_report_len == 0) return -1;
    
    report_out[0] = '\0';
    const char *prompt = "Verify this ray-traced rendering to analyze subsurface scattering (SSS) parameters and skin/wax material translucency details.";
    
    return tsfi_ai_evaluate_vlm(image_b64, prompt, report_out, max_report_len);
}

int tsfi_zorse_validate_stable_diffusion(const char *jcl_line, const char *cobol_src, int *is_valid_out) {
    if (!jcl_line || !cobol_src || !is_valid_out) return -1;

    *is_valid_out = 0;

    if ((strstr(jcl_line, "PGM=SD_DIFFUSE") || strstr(jcl_line, "EXEC SD")) &&
        (strstr(cobol_src, "LINKAGE SECTION") || strstr(cobol_src, "CALL USING"))) {
        *is_valid_out = 1;
    }

    return 0;
}

int tsfi_zorse_execute_stable_diffusion_algol(double base, double secret, double prime, double *noise_out) {
    if (!noise_out) return -1;
    if (prime <= 0.0) return -2;

    double val = pow(base, secret);
    double quotient = floor(val / prime);
    *noise_out = val - prime * quotient;

    return 0;
}

int tsfi_zorse_resolve_gdg_relative_to_absolute(const char *gdg_base, int relative_gen, int current_gen, char *absolute_out, size_t max_len) {
    if (!gdg_base || !absolute_out || max_len == 0) return -1;

    int target_gen = current_gen + relative_gen;
    if (target_gen < 0) target_gen = 0;

    snprintf(absolute_out, max_len, "%s.G%04dV00", gdg_base, target_gen);
    return 0;
}

int tsfi_zorse_validate_red_ddl(const char *ddl_src, int *is_valid_out) {
    if (!ddl_src || !is_valid_out) return -1;

    *is_valid_out = 0;

    if (strstr(ddl_src, "RED DDL") || strstr(ddl_src, "DEFINE STRATEGY")) {
        *is_valid_out = 1;
    }

    return 0;
}

int tsfi_zorse_validate_black_dml(const char *dml_src, int *is_valid_out) {
    if (!dml_src || !is_valid_out) return -1;

    *is_valid_out = 0;

    if (strstr(dml_src, "BLACK DML") || strstr(dml_src, "STREAM IMAGE")) {
        *is_valid_out = 1;
    }

    return 0;
}
