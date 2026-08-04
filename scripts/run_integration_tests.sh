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

# Define the test suites to run
declare -a test_suites=(
    "test-auncient-alu-wmq-integration:ALU WinchesterMQ Integration"
    "test-auncient-tsv-wmq-integration:TSV WinchesterMQ Integration"
    "test-tsfi2-loader:Loader Platform"
    "test-tsfi2-compiler:Compiler Front-End"
    "test-hathitrust-hathifile:HathiTrust VSAM Exporter"
    "test-auncient-quadtree-ksds:Quadtree KSDS Operations"
)

exit_code=0

for suite in "${test_suites[@]}"; do
    target="${suite%%:*}"
    description="${suite##*:}"
    log_file="/tmp/harness_${target}.log"

    echo -e "\n${YELLOW}[Harness] Running ${description} [${target}]...${NC}"
    
    if make "${target}" > "${log_file}" 2>&1; then
        echo -e "  ${GREEN}[✔] Platform Execution passed${NC}"
        
        # Validate LAU memory metrics if tracked on this platform
        if grep -q "LAU MEMORY METRICS" "${log_file}"; then
            if grep -q "LAU MEMORY CLEAN" "${log_file}"; then
                echo -e "  ${GREEN}[✔] Memory Leak Check passed (No leaks detected)${NC}"
            else
                echo -e "  ${RED}[✘] Memory Leak Check failed (Leaks detected)${NC}"
                grep "LAU MEMORY" "${log_file}" || true
                exit_code=1
            fi
        else
            echo -e "  ${BLUE}[-] Memory Leak Check skipped (Not instrumented)${NC}"
        fi

        # Validate general diagnostics format
        if grep -q -i "warning" "${log_file}"; then
            echo -e "  ${YELLOW}[!] Diagnostics warning captured in log${NC}"
        fi
    else
        echo -e "  ${RED}[✘] Platform Execution failed${NC}"
        cat "${log_file}"
        exit_code=1
    fi
done

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
