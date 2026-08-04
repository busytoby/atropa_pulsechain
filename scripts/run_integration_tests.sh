#!/usr/bin/env bash
#
# AUNCIENT INTEGRATION SUITE TEST HARNESS & CHECKLIST VALIDATOR
#
set -euo pipefail

# ANSI color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color
BLUE='\033[0;34m'
YELLOW='\033[1;33m'

echo -e "${BLUE}=============================================================${NC}"
echo -e "${BLUE}        AUNCIENT MAINFRAME INTEGRATION CHECKLIST VALIDATOR   ${NC}"
echo -e "${BLUE}=============================================================${NC}"

# Ensure temporary files are clean
rm -f /tmp/hasp_spool_audit.log

exit_code=0

# =====================================================================
# GROUP 1: ALU WinchesterMQ Core
# =====================================================================
echo -e "\n${BLUE}-------------------------------------------------------------${NC}"
echo -e "${BLUE} GROUP 1: ALU WinchesterMQ Core Validation                   ${NC}"
echo -e "${BLUE}-------------------------------------------------------------${NC}"

log_file_alu="/tmp/harness_alu_core.log"
if make test-auncient-alu-wmq-integration > "${log_file_alu}" 2>&1; then
    echo -e "  ${GREEN}[✔] ALU Core Platform Execution passed${NC}"
    
    if grep -q "LAU MEMORY METRICS" "${log_file_alu}"; then
        if grep -q "LAU MEMORY CLEAN" "${log_file_alu}"; then
            echo -e "  ${GREEN}[✔] ALU Core Memory Leak Check passed (No leaks detected)${NC}"
        else
            echo -e "  ${RED}[✘] ALU Core Memory Leak Check failed (Leaks detected)${NC}"
            grep "LAU MEMORY" "${log_file_alu}" || true
            exit_code=1
        fi
    else
        echo -e "  ${RED}[✘] ALU Core Memory Leak Check missing (Not instrumented)${NC}"
        exit_code=1
    fi
else
    echo -e "  ${RED}[✘] ALU Core Platform Execution failed${NC}"
    cat "${log_file_alu}"
    exit_code=1
fi

# =====================================================================
# GROUP 2: TSV WinchesterMQ Core
# =====================================================================
echo -e "\n${BLUE}-------------------------------------------------------------${NC}"
echo -e "${BLUE} GROUP 2: TSV WinchesterMQ Core Validation                   ${NC}"
echo -e "${BLUE}-------------------------------------------------------------${NC}"

log_file_tsv="/tmp/harness_tsv_core.log"
if make test-auncient-tsv-wmq-integration > "${log_file_tsv}" 2>&1; then
    echo -e "  ${GREEN}[✔] TSV Core Platform Execution passed${NC}"
    
    if grep -q "LAU MEMORY METRICS" "${log_file_tsv}"; then
        if grep -q "LAU MEMORY CLEAN" "${log_file_tsv}"; then
            echo -e "  ${GREEN}[✔] TSV Core Memory Leak Check passed (No leaks detected)${NC}"
        else
            echo -e "  ${RED}[✘] TSV Core Memory Leak Check failed (Leaks detected)${NC}"
            grep "LAU MEMORY" "${log_file_tsv}" || true
            exit_code=1
        fi
    else
        echo -e "  ${RED}[✘] TSV Core Memory Leak Check missing (Not instrumented)${NC}"
        exit_code=1
    fi
else
    echo -e "  ${RED}[✘] TSV Core Platform Execution failed${NC}"
    cat "${log_file_tsv}"
    exit_code=1
fi

# Validate Spool Trace Log
if [ -f /tmp/hasp_spool_audit.log ]; then
    echo -e "\n${GREEN}[✔] Spool Queue Audit Trace verified successfully.${NC}"
else
    echo -e "\n${RED}[✘] Spool Queue Audit Trace is missing.${NC}"
    exit_code=1
fi

if [ "${exit_code}" -eq 0 ]; then
    echo -e "\n${GREEN}=============================================================${NC}"
    echo -e "${GREEN}      ALL PLATFORMS VERIFIED SUCCESSFULLY UNDER CRITERIA     ${NC}"
    echo -e "${GREEN}=============================================================${NC}"
else
    echo -e "\n${RED}=============================================================${NC}"
    echo -e "${RED}             SOME INTEGRATION VERIFICATIONS FAILED           ${NC}"
    echo -e "${RED}=============================================================${NC}"
fi

exit "${exit_code}"
