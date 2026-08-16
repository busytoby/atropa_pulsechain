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
