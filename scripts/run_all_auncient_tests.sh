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

# Compile and run Integration Benchmarks
gcc -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=199309L -O3 tests/test_auncient_integration_bench.c -o tests/test_auncient_integration_bench -lssl -lcrypto -lm -lrt
./tests/test_auncient_integration_bench

echo "=== ALL AUNCIENT SUITES PASSED SUCCESSFULLY ==="
