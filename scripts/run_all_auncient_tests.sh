#!/usr/bin/env bash
set -e

echo "=== STARTING ALL AUNCIENT SUITES ==="

# Compile and run VDM Isolation
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_vdm_isolation.c -o tests/test_auncient_vdm_isolation -lssl -lcrypto -lm -lrt
./tests/test_auncient_vdm_isolation

# Compile and run I/PL Hypervisor
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_ipl_hypervisor.c -o tests/test_auncient_ipl_hypervisor -lssl -lcrypto -lm -lrt
./tests/test_auncient_ipl_hypervisor

# Compile and run I/PL Preprocessor
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_ipl_preproc.c -o tests/test_auncient_ipl_preproc -lm -lrt
./tests/test_auncient_ipl_preproc

# Compile and run PL/I Attributes
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pli_attributes.c -o tests/test_auncient_pli_attributes -lssl -lcrypto -lm -lrt
./tests/test_auncient_pli_attributes

# Compile and run PL-CANG Gateway
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=199309L -O3 tests/test_auncient_pl_cang.c -o tests/test_auncient_pl_cang -lssl -lcrypto -lm -lrt
./tests/test_auncient_pl_cang

# Compile and run PL-CANG CAD
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pl_cang_cad.c -o tests/test_auncient_pl_cang_cad -lm -lrt
./tests/test_auncient_pl_cang_cad

# Compile and run PDL Gating
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_gating.c -o tests/test_auncient_pdl_gating -lm -lrt
./tests/test_auncient_pdl_gating

# Compile and run Kleene Room
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_kleene_room.c -o tests/test_auncient_kleene_room -lssl -lcrypto -lm -lrt
./tests/test_auncient_kleene_room

# Compile and run PDL Converse
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_converse.c -o tests/test_auncient_pdl_converse -lssl -lcrypto -lm -lrt
./tests/test_auncient_pdl_converse

# Compile and run PDL Loop Guard
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_loop_guard.c -o tests/test_auncient_pdl_loop_guard -lm -lrt
./tests/test_auncient_pdl_loop_guard

# Compile and run PDL Contract
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_contract.c -o tests/test_auncient_pdl_contract -lssl -lcrypto -lm -lrt
./tests/test_auncient_pdl_contract

# Compile and run PDL Implication
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_implication.c -o tests/test_auncient_pdl_implication -lm -lrt
./tests/test_auncient_pdl_implication

# Compile and run Fourier Implication
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_fourier_implication.c -o tests/test_auncient_fourier_implication -lssl -lcrypto -lm -lrt
./tests/test_auncient_fourier_implication

# Compile and run Coaxial Only
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_coaxial_only.c -o tests/test_auncient_coaxial_only -lssl -lcrypto -lm -lrt
./tests/test_auncient_coaxial_only

# Compile and run PDL Oversight
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_oversight.c -o tests/test_auncient_pdl_oversight -lssl -lcrypto -lm -lrt
./tests/test_auncient_pdl_oversight

# Compile and run Coaxial Kermit
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_kermit_coax.c -o tests/test_auncient_kermit_coax -lssl -lcrypto -lm -lrt
./tests/test_auncient_kermit_coax

# Compile and run DECnet Routing
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_decnet_routing.c -o tests/test_auncient_decnet_routing -lssl -lcrypto -lm -lrt
./tests/test_auncient_decnet_routing

# Compile and run PDL Hoare Proof Assistant
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_pdl_hoare.c -o tests/test_auncient_pdl_hoare -lm -lrt
./tests/test_auncient_pdl_hoare

# Compile and run Safety Learning
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_safety_learning.c -o tests/test_auncient_safety_learning -lssl -lcrypto -lm -lrt
./tests/test_auncient_safety_learning

# Compile and run Black-Red PDL
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_black_red_pdl.c -o tests/test_auncient_black_red_pdl -lm -lrt
./tests/test_auncient_black_red_pdl

# Compile and run XplOS APDL
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_apdl_xplos.c -o tests/test_auncient_apdl_xplos -lm -lrt
./tests/test_auncient_apdl_xplos

# Compile and run APDL Convergence
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_apdl_convergence.c -o tests/test_auncient_apdl_convergence -lm -lrt
./tests/test_auncient_apdl_convergence

# Compile and run VCA FET Discharge
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_vca_fet.c -o tests/test_auncient_vca_fet -lm -lrt
./tests/test_auncient_vca_fet

# Compile and run MKK FET Discharge
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_mkk_fet.c -o tests/test_auncient_mkk_fet -lm -lrt
./tests/test_auncient_mkk_fet

# Compile and run APDL Token
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_apdl_token.c -o tests/test_auncient_apdl_token -lm -lrt
./tests/test_auncient_apdl_token

# Compile and run EXOSKELETON Hypervisor
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_exoskeleton.c -o tests/test_auncient_exoskeleton -lm -lrt
./tests/test_auncient_exoskeleton

# Compile and run Black Skeleton Red Exo Gating
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_black_skeleton_red_exo.c -o tests/test_auncient_black_skeleton_red_exo -lm -lrt
./tests/test_auncient_black_skeleton_red_exo

# Compile and run Unified ALU Coax
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_unified_alu_coax.c -o tests/test_auncient_unified_alu_coax -lssl -lcrypto -lm -lrt
./tests/test_auncient_unified_alu_coax

# Compile and run Transistor-As-ALU
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_transistor_alu.c -o tests/test_auncient_transistor_alu -lm -lrt
./tests/test_auncient_transistor_alu

# Compile and run APDL Compiler
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_apdl_compiler.c -o tests/test_auncient_apdl_compiler -lm -lrt
./tests/test_auncient_apdl_compiler

# Compile and run APDL XPL Integration
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_apdl_xpl_integration.c -o tests/test_auncient_apdl_xpl_integration -lm -lrt
./tests/test_auncient_apdl_xpl_integration

# Compile and run dat.bin Transaction Ingestion
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_dat_bin_transaction.c -o tests/test_auncient_dat_bin_transaction -lssl -lcrypto -lm -lrt
./tests/test_auncient_dat_bin_transaction

# Compile and run Chin Monopole
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_chin_monopole.c -o tests/test_auncient_chin_monopole -lm -lrt
./tests/test_auncient_chin_monopole

# Compile and run Interactive Debugger Front End
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_interactive_db.c -o tests/test_auncient_interactive_db -lm -lrt
./tests/test_auncient_interactive_db --non-interactive

# Compile and run IPL VOL1 Verification
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -O3 tests/test_auncient_ipl_vol1.c -o tests/test_auncient_ipl_vol1 -lssl -lcrypto -lm -lrt
./tests/test_auncient_ipl_vol1

# Compile and run SIGPLAN Loop Bounds Verification
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_loop_bound.c -o tests/test_auncient_sigplan_loop_bound -lm -lrt
./tests/test_auncient_sigplan_loop_bound

# Compile and run SIGPLAN Region-Based Memory Safety Verification
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_regions.c -o tests/test_auncient_sigplan_regions -lm -lrt
./tests/test_auncient_sigplan_regions

# Compile and run SIGPLAN Linear Resource Auditor
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_linear.c -o tests/test_auncient_sigplan_linear -lm -lrt
./tests/test_auncient_sigplan_linear

# Compile and run Voodoo TMU Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_voodoo_tmu.c -o tests/test_auncient_voodoo_tmu -lm -lrt
./tests/test_auncient_voodoo_tmu

# Compile and run Integration Benchmarks
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=199309L -O3 tests/test_auncient_integration_bench.c -o tests/test_auncient_integration_bench -lssl -lcrypto -lm -lrt
./tests/test_auncient_integration_bench

echo "=== ALL AUNCIENT SUITES PASSED SUCCESSFULLY ==="
