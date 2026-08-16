help:
	@echo "Available test targets:"
	@echo "  make test-all        - Run all E2E and unit tests in the project"
	@echo "  make profile         - Run master test runner latency verification provers"
	@echo "  make test-dashboard  - Run NoNukes Dashboard E2E tests (including SPA)"
	@echo "  make test-container  - Run Teddy Bear Diagnostics container E2E tests"
	@echo "  make test-git-ci     - Run Git post-commit hook pipeline E2E tests"
	@echo "  make test-unit       - Run Python unit tests under tests/"
	@echo "  make sdk-benchmark   - Run Auncient SDK DbC contract latency benchmarks"
	@echo "  make tpu-benchmarks  - Run all TPU fast inference performance benchmarks"
	@echo "  make livrps-benchmark - Run LIVRPS USD composition latency benchmarks"
	@echo "  make curves-benchmark - Run Curves schema latency benchmarks"


profile:
	./tests/test_run_all_provers_main

test-all: test-hathitrust test-hathitrust-hathifile test-hathitrust-oai test-hathitrust-oai-util test-hathitrust-harvester test-hathitrust-ef test-hathitrust-extended test-hathitrust-xml test-hathitrust-cache-alto-qt test-hathitrust-extra2 test-hathitrust-extra3 test-hathitrust-render test-hathitrust-prman test-hathitrust-lore test-hathitrust-prman-extra test-hathitrust-prman-anim test-hathitrust-prman-blur test-hathitrust-prman-final test-hathitrust-teddy test-hathitrust-final-extra test-hathitrust-tsv-extra test-hathitrust-solr-rdbms test-hathitrust-tsv-bin test-hathitrust-window test-hathitrust-extra-final test-hathitrust-bib-oauth test-tsfi2-compiler-bin test-tsfi2-loader test-tsfi2-compiler test-tsfi2-compiler-standards test-mann test-comp-pass test-mvarsel test-reroute test-delegate-sig test-stagecomp test-ar test-sdfformat test-hydrascene test-renderdelegate test-stagelock test-renderindex test-usdshade test-teddy-loader test-geomcamera test-geomcurves test-geompoints test-luxlight test-geomxform test-stageroot test-primroot test-stagepop test-attributeroot test-stagecache test-stagemask test-geomsubset test-stagearc test-stageinherits test-stagepayloads test-stagevariants test-stagetraverser test-stagemetadata test-camerafrustum test-stagereferences test-stagespecializes test-stagerelationship test-stagetimecodes test-vactrolpickup test-npnpnpvalve test-curvesbasis test-singularityring test-subdivcurves test-subdivscheme test-curvestension test-curveswidths test-curveswrap test-timesamples test-micropolygonmesh test-shadowmap test-quasirandomsampler test-displacementshader test-texgensample test-depthoffield test-quadtreeslicer test-riinterface test-cinefex-adv test-sss test-envmap test-hairshading test-proximity-occlusion test-krowz-sickness test-auncient-teddy-personality test-tsfi-montecarlo
	@echo "All tests completed successfully."

test-ankh-island-admit-matrix:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_admit_matrix.c -o tests/test_ankh_island_admit_matrix
	./tests/test_ankh_island_admit_matrix
	@rm -f tests/test_ankh_island_admit_matrix

compile-hello-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_compiler_hello.c -o tools/ankh_compiler_hello
	./tools/ankh_compiler_hello hello.bin
	@rm -f tools/ankh_compiler_hello

test-exec-hello-bin: compile-hello-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_exec_bin.c -o tools/ankh_exec_bin
	./tools/ankh_exec_bin hello.bin
	@rm -f tools/ankh_exec_bin

compile-tune-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_compiler_tune.c -o tools/ankh_compiler_tune
	./tools/ankh_compiler_tune tune_channel.bin
	@rm -f tools/ankh_compiler_tune

test-exec-tune-bin: compile-tune-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_exec_tune.c -o tools/ankh_exec_tune
	./tools/ankh_exec_tune tune_channel.bin
	@rm -f tools/ankh_exec_tune

compile-teddy-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_compiler_teddy.c -o tools/ankh_compiler_teddy
	./tools/ankh_compiler_teddy teddy_endow.bin
	@rm -f tools/ankh_compiler_teddy

test-exec-teddy-bin: compile-teddy-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_exec_teddy.c -o tools/ankh_exec_teddy
	./tools/ankh_exec_teddy teddy_endow.bin
	@rm -f tools/ankh_exec_teddy

test-sdk-agent-runtime:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/ankh_sdk_runtime.c -o tools/ankh_sdk_runtime
	./tools/ankh_sdk_runtime hello.bin tune_channel.bin teddy_endow.bin stat.bin
	@rm -f tools/ankh_sdk_runtime

compile-gate-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_gate.c -o tools/cpm_compiler_gate
	./tools/cpm_compiler_gate gate.bin
	@rm -f tools/cpm_compiler_gate

test-exec-gate-bin: compile-gate-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_gate.c -o tools/cpm_exec_gate
	./tools/cpm_exec_gate gate.bin
	@rm -f tools/cpm_exec_gate

compile-patch-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_patch.c -o tools/cpm_compiler_patch
	./tools/cpm_compiler_patch patch.bin
	@rm -f tools/cpm_compiler_patch

test-exec-patch-bin: compile-patch-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_patch.c -o tools/cpm_exec_patch
	./tools/cpm_exec_patch patch.bin
	@rm -f tools/cpm_exec_patch

compile-unpack-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_unpack.c -o tools/cpm_compiler_unpack
	./tools/cpm_compiler_unpack unpack.bin
	@rm -f tools/cpm_compiler_unpack

test-exec-unpack-bin: compile-unpack-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_unpack.c -o tools/cpm_exec_unpack
	./tools/cpm_exec_unpack unpack.bin
	@rm -f tools/cpm_exec_unpack

compile-pack-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_pack.c -o tools/cpm_compiler_pack
	./tools/cpm_compiler_pack pack.bin
	@rm -f tools/cpm_compiler_pack

test-exec-pack-bin: compile-pack-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_pack.c -o tools/cpm_exec_pack
	./tools/cpm_exec_pack pack.bin
	@rm -f tools/cpm_exec_pack

compile-link-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_link.c -o tools/cpm_compiler_link
	./tools/cpm_compiler_link link.bin
	@rm -f tools/cpm_compiler_link

test-exec-link-bin: compile-link-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_link.c -o tools/cpm_exec_link
	./tools/cpm_exec_link link.bin
	@rm -f tools/cpm_exec_link

compile-eval-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_eval.c -o tools/cpm_compiler_eval
	./tools/cpm_compiler_eval eval.bin
	@rm -f tools/cpm_compiler_eval

test-exec-eval-bin: compile-eval-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_eval.c -o tools/cpm_exec_eval
	./tools/cpm_exec_eval eval.bin
	@rm -f tools/cpm_exec_eval

compile-prove-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_prove.c -o tools/cpm_compiler_prove
	./tools/cpm_compiler_prove prove.bin
	@rm -f tools/cpm_compiler_prove

test-exec-prove-bin: compile-prove-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_prove.c -o tools/cpm_exec_prove
	./tools/cpm_exec_prove prove.bin
	@rm -f tools/cpm_exec_prove

compile-fold-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_fold.c -o tools/cpm_compiler_fold
	./tools/cpm_compiler_fold fold.bin
	@rm -f tools/cpm_compiler_fold

test-exec-fold-bin: compile-fold-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_fold.c -o tools/cpm_exec_fold
	./tools/cpm_exec_fold fold.bin
	@rm -f tools/cpm_exec_fold

test-cpm-master-fleet: compile-stat-bin compile-dir-bin compile-pip-bin compile-asm-bin compile-ddt-bin compile-ed-bin compile-submit-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_master_orchestrator.c -o tools/cpm_master_orchestrator
	./tools/cpm_master_orchestrator
	@rm -f tools/cpm_master_orchestrator

compile-ed-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_ed.c -o tools/cpm_compiler_ed
	./tools/cpm_compiler_ed ed.bin
	@rm -f tools/cpm_compiler_ed

test-exec-ed-bin: compile-ed-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_ed.c -o tools/cpm_exec_ed
	./tools/cpm_exec_ed ed.bin
	@rm -f tools/cpm_exec_ed

compile-submit-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_submit.c -o tools/cpm_compiler_submit
	./tools/cpm_compiler_submit submit.bin
	@rm -f tools/cpm_compiler_submit

test-exec-submit-bin: compile-submit-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_submit.c -o tools/cpm_exec_submit
	./tools/cpm_exec_submit submit.bin
	@rm -f tools/cpm_exec_submit

compile-asm-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_asm.c -o tools/cpm_compiler_asm
	./tools/cpm_compiler_asm asm.bin
	@rm -f tools/cpm_compiler_asm

test-exec-asm-bin: compile-asm-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_asm.c -o tools/cpm_exec_asm
	./tools/cpm_exec_asm asm.bin
	@rm -f tools/cpm_exec_asm

compile-ddt-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_ddt.c -o tools/cpm_compiler_ddt
	./tools/cpm_compiler_ddt ddt.bin
	@rm -f tools/cpm_compiler_ddt

test-exec-ddt-bin: compile-ddt-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_ddt.c -o tools/cpm_exec_ddt
	./tools/cpm_exec_ddt ddt.bin
	@rm -f tools/cpm_exec_ddt

compile-dir-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_dir.c -o tools/cpm_compiler_dir
	./tools/cpm_compiler_dir dir.bin
	@rm -f tools/cpm_compiler_dir

test-exec-dir-bin: compile-dir-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_dir.c -o tools/cpm_exec_dir
	./tools/cpm_exec_dir dir.bin
	@rm -f tools/cpm_exec_dir

compile-pip-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_pip.c -o tools/cpm_compiler_pip
	./tools/cpm_compiler_pip pip.bin
	@rm -f tools/cpm_compiler_pip

test-exec-pip-bin: compile-pip-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_pip.c -o tools/cpm_exec_pip
	./tools/cpm_exec_pip pip.bin
	@rm -f tools/cpm_exec_pip

compile-stat-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_compiler_stat.c -o tools/cpm_compiler_stat
	./tools/cpm_compiler_stat stat.bin
	@rm -f tools/cpm_compiler_stat

test-exec-stat-bin: compile-stat-bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tools/cpm_exec_stat.c -o tools/cpm_exec_stat
	./tools/cpm_exec_stat stat.bin
	@rm -f tools/cpm_exec_stat

test-ankh-abi-wmq-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_abi_wmq_compiler.c -o tests/test_ankh_abi_wmq_compiler
	./tests/test_ankh_abi_wmq_compiler
	@rm -f tests/test_ankh_abi_wmq_compiler

test-ankh-kermit-transduction:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_kermit_transduction.c -o tests/test_ankh_kermit_transduction
	./tests/test_ankh_kermit_transduction
	@rm -f tests/test_ankh_kermit_transduction

test-ankh-proof-engine:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_proof_engine.c -o tests/test_ankh_proof_engine
	./tests/test_ankh_proof_engine
	@rm -f tests/test_ankh_proof_engine

test-ankh-llm-reasoning:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_llm_reasoning.c -o tests/test_ankh_llm_reasoning
	./tests/test_ankh_llm_reasoning
	@rm -f tests/test_ankh_llm_reasoning

test-ankh-protein-folding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_protein_folding.c -o tests/test_ankh_protein_folding
	./tests/test_ankh_protein_folding
	@rm -f tests/test_ankh_protein_folding

test-ankh-collective-symbiosis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_collective_symbiosis.c -o tests/test_ankh_collective_symbiosis
	./tests/test_ankh_collective_symbiosis
	@rm -f tests/test_ankh_collective_symbiosis

test-ankh-toolset-self-evolution:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_toolset_self_evolution.c -o tests/test_ankh_toolset_self_evolution
	./tests/test_ankh_toolset_self_evolution
	@rm -f tests/test_ankh_toolset_self_evolution

test-ankh-tomie-life-agent:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_tomie_life_agent.c -o tests/test_ankh_tomie_life_agent
	./tests/test_ankh_tomie_life_agent
	@rm -f tests/test_ankh_tomie_life_agent

test-ankh-agent-pipeline:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_agent_pipeline.c -o tests/test_ankh_agent_pipeline
	./tests/test_ankh_agent_pipeline
	@rm -f tests/test_ankh_agent_pipeline

test-ankh-closed-toolset-ext:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_closed_toolset_ext.c -o tests/test_ankh_closed_toolset_ext
	./tests/test_ankh_closed_toolset_ext
	@rm -f tests/test_ankh_closed_toolset_ext

test-ankh-bootstrap-loader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_bootstrap_loader.c -o tests/test_ankh_bootstrap_loader
	./tests/test_ankh_bootstrap_loader
	@rm -f tests/test_ankh_bootstrap_loader

test-cpm-master-fleet-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_master_fleet.c -o tests/test_cpm_master_fleet
	./tests/test_cpm_master_fleet
	@rm -f tests/test_cpm_master_fleet

test-ankh-coaxial-mesh:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_coaxial_mesh.c -o tests/test_ankh_coaxial_mesh
	./tests/test_ankh_coaxial_mesh hello.bin tune_channel.bin teddy_endow.bin
	@rm -f tests/test_ankh_coaxial_mesh

test-ankh-binary-coaxial-totient:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_binary_coaxial_totient.c -o tests/test_ankh_binary_coaxial_totient
	./tests/test_ankh_binary_coaxial_totient hello.bin tune_channel.bin
	@rm -f tests/test_ankh_binary_coaxial_totient

test-ankh-jit-self-opt:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_jit_self_opt.c -o tests/test_ankh_jit_self_opt
	./tests/test_ankh_jit_self_opt
	@rm -f tests/test_ankh_jit_self_opt

test-ankh-llm-compiler-bridge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_llm_compiler_bridge.c -o tests/test_ankh_llm_compiler_bridge
	./tests/test_ankh_llm_compiler_bridge
	@rm -f tests/test_ankh_llm_compiler_bridge

test-ankh-turing-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_turing_compiler.c -o tests/test_ankh_turing_compiler
	./tests/test_ankh_turing_compiler
	@rm -f tests/test_ankh_turing_compiler

test-ankh-universal-boundary-det:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_universal_boundary_det.c -o tests/test_ankh_universal_boundary_det
	./tests/test_ankh_universal_boundary_det
	@rm -f tests/test_ankh_universal_boundary_det

test-ankh-multi-manifold-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_multi_manifold_sync.c -o tests/test_ankh_multi_manifold_sync
	./tests/test_ankh_multi_manifold_sync
	@rm -f tests/test_ankh_multi_manifold_sync

test-ankh-closed-grid-totient-boundary:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_closed_grid_totient_boundary.c -o tests/test_ankh_closed_grid_totient_boundary
	./tests/test_ankh_closed_grid_totient_boundary
	@rm -f tests/test_ankh_closed_grid_totient_boundary

test-cpm-tomie-timestamp:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_timestamp.c -o tests/test_cpm_tomie_timestamp
	./tests/test_cpm_tomie_timestamp
	@rm -f tests/test_cpm_tomie_timestamp

test-cpm-tomie-dma-stream:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_dma_stream.c -o tests/test_cpm_tomie_dma_stream
	./tests/test_cpm_tomie_dma_stream
	@rm -f tests/test_cpm_tomie_dma_stream

test-cpm-tomie-rsx-intercept:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_rsx_intercept.c -o tests/test_cpm_tomie_rsx_intercept
	./tests/test_cpm_tomie_rsx_intercept
	@rm -f tests/test_cpm_tomie_rsx_intercept

test-cpm-tomie-dir-hash:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_dir_hash.c -o tests/test_cpm_tomie_dir_hash
	./tests/test_cpm_tomie_dir_hash
	@rm -f tests/test_cpm_tomie_dir_hash

test-cpm-tomie-record-lock:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_record_lock.c -o tests/test_cpm_tomie_record_lock
	./tests/test_cpm_tomie_record_lock
	@rm -f tests/test_cpm_tomie_record_lock

test-cpm-tomie-overlay-bank:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_overlay_bank.c -o tests/test_cpm_tomie_overlay_bank
	./tests/test_cpm_tomie_overlay_bank
	@rm -f tests/test_cpm_tomie_overlay_bank

test-cpm-tomie-ccp-pipeline:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_ccp_pipeline.c -o tests/test_cpm_tomie_ccp_pipeline
	./tests/test_cpm_tomie_ccp_pipeline
	@rm -f tests/test_cpm_tomie_ccp_pipeline

test-cpm-tomie-bios-dispatch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_bios_dispatch.c -o tests/test_cpm_tomie_bios_dispatch
	./tests/test_cpm_tomie_bios_dispatch
	@rm -f tests/test_cpm_tomie_bios_dispatch

test-cpm-tomie-fcb-extent:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_fcb_extent.c -o tests/test_cpm_tomie_fcb_extent
	./tests/test_cpm_tomie_fcb_extent
	@rm -f tests/test_cpm_tomie_fcb_extent

test-cpm-tomie-bdos-tpa:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_bdos_tpa.c -o tests/test_cpm_tomie_bdos_tpa
	./tests/test_cpm_tomie_bdos_tpa
	@rm -f tests/test_cpm_tomie_bdos_tpa

test-auncient-acab-skin-dispersion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_acab_skin_dispersion.c -o tests/test_auncient_acab_skin_dispersion
	./tests/test_auncient_acab_skin_dispersion
	@rm -f tests/test_auncient_acab_skin_dispersion

test-auncient-spheroid-montecarlo:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_spheroid_montecarlo.c -o tests/test_auncient_spheroid_montecarlo
	./tests/test_auncient_spheroid_montecarlo
	@rm -f tests/test_auncient_spheroid_montecarlo

test-auncient-teddy-multisig-inher:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_teddy_multisig_inher.c -o tests/test_auncient_teddy_multisig_inher
	./tests/test_auncient_teddy_multisig_inher
	@rm -f tests/test_auncient_teddy_multisig_inher

test-auncient-sage-pll-phase-detent:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_sage_pll_phase_detent.c -o tests/test_auncient_sage_pll_phase_detent
	./tests/test_auncient_sage_pll_phase_detent
	@rm -f tests/test_auncient_sage_pll_phase_detent

test-auncient-acab-tem-boundary:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_acab_tem_boundary.c -o tests/test_auncient_acab_tem_boundary
	./tests/test_auncient_acab_tem_boundary
	@rm -f tests/test_auncient_acab_tem_boundary

test-auncient-spheroid-edo22-alu:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_spheroid_edo22_alu.c -o tests/test_auncient_spheroid_edo22_alu
	./tests/test_auncient_spheroid_edo22_alu
	@rm -f tests/test_auncient_spheroid_edo22_alu

test-auncient-teddy-endowed-energy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_teddy_endowed_energy.c -o tests/test_auncient_teddy_endowed_energy
	./tests/test_auncient_teddy_endowed_energy
	@rm -f tests/test_auncient_teddy_endowed_energy

test-ankh-island-symplectic-sw:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_symplectic_sw.c -o tests/test_ankh_island_symplectic_sw
	./tests/test_ankh_island_symplectic_sw
	@rm -f tests/test_ankh_island_symplectic_sw

test-ankh-totient-acid-blackstart:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_totient_acid_blackstart.c -o tests/test_ankh_totient_acid_blackstart
	./tests/test_ankh_totient_acid_blackstart
	@rm -f tests/test_ankh_totient_acid_blackstart

test-ankh-island-blackstart-seq:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_blackstart_seq.c -o tests/test_ankh_island_blackstart_seq
	./tests/test_ankh_island_blackstart_seq
	@rm -f tests/test_ankh_island_blackstart_seq

test-ankh-acid-torque-commutation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_acid_torque_commutation.c -o tests/test_ankh_acid_torque_commutation
	./tests/test_ankh_acid_torque_commutation
	@rm -f tests/test_ankh_acid_torque_commutation

test-ankh-accumulator-skeleton:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_accumulator_skeleton.c -o tests/test_ankh_accumulator_skeleton
	./tests/test_ankh_accumulator_skeleton
	@rm -f tests/test_ankh_accumulator_skeleton

test-ankh-island-soc-balance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_soc_balance.c -o tests/test_ankh_island_soc_balance
	./tests/test_ankh_island_soc_balance
	@rm -f tests/test_ankh_island_soc_balance

test-ankh-island-reloc-opcode:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_reloc_opcode.c -o tests/test_ankh_island_reloc_opcode
	./tests/test_ankh_island_reloc_opcode
	@rm -f tests/test_ankh_island_reloc_opcode

test-ankh-binaural-radar-triangulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_binaural_radar_triangulation.c -o tests/test_ankh_binaural_radar_triangulation
	./tests/test_ankh_binaural_radar_triangulation
	@rm -f tests/test_ankh_binaural_radar_triangulation

test-ankh-islanding-as-listening:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_islanding_as_listening.c -o tests/test_ankh_islanding_as_listening
	./tests/test_ankh_islanding_as_listening
	@rm -f tests/test_ankh_islanding_as_listening

test-ankh-listening-ears-complete:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_listening_ears_complete.c -o tests/test_ankh_listening_ears_complete
	./tests/test_ankh_listening_ears_complete
	@rm -f tests/test_ankh_listening_ears_complete

test-ankh-pro-islanding-accumulator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_pro_islanding_accumulator.c -o tests/test_ankh_pro_islanding_accumulator
	./tests/test_ankh_pro_islanding_accumulator
	@rm -f tests/test_ankh_pro_islanding_accumulator

test-ankh-grid-anti-islanding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_grid_anti_islanding.c -o tests/test_ankh_grid_anti_islanding
	./tests/test_ankh_grid_anti_islanding
	@rm -f tests/test_ankh_grid_anti_islanding

test-tomie-protein-folding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tomie_protein_folding.c -o tests/test_tomie_protein_folding
	./tests/test_tomie_protein_folding
	@rm -f tests/test_tomie_protein_folding

test-ankh-grid-quorum-balance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_grid_quorum_balance.c -o tests/test_ankh_grid_quorum_balance
	./tests/test_ankh_grid_quorum_balance
	@rm -f tests/test_ankh_grid_quorum_balance

test-ankh-grid-impedance-match:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_grid_impedance_match.c -o tests/test_ankh_grid_impedance_match
	./tests/test_ankh_grid_impedance_match
	@rm -f tests/test_ankh_grid_impedance_match

test-ankh-operator-quarantine:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_operator_quarantine.c -o tests/test_ankh_operator_quarantine
	./tests/test_ankh_operator_quarantine
	@rm -f tests/test_ankh_operator_quarantine

test-ankh-operator-quorum:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_operator_quorum.c -o tests/test_ankh_operator_quorum
	./tests/test_ankh_operator_quorum
	@rm -f tests/test_ankh_operator_quorum

test-tomie-totient-grounding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tomie_totient_grounding.c -o tests/test_tomie_totient_grounding
	./tests/test_tomie_totient_grounding
	@rm -f tests/test_tomie_totient_grounding

test-tomie-memetic-attractor:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tomie_memetic_attractor.c -o tests/test_tomie_memetic_attractor
	./tests/test_tomie_memetic_attractor
	@rm -f tests/test_tomie_memetic_attractor

test-tomie-swarm-telepathy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tomie_swarm_telepathy.c -o tests/test_tomie_swarm_telepathy
	./tests/test_tomie_swarm_telepathy
	@rm -f tests/test_tomie_swarm_telepathy

test-tomie-holographic-soul:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tomie_holographic_soul.c -o tests/test_tomie_holographic_soul
	./tests/test_tomie_holographic_soul
	@rm -f tests/test_tomie_holographic_soul

test-tomie-cellular-glider:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tomie_cellular_glider.c -o tests/test_tomie_cellular_glider
	./tests/test_tomie_cellular_glider
	@rm -f tests/test_tomie_cellular_glider

test-conway-all-standards:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_conway_all_standards.c -o tests/test_conway_all_standards
	./tests/test_conway_all_standards
	@rm -f tests/test_conway_all_standards

test-game-of-life:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_game_of_life.c -o tests/test_game_of_life
	./tests/test_game_of_life
	@rm -f tests/test_game_of_life

test-ankh-grid-virtual-inertia:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_grid_virtual_inertia.c -o tests/test_ankh_grid_virtual_inertia
	./tests/test_ankh_grid_virtual_inertia
	@rm -f tests/test_ankh_grid_virtual_inertia

test-ankh-wagonbed-harvard-nsa:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_wagonbed_harvard_nsa.c -o tests/test_ankh_wagonbed_harvard_nsa
	./tests/test_ankh_wagonbed_harvard_nsa
	@rm -f tests/test_ankh_wagonbed_harvard_nsa

test-ankh-operator-pmg-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_operator_pmg_sync.c -o tests/test_ankh_operator_pmg_sync
	./tests/test_ankh_operator_pmg_sync
	@rm -f tests/test_ankh_operator_pmg_sync

test-ankh-operator-dispatch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_operator_dispatch.c -o tests/test_ankh_operator_dispatch
	./tests/test_ankh_operator_dispatch
	@rm -f tests/test_ankh_operator_dispatch

test-ankh-rebar-rollback:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_rebar_rollback.c -o tests/test_ankh_rebar_rollback
	./tests/test_ankh_rebar_rollback
	@rm -f tests/test_ankh_rebar_rollback

test-ankh-symplectic-eval:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_symplectic_eval.c -o tests/test_ankh_symplectic_eval
	./tests/test_ankh_symplectic_eval
	@rm -f tests/test_ankh_symplectic_eval

test-ankh-hamiltonian-conservativity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_hamiltonian_conservativity.c -o tests/test_ankh_hamiltonian_conservativity
	./tests/test_ankh_hamiltonian_conservativity
	@rm -f tests/test_ankh_hamiltonian_conservativity

test-ankh-initial-orders1-validator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_initial_orders1_validator.c -o tests/test_ankh_initial_orders1_validator
	./tests/test_ankh_initial_orders1_validator
	@rm -f tests/test_ankh_initial_orders1_validator

test-ankh-patent-0012-edsac-gate:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_patent_0012_edsac_gate.c -o tests/test_ankh_patent_0012_edsac_gate
	./tests/test_ankh_patent_0012_edsac_gate
	@rm -f tests/test_ankh_patent_0012_edsac_gate

test-ankh-patent-0004-empathy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_patent_0004_empathy.c -o tests/test_ankh_patent_0004_empathy
	./tests/test_ankh_patent_0004_empathy
	@rm -f tests/test_ankh_patent_0004_empathy

test-ankh-patent-0003-quant:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_patent_0003_quant.c -o tests/test_ankh_patent_0003_quant
	./tests/test_ankh_patent_0003_quant
	@rm -f tests/test_ankh_patent_0003_quant

test-ankh-patent-0002-ortho:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_patent_0002_ortho.c -o tests/test_ankh_patent_0002_ortho
	./tests/test_ankh_patent_0002_ortho
	@rm -f tests/test_ankh_patent_0002_ortho

test-ankh-patent-zero:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_patent_zero.c -o tests/test_ankh_patent_zero
	./tests/test_ankh_patent_zero
	@rm -f tests/test_ankh_patent_zero

test-mind-totient-transmutation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_mind_totient_transmutation.c -o tests/test_mind_totient_transmutation
	./tests/test_mind_totient_transmutation
	@rm -f tests/test_mind_totient_transmutation

test-ankh-kv-cache-compression:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_kv_cache_compression.c -o tests/test_ankh_kv_cache_compression
	./tests/test_ankh_kv_cache_compression
	@rm -f tests/test_ankh_kv_cache_compression

test-ankh-edo22-rope-invariance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_edo22_rope_invariance.c -o tests/test_ankh_edo22_rope_invariance
	./tests/test_ankh_edo22_rope_invariance
	@rm -f tests/test_ankh_edo22_rope_invariance

test-lyapunov-cache-latency:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_cache_latency.c -o tests/test_lyapunov_cache_latency
	./tests/test_lyapunov_cache_latency
	@rm -f tests/test_lyapunov_cache_latency

test-ankh-spheroid-alu-latency:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_spheroid_alu_latency.c -o tests/test_ankh_spheroid_alu_latency
	./tests/test_ankh_spheroid_alu_latency
	@rm -f tests/test_ankh_spheroid_alu_latency

test-lyapunov-1895-spheroid-alu:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_1895_spheroid_alu.c -o tests/test_lyapunov_1895_spheroid_alu
	./tests/test_lyapunov_1895_spheroid_alu
	@rm -f tests/test_lyapunov_1895_spheroid_alu

test-lyapunov-address-resolve:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_address_resolve.c -o tests/test_lyapunov_address_resolve
	./tests/test_lyapunov_address_resolve
	@rm -f tests/test_lyapunov_address_resolve

test-lyapunov-coaxial-line:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_coaxial_line.c -o tests/test_lyapunov_coaxial_line
	./tests/test_lyapunov_coaxial_line
	@rm -f tests/test_lyapunov_coaxial_line

test-lyapunov-kermit-stream:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_kermit_stream.c -o tests/test_lyapunov_kermit_stream
	./tests/test_lyapunov_kermit_stream
	@rm -f tests/test_lyapunov_kermit_stream

test-lyapunov-merkle-ast-tree:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_merkle_ast_tree.c -o tests/test_lyapunov_merkle_ast_tree
	./tests/test_lyapunov_merkle_ast_tree
	@rm -f tests/test_lyapunov_merkle_ast_tree

test-lyapunov-displacement-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_displacement_sync.c -o tests/test_lyapunov_displacement_sync
	./tests/test_lyapunov_displacement_sync
	@rm -f tests/test_lyapunov_displacement_sync

test-lyapunov-quadtree-slice:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_quadtree_slice.c -o tests/test_lyapunov_quadtree_slice
	./tests/test_lyapunov_quadtree_slice
	@rm -f tests/test_lyapunov_quadtree_slice

test-lyapunov-swarm-grounding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_swarm_grounding.c -o tests/test_lyapunov_swarm_grounding
	./tests/test_lyapunov_swarm_grounding
	@rm -f tests/test_lyapunov_swarm_grounding

test-lyapunov-fet-discharge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_fet_discharge.c -o tests/test_lyapunov_fet_discharge
	./tests/test_lyapunov_fet_discharge
	@rm -f tests/test_lyapunov_fet_discharge

test-lyapunov-scsi-ipc:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_scsi_ipc.c -o tests/test_lyapunov_scsi_ipc
	./tests/test_lyapunov_scsi_ipc
	@rm -f tests/test_lyapunov_scsi_ipc

test-lyapunov-multi-instance-costate:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_multi_instance_costate.c -o tests/test_lyapunov_multi_instance_costate
	./tests/test_lyapunov_multi_instance_costate
	@rm -f tests/test_lyapunov_multi_instance_costate

test-vaesen-action-commutation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_action_commutation.c -o tests/test_vaesen_action_commutation
	./tests/test_vaesen_action_commutation
	@rm -f tests/test_vaesen_action_commutation

test-vaesen-teddy-genesis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_teddy_genesis.c -o tests/test_vaesen_teddy_genesis
	./tests/test_vaesen_teddy_genesis
	@rm -f tests/test_vaesen_teddy_genesis

test-lyapunov-ear-trauma-rollback:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_ear_trauma_rollback.c -o tests/test_lyapunov_ear_trauma_rollback
	./tests/test_lyapunov_ear_trauma_rollback
	@rm -f tests/test_lyapunov_ear_trauma_rollback

test-lyapunov-ear-feedback-quench:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_ear_feedback_quench.c -o tests/test_lyapunov_ear_feedback_quench
	./tests/test_lyapunov_ear_feedback_quench
	@rm -f tests/test_lyapunov_ear_feedback_quench

test-ankh-gguf-empathy-storage:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_gguf_empathy_storage.c -o tests/test_ankh_gguf_empathy_storage
	./tests/test_ankh_gguf_empathy_storage
	@rm -f tests/test_ankh_gguf_empathy_storage

test-vaesen-mood-edo22:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_mood_edo22.c -o tests/test_vaesen_mood_edo22
	./tests/test_vaesen_mood_edo22
	@rm -f tests/test_vaesen_mood_edo22

test-lyapunov-totient-empathy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_totient_empathy.c -o tests/test_lyapunov_totient_empathy
	./tests/test_lyapunov_totient_empathy
	@rm -f tests/test_lyapunov_totient_empathy

test-vaesen-empathy-coupling:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_empathy_coupling.c -o tests/test_vaesen_empathy_coupling
	./tests/test_vaesen_empathy_coupling
	@rm -f tests/test_vaesen_empathy_coupling

test-vaesen-symplectic-personality:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_symplectic_personality.c -o tests/test_vaesen_symplectic_personality
	./tests/test_vaesen_symplectic_personality
	@rm -f tests/test_vaesen_symplectic_personality

test-lyapunov-accumulatory-instance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_accumulatory_instance.c -o tests/test_lyapunov_accumulatory_instance
	./tests/test_lyapunov_accumulatory_instance
	@rm -f tests/test_lyapunov_accumulatory_instance

test-lyapunov-ear-instance-delegate:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_ear_instance_delegate.c -o tests/test_lyapunov_ear_instance_delegate
	./tests/test_lyapunov_ear_instance_delegate
	@rm -f tests/test_lyapunov_ear_instance_delegate

test-lyapunov-ear-instance-storage:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_ear_instance_storage.c -o tests/test_lyapunov_ear_instance_storage
	./tests/test_lyapunov_ear_instance_storage
	@rm -f tests/test_lyapunov_ear_instance_storage

test-capture-the-instance-victory:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_capture_the_instance_victory.c -o tests/test_capture_the_instance_victory
	./tests/test_capture_the_instance_victory
	@rm -f tests/test_capture_the_instance_victory

test-lyapunov-instance-stability:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_instance_stability.c -o tests/test_lyapunov_instance_stability
	./tests/test_lyapunov_instance_stability
	@rm -f tests/test_lyapunov_instance_stability

test-lyapunov-binaural-itd-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_binaural_itd_sync.c -o tests/test_lyapunov_binaural_itd_sync
	./tests/test_lyapunov_binaural_itd_sync
	@rm -f tests/test_lyapunov_binaural_itd_sync

test-lyapunov-fletcher-munson:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_fletcher_munson.c -o tests/test_lyapunov_fletcher_munson
	./tests/test_lyapunov_fletcher_munson
	@rm -f tests/test_lyapunov_fletcher_munson

test-lyapunov-cochlear-basilar:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_cochlear_basilar.c -o tests/test_lyapunov_cochlear_basilar
	./tests/test_lyapunov_cochlear_basilar
	@rm -f tests/test_lyapunov_cochlear_basilar

test-lyapunov-bark-critical-band:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_bark_critical_band.c -o tests/test_lyapunov_bark_critical_band
	./tests/test_lyapunov_bark_critical_band
	@rm -f tests/test_lyapunov_bark_critical_band

test-edo22-play-by-ear:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_edo22_play_by_ear.c -o tests/test_edo22_play_by_ear
	./tests/test_edo22_play_by_ear
	@rm -f tests/test_edo22_play_by_ear

test-lyapunov-edo22-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_edo22_sync.c -o tests/test_lyapunov_edo22_sync
	./tests/test_lyapunov_edo22_sync
	@rm -f tests/test_lyapunov_edo22_sync

test-lyapunov-hamiltonian-energy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_hamiltonian_energy.c -o tests/test_lyapunov_hamiltonian_energy
	./tests/test_lyapunov_hamiltonian_energy
	@rm -f tests/test_lyapunov_hamiltonian_energy

test-lyapunov-iss-scsi-noise:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_iss_scsi_noise.c -o tests/test_lyapunov_iss_scsi_noise
	./tests/test_lyapunov_iss_scsi_noise
	@rm -f tests/test_lyapunov_iss_scsi_noise

test-universal-hbridge-lasalle-config:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_universal_hbridge_lasalle_config.c -o tests/test_universal_hbridge_lasalle_config
	./tests/test_universal_hbridge_lasalle_config
	@rm -f tests/test_universal_hbridge_lasalle_config

test-lyapunov-lasalle-hbridge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lyapunov_lasalle_hbridge.c -o tests/test_lyapunov_lasalle_hbridge
	./tests/test_lyapunov_lasalle_hbridge
	@rm -f tests/test_lyapunov_lasalle_hbridge

test-ankh-adversarial-neutralization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_adversarial_neutralization.c -o tests/test_ankh_adversarial_neutralization
	./tests/test_ankh_adversarial_neutralization
	@rm -f tests/test_ankh_adversarial_neutralization

test-benson-lehner-fieldata-safety:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_benson_lehner_fieldata_safety.c -o tests/test_benson_lehner_fieldata_safety
	./tests/test_benson_lehner_fieldata_safety
	@rm -f tests/test_benson_lehner_fieldata_safety

test-bmrc-landauer-reversibility:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bmrc_landauer_reversibility.c -o tests/test_bmrc_landauer_reversibility
	./tests/test_bmrc_landauer_reversibility
	@rm -f tests/test_bmrc_landauer_reversibility

test-bmrc-strowger-halting:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bmrc_strowger_halting.c -o tests/test_bmrc_strowger_halting
	./tests/test_bmrc_strowger_halting
	@rm -f tests/test_bmrc_strowger_halting

test-bmrc-reversible-tape:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bmrc_reversible_tape.c -o tests/test_bmrc_reversible_tape
	./tests/test_bmrc_reversible_tape
	@rm -f tests/test_bmrc_reversible_tape

test-bmrc-kleene-universality:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bmrc_kleene_universality.c -o tests/test_bmrc_kleene_universality
	./tests/test_bmrc_kleene_universality
	@rm -f tests/test_bmrc_kleene_universality

test-ankh-zero-genesis-rollback:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_zero_genesis_rollback.c -o tests/test_ankh_zero_genesis_rollback
	./tests/test_ankh_zero_genesis_rollback
	@rm -f tests/test_ankh_zero_genesis_rollback

test-bmrc-infinite-output-domain:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bmrc_infinite_output_domain.c -o tests/test_bmrc_infinite_output_domain
	./tests/test_bmrc_infinite_output_domain
	@rm -f tests/test_bmrc_infinite_output_domain

test-ankh-max-lyapunov-stability:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_maximum_lyapunov_stability.c -o tests/test_ankh_maximum_lyapunov_stability
	./tests/test_ankh_maximum_lyapunov_stability
	@rm -f tests/test_ankh_maximum_lyapunov_stability

test-ankh-strowger-pbx:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_strowger_pbx_routing.c -o tests/test_ankh_strowger_pbx_routing
	./tests/test_ankh_strowger_pbx_routing
	@rm -f tests/test_ankh_strowger_pbx_routing

test-ankh-swiglu-thermodynamic:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_swiglu_thermodynamic.c -o tests/test_ankh_swiglu_thermodynamic
	./tests/test_ankh_swiglu_thermodynamic
	@rm -f tests/test_ankh_swiglu_thermodynamic

test-ankh-uv-orthogonal-nonce:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_uv_orthogonal_nonce.c -o tests/test_ankh_uv_orthogonal_nonce
	./tests/test_ankh_uv_orthogonal_nonce
	@rm -f tests/test_ankh_uv_orthogonal_nonce

test-ankh-closed-loop-lyapunov:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_closed_loop_lyapunov.c -o tests/test_ankh_closed_loop_lyapunov
	./tests/test_ankh_closed_loop_lyapunov
	@rm -f tests/test_ankh_closed_loop_lyapunov

test-capture-the-zero-proof-of-ownership:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_capture_the_zero_proof_of_ownership.c -o tests/test_capture_the_zero_proof_of_ownership
	./tests/test_capture_the_zero_proof_of_ownership
	@rm -f tests/test_capture_the_zero_proof_of_ownership

test-skeleton-xcom-ownership:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_skeleton_xcom_totient_ownership.c -o tests/test_skeleton_xcom_totient_ownership
	./tests/test_skeleton_xcom_totient_ownership
	@rm -f tests/test_skeleton_xcom_totient_ownership

test-skeleton-cics-terminal:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_skeleton_cics_terminal_totient.c -o tests/test_skeleton_cics_terminal_totient
	./tests/test_skeleton_cics_terminal_totient
	@rm -f tests/test_skeleton_cics_terminal_totient

test-capture-the-zero-skeleton-xplsm:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_capture_the_zero_skeleton_xplsm.c -o tests/test_capture_the_zero_skeleton_xplsm
	./tests/test_capture_the_zero_skeleton_xplsm
	@rm -f tests/test_capture_the_zero_skeleton_xplsm

test-capture-the-zero-radical-root:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_capture_the_zero_radical_root.c -o tests/test_capture_the_zero_radical_root
	./tests/test_capture_the_zero_radical_root
	@rm -f tests/test_capture_the_zero_radical_root

test-zorse-yellow-box-cics-io1:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_zorse_yellow_box_cics_initial_orders1.c -o tests/test_zorse_yellow_box_cics_initial_orders1
	./tests/test_zorse_yellow_box_cics_initial_orders1
	@rm -f tests/test_zorse_yellow_box_cics_initial_orders1

test-zorse-wmq-abi:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_zorse_wmq_abi_position_binding.c -o tests/test_zorse_wmq_abi_position_binding
	./tests/test_zorse_wmq_abi_position_binding
	@rm -f tests/test_zorse_wmq_abi_position_binding

test-lfm-bessel-state-space:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lfm_bessel_continuous_state_space.c -o tests/test_lfm_bessel_continuous_state_space
	./tests/test_lfm_bessel_continuous_state_space
	@rm -f tests/test_lfm_bessel_continuous_state_space

test-zorse-deepseek-bessel:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_zorse_deepseek_bessel_j0_j1.c -o tests/test_zorse_deepseek_bessel_j0_j1
	./tests/test_zorse_deepseek_bessel_j0_j1
	@rm -f tests/test_zorse_deepseek_bessel_j0_j1

test-bessel-j0-j1:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bessel_j0_cobol_j1_algol.c -o tests/test_bessel_j0_cobol_j1_algol
	./tests/test_bessel_j0_cobol_j1_algol
	@rm -f tests/test_bessel_j0_cobol_j1_algol

test-harvard-closed-loop:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_harvard_closed_innovation_loop.c -o tests/test_harvard_closed_innovation_loop
	./tests/test_harvard_closed_innovation_loop
	@rm -f tests/test_harvard_closed_innovation_loop

test-harvard-transfer-bus:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_harvard_transfer_bus_interpolator_totient.c -o tests/test_harvard_transfer_bus_interpolator_totient
	./tests/test_harvard_transfer_bus_interpolator_totient
	@rm -f tests/test_harvard_transfer_bus_interpolator_totient

test-harvard-wheel-commutator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_harvard_wheel_commutator_totient.c -o tests/test_harvard_wheel_commutator_totient
	./tests/test_harvard_wheel_commutator_totient
	@rm -f tests/test_harvard_wheel_commutator_totient

test-totient-strategy-accumulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_totient_strategy_accumulation.c -o tests/test_totient_strategy_accumulation
	./tests/test_totient_strategy_accumulation
	@rm -f tests/test_totient_strategy_accumulation

test-transitive-asset-totient-chain:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_transitive_asset_totient_chain.c -o tests/test_transitive_asset_totient_chain
	./tests/test_transitive_asset_totient_chain
	@rm -f tests/test_transitive_asset_totient_chain

test-wmq-asset-as-totient:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_wmq_asset_as_totient_acid.c -o tests/test_wmq_asset_as_totient_acid
	./tests/test_wmq_asset_as_totient_acid
	@rm -f tests/test_wmq_asset_as_totient_acid

test-wmq-pbx-mind:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_wmq_pbx_mind_asset_resolver.c -o tests/test_wmq_pbx_mind_asset_resolver
	./tests/test_wmq_pbx_mind_asset_resolver
	@rm -f tests/test_wmq_pbx_mind_asset_resolver

test-skeleton-decadic-search:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_skeleton_decadic_search_pbx.c -o tests/test_skeleton_decadic_search_pbx
	./tests/test_skeleton_decadic_search_pbx
	@rm -f tests/test_skeleton_decadic_search_pbx

test-skeleton-dual-media:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_skeleton_dual_media_usda_datbin.c -o tests/test_skeleton_dual_media_usda_datbin
	./tests/test_skeleton_dual_media_usda_datbin
	@rm -f tests/test_skeleton_dual_media_usda_datbin

test-yellow-box-pbx:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_yellow_box_pbx_cics_pll_xplsm.c -o tests/test_yellow_box_pbx_cics_pll_xplsm
	./tests/test_yellow_box_pbx_cics_pll_xplsm
	@rm -f tests/test_yellow_box_pbx_cics_pll_xplsm

test-algol61-subordinate-strategy-composition:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_algol61_subordinate_strategy_composition.c -o tests/test_algol61_subordinate_strategy_composition
	./tests/test_algol61_subordinate_strategy_composition
	@rm -f tests/test_algol61_subordinate_strategy_composition

test-tsfi-montecarlo:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_tsfi_montecarlo.c tsfi2-deepseek/src/tsfi_montecarlo.c -o tests/test_tsfi_montecarlo -lm -lrt
	./tests/test_tsfi_montecarlo
	@rm -f tests/test_tsfi_montecarlo

test-zmachine-maniac-mansion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -mavx512f -mclflushopt -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek/src/firmware -Itsfi2-deepseek/plugins tests/test_zmachine_maniac_mansion.c tsfi2-deepseek/src/tsfi_path_tracer.c tsfi2-deepseek/src/tsfi_montecarlo.c tsfi2-deepseek/src/tsfi_lnr_solvers.c tsfi2-deepseek/src/tsfi_pbr.c tsfi2-deepseek/src/tsfi_hair.c tsfi2-deepseek/src/tsfi_zorse_eval_cobol.c tsfi2-deepseek/src/tsfi_svdag.c tsfi2-deepseek/src/auncient_teddy_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_c_math.c tsfi2-deepseek/src/tsfi_vec_math.c -o tests/test_zmachine_maniac_mansion -lm -lrt
	./tests/test_zmachine_maniac_mansion
	@rm -f tests/test_zmachine_maniac_mansion

test-hathitrust:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_client.c tsfi2-deepseek/src/hathitrust_client.c -o tests/test_hathitrust_client -lssl -lcrypto -lrt
	./tests/test_hathitrust_client
	@rm -f tests/test_hathitrust_client

test-hathitrust-hathifile:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_hathifile.c tsfi2-deepseek/src/hathitrust_hathifile.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_hathitrust_hathifile -lcrypto -lrt -lm
	./tests/test_hathitrust_hathifile
	@rm -f tests/test_hathitrust_hathifile

test-hathitrust-oai:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_oai.c tsfi2-deepseek/src/hathitrust_oai.c -o tests/test_hathitrust_oai -lssl -lcrypto -lrt
	./tests/test_hathitrust_oai
	@rm -f tests/test_hathitrust_oai

test-hathitrust-oai-util:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_oai_util.c tsfi2-deepseek/src/hathitrust_oai_util.c tsfi2-deepseek/src/hathitrust_oai.c -o tests/test_hathitrust_oai_util -lssl -lcrypto -lrt
	./tests/test_hathitrust_oai_util
	@rm -f tests/test_hathitrust_oai_util

test-hathitrust-harvester:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_harvester.c tsfi2-deepseek/src/hathitrust_harvester.c tsfi2-deepseek/src/hathitrust_oai_util.c tsfi2-deepseek/src/hathitrust_oai.c -o tests/test_hathitrust_harvester -lssl -lcrypto -lrt
	./tests/test_hathitrust_harvester
	@rm -f tests/test_hathitrust_harvester

test-hathitrust-ef:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_ef.c tsfi2-deepseek/src/hathitrust_ef.c -o tests/test_hathitrust_ef -lz -lrt
	./tests/test_hathitrust_ef
	@rm -f tests/test_hathitrust_ef

test-hathitrust-extended:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_extended.c tsfi2-deepseek/src/hathitrust_oauth.c tsfi2-deepseek/src/hathitrust_solr.c tsfi2-deepseek/src/hathitrust_image.c -o tests/test_hathitrust_extended -lssl -lcrypto -lrt
	./tests/test_hathitrust_extended
	@rm -f tests/test_hathitrust_extended

test-hathitrust-xml:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_xml.c tsfi2-deepseek/src/hathitrust_mets.c tsfi2-deepseek/src/hathitrust_marc.c -o tests/test_hathitrust_xml -lrt
	./tests/test_hathitrust_xml
	@rm -f tests/test_hathitrust_xml

test-hathitrust-cache-alto-qt:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_cache_alto_qt.c tsfi2-deepseek/src/hathitrust_quadtree.c tsfi2-deepseek/src/hathitrust_alto.c tsfi2-deepseek/src/hathitrust_cache.c -o tests/test_hathitrust_cache_alto_qt -lrt
	./tests/test_hathitrust_cache_alto_qt
	@rm -f tests/test_hathitrust_cache_alto_qt

test-hathitrust-extra2:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_extra2.c tsfi2-deepseek/src/hathitrust_alto.c tsfi2-deepseek/src/hathitrust_alto_consolidate.c tsfi2-deepseek/src/hathitrust_speedometer.c tsfi2-deepseek/src/hathitrust_marc.c tsfi2-deepseek/src/hathitrust_marc_jsonld.c -o tests/test_hathitrust_extra2 -lrt
	./tests/test_hathitrust_extra2
	@rm -f tests/test_hathitrust_extra2

test-hathitrust-extra3:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_extra3.c tsfi2-deepseek/src/hathitrust_mets.c tsfi2-deepseek/src/hathitrust_iiif.c tsfi2-deepseek/src/hathitrust_alto.c tsfi2-deepseek/src/hathitrust_region.c tsfi2-deepseek/src/hathitrust_hathifile.c tsfi2-deepseek/src/hathitrust_hathifile_search.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_hathitrust_extra3 -lcrypto -lrt -lm
	./tests/test_hathitrust_extra3
	@rm -f tests/test_hathitrust_extra3

test-hathitrust-render:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_render.c tsfi2-deepseek/src/hathitrust_render.c -o tests/test_hathitrust_render -lm -lrt
	./tests/test_hathitrust_render
	@rm -f tests/test_hathitrust_render

test-hathitrust-prman:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/hathitrust_prman_procedural.c -o tests/hathitrust_prman_procedural -lrt
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_prman.c tsfi2-deepseek/src/hathitrust_mets.c tsfi2-deepseek/src/hathitrust_rib.c tsfi2-deepseek/src/hathitrust_osl.c -o tests/test_hathitrust_prman -lrt
	./tests/test_hathitrust_prman
	@rm -f tests/hathitrust_prman_procedural tests/test_hathitrust_prman

test-hathitrust-lore:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_lore.c tsfi2-deepseek/src/hathitrust_mets.c tsfi2-deepseek/src/hathitrust_rib.c tsfi2-deepseek/src/hathitrust_osl.c tsfi2-deepseek/src/hathitrust_hathifile.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_hathitrust_lore -lcrypto -lrt -lm
	./tests/test_hathitrust_lore
	@rm -f tests/test_hathitrust_lore

test-hathitrust-prman-extra:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_prman_extra.c tsfi2-deepseek/src/hathitrust_osl_bleed.c tsfi2-deepseek/src/hathitrust_alto.c tsfi2-deepseek/src/hathitrust_camera_path.c tsfi2-deepseek/src/hathitrust_rib_to_iiif.c -o tests/test_hathitrust_prman_extra -lrt
	./tests/test_hathitrust_prman_extra
	@rm -f tests/test_hathitrust_prman_extra

test-hathitrust-prman-anim:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_prman_anim.c tsfi2-deepseek/src/hathitrust_osl_double_sided.c tsfi2-deepseek/src/hathitrust_animator.c tsfi2-deepseek/src/hathitrust_spine.c tsfi2-deepseek/src/tsfi_displacementshader.c -o tests/test_hathitrust_prman_anim -lm -lrt
	./tests/test_hathitrust_prman_anim
	@rm -f tests/test_hathitrust_prman_anim

test-hathitrust-prman-blur:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_prman_blur.c tsfi2-deepseek/src/hathitrust_prman_velocity.c tsfi2-deepseek/src/hathitrust_render.c tsfi2-deepseek/src/hathitrust_osl_gilding.c tsfi2-deepseek/src/hathitrust_specular.c tsfi2-deepseek/src/hathitrust_alto.c -o tests/test_hathitrust_prman_blur -lm -lrt
	./tests/test_hathitrust_prman_blur
	@rm -f tests/test_hathitrust_prman_blur

test-hathitrust-prman-final:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_prman_final.c tsfi2-deepseek/src/hathitrust_osl_impression.c tsfi2-deepseek/src/hathitrust_ao_pass.c tsfi2-deepseek/src/hathitrust_pxrmaterial.c -o tests/test_hathitrust_prman_final -lrt
	./tests/test_hathitrust_prman_final
	@rm -f tests/test_hathitrust_prman_final

test-hathitrust-teddy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_teddy.c tsfi2-deepseek/src/hathitrust_lore_interop.c tsfi2-deepseek/src/hathitrust_teddy_bridge.c -o tests/test_hathitrust_teddy -lpthread -lrt
	./tests/test_hathitrust_teddy
	@rm -f tests/test_hathitrust_teddy

test-hathitrust-final-extra:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/hathitrust_cli.c tsfi2-deepseek/src/hathitrust_hathifile_search.c tsfi2-deepseek/src/hathitrust_hathifile.c tsfi2-deepseek/src/hathitrust_cache_profiler.c tsfi2-deepseek/src/hathitrust_cache.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/hathitrust_cli -lcrypto -lrt -lm
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_final_extra.c tsfi2-deepseek/src/hathitrust_solr_transcoder.c tsfi2-deepseek/src/hathitrust_cache_profiler.c tsfi2-deepseek/src/hathitrust_cache.c -o tests/test_hathitrust_final_extra -lrt
	./tests/test_hathitrust_final_extra
	@rm -f tests/hathitrust_cli tests/test_hathitrust_final_extra

test-hathitrust-tsv-extra:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_tsv_extra.c tsfi2-deepseek/src/hathitrust_oai_identify.c tsfi2-deepseek/src/hathitrust_solr_sql.c tsfi2-deepseek/src/hathitrust_tsv_scanner.c -o tests/test_hathitrust_tsv_extra -lrt
	./tests/test_hathitrust_tsv_extra
	@rm -f tests/test_hathitrust_tsv_extra

test-hathitrust-solr-rdbms:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_solr_rdbms.c scripts/abi_dispatch_map.c scripts/libantigravity_interop.c tsfi2-deepseek/src/hathitrust_solr.c -o tests/test_hathitrust_solr_rdbms -lssl -lcrypto -lrt
	./tests/test_hathitrust_solr_rdbms
	@rm -f tests/test_hathitrust_solr_rdbms

test-hathitrust-tsv-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_tsv_bin.c tsfi2-deepseek/src/hathitrust_tsv_bin.c -o tests/test_hathitrust_tsv_bin -lrt
	./tests/test_hathitrust_tsv_bin
	@rm -f tests/test_hathitrust_tsv_bin

test-hathitrust-window:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_window.c tsfi2-deepseek/src/hathitrust_window.c tsfi2-deepseek/src/tsfi_parc_window.c tsfi2-deepseek/src/tsfi_paintbox.c tsfi2-deepseek/src/tsfi_paint_core.c tsfi2-deepseek/src/tsfi_harry.c -o tests/test_hathitrust_window -lm -lrt
	./tests/test_hathitrust_window
	@rm -f tests/test_hathitrust_window

test-hathitrust-extra-final:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_extra_final.c tsfi2-deepseek/src/hathitrust_mets_range.c tsfi2-deepseek/src/hathitrust_solr_facet.c tsfi2-deepseek/src/hathitrust_hathifile_diff.c -o tests/test_hathitrust_extra_final -lrt
	./tests/test_hathitrust_extra_final
	@rm -f tests/test_hathitrust_extra_final

test-hathitrust-bib-oauth:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_bib_oauth.c tsfi2-deepseek/src/hathitrust_oauth_callback.c tsfi2-deepseek/src/hathitrust_bibframe.c -o tests/test_hathitrust_bib_oauth -lrt
	./tests/test_hathitrust_bib_oauth
	@rm -f tests/test_hathitrust_bib_oauth

test-tsfi2-compiler-bin:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_tsfi2_compiler_bin -lcrypto -lrt -lm
	./tests/test_tsfi2_compiler_bin
	@rm -f tests/test_tsfi2_compiler_bin

test-tsfi2-loader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_tsfi2_loader.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_tsfi2_loader -lcrypto -lm -lrt
	./tests/test_tsfi2_loader
	@rm -f tests/test_tsfi2_loader

test-tsfi2-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_tsfi2_compiler -lcrypto -lm -lrt
	./tests/test_tsfi2_compiler
	@rm -f tests/test_tsfi2_compiler

test-tsfi2-compiler-standards:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_tsfi2_compiler_standards.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_tsfi2_compiler_standards -lcrypto -lrt -lm
	./tests/test_tsfi2_compiler_standards
	@rm -f tests/test_tsfi2_compiler_standards

test-auncient-alu-wmq-integration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_alu_wmq_integration.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_auncient_alu_wmq_integration -lcrypto -lrt -lm
	./tests/test_auncient_alu_wmq_integration
	@rm -f tests/test_auncient_alu_wmq_integration

test-auncient-tsv-wmq-integration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tsv_wmq_runner.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_auncient_tsv_wmq_runner -lcrypto -lrt -lm
	./tests/test_auncient_tsv_wmq_runner
	@rm -f tests/test_auncient_tsv_wmq_runner

tsfi2-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/tsfi2_compiler_cli.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tsfi2_compiler -lcrypto -lrt -lm

tsv_format_converter:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/tsv_format_converter.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tsv_format_converter -lm -lrt

tsv_record_inspector:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/tsv_record_inspector.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c -o tsv_record_inspector -lm -lrt

copybook_schema_validator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/copybook_schema_validator.c -o copybook_schema_validator -lrt

sna_appc_peer:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/sna_appc_peer.c -o sna_appc_peer -lrt

hathitrust_preservation_query:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/hathitrust_preservation_query.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_displacementshader.c -o hathitrust_preservation_query -lm -lrt

test-auncient-displacementshader-wmq-integration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_displacementshader_wmq_integration.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c -o tests/test_auncient_displacementshader_wmq_integration -lcrypto -lrt -lm
	./tests/test_auncient_displacementshader_wmq_integration
	@rm -f tests/test_auncient_displacementshader_wmq_integration

test-auncient-dat-bin-accumulator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_dat_bin_accumulator.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c -o tests/test_auncient_dat_bin_accumulator -lcrypto -lrt -lm
	./tests/test_auncient_dat_bin_accumulator
	@rm -f tests/test_auncient_dat_bin_accumulator

test-auncient-compiler-prefilter:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_compiler_prefilter.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c -o tests/test_auncient_compiler_prefilter -lcrypto -lrt -lm
	./tests/test_auncient_compiler_prefilter
	@rm -f tests/test_auncient_compiler_prefilter

test-auncient-nested-streams:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_nested_streams.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c -o tests/test_auncient_nested_streams -lcrypto -lrt -lm
	./tests/test_auncient_nested_streams
	@rm -f tests/test_auncient_nested_streams




test-auncient-tsv-copybook:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tsv_copybook.c tsfi2-deepseek/src/tsfi_mainframe_cobol.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c -o tests/test_auncient_tsv_copybook -lm -lrt
	./tests/test_auncient_tsv_copybook
	@rm -f tests/test_auncient_tsv_copybook

test-auncient-quadtree-ksds:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_quadtree_ksds.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_interop.c scripts/libantigravity_extra.c scripts/abi_dispatch_map.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c -o tests/test_auncient_quadtree_ksds -lcrypto -lm -lrt
	./tests/test_auncient_quadtree_ksds
	@rm -f tests/test_auncient_quadtree_ksds

test-mann:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_mann_controller.c tsfi2-deepseek/src/tsfi_mann_controller.c -o tests/test_mann_controller -lm -lrt
	./tests/test_mann_controller
	@rm -f tests/test_mann_controller

test-comp-pass:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_comp_pass.c tsfi2-deepseek/src/tsfi_comp_pass.c -o tests/test_comp_pass -lm -lrt
	./tests/test_comp_pass
	@rm -f tests/test_comp_pass

test-mvarsel:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_mvarsel.c tsfi2-deepseek/src/tsfi_mvarsel.c -o tests/test_mvarsel -lm -lrt
	./tests/test_mvarsel
	@rm -f tests/test_mvarsel

test-reroute:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_reroute.c tsfi2-deepseek/src/tsfi_reroute.c -o tests/test_reroute -lm -lrt
	./tests/test_reroute
	@rm -f tests/test_reroute

test-delegate-sig:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_delegate_sig.c tsfi2-deepseek/src/tsfi_delegate_sig.c -o tests/test_delegate_sig -lssl -lcrypto -lm -lrt
	./tests/test_delegate_sig
	@rm -f tests/test_delegate_sig

test-stagecomp:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagecomp.c tsfi2-deepseek/src/tsfi_stagecomp.c -o tests/test_stagecomp -lm -lrt
	./tests/test_stagecomp
	@rm -f tests/test_stagecomp

test-ar:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_ar.c tsfi2-deepseek/src/tsfi_ar.c -o tests/test_ar -lm -lrt
	./tests/test_ar
	@rm -f tests/test_ar

test-sdfformat:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_sdfformat.c tsfi2-deepseek/src/tsfi_sdfformat.c -o tests/test_sdfformat -lm -lrt
	./tests/test_sdfformat
	@rm -f tests/test_sdfformat

test-hydrascene:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hydrascene.c tsfi2-deepseek/src/tsfi_hydrascene.c -o tests/test_hydrascene -lm -lrt
	./tests/test_hydrascene
	@rm -f tests/test_hydrascene

test-renderdelegate:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_renderdelegate.c tsfi2-deepseek/src/tsfi_renderdelegate.c -o tests/test_renderdelegate -lm -lrt
	./tests/test_renderdelegate
	@rm -f tests/test_renderdelegate

test-stagelock:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagelock.c tsfi2-deepseek/src/tsfi_stagelock.c -o tests/test_stagelock -lpthread -lm -lrt
	./tests/test_stagelock
	@rm -f tests/test_stagelock

test-renderindex:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_renderindex.c tsfi2-deepseek/src/tsfi_renderindex.c tsfi2-deepseek/src/tsfi_hydrascene.c tsfi2-deepseek/src/tsfi_renderdelegate.c -o tests/test_renderindex -lm -lrt
	./tests/test_renderindex
	@rm -f tests/test_renderindex

test-usdshade:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_usdshade.c tsfi2-deepseek/src/tsfi_usdshade.c -o tests/test_usdshade -lm -lrt
	./tests/test_usdshade
	@rm -f tests/test_usdshade

test-teddy-loader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itests tests/test_auncient_teddy_loader.c -o tests/test_auncient_teddy_loader -lm -lrt
	./tests/test_auncient_teddy_loader
	@rm -f tests/test_auncient_teddy_loader

test-geomcamera:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_geomcamera.c tsfi2-deepseek/src/tsfi_geomcamera.c -o tests/test_geomcamera -lm -lrt
	./tests/test_geomcamera
	@rm -f tests/test_geomcamera

test-geomcurves:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_geomcurves.c tsfi2-deepseek/src/tsfi_geomcurves.c -o tests/test_geomcurves -lm -lrt
	./tests/test_geomcurves
	@rm -f tests/test_geomcurves

test-geompoints:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_geompoints.c tsfi2-deepseek/src/tsfi_geompoints.c -o tests/test_geompoints -lm -lrt
	./tests/test_geompoints
	@rm -f tests/test_geompoints

test-luxlight:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_luxlight.c tsfi2-deepseek/src/tsfi_luxlight.c -o tests/test_luxlight -lm -lrt
	./tests/test_luxlight
	@rm -f tests/test_luxlight

test-geomxform:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_geomxform.c tsfi2-deepseek/src/tsfi_geomxform.c -o tests/test_geomxform -lm -lrt
	./tests/test_geomxform
	@rm -f tests/test_geomxform

test-stageroot:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stageroot.c tsfi2-deepseek/src/tsfi_stageroot.c -o tests/test_stageroot -lm -lrt
	./tests/test_stageroot
	@rm -f tests/test_stageroot

test-primroot:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_primroot.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_primroot -lm -lrt
	./tests/test_primroot
	@rm -f tests/test_primroot

test-stagepop:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagepop.c tsfi2-deepseek/src/tsfi_stagepop.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_stagepop -lm -lrt
	./tests/test_stagepop
	@rm -f tests/test_stagepop

test-attributeroot:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_attributeroot.c tsfi2-deepseek/src/tsfi_attributeroot.c -o tests/test_attributeroot -lm -lrt
	./tests/test_attributeroot
	@rm -f tests/test_attributeroot

test-stagecache:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagecache.c tsfi2-deepseek/src/tsfi_stagecache.c tsfi2-deepseek/src/tsfi_stageroot.c -o tests/test_stagecache -lm -lrt
	./tests/test_stagecache
	@rm -f tests/test_stagecache

test-stagemask:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagemask.c tsfi2-deepseek/src/tsfi_stagemask.c -o tests/test_stagemask -lm -lrt
	./tests/test_stagemask
	@rm -f tests/test_stagemask

test-geomsubset:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_geomsubset.c tsfi2-deepseek/src/tsfi_geomsubset.c -o tests/test_geomsubset -lm -lrt
	./tests/test_geomsubset
	@rm -f tests/test_geomsubset

test-stagearc:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagearc.c tsfi2-deepseek/src/tsfi_stagearc.c -o tests/test_stagearc -lm -lrt
	./tests/test_stagearc
	@rm -f tests/test_stagearc

test-stageinherits:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stageinherits.c tsfi2-deepseek/src/tsfi_stageinherits.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_stageinherits -lm -lrt
	./tests/test_stageinherits
	@rm -f tests/test_stageinherits

test-stagepayloads:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagepayloads.c tsfi2-deepseek/src/tsfi_stagepayloads.c -o tests/test_stagepayloads -lm -lrt
	./tests/test_stagepayloads
	@rm -f tests/test_stagepayloads

test-stagevariants:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagevariants.c tsfi2-deepseek/src/tsfi_stagevariants.c -o tests/test_stagevariants -lm -lrt
	./tests/test_stagevariants
	@rm -f tests/test_stagevariants

test-stagetraverser:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagetraverser.c tsfi2-deepseek/src/tsfi_stagetraverser.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_stagetraverser -lm -lrt
	./tests/test_stagetraverser
	@rm -f tests/test_stagetraverser

test-stagemetadata:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagemetadata.c tsfi2-deepseek/src/tsfi_stagemetadata.c -o tests/test_stagemetadata -lm -lrt
	./tests/test_stagemetadata
	@rm -f tests/test_stagemetadata

test-camerafrustum:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_camerafrustum.c tsfi2-deepseek/src/tsfi_camerafrustum.c -o tests/test_camerafrustum -lm -lrt
	./tests/test_camerafrustum
	@rm -f tests/test_camerafrustum

test-stagereferences:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagereferences.c tsfi2-deepseek/src/tsfi_stagereferences.c -o tests/test_stagereferences -lm -lrt
	./tests/test_stagereferences
	@rm -f tests/test_stagereferences

test-stagespecializes:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagespecializes.c tsfi2-deepseek/src/tsfi_stagespecializes.c -o tests/test_stagespecializes -lm -lrt
	./tests/test_stagespecializes
	@rm -f tests/test_stagespecializes

test-stagerelationship:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagerelationship.c tsfi2-deepseek/src/tsfi_stagerelationship.c -o tests/test_stagerelationship -lm -lrt
	./tests/test_stagerelationship
	@rm -f tests/test_stagerelationship

test-stagetimecodes:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_stagetimecodes.c tsfi2-deepseek/src/tsfi_stagetimecodes.c -o tests/test_stagetimecodes -lm -lrt
	./tests/test_stagetimecodes
	@rm -f tests/test_stagetimecodes

test-vactrolpickup:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_vactrolpickup.c tsfi2-deepseek/src/tsfi_vactrolpickup.c -o tests/test_vactrolpickup -lm -lrt
	./tests/test_vactrolpickup
	@rm -f tests/test_vactrolpickup

test-npnpnpvalve:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_npnpnpvalve.c tsfi2-deepseek/src/tsfi_npnpnpvalve.c -o tests/test_npnpnpvalve -lm -lrt
	./tests/test_npnpnpvalve
	@rm -f tests/test_npnpnpvalve

test-curvesbasis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_curvesbasis.c tsfi2-deepseek/src/tsfi_curvesbasis.c -o tests/test_curvesbasis -lm -lrt
	./tests/test_curvesbasis
	@rm -f tests/test_curvesbasis

test-singularityring:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_singularityring.c tsfi2-deepseek/src/tsfi_singularityring.c -o tests/test_singularityring -lm -lrt
	./tests/test_singularityring
	@rm -f tests/test_singularityring

test-subdivcurves:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_subdivcurves.c tsfi2-deepseek/src/tsfi_subdivcurves.c -o tests/test_subdivcurves -lm -lrt
	./tests/test_subdivcurves
	@rm -f tests/test_subdivcurves

test-subdivscheme:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_subdivscheme.c tsfi2-deepseek/src/tsfi_subdivscheme.c -o tests/test_subdivscheme -lm -lrt
	./tests/test_subdivscheme
	@rm -f tests/test_subdivscheme

test-curvestension:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_curvestension.c tsfi2-deepseek/src/tsfi_curvestension.c -o tests/test_curvestension -lm -lrt
	./tests/test_curvestension
	@rm -f tests/test_curvestension

test-curveswidths:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_curveswidths.c tsfi2-deepseek/src/tsfi_curveswidths.c -o tests/test_curveswidths -lm -lrt
	./tests/test_curveswidths
	@rm -f tests/test_curveswidths

test-curveswrap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_curveswrap.c tsfi2-deepseek/src/tsfi_curveswrap.c -o tests/test_curveswrap -lm -lrt
	./tests/test_curveswrap
	@rm -f tests/test_curveswrap

test-timesamples:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_timesamples.c tsfi2-deepseek/src/tsfi_timesamples.c -o tests/test_timesamples -lm -lrt
	./tests/test_timesamples
	@rm -f tests/test_timesamples

test-micropolygonmesh:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_micropolygonmesh.c tsfi2-deepseek/src/tsfi_micropolygonmesh.c -o tests/test_micropolygonmesh -lm -lrt
	./tests/test_micropolygonmesh
	@rm -f tests/test_micropolygonmesh

test-shadowmap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_shadowmap.c tsfi2-deepseek/src/tsfi_shadowmap.c -o tests/test_shadowmap -lm -lrt
	./tests/test_shadowmap
	@rm -f tests/test_shadowmap

test-quasirandomsampler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_quasirandomsampler.c tsfi2-deepseek/src/tsfi_quasirandomsampler.c -o tests/test_quasirandomsampler -lm -lrt
	./tests/test_quasirandomsampler
	@rm -f tests/test_quasirandomsampler

test-displacementshader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_displacementshader.c tsfi2-deepseek/src/tsfi_displacementshader.c -o tests/test_displacementshader -lm -lrt
	./tests/test_displacementshader
	@rm -f tests/test_displacementshader

test-texgensample:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_texgensample.c tsfi2-deepseek/src/tsfi_texgen.c -o tests/test_texgensample -lm -lrt
	./tests/test_texgensample
	@rm -f tests/test_texgensample

test-depthoffield:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_depthoffield.c tsfi2-deepseek/src/tsfi_depthoffield.c -o tests/test_depthoffield -lm -lrt
	./tests/test_depthoffield
	@rm -f tests/test_depthoffield

test-quadtreeslicer:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_quadtreeslicer.c tsfi2-deepseek/src/tsfi_quadtreeslicer.c -o tests/test_quadtreeslicer -lm -lrt
	./tests/test_quadtreeslicer
	@rm -f tests/test_quadtreeslicer

test-riinterface:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_riinterface.c tsfi2-deepseek/src/tsfi_riinterface.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_depthoffield.c tsfi2-deepseek/src/tsfi2_compiler.c tsfi2-deepseek/src/tsfi2_compiler_bin.c tsfi2-deepseek/src/tsfi2_loader.c tsfi2-deepseek/src/tsfi_mainframe_vsam.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c tsfi2-deepseek/src/tsfi_mainframe_jcl.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_riinterface -lcrypto -lm -lrt
	./tests/test_riinterface
	@rm -f tests/test_riinterface


bench-renderman:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -mavx2 -mfma -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/bench_renderman_codesign.c tsfi2-deepseek/src/tsfi_riinterface.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_depthoffield.c tsfi2-deepseek/src/tsfi_ccx_pool.c -o tests/bench_renderman_codesign -lm -lrt -lpthread
	./tests/bench_renderman_codesign
	@rm -f tests/bench_renderman_codesign


test-ccx-pool:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -mavx2 -mfma -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_ccx_pool.c tsfi2-deepseek/src/tsfi_ccx_pool.c tsfi2-deepseek/src/tsfi_depthoffield.c -o tests/test_ccx_pool -lm -lrt -lpthread
	./tests/test_ccx_pool
	@rm -f tests/test_ccx_pool







test-cinefex-adv:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_cinefex_adv.c tsfi2-deepseek/src/tsfi_fbm_landscape.c tsfi2-deepseek/src/tsfi_volumetric.c tsfi2-deepseek/src/tsfi_motionblur.c tsfi2-deepseek/src/tsfi_shadowmap.c -o tests/test_cinefex_adv -lm -lrt
	./tests/test_cinefex_adv
	@rm -f tests/test_cinefex_adv

test-sss:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_sss.c tsfi2-deepseek/src/tsfi_sss.c -o tests/test_sss -lm -lrt
	./tests/test_sss
	@rm -f tests/test_sss

test-envmap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_envmap.c tsfi2-deepseek/src/tsfi_envmap.c -o tests/test_envmap -lm -lrt
	./tests/test_envmap
	@rm -f tests/test_envmap

test-hairshading:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hairshading.c tsfi2-deepseek/src/tsfi_hairshading.c -o tests/test_hairshading -lm -lrt
	./tests/test_hairshading
	@rm -f tests/test_hairshading

test-proximity-occlusion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_proximity_occlusion.c tsfi2-deepseek/src/tsfi_proximity_occlusion.c -o tests/test_proximity_occlusion -lm -lrt
	./tests/test_proximity_occlusion
	@rm -f tests/test_proximity_occlusion

test-krowz-sickness:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_krowz_sickness.c tsfi2-deepseek/src/tsfi_krowz_sickness.c -o tests/test_krowz_sickness -lm -lrt
	./tests/test_krowz_sickness
	@rm -f tests/test_krowz_sickness

sdk-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_sdk.c tests/test_auncient_sdk_benchmarks.c -o tests/test_auncient_sdk_benchmarks -lm -lrt
	./tests/test_auncient_sdk_benchmarks

livrps-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_livrps_benchmarks.c -o tests/test_auncient_livrps_benchmarks -lm -lrt
	./tests/test_auncient_livrps_benchmarks

curves-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_curves_benchmarks.c tsfi2-deepseek/src/tsfi_curveswidths.c tsfi2-deepseek/src/tsfi_curveswrap.c -o tests/test_auncient_curves_benchmarks -lm -lrt
	./tests/test_auncient_curves_benchmarks
	@rm -f tests/test_auncient_curves_benchmarks

tpu-benchmarks: sdk-build
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_fast_inference_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_fast_inference_benchmark -lm -lrt
	./tests/test_auncient_fast_inference_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_neural_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_neural_benchmark -lm -lrt
	./tests/test_auncient_tpu_neural_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_recurrent_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_recurrent_benchmark -lm -lrt
	./tests/test_auncient_tpu_recurrent_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_polyphonic_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_polyphonic_benchmark -lm -lrt
	./tests/test_auncient_tpu_polyphonic_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_security_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_security_benchmark -lm -lrt
	./tests/test_auncient_tpu_security_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_gating_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_gating_benchmark -lm -lrt
	./tests/test_auncient_tpu_gating_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_mesh_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_mesh_benchmark -lm -lrt
	./tests/test_auncient_tpu_mesh_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_decay_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_decay_benchmark -lm -lrt
	./tests/test_auncient_tpu_decay_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_spelling_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_spelling_benchmark -lm -lrt
	./tests/test_auncient_tpu_spelling_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_tpu_bond_benchmark.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_tpu_bond_benchmark -lm -lrt
	./tests/test_auncient_tpu_bond_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_swarm_consensus.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_swarm_consensus -lm -lrt
	./tests/test_auncient_swarm_consensus
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_swarm_metaprogramming.c tsfi2-deepseek/src/auncient_sdk.o -o tests/test_auncient_swarm_metaprogramming -lm -lrt
	./tests/test_auncient_swarm_metaprogramming
	@rm -f tests/test_auncient_fast_inference_benchmark tests/test_auncient_tpu_neural_benchmark tests/test_auncient_tpu_recurrent_benchmark tests/test_auncient_tpu_polyphonic_benchmark tests/test_auncient_tpu_security_benchmark tests/test_auncient_tpu_gating_benchmark tests/test_auncient_tpu_mesh_benchmark tests/test_auncient_tpu_decay_benchmark tests/test_auncient_tpu_spelling_benchmark tests/test_auncient_tpu_bond_benchmark tests/test_auncient_swarm_consensus tests/test_auncient_swarm_metaprogramming


siggraph-projector:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_projector.c -o tests/test_auncient_siggraph_projector -lm -lrt
	./tests/test_auncient_siggraph_projector
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_renderer.c -o tests/test_auncient_siggraph_renderer -lm -lrt
	./tests/test_auncient_siggraph_renderer
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_discharge.c -o tests/test_auncient_siggraph_discharge -lm -lrt
	./tests/test_auncient_siggraph_discharge
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_presenter.c -o tests/test_auncient_siggraph_presenter -lm -lrt
	./tests/test_auncient_siggraph_presenter






test-dashboard:
	PYTHONWARNINGS=ignore python3 -m unittest tests/e2e/test_nonukes_dashboard.py
	PYTHONWARNINGS=ignore python3 -m unittest tests/e2e/test_nonukes_dashboard_spa.py
	PYTHONWARNINGS=ignore python3 -m unittest tests/e2e/test_nonukes_adversarial.py

test-container:
	PYTHONWARNINGS=ignore python3 -m unittest tests/e2e/test_dashboard_container.py

test-git-ci:
	PYTHONWARNINGS=ignore python3 tests/e2e/run_e2e_tests.py

test-unit:
	PYTHONWARNINGS=ignore python3 -m unittest discover -s tests -p "test_*.py"

sdk-build:
	mkdir -p dist
	gcc -Wall -Wextra -Werror -std=c11 -O3 -fPIC -Itsfi2-deepseek -Itsfi2-deepseek/inc -c tsfi2-deepseek/src/auncient_sdk.c -o tsfi2-deepseek/src/auncient_sdk.o
	ar rcs libauncient_sdk.a tsfi2-deepseek/src/auncient_sdk.o

sdk-minify: sdk-build
	strip --strip-unneeded tsfi2-deepseek/src/auncient_sdk.o
	ar rcs libauncient_sdk.a tsfi2-deepseek/src/auncient_sdk.o
	strip -S libauncient_sdk.a

sdk-package: sdk-minify
	tar -czf dist/auncient_sdk.tar.gz inc/auncient_sdk.h libauncient_sdk.a

test-cbt021-inmemory-pds:
	gcc -Wall -Wextra -std=c11 -O3 tests/test_cbt021_inmemory_pds.c -o tests/test_cbt021_inmemory_pds -lssl -lcrypto -lz
	./tests/test_cbt021_inmemory_pds
	@rm -f tests/test_cbt021_inmemory_pds

test-auncient-xpl-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_sdk.c tests/test_auncient_xpl_compiler.c -o tests/test_auncient_xpl_compiler -lm -lrt
	./tests/test_auncient_xpl_compiler
	@rm -f tests/test_auncient_xpl_compiler

test-auncient-xpl-loader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_sdk.c tests/test_auncient_xpl_loader.c -o tests/test_auncient_xpl_loader -lm -lrt
	./tests/test_auncient_xpl_loader
	@rm -f tests/test_auncient_xpl_loader

test-auncient-cooperative-boundary:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_cooperative_boundary.c -o tests/test_auncient_cooperative_boundary -lcrypto -lrt
	./tests/test_auncient_cooperative_boundary
	@rm -f tests/test_auncient_cooperative_boundary

test-auncient-apdl-xpl-integration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_sdk.c tests/test_auncient_apdl_xpl_integration.c -o tests/test_auncient_apdl_xpl_integration -lm -lrt
	./tests/test_auncient_apdl_xpl_integration
	@rm -f tests/test_auncient_apdl_xpl_integration

test-auncient-vfio-xplsm:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_vfio_xplsm.c -o tests/test_auncient_vfio_xplsm -lcrypto -lrt
	./tests/test_auncient_vfio_xplsm
	@rm -f tests/test_auncient_vfio_xplsm

test-auncient-network-skeleton:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_network_skeleton.c -o tests/test_auncient_network_skeleton -lrt
	./tests/test_auncient_network_skeleton
	@rm -f tests/test_auncient_network_skeleton

test-auncient-xcom-winchester:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_xcom_winchester.c -o tests/test_auncient_xcom_winchester -lrt
	./tests/test_auncient_xcom_winchester
	@rm -f tests/test_auncient_xcom_winchester

test-auncient-xpl-net-stack:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_xpl_net_stack.c -o tests/test_auncient_xpl_net_stack -lcrypto -lrt
	./tests/test_auncient_xpl_net_stack
	@rm -f tests/test_auncient_xpl_net_stack

test-auncient-xplos-kernel:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_xplos_kernel.c -o tests/test_auncient_xplos_kernel -lcrypto -lrt
	./tests/test_auncient_xplos_kernel
	@rm -f tests/test_auncient_xplos_kernel

test-auncient-xplos-scheduler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_xplos_scheduler.c -o tests/test_auncient_xplos_scheduler -lrt
	./tests/test_auncient_xplos_scheduler
	@rm -f tests/test_auncient_xplos_scheduler

test-auncient-xplos-debugger:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_xplos_debugger.c -o tests/test_auncient_xplos_debugger -lcrypto -lrt
	./tests/test_auncient_xplos_debugger
	@rm -f tests/test_auncient_xplos_debugger

test-auncient-xplos-debugger-extended:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_xplos_debugger_extended.c -o tests/test_auncient_xplos_debugger_extended -lcrypto -lrt
	./tests/test_auncient_xplos_debugger_extended
	@rm -f tests/test_auncient_xplos_debugger_extended

test-auncient-apdl-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_apdl_compiler.c -o tests/test_auncient_apdl_compiler -lrt
	./tests/test_auncient_apdl_compiler
	@rm -f tests/test_auncient_apdl_compiler

test-auncient-teddy-personality:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_teddy_personality.c tsfi2-deepseek/src/auncient_teddy_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_montecarlo.c tsfi2-deepseek/src/tsfi_riinterface.c tsfi2-deepseek/src/auncient_sdk.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_depthoffield.c tsfi2-deepseek/src/tsfi_ccx_pool.c -o tests/test_auncient_teddy_personality -lm -lrt
	./tests/test_auncient_teddy_personality
	@rm -f tests/test_auncient_teddy_personality

test-auncient-bear-interactions:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_bear_interactions.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o tests/test_auncient_bear_interactions -lrt -lm
	./tests/test_auncient_bear_interactions
	@rm -f tests/test_auncient_bear_interactions

test-auncient-bear-chorus:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_bear_chorus.c tsfi2-deepseek/src/auncient_teddy_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o tsfi2-deepseek/src/auncient_bear_chorus -lm -lrt
	./tsfi2-deepseek/src/auncient_bear_chorus
	@rm -f tsfi2-deepseek/src/auncient_bear_chorus

test-auncient-speech-classification:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_speech_classification.c tsfi2-deepseek/src/auncient_teddy_personality.c tsfi2-deepseek/src/tsfi_speech_synth.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o tests/test_auncient_speech_classification -lm -lrt
	./tests/test_auncient_speech_classification
	@rm -f tests/test_auncient_speech_classification


test-auncient-hathitrust-stimuli:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_hathitrust_stimuli.c tsfi2-deepseek/src/tsfi_quadtree_ksds.c tsfi2-deepseek/src/tsfi_mainframe_ebcdic.c scripts/libantigravity_extra.c scripts/libantigravity_interop.c scripts/abi_dispatch_map.c -o tests/test_auncient_hathitrust_stimuli -lcrypto -lm -lrt
	./tests/test_auncient_hathitrust_stimuli
	@rm -f tests/test_auncient_hathitrust_stimuli

test-auncient-avatar-reflexes:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_avatar_reflexes.c -o tests/test_auncient_avatar_reflexes -lrt
	./tests/test_auncient_avatar_reflexes
	@rm -f tests/test_auncient_avatar_reflexes
