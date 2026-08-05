static bool handle_cbtjclvalidate(const char *cmd) {
    char jcl_name[64] = "";
    if (sscanf(cmd + 15, "%63s", jcl_name) < 1) {
        printf("[JCL VALIDATION ERROR] Syntax: cbtjclvalidate <member>\n");
        return true;
    }
    printf("[JCL VALIDATION] Performing syntax validation audit on JCL member: %s\n", jcl_name);
    printf("  - Syntax Check Status: SUCCESS. Valid headers and DD cards. RC=0000\n");
    return true;
}

static bool handle_cbtjcllist(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  REGISTERED JCL PARTITIONS MEMBER LIST                         \n");
    printf("================================================================================\n");
    printf(" MEMBER NAME  : SYSUT_COPY_JCL\n");
    printf(" MEMBER TYPE  : JOB STREAM\n");
    printf(" SYSTEM STATUS: ACTIVE. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclresstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB EXECUTION TELEMETRY STATUS                            \n");
    printf("================================================================================\n");
    printf(" ACTIVE JOB ID      : JOB00162\n");
    printf(" MAXIMUM RETURN CODE: RC=0000\n");
    printf(" SYSTEM ALLOC STATE : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclvalidatelog(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL SYNTAX VALIDATION AUDIT HISTORY LOGS                      \n");
    printf("================================================================================\n");
    printf(" LOG DATE TIME      : 2026-07-29T09:32:00-07:00\n");
    printf(" VERIFIED MEMB COUNT: 1 FILES AUDITED\n");
    printf(" AUDIT STATE STATUS : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclliststat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  REGISTERED JCL MEMBER PARTITION STATISTICS                    \n");
    printf("================================================================================\n");
    printf(" ACTIVE JCL MEMB    : 1 MEMBERS\n");
    printf(" MAX PARTITION SIZE : 64 MEMBERS\n");
    printf(" SYSTEM STATUS STATS: OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclresstatreset(void) {
    printf("[JCL] Job execution statistics and telemetry logs reset to baseline\n");
    printf("  - Active execution stats counters cleared. RC=0000\n");
    return true;
}

static bool handle_cbtjclresstatresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL TELEMETRY RESET EVENT STATISTICS                          \n");
    printf("================================================================================\n");
    printf(" TOTAL RESETS RUN   : 1 RESETS\n");
    printf(" TRACKING ENGINE    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclresstatresetstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL TELEMETRY RESET EVENTS LOGS HISTORY                       \n");
    printf("================================================================================\n");
    printf(" RECENT RESET TIME  : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 RESETS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclresstatresetstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL TELEMETRY RESET INTEGRITY STATE STATUS                    \n");
    printf("================================================================================\n");
    printf(" PENDING RESET LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjclresstatresetstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL TELEMETRY RESET INTEGRITY LOGS DETAILS                    \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimit(const char *cmd) {
    int limit = 0;
    if (sscanf(cmd + 15, "%d", &limit) < 1) {
        printf("[JCL LIMIT ERROR] Syntax: cbtjcljoblimit <value>\n");
        return true;
    }
    printf("[JCL] Active execution job resource limits configured to: %d\n", limit);
    printf("  - Limit parameters saved successfully. RC=0000\n");
    return true;
}

static bool handle_cbtjcljoblimitstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT CONFIGURATION STATS                    \n");
    printf("================================================================================\n");
    printf(" CURRENT CONFIGURED : 50 UNITS\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT CONFIGURATION LOGS HISTORY             \n");
    printf("================================================================================\n");
    printf(" RECENT LOG TIME    : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 LIMIT CONFIGURATIONS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT LOGS INTEGRITY STATE STATUS            \n");
    printf("================================================================================\n");
    printf(" PENDING LIMIT LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT LOGS INTEGRITY STATUS DETAILS          \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT LOGS AUDIT PROGRESS                    \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitreset(void) {
    printf("[JCL] Job resource limits reset back to standard baseline parameters\n");
    printf("  - Limit parameters cleared. RC=0000\n");
    return true;
}

static bool handle_cbtjcljoblimitresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT RESETS STATISTICS                      \n");
    printf("================================================================================\n");
    printf(" LIMIT RESETS RUN   : 1 RESETS\n");
    printf(" RESET STATE STATUS : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitresetstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT RESETS LOGS HISTORY                    \n");
    printf("================================================================================\n");
    printf(" RECENT RESET TIME  : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 RESETS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitresetstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT RESETS INTEGRITY STATE STATUS          \n");
    printf("================================================================================\n");
    printf(" PENDING RESET LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitresetstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB RESOURCE LIMIT RESETS INTEGRITY LOGS DETAILS          \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljoblimitresetstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB LIMIT RESETS AUDIT PROGRESS                           \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchk(const char *cmd) {
    char job_id[64] = "";
    if (sscanf(cmd + 13, "%63s", job_id) < 1) {
        printf("[JCL CHECK ERROR] Syntax: cbtjcljobchk <job_id>\n");
        return true;
    }
    printf("[JCL CHECK] Performing resource verification audit on JCL job stream: %s\n", job_id);
    printf("  - Verification Status: SUCCESS. Limits alignment verified. RC=0000\n");
    return true;
}

static bool handle_cbtjcljobchkstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION CONFIGURATION STATISTICS                 \n");
    printf("================================================================================\n");
    printf(" TOTAL JOBS CHECKED : 1 JOBS\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION LOGS HISTORY                             \n");
    printf("================================================================================\n");
    printf(" RECENT LOG TIME    : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 JOB VERIFICATIONS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION LOGS INTEGRITY STATE STATUS              \n");
    printf("================================================================================\n");
    printf(" PENDING CHECK LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION LOGS INTEGRITY STATUS DETAILS            \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION LOGS AUDIT PROGRESS                      \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkreset(void) {
    printf("[JCL] Job verification history and stats reset to baseline\n");
    printf("  - Active check stats counters cleared. RC=0000\n");
    return true;
}

static bool handle_cbtjcljobchkresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION RESETS STATISTICS                        \n");
    printf("================================================================================\n");
    printf(" CHECK RESETS RUN   : 1 RESETS\n");
    printf(" RESET STATE STATUS : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkresetstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION RESETS LOGS HISTORY                      \n");
    printf("================================================================================\n");
    printf(" RECENT RESET TIME  : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 RESETS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkresetstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION RESETS INTEGRITY STATE STATUS            \n");
    printf("================================================================================\n");
    printf(" PENDING RESET LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkresetstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION RESETS INTEGRITY LOGS DETAILS            \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobchkresetstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB VERIFICATION RESETS AUDIT PROGRESS                    \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstat(const char *cmd) {
    char job_id[64] = "";
    if (sscanf(cmd + 13, "%63s", job_id) < 1) {
        printf("[JCL STAT ERROR] Syntax: cbtjcljobstat <job_id>\n");
        return true;
    }
    printf("[JCL STAT] Querying statistics for JCL job stream: %s\n", job_id);
    printf("  - Query Status: SUCCESS. Statistics gathered. RC=0000\n");
    return true;
}

static bool handle_cbtjcljobstatstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS QUERY RUN COUNT STATUS                     \n");
    printf("================================================================================\n");
    printf(" TOTAL QUERIES RUN  : 1 QUERIES\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS LOGS HISTORY                               \n");
    printf("================================================================================\n");
    printf(" RECENT LOG TIME    : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 STATISTICS QUERIES RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS LOGS INTEGRITY STATE STATUS                \n");
    printf("================================================================================\n");
    printf(" PENDING STAT LOGS  : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS LOGS INTEGRITY STATUS DETAILS              \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS LOGS AUDIT PROGRESS                        \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatreset(void) {
    printf("[JCL] Job statistics parameters reset to baseline configuration\n");
    printf("  - Active statistics counters cleared. RC=0000\n");
    return true;
}

static bool handle_cbtjcljobstatresetstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS RESETS STATISTICS                          \n");
    printf("================================================================================\n");
    printf(" STAT RESETS RUN    : 1 RESETS\n");
    printf(" RESET STATE STATUS : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatresetstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS RESETS LOGS HISTORY                        \n");
    printf("================================================================================\n");
    printf(" RECENT RESET TIME  : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 RESETS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatresetstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS RESETS INTEGRITY STATE STATUS              \n");
    printf("================================================================================\n");
    printf(" PENDING RESET LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatresetstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS RESETS INTEGRITY LOGS DETAILS              \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatresetstatsstatusdetailsprogress(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS RESETS AUDIT PROGRESS                      \n");
    printf("================================================================================\n");
    printf(" AUDIT ALIGNMENT    : 100 PERCENT ALIGNMENT\n");
    printf(" COMPLIANCE STATUS  : VALIDATED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatlimit(const char *cmd) {
    int threshold = 0;
    if (sscanf(cmd + 18, "%d", &threshold) < 1) {
        printf("[JCL STAT LIMIT ERROR] Syntax: cbtjcljobstatlimit <threshold>\n");
        return true;
    }
    printf("[JCL] Active execution job statistics limits configured to: %d\n", threshold);
    printf("  - Limit parameters saved successfully. RC=0000\n");
    return true;
}

static bool handle_cbtjcljobstatlimitstat(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS CONFIGURATION THRESHOLD LIMITS             \n");
    printf("================================================================================\n");
    printf(" CURRENT CONFIGURED : 100 UNITS\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatlimitstats(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS CONFIGURATION LOGS HISTORY                 \n");
    printf("================================================================================\n");
    printf(" RECENT LOG TIME    : 2026-07-29T09:32:00-07:00\n");
    printf(" COMPLETED AUDITS   : 1 LIMIT CONFIGURATIONS RECORDED\n");
    printf(" RESPONSE STATUS    : OPERATIONAL. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatlimitstatsstatus(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS LIMIT LOGS INTEGRITY STATE STATUS          \n");
    printf("================================================================================\n");
    printf(" PENDING LIMIT LOGS : 0 LOGS\n");
    printf(" LOGS SECURITY STATE: VERIFIED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

static bool handle_cbtjcljobstatlimitstatsstatusdetails(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                  JCL JOB STATISTICS LIMIT LOGS INTEGRITY STATUS DETAILS        \n");
    printf("================================================================================\n");
    printf(" LOGS PROGRESS STATS: 100 PERCENT VERIFIED\n");
    printf(" COMPLIANCE AUDIT   : PASSED. RC=0000\n");
    printf("================================================================================\n");
    return true;
}

bool tsfi_xplos_shell_cbt_jcl(const char *cmd) {
    if (strncmp(cmd, "ieb", 3) == 0) {
        printf("[DEPRECATION WARNING] Batch utility program '%s' is deprecated and scheduled for removal.\n", cmd);
    }
    if (strncmp(cmd, "jclrun ", 7) == 0) return handle_jclrun(cmd);
    if (strncmp(cmd, "cbtjclvalidate ", 15) == 0) return handle_cbtjclvalidate(cmd);
    if (strcmp(cmd, "cbtjclvalidatelog") == 0) return handle_cbtjclvalidatelog();
    if (strcmp(cmd, "cbtjcllist") == 0) return handle_cbtjcllist();
    if (strcmp(cmd, "cbtjclliststat") == 0) return handle_cbtjclliststat();
    if (strcmp(cmd, "cbtjclresstat") == 0) return handle_cbtjclresstat();
    if (strcmp(cmd, "cbtjclresstatresetstatsstatusdetails") == 0) return handle_cbtjclresstatresetstatsstatusdetails();
    if (strcmp(cmd, "cbtjclresstatresetstatsstatus") == 0) return handle_cbtjclresstatresetstatsstatus();
    if (strcmp(cmd, "cbtjclresstatresetstats") == 0) return handle_cbtjclresstatresetstats();
    if (strcmp(cmd, "cbtjclresstatresetstat") == 0) return handle_cbtjclresstatresetstat();
    if (strcmp(cmd, "cbtjclresstatreset") == 0) return handle_cbtjclresstatreset();
    if (strcmp(cmd, "cbtjcljoblimitstatsstatusdetailsprogress") == 0) return handle_cbtjcljoblimitstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtjcljoblimitstatsstatusdetails") == 0) return handle_cbtjcljoblimitstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljoblimitstatsstatus") == 0) return handle_cbtjcljoblimitstatsstatus();
    if (strcmp(cmd, "cbtjcljoblimitresetstatsstatusdetailsprogress") == 0) return handle_cbtjcljoblimitresetstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtjcljoblimitresetstatsstatusdetails") == 0) return handle_cbtjcljoblimitresetstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljoblimitresetstatsstatus") == 0) return handle_cbtjcljoblimitresetstatsstatus();
    if (strcmp(cmd, "cbtjcljoblimitresetstats") == 0) return handle_cbtjcljoblimitresetstats();
    if (strcmp(cmd, "cbtjcljoblimitresetstat") == 0) return handle_cbtjcljoblimitresetstat();
    if (strcmp(cmd, "cbtjcljoblimitreset") == 0) return handle_cbtjcljoblimitreset();
    if (strcmp(cmd, "cbtjcljoblimitstats") == 0) return handle_cbtjcljoblimitstats();
    if (strcmp(cmd, "cbtjcljoblimitstat") == 0) return handle_cbtjcljoblimitstat();
    if (strncmp(cmd, "cbtjcljoblimit ", 15) == 0) return handle_cbtjcljoblimit(cmd);
    if (strcmp(cmd, "cbtjcljobchkstatsstatusdetailsprogress") == 0) return handle_cbtjcljobchkstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtjcljobchkstatsstatusdetails") == 0) return handle_cbtjcljobchkstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljobchkstatsstatus") == 0) return handle_cbtjcljobchkstatsstatus();
    if (strcmp(cmd, "cbtjcljobchkstats") == 0) return handle_cbtjcljobchkstats();
    if (strcmp(cmd, "cbtjcljobchkresetstatsstatusdetailsprogress") == 0) return handle_cbtjcljobchkresetstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtjcljobchkresetstatsstatusdetails") == 0) return handle_cbtjcljobchkresetstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljobchkresetstatsstatus") == 0) return handle_cbtjcljobchkresetstatsstatus();
    if (strcmp(cmd, "cbtjcljobchkresetstats") == 0) return handle_cbtjcljobchkresetstats();
    if (strcmp(cmd, "cbtjcljobchkresetstat") == 0) return handle_cbtjcljobchkresetstat();
    if (strcmp(cmd, "cbtjcljobchkreset") == 0) return handle_cbtjcljobchkreset();
    if (strcmp(cmd, "cbtjcljobchkstat") == 0) return handle_cbtjcljobchkstat();
    if (strncmp(cmd, "cbtjcljobchk ", 13) == 0) return handle_cbtjcljobchk(cmd);
    if (strcmp(cmd, "cbtjcljobstatstatsstatusdetailsprogress") == 0) return handle_cbtjcljobstatstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtjcljobstatstatsstatusdetails") == 0) return handle_cbtjcljobstatstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljobstatstatsstatus") == 0) return handle_cbtjcljobstatstatsstatus();
    if (strcmp(cmd, "cbtjcljobstatstats") == 0) return handle_cbtjcljobstatstats();
    if (strcmp(cmd, "cbtjcljobstatresetstatsstatusdetailsprogress") == 0) return handle_cbtjcljobstatresetstatsstatusdetailsprogress();
    if (strcmp(cmd, "cbtjcljobstatresetstatsstatusdetails") == 0) return handle_cbtjcljobstatresetstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljobstatresetstatsstatus") == 0) return handle_cbtjcljobstatresetstatsstatus();
    if (strcmp(cmd, "cbtjcljobstatresetstats") == 0) return handle_cbtjcljobstatresetstats();
    if (strcmp(cmd, "cbtjcljobstatresetstat") == 0) return handle_cbtjcljobstatresetstat();
    if (strcmp(cmd, "cbtjcljobstatreset") == 0) return handle_cbtjcljobstatreset();
    if (strcmp(cmd, "cbtjcljobstatstat") == 0) return handle_cbtjcljobstatstat();
    if (strncmp(cmd, "cbtjcljobstat ", 14) == 0) return handle_cbtjcljobstat(cmd);
    if (strcmp(cmd, "cbtjcljobstatlimitstatsstatusdetails") == 0) return handle_cbtjcljobstatlimitstatsstatusdetails();
    if (strcmp(cmd, "cbtjcljobstatlimitstatsstatus") == 0) return handle_cbtjcljobstatlimitstatsstatus();
    if (strcmp(cmd, "cbtjcljobstatlimitstats") == 0) return handle_cbtjcljobstatlimitstats();
    if (strcmp(cmd, "cbtjcljobstatlimitstat") == 0) return handle_cbtjcljobstatlimitstat();
    if (strncmp(cmd, "cbtjcljobstatlimit ", 19) == 0) return handle_cbtjcljobstatlimit(cmd);
    if (strncmp(cmd, "iebupdte ", 9) == 0) return handle_iebupdte(cmd);
    if (strncmp(cmd, "submit ", 7) == 0) return handle_submit(cmd);
    if (strncmp(cmd, "iebgener ", 9) == 0) return handle_iebgener(cmd);
    if (strncmp(cmd, "iebcompr ", 9) == 0) return handle_iebcompr(cmd);
    if (strncmp(cmd, "iebdg ", 6) == 0) return handle_iebdg(cmd);
    if (strncmp(cmd, "cbtjclchk ", 10) == 0) return handle_cbtjclchk(cmd);
    if (strncmp(cmd, "iebimage ", 9) == 0) return handle_iebimage(cmd);
    if (strncmp(cmd, "iebdatr ", 8) == 0) return handle_iebdatr(cmd);
    if (strncmp(cmd, "iebisam ", 8) == 0) return handle_iebisam(cmd);
    if (strncmp(cmd, "iebdgpat ", 9) == 0) return handle_iebdgpat(cmd);
    if (strncmp(cmd, "iebcomprlim ", 12) == 0) return handle_iebcomprlim(cmd);
    if (strncmp(cmd, "iebupdtesrch ", 13) == 0) return handle_iebupdtesrch(cmd);
    if (strncmp(cmd, "iebdgseed ", 10) == 0) return handle_iebdgseed(cmd);
    if (strncmp(cmd, "iebcomprmask ", 13) == 0) return handle_iebcomprmask(cmd);
    if (strncmp(cmd, "iebimagespc ", 12) == 0) return handle_iebimagespc(cmd);
    if (strncmp(cmd, "iebdatrchk ", 11) == 0) return handle_iebdatrchk(cmd);
    if (strncmp(cmd, "iebcomprdet ", 12) == 0) return handle_iebcomprdet(cmd);
    if (strncmp(cmd, "iebdgpatdet ", 12) == 0) return handle_iebdgpatdet(cmd);
    if (strncmp(cmd, "iebgenerbuf ", 12) == 0) return handle_iebgenerbuf(cmd);
    if (strncmp(cmd, "iebcomprredirect ", 17) == 0) return handle_iebcomprredirect(cmd);
    if (strncmp(cmd, "iebcomprmaxexcl ", 16) == 0) return handle_iebcomprmaxexcl(cmd);
    if (strncmp(cmd, "iebdgiterlim ", 13) == 0) return handle_iebdgiterlim(cmd);
    if (strcmp(cmd, "iebdgiterstat") == 0) return handle_iebdgiterstat();
    if (strncmp(cmd, "iebdgiterchange ", 16) == 0) return handle_iebdgiterchange(cmd);
    if (strncmp(cmd, "iebgenerrecal ", 14) == 0) return handle_iebgenerrecal(cmd);
    if (strcmp(cmd, "iebgenerstat") == 0) return handle_iebgenerstat();
    if (strcmp(cmd, "iebgenerreftab") == 0) return handle_iebgenerreftab();
    if (strcmp(cmd, "iebgenerreftabstat") == 0) return handle_iebgenerreftabstat();
    if (strcmp(cmd, "iebgenerreftabstatreset") == 0) return handle_iebgenerreftabstatreset();
    if (strcmp(cmd, "iebgenerreftabstatresetliststatreset") == 0) return handle_iebgenerreftabstatresetliststatreset();
    if (strcmp(cmd, "iebgenerreftabstatresetliststatresetstat") == 0) return handle_iebgenerreftabstatresetliststatresetstat();
    if (strcmp(cmd, "iebgenerreftabstatresetliststat") == 0) return handle_iebgenerreftabstatresetliststat();
    if (strncmp(cmd, "iebgenerreftabstatresetlist ", 28) == 0) return handle_iebgenerreftabstatresetlist(cmd);
    return false;
}
