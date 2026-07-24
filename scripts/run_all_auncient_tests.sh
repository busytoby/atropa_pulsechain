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

# Compile and run SIGPLAN TMU Auditor
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_tmu.c -o tests/test_auncient_sigplan_tmu -lm -lrt
./tests/test_auncient_sigplan_tmu

# Compile and run SIGPLAN Voodoo TMU Pipeline
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_voodoo.c -o tests/test_auncient_sigplan_voodoo -lm -lrt
./tests/test_auncient_sigplan_voodoo

# Compile and run Jin Li Concurrent Cache Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_jin_li_cache.c -o tests/test_auncient_jin_li_cache -lm -lrt
./tests/test_auncient_jin_li_cache

# Compile and run Jin Li Rate Control Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_jin_li_rate.c -o tests/test_auncient_jin_li_rate -lm -lrt
./tests/test_auncient_jin_li_rate

# Compile and run Jin Li Quantization Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_jin_li_quant.c -o tests/test_auncient_jin_li_quant -lm -lrt
./tests/test_auncient_jin_li_quant

# Compile and run SIGPLAN Information Flow Control
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_ifc.c -o tests/test_auncient_sigplan_ifc -lm -lrt
./tests/test_auncient_sigplan_ifc

# Compile and run AUTODIN SIGPLAN IFC Router
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_autodin_ifc.c -o tests/test_auncient_autodin_ifc -lm -lrt
./tests/test_auncient_autodin_ifc

# Compile and run SIGPLAN VDM Contracts
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigplan_contracts.c -o tests/test_auncient_sigplan_contracts -lm -lrt
./tests/test_auncient_sigplan_contracts

# Compile and run CSCW CRDT Eventual Consistency
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_cscw_crdt.c -o tests/test_auncient_cscw_crdt -lm -lrt
./tests/test_auncient_cscw_crdt

# Compile and run CSCW-Black CRDT-Red Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_cscw_black_crdt_red.c -o tests/test_auncient_cscw_black_crdt_red -lm -lrt
./tests/test_auncient_cscw_black_crdt_red

# Compile and run CSCW PN-Counter CRDT
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_cscw_pn_counter.c -o tests/test_auncient_cscw_pn_counter -lm -lrt
./tests/test_auncient_cscw_pn_counter

# Compile and run Rule 12 Accumulator Redirection
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_accumulator_redirection.c -o tests/test_auncient_accumulator_redirection -lm -lrt
./tests/test_auncient_accumulator_redirection

# Compile and run Voodoo GPU Best Practices
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_voodoo_best_practices.c -o tests/test_auncient_voodoo_best_practices -lm -lrt
./tests/test_auncient_voodoo_best_practices

# Compile and run Ackerman Collaborative Consensus
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_ackerman_consensus.c -o tests/test_auncient_ackerman_consensus -lm -lrt
./tests/test_auncient_ackerman_consensus

# Compile and run Kermit Policy Aho-Corasick Cache
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=199309L -O3 tests/test_auncient_kermit_cache.c -o tests/test_auncient_kermit_cache -lm -lrt
./tests/test_auncient_kermit_cache

# Compile and run AUTODIN Ackerman Declassification Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_autodin_ackerman.c -o tests/test_auncient_autodin_ackerman -lm -lrt
./tests/test_auncient_autodin_ackerman

# Compile and run Kermit State Consensus
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_kermit_state_consensus.c -o tests/test_auncient_kermit_state_consensus -lm -lrt
./tests/test_auncient_kermit_state_consensus

# Compile and run Lutters Boundary Object
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_lutters_boundary.c -o tests/test_auncient_lutters_boundary -lm -lrt
./tests/test_auncient_lutters_boundary

# Compile and run SIGACT Byzantine Fault Tolerance
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sigact_byzantine.c -o tests/test_auncient_sigact_byzantine -lm -lrt
./tests/test_auncient_sigact_byzantine

# Compile and run EDTC Self-Checking DMR
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_edtc_self_test.c -o tests/test_auncient_edtc_self_test -lm -lrt
./tests/test_auncient_edtc_self_test

# Compile and run Coupled Simulator
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_coupled_simulator.c -o tests/test_auncient_coupled_simulator -lm -lrt
./tests/test_auncient_coupled_simulator

# Compile and run Wünsche Entropy Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_wunsche_entropy.c -o tests/test_auncient_wunsche_entropy -lm -lrt
./tests/test_auncient_wunsche_entropy

# Compile and run Coupled Byzantine Entropy Gating
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_coupled_byzantine_entropy.c -o tests/test_auncient_coupled_byzantine_entropy -lm -lrt
./tests/test_auncient_coupled_byzantine_entropy

# Compile and run Structured Transaction Benchmarks
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_structured_bench.c -o tests/test_auncient_structured_bench -lm -lrt
./tests/test_auncient_structured_bench

# Compile and run Fault-Tolerant Clock Synchronization
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_fault_tolerant_clock.c -o tests/test_auncient_fault_tolerant_clock -lm -lrt
./tests/test_auncient_fault_tolerant_clock

# Compile and run Bakery Distributed Lock
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_bakery_lock.c -o tests/test_auncient_bakery_lock -lm -lrt
./tests/test_auncient_bakery_lock

# Compile and run Welch Link-State Update Validation
gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_welch_link_state.c -o tests/test_auncient_welch_link_state -lm -lrt
./tests/test_auncient_welch_link_state

# Compile and run Integration Benchmarks
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=199309L -O3 tests/test_auncient_integration_bench.c -o tests/test_auncient_integration_bench -lssl -lcrypto -lm -lrt
./tests/test_auncient_integration_bench

echo "=== ALL AUNCIENT SUITES PASSED SUCCESSFULLY ==="
