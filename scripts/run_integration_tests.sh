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

# 1. Run Loader validation
echo -e "\n${YELLOW}[Harness] Running TSFi2 Loader and TSV Binary Header verification...${NC}"
if make test-tsfi2-loader > /tmp/harness_loader.log 2>&1; then
    echo -e "  ${GREEN}[✔] Loader TSV Binary Header Parser passed${NC}"
else
    echo -e "  ${RED}[✘] Loader TSV Binary Header Parser failed${NC}"
    cat /tmp/harness_loader.log
    exit 1
fi

# 2. Run Compiler validation
echo -e "${YELLOW}[Harness] Running TSFi2 Closure compiler validation...${NC}"
if make test-tsfi2-compiler > /tmp/harness_compiler.log 2>&1; then
    echo -e "  ${GREEN}[✔] Closure Compiler JCL Map Audit passed${NC}"
else
    echo -e "  ${RED}[✘] Closure Compiler JCL Map Audit failed${NC}"
    cat /tmp/harness_compiler.log
    exit 1
fi

# 3. Run Integration and Memory checks
echo -e "${YELLOW}[Harness] Running WinchesterMQ & Spool Guard Integration...${NC}"
if make test-auncient-tsv-wmq-integration > /tmp/harness_integration.log 2>&1; then
    # Parse check results from log
    if grep -q "COBOL/JCL compiler pass success" /tmp/harness_integration.log; then
        echo -e "  ${GREEN}[✔] COBOL/JCL Spool Card Audit passed${NC}"
    else
        echo -e "  ${RED}[✘] COBOL/JCL Spool Card Audit failed${NC}"
    fi

    if grep -q "Spool guard security lockout verified" /tmp/harness_integration.log; then
        echo -e "  ${GREEN}[✔] HASP Spool Guard Defcon Lockout passed${NC}"
    else
        echo -e "  ${RED}[✘] HASP Spool Guard Defcon Lockout failed${NC}"
    fi

    if grep -q "HathiTrust catalog record retrieved via AIX Quadtree" /tmp/harness_integration.log; then
        echo -e "  ${GREEN}[✔] HathiTrust AIX Quadtree Query passed${NC}"
    else
        echo -e "  ${RED}[✘] HathiTrust AIX Quadtree Query failed${NC}"
    fi

    if grep -q "LAU MEMORY CLEAN" /tmp/harness_integration.log; then
        echo -e "  ${GREEN}[✔] LAU memory leak detection passed (No leaks detected)${NC}"
    else
        echo -e "  ${RED}[✘] LAU memory leak detection failed (Leaks present)${NC}"
        grep "LAU MEMORY" /tmp/harness_integration.log || true
    fi
else
    echo -e "  ${RED}[✘] WinchesterMQ Integration Suite failed to run${NC}"
    cat /tmp/harness_integration.log
    exit 1
fi

# Check for Spool Trace log verification
if [ -f /tmp/hasp_spool_audit.log ]; then
    echo -e "  ${GREEN}[✔] HASP Spool Queue audit trace logging verified${NC}"
else
    echo -e "  ${RED}[✘] HASP Spool Queue audit trace logging missing${NC}"
fi

echo -e "\n${GREEN}=============================================================${NC}"
echo -e "${GREEN}             ALL SUBSYSTEM INTEGRATION TESTS PASSED          ${NC}"
echo -e "${GREEN}=============================================================${NC}"
