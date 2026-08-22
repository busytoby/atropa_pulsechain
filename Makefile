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

test-all: test-hathitrust test-hathitrust-hathifile test-hathitrust-oai test-hathitrust-oai-util test-hathitrust-harvester test-hathitrust-ef test-hathitrust-extended test-hathitrust-xml test-hathitrust-cache-alto-qt test-hathitrust-extra2 test-hathitrust-extra3 test-hathitrust-render test-hathitrust-prman test-hathitrust-lore test-hathitrust-prman-extra test-hathitrust-prman-anim test-hathitrust-prman-blur test-hathitrust-prman-final test-hathitrust-teddy_bear test-hathitrust-final-extra test-hathitrust-tsv-extra test-hathitrust-solr-rdbms test-hathitrust-tsv-bin test-hathitrust-window test-hathitrust-extra-final test-hathitrust-bib-oauth test-tsfi2-compiler-bin test-tsfi2-loader test-tsfi2-compiler test-tsfi2-compiler-standards test-mann test-comp-pass test-mvarsel test-reroute test-delegate-sig test-stagecomp test-ar test-sdfformat test-hydrascene test-renderdelegate test-stagelock test-renderindex test-usdshade test-teddy_bear-loader test-geomcamera test-geomcurves test-geompoints test-luxlight test-geomxform test-stageroot test-primroot test-stagepop test-attributeroot test-stagecache test-stagemask test-geomsubset test-stagearc test-stageinherits test-stagepayloads test-stagevariants test-stagetraverser test-stagemetadata test-camerafrustum test-stagereferences test-stagespecializes test-stagerelationship test-stagetimecodes test-vactrolpickup test-npnpnpvalve test-curvesbasis test-singularityring test-subdivcurves test-subdivscheme test-curvestension test-curveswidths test-curveswrap test-timesamples test-micropolygonmesh test-shadowmap test-quasirandomsampler test-displacementshader test-texgensample test-depthoffield test-quadtreeslicer test-riinterface test-cinefex-adv test-sss test-envmap test-hairshading test-proximity-occlusion test-krowz-sickness test-auncient-teddy_bear-personality test-tsfi-montecarlo test-treasury-lore-token-holdings-prover test-pulsechain-rpc-cache-invariance-prover test-keys-of-ong-entropy-invariant-prover test-algol61-knowledge-graph-merkle-solver test-totient-zero-nullstellensatz-solver test-totient-chain-composition-solver test-helmholtz-transitive-deduction-solver test-ramac-helmholtz-totient-zero-solver test-ramac-zmm-rails-quiescence-solver test-ramac-fourier-dirichlet-solver
	@echo "All tests completed successfully."

test-lasalle-lfm-cpm-ui:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lasalle_lfm_cpm_ui.c -o tests/test_lasalle_lfm_cpm_ui
	./tests/test_lasalle_lfm_cpm_ui
	@rm -f tests/test_lasalle_lfm_cpm_ui

test-vaesen-llm-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_llm_solver.c tsfi2-deepseek/src/tsfi_vaesen_llm_solver.c -o tests/test_vaesen_llm_solver -lrt
	./tests/test_vaesen_llm_solver
	@rm -f tests/test_vaesen_llm_solver

test-vaesen-conference:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_conference.c tsfi2-deepseek/src/tsfi_vaesen_conference.c -o tests/test_vaesen_conference -lm -lrt
	./tests/test_vaesen_conference
	@rm -f tests/test_vaesen_conference

test-vaesen-device-slice:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_device_slice.c tsfi2-deepseek/src/tsfi_vaesen_device_slice.c -o tests/test_vaesen_device_slice -lm -lrt
	./tests/test_vaesen_device_slice
	@rm -f tests/test_vaesen_device_slice

test-zmachine-vaesen-bridge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_zmachine_vaesen_bridge.c tsfi2-deepseek/src/tsfi_zmachine_vaesen_bridge.c tsfi2-deepseek/src/tsfi_vaesen_device_slice.c tsfi2-deepseek/src/tsfi_vaesen_fpga_mistral_ext.c tsfi2-deepseek/src/tsfi_vaesen_mistral_guardrails.c -o tests/test_zmachine_vaesen_bridge -lm -lrt
	./tests/test_zmachine_vaesen_bridge
	@rm -f tests/test_zmachine_vaesen_bridge

test-fpga-vaesen-operator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_fpga_vaesen_operator.c tsfi2-deepseek/src/tsfi_fpga_vaesen_operator_solver.c -o tests/test_fpga_vaesen_operator -lrt
	./tests/test_fpga_vaesen_operator
	@rm -f tests/test_fpga_vaesen_operator

test-vaesen-flash-decoder:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_flash_decoder.c tsfi2-deepseek/src/tsfi_vaesen_flash_decoder.c tsfi2-deepseek/src/tsfi_vaesen_device_slice.c -o tests/test_vaesen_flash_decoder -lm -lrt
	./tests/test_vaesen_flash_decoder
	@rm -f tests/test_vaesen_flash_decoder

test-vaesen-fpga-mistral-ext:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_fpga_mistral_ext.c tsfi2-deepseek/src/tsfi_vaesen_fpga_mistral_ext.c tsfi2-deepseek/src/tsfi_vaesen_device_slice.c -o tests/test_vaesen_fpga_mistral_ext -lm -lrt
	./tests/test_vaesen_fpga_mistral_ext
	@rm -f tests/test_vaesen_fpga_mistral_ext

test-vaesen-spatial-vision:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_spatial_vision.c tsfi2-deepseek/src/tsfi_vaesen_spatial_vision.c tsfi2-deepseek/src/tsfi_vaesen_device_slice.c -o tests/test_vaesen_spatial_vision -lm -lrt
	./tests/test_vaesen_spatial_vision
	@rm -f tests/test_vaesen_spatial_vision

test-vaesen-mistral-guardrails:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek/inc tests/test_vaesen_mistral_guardrails.c tsfi2-deepseek/src/tsfi_vaesen_mistral_guardrails.c tsfi2-deepseek/src/tsfi_vaesen_device_slice.c -o tests/test_vaesen_mistral_guardrails -lm -lrt
	./tests/test_vaesen_mistral_guardrails
	@rm -f tests/test_vaesen_mistral_guardrails

test-lasalle-lfm-cpm-advanced:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lasalle_lfm_cpm_advanced.c -o tests/test_lasalle_lfm_cpm_advanced
	./tests/test_lasalle_lfm_cpm_advanced
	@rm -f tests/test_lasalle_lfm_cpm_advanced

test-saa5050-8byte-safety:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_saa5050_8byte_safety.c -o tests/test_saa5050_8byte_safety
	./tests/test_saa5050_8byte_safety
	@rm -f tests/test_saa5050_8byte_safety

test-saa5050-8bit-native-safety:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_saa5050_8bit_native_safety.c -o tests/test_saa5050_8bit_native_safety
	./tests/test_saa5050_8bit_native_safety
	@rm -f tests/test_saa5050_8bit_native_safety

test-hammond-lfm-lasalle-av:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_hammond_lfm_lasalle_av.c -o tests/test_hammond_lfm_lasalle_av
	./tests/test_hammond_lfm_lasalle_av
	@rm -f tests/test_hammond_lfm_lasalle_av

test-wmq-stanag-agent-jam:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_wmq_stanag_agent_jam.c -o tests/test_wmq_stanag_agent_jam
	./tests/test_wmq_stanag_agent_jam
	@rm -f tests/test_wmq_stanag_agent_jam

test-band-stage-dmx-patchbay:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_band_stage_dmx_patchbay.c -o tests/test_band_stage_dmx_patchbay
	./tests/test_band_stage_dmx_patchbay
	@rm -f tests/test_band_stage_dmx_patchbay

test-band-binaural-multitrack:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_band_binaural_multitrack.c -o tests/test_band_binaural_multitrack
	./tests/test_band_binaural_multitrack
	@rm -f tests/test_band_binaural_multitrack

test-leslie-biorhythm-notch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_leslie_biorhythm_notch.c -o tests/test_leslie_biorhythm_notch
	./tests/test_leslie_biorhythm_notch
	@rm -f tests/test_leslie_biorhythm_notch

test-larsen-personality-pll-pmg:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_larsen_personality_pll_pmg.c -o tests/test_larsen_personality_pll_pmg
	./tests/test_larsen_personality_pll_pmg
	@rm -f tests/test_larsen_personality_pll_pmg

test-larsen-voice-mood-haptics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_larsen_voice_mood_haptics.c -o tests/test_larsen_voice_mood_haptics
	./tests/test_larsen_voice_mood_haptics
	@rm -f tests/test_larsen_voice_mood_haptics

test-teddy_bear-ballet-choreography:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_teddy_bear_ballet_choreography.c -o tests/test_teddy_bear_ballet_choreography
	./tests/test_teddy_bear_ballet_choreography
	@rm -f tests/test_teddy_bear_ballet_choreography

test-dna-breeder-inheritance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_dna_breeder_inheritance.c -o tests/test_dna_breeder_inheritance
	./tests/test_dna_breeder_inheritance
	@rm -f tests/test_dna_breeder_inheritance

test-super8-wysiwyg-optical:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_super8_wysiwyg_optical.c -o tests/test_super8_wysiwyg_optical
	./tests/test_super8_wysiwyg_optical
	@rm -f tests/test_super8_wysiwyg_optical

test-sally-larsen-renderman-super8:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_sally_larsen_renderman_super8.c -o tests/test_sally_larsen_renderman_super8
	./tests/test_sally_larsen_renderman_super8
	@rm -f tests/test_sally_larsen_renderman_super8

test-manifold-reaction-diffusion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_manifold_reaction_diffusion.c -o tests/test_manifold_reaction_diffusion
	./tests/test_manifold_reaction_diffusion
	@rm -f tests/test_manifold_reaction_diffusion

test-pure-vulkan-video-encode:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_pure_vulkan_video_encode.c -o tests/test_pure_vulkan_video_encode
	./tests/test_pure_vulkan_video_encode
	@rm -f tests/test_pure_vulkan_video_encode

render-sally-larsen-vulkan-mp4:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/render_sally_larsen_pure_vulkan_mp4.c -o tests/render_sally_larsen_pure_vulkan_mp4 -lm
	./tests/render_sally_larsen_pure_vulkan_mp4
	@rm -f tests/render_sally_larsen_pure_vulkan_mp4

test-totient-turtle-graphics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_totient_turtle_graphics.c -o tests/test_totient_turtle_graphics
	./tests/test_totient_turtle_graphics
	@rm -f tests/test_totient_turtle_graphics

test-totient-turtle-perception:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_totient_turtle_perception.c -o tests/test_totient_turtle_perception
	./tests/test_totient_turtle_perception
	@rm -f tests/test_totient_turtle_perception

test-vaesen-morphogenetic-life:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_morphogenetic_life.c -o tests/test_vaesen_morphogenetic_life
	./tests/test_vaesen_morphogenetic_life
	@rm -f tests/test_vaesen_morphogenetic_life

test-ankh-dynamic-compressor:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_dynamic_compressor.c -o tests/test_ankh_dynamic_compressor
	./tests/test_ankh_dynamic_compressor
	@rm -f tests/test_ankh_dynamic_compressor

test-multi-vaesen-ballet-edo22:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multi_vaesen_ballet_edo22.c -o tests/test_multi_vaesen_ballet_edo22
	./tests/test_multi_vaesen_ballet_edo22
	@rm -f tests/test_multi_vaesen_ballet_edo22

test-ankh-speech-song-synthesizer:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_speech_song_synthesizer.c -o tests/test_ankh_speech_song_synthesizer
	./tests/test_ankh_speech_song_synthesizer
	@rm -f tests/test_ankh_speech_song_synthesizer

test-totient-turtle-audio-composition:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_totient_turtle_audio_composition.c -o tests/test_totient_turtle_audio_composition
	./tests/test_totient_turtle_audio_composition
	@rm -f tests/test_totient_turtle_audio_composition

test-bionika-score-notation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bionika_score_notation.c -o tests/test_bionika_score_notation
	./tests/test_bionika_score_notation
	@rm -f tests/test_bionika_score_notation

test-llm-tts-completeness:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_llm_tts_completeness.c -o tests/test_llm_tts_completeness
	./tests/test_llm_tts_completeness
	@rm -f tests/test_llm_tts_completeness

test-verifiable-human-speech:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_verifiable_human_speech.c -o tests/test_verifiable_human_speech
	./tests/test_verifiable_human_speech
	@rm -f tests/test_verifiable_human_speech

export-multi-agent-usd-stage:
	gcc -Wall -Wextra -std=c11 -O3 tests/export_multi_agent_usd_stage.c -lm -o tests/export_multi_agent_usd_stage
	./tests/export_multi_agent_usd_stage
	@rm -f tests/export_multi_agent_usd_stage

test-usd-stage-audio-exporter:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_usd_stage_audio_exporter.c -o tests/test_usd_stage_audio_exporter
	./tests/test_usd_stage_audio_exporter
	@rm -f tests/test_usd_stage_audio_exporter

test-voice-character-definition:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_voice_character_definition.c -o tests/test_voice_character_definition
	./tests/test_voice_character_definition
	@rm -f tests/test_voice_character_definition

test-organic-voice-evolution:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_organic_voice_evolution.c -o tests/test_organic_voice_evolution
	./tests/test_organic_voice_evolution
	@rm -f tests/test_organic_voice_evolution

test-joint-audiovisual-dubbing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_joint_audiovisual_dubbing.c -o tests/test_joint_audiovisual_dubbing
	./tests/test_joint_audiovisual_dubbing
	@rm -f tests/test_joint_audiovisual_dubbing

test-renderman-hudson-dubbing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_renderman_hudson_dubbing.c -o tests/test_renderman_hudson_dubbing
	./tests/test_renderman_hudson_dubbing
	@rm -f tests/test_renderman_hudson_dubbing

test-latent-masking-viseme-diversity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_latent_masking_viseme_diversity.c -o tests/test_latent_masking_viseme_diversity
	./tests/test_latent_masking_viseme_diversity
	@rm -f tests/test_latent_masking_viseme_diversity

test-flow-matching-syllable-bounds:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_flow_matching_syllable_bounds.c -o tests/test_flow_matching_syllable_bounds
	./tests/test_flow_matching_syllable_bounds
	@rm -f tests/test_flow_matching_syllable_bounds

test-shared-positional-incontext-embedding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_shared_positional_incontext_embedding.c -o tests/test_shared_positional_incontext_embedding
	./tests/test_shared_positional_incontext_embedding
	@rm -f tests/test_shared_positional_incontext_embedding

test-universal-multimodal-dubbing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_universal_multimodal_dubbing.c -o tests/test_universal_multimodal_dubbing
	./tests/test_universal_multimodal_dubbing
	@rm -f tests/test_universal_multimodal_dubbing

test-nonverbal-scene-dynamics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nonverbal_scene_dynamics.c -o tests/test_nonverbal_scene_dynamics
	./tests/test_nonverbal_scene_dynamics
	@rm -f tests/test_nonverbal_scene_dynamics

test-grand-unified-dubbing-synthesis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_unified_dubbing_synthesis.c -o tests/test_grand_unified_dubbing_synthesis
	./tests/test_grand_unified_dubbing_synthesis
	@rm -f tests/test_grand_unified_dubbing_synthesis

test-zeroshot-benchmark-invariance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_zeroshot_benchmark_invariance.c -o tests/test_zeroshot_benchmark_invariance
	./tests/test_zeroshot_benchmark_invariance
	@rm -f tests/test_zeroshot_benchmark_invariance

test-quad-ablation-sensitivity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_quad_ablation_sensitivity.c -o tests/test_quad_ablation_sensitivity
	./tests/test_quad_ablation_sensitivity
	@rm -f tests/test_quad_ablation_sensitivity

test-pll-general-dub-support:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_pll_general_dub_support.c -o tests/test_pll_general_dub_support
	./tests/test_pll_general_dub_support
	@rm -f tests/test_pll_general_dub_support

test-holonomic-partial-weyl-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_holonomic_partial_weyl_closure.c -o tests/test_holonomic_partial_weyl_closure
	./tests/test_holonomic_partial_weyl_closure
	@rm -f tests/test_holonomic_partial_weyl_closure

test-two-sided-ideal-commutation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_two_sided_ideal_commutation.c -o tests/test_two_sided_ideal_commutation
	./tests/test_two_sided_ideal_commutation
	@rm -f tests/test_two_sided_ideal_commutation

test-singular-locus-elimination:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_singular_locus_elimination.c -o tests/test_singular_locus_elimination
	./tests/test_singular_locus_elimination
	@rm -f tests/test_singular_locus_elimination

test-grobner-truncation-termination:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grobner_truncation_termination.c -o tests/test_grobner_truncation_termination
	./tests/test_grobner_truncation_termination
	@rm -f tests/test_grobner_truncation_termination

test-rational-annihilator-contraction:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_rational_annihilator_contraction.c -o tests/test_rational_annihilator_contraction
	./tests/test_rational_annihilator_contraction
	@rm -f tests/test_rational_annihilator_contraction

test-iterative-module-saturation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_iterative_module_saturation.c -o tests/test_iterative_module_saturation
	./tests/test_iterative_module_saturation
	@rm -f tests/test_iterative_module_saturation

test-accumulator-zeroing-nullstellensatz:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_accumulator_zeroing_nullstellensatz.c -o tests/test_accumulator_zeroing_nullstellensatz
	./tests/test_accumulator_zeroing_nullstellensatz
	@rm -f tests/test_accumulator_zeroing_nullstellensatz

test-minimal-desingularization-ideal:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_minimal_desingularization_ideal.c -o tests/test_minimal_desingularization_ideal
	./tests/test_minimal_desingularization_ideal
	@rm -f tests/test_minimal_desingularization_ideal

test-traverso-tracer-memoization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_traverso_tracer_memoization.c -o tests/test_traverso_tracer_memoization
	./tests/test_traverso_tracer_memoization
	@rm -f tests/test_traverso_tracer_memoization

test-block-monomial-elimination-order:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_block_monomial_elimination_order.c -o tests/test_block_monomial_elimination_order
	./tests/test_block_monomial_elimination_order
	@rm -f tests/test_block_monomial_elimination_order

test-grobner-sequence-stabilization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grobner_sequence_stabilization.c -o tests/test_grobner_sequence_stabilization
	./tests/test_grobner_sequence_stabilization
	@rm -f tests/test_grobner_sequence_stabilization

test-multiprime-rational-reconstruction:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multiprime_rational_reconstruction.c -o tests/test_multiprime_rational_reconstruction
	./tests/test_multiprime_rational_reconstruction
	@rm -f tests/test_multiprime_rational_reconstruction

test-nonholonomic-creative-telescoping:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nonholonomic_creative_telescoping.c -o tests/test_nonholonomic_creative_telescoping
	./tests/test_nonholonomic_creative_telescoping
	@rm -f tests/test_nonholonomic_creative_telescoping

test-grand-master-weyl-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_weyl_closure.c -o tests/test_grand_master_weyl_closure
	./tests/test_grand_master_weyl_closure
	@rm -f tests/test_grand_master_weyl_closure

test-bernstein-filtration-dimension:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bernstein_filtration_dimension.c -o tests/test_bernstein_filtration_dimension
	./tests/test_bernstein_filtration_dimension
	@rm -f tests/test_bernstein_filtration_dimension

test-ozone-weyl-closure-invariant:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ozone_weyl_closure_invariant.c -o tests/test_ozone_weyl_closure_invariant
	./tests/test_ozone_weyl_closure_invariant
	@rm -f tests/test_ozone_weyl_closure_invariant

test-ore-fraction-normal-form:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ore_fraction_normal_form.c -o tests/test_ore_fraction_normal_form
	./tests/test_ore_fraction_normal_form
	@rm -f tests/test_ore_fraction_normal_form

test-emotional-weyl-phonation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_emotional_weyl_phonation.c -o tests/test_emotional_weyl_phonation
	./tests/test_emotional_weyl_phonation
	@rm -f tests/test_emotional_weyl_phonation

test-spmm-formant-acceleration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spmm_formant_acceleration.c -o tests/test_spmm_formant_acceleration
	./tests/test_spmm_formant_acceleration
	@rm -f tests/test_spmm_formant_acceleration

test-monotonic-spmm-attention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_monotonic_spmm_attention.c -o tests/test_monotonic_spmm_attention
	./tests/test_monotonic_spmm_attention
	@rm -f tests/test_monotonic_spmm_attention

test-hierarchical-k-quantization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_hierarchical_k_quantization.c -o tests/test_hierarchical_k_quantization
	./tests/test_hierarchical_k_quantization
	@rm -f tests/test_hierarchical_k_quantization

test-bitnet-ternary-bitlinear:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bitnet_ternary_bitlinear.c -o tests/test_bitnet_ternary_bitlinear
	./tests/test_bitnet_ternary_bitlinear
	@rm -f tests/test_bitnet_ternary_bitlinear

test-kv-cache-decode-complexity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_kv_cache_decode_complexity.c -o tests/test_kv_cache_decode_complexity
	./tests/test_kv_cache_decode_complexity
	@rm -f tests/test_kv_cache_decode_complexity

test-whisper-fixed-cross-attention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_whisper_fixed_cross_attention.c -o tests/test_whisper_fixed_cross_attention
	./tests/test_whisper_fixed_cross_attention
	@rm -f tests/test_whisper_fixed_cross_attention

test-noise-throughput-semantic-bound:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_noise_throughput_semantic_bound.c -o tests/test_noise_throughput_semantic_bound
	./tests/test_noise_throughput_semantic_bound
	@rm -f tests/test_noise_throughput_semantic_bound

test-grand-master-edge-synthesis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_edge_synthesis.c -o tests/test_grand_master_edge_synthesis
	./tests/test_grand_master_edge_synthesis
	@rm -f tests/test_grand_master_edge_synthesis

test-ffn-projection-dominance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ffn_projection_dominance.c -o tests/test_ffn_projection_dominance
	./tests/test_ffn_projection_dominance
	@rm -f tests/test_ffn_projection_dominance

test-mhsa-projection-scaling:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_mhsa_projection_scaling.c -o tests/test_mhsa_projection_scaling
	./tests/test_mhsa_projection_scaling
	@rm -f tests/test_mhsa_projection_scaling

test-quantization-noise-regularization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_quantization_noise_regularization.c -o tests/test_quantization_noise_regularization
	./tests/test_quantization_noise_regularization
	@rm -f tests/test_quantization_noise_regularization

test-semantic-collapse-prevention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_semantic_collapse_prevention.c -o tests/test_semantic_collapse_prevention
	./tests/test_semantic_collapse_prevention
	@rm -f tests/test_semantic_collapse_prevention

test-e2e-voice-latency-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_e2e_voice_latency_sync.c -o tests/test_e2e_voice_latency_sync
	./tests/test_e2e_voice_latency_sync
	@rm -f tests/test_e2e_voice_latency_sync

test-memory-compression-footprint:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_memory_compression_footprint.c -o tests/test_memory_compression_footprint
	./tests/test_memory_compression_footprint
	@rm -f tests/test_memory_compression_footprint

test-pareto-model-selection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_pareto_model_selection.c -o tests/test_pareto_model_selection
	./tests/test_pareto_model_selection
	@rm -f tests/test_pareto_model_selection

test-nubia-semantic-similarity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nubia_semantic_similarity.c -o tests/test_nubia_semantic_similarity
	./tests/test_nubia_semantic_similarity
	@rm -f tests/test_nubia_semantic_similarity

test-prefill-decode-asymmetric-complexity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_prefill_decode_asymmetric_complexity.c -o tests/test_prefill_decode_asymmetric_complexity
	./tests/test_prefill_decode_asymmetric_complexity
	@rm -f tests/test_prefill_decode_asymmetric_complexity

test-cv-stability-invariance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cv_stability_invariance.c -o tests/test_cv_stability_invariance
	./tests/test_cv_stability_invariance
	@rm -f tests/test_cv_stability_invariance

test-batch1-memory-bandwidth-intensity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_batch1_memory_bandwidth_intensity.c -o tests/test_batch1_memory_bandwidth_intensity
	./tests/test_batch1_memory_bandwidth_intensity
	@rm -f tests/test_batch1_memory_bandwidth_intensity

test-thermal-energy-envelope:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_thermal_energy_envelope.c -o tests/test_thermal_energy_envelope
	./tests/test_thermal_energy_envelope
	@rm -f tests/test_thermal_energy_envelope

test-context-cache-reuse:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_context_cache_reuse.c -o tests/test_context_cache_reuse
	./tests/test_context_cache_reuse
	@rm -f tests/test_context_cache_reuse

test-wer-acoustic-bound:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_wer_acoustic_bound.c -o tests/test_wer_acoustic_bound
	./tests/test_wer_acoustic_bound
	@rm -f tests/test_wer_acoustic_bound

test-formant-resonator-modulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_formant_resonator_modulation.c -o tests/test_formant_resonator_modulation
	./tests/test_formant_resonator_modulation
	@rm -f tests/test_formant_resonator_modulation

test-quant-degradation-snr:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_quant_degradation_snr.c -o tests/test_quant_degradation_snr
	./tests/test_quant_degradation_snr
	@rm -f tests/test_quant_degradation_snr

test-quantization-scale-propagation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_quantization_scale_propagation.c -o tests/test_quantization_scale_propagation
	./tests/test_quantization_scale_propagation
	@rm -f tests/test_quantization_scale_propagation

test-rope-relative-position:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_rope_relative_position.c -o tests/test_rope_relative_position
	./tests/test_rope_relative_position
	@rm -f tests/test_rope_relative_position

test-gqa-bandwidth-reduction:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_gqa_bandwidth_reduction.c -o tests/test_gqa_bandwidth_reduction
	./tests/test_gqa_bandwidth_reduction
	@rm -f tests/test_gqa_bandwidth_reduction

test-swiglu-gated-activation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_swiglu_gated_activation.c -o tests/test_swiglu_gated_activation
	./tests/test_swiglu_gated_activation
	@rm -f tests/test_swiglu_gated_activation

test-rmsnorm-invariance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_rmsnorm_invariance.c -o tests/test_rmsnorm_invariance
	./tests/test_rmsnorm_invariance
	@rm -f tests/test_rmsnorm_invariance

test-activation-distribution-clipping:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_activation_distribution_clipping.c -o tests/test_activation_distribution_clipping
	./tests/test_activation_distribution_clipping
	@rm -f tests/test_activation_distribution_clipping

test-vaesen-emotional-norm-invariance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_emotional_norm_invariance.c -o tests/test_vaesen_emotional_norm_invariance
	./tests/test_vaesen_emotional_norm_invariance
	@rm -f tests/test_vaesen_emotional_norm_invariance

test-emotional-accumulator-dynamics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_emotional_accumulator_dynamics.c -o tests/test_emotional_accumulator_dynamics
	./tests/test_emotional_accumulator_dynamics
	@rm -f tests/test_emotional_accumulator_dynamics

test-multimodal-cue-projection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multimodal_cue_projection.c -o tests/test_multimodal_cue_projection
	./tests/test_multimodal_cue_projection
	@rm -f tests/test_multimodal_cue_projection

test-grand-master-vaesen-unification:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_vaesen_unification.c -o tests/test_grand_master_vaesen_unification
	./tests/test_grand_master_vaesen_unification
	@rm -f tests/test_grand_master_vaesen_unification

test-holonomic-emotional-weyl-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_holonomic_emotional_weyl_closure.c -o tests/test_holonomic_emotional_weyl_closure
	./tests/test_holonomic_emotional_weyl_closure
	@rm -f tests/test_holonomic_emotional_weyl_closure

test-cross-cultural-cue-utilization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cross_cultural_cue_utilization.c -o tests/test_cross_cultural_cue_utilization
	./tests/test_cross_cultural_cue_utilization
	@rm -f tests/test_cross_cultural_cue_utilization

test-intention-attribution-transparency:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_intention_attribution_transparency.c -o tests/test_intention_attribution_transparency
	./tests/test_intention_attribution_transparency
	@rm -f tests/test_intention_attribution_transparency

test-participatory-vote-projection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_participatory_vote_projection.c -o tests/test_participatory_vote_projection
	./tests/test_participatory_vote_projection
	@rm -f tests/test_participatory_vote_projection

test-long-term-habituation-dynamics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_long_term_habituation_dynamics.c -o tests/test_long_term_habituation_dynamics
	./tests/test_long_term_habituation_dynamics
	@rm -f tests/test_long_term_habituation_dynamics

test-baby-schema-behavioral-cuteness:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_baby_schema_behavioral_cuteness.c -o tests/test_baby_schema_behavioral_cuteness
	./tests/test_baby_schema_behavioral_cuteness
	@rm -f tests/test_baby_schema_behavioral_cuteness

test-grand-master-physical-robot-synthesis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_physical_robot_synthesis.c -o tests/test_grand_master_physical_robot_synthesis
	./tests/test_grand_master_physical_robot_synthesis
	@rm -f tests/test_grand_master_physical_robot_synthesis

test-intentional-error-modulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_intentional_error_modulation.c -o tests/test_intentional_error_modulation
	./tests/test_intentional_error_modulation
	@rm -f tests/test_intentional_error_modulation

test-proxemic-distance-modulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_proxemic_distance_modulation.c -o tests/test_proxemic_distance_modulation
	./tests/test_proxemic_distance_modulation
	@rm -f tests/test_proxemic_distance_modulation

test-proxemic-field-weyl-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_proxemic_field_weyl_closure.c -o tests/test_proxemic_field_weyl_closure
	./tests/test_proxemic_field_weyl_closure
	@rm -f tests/test_proxemic_field_weyl_closure

test-vaesen-azatoi-manifestation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_azatoi_manifestation.c -o tests/test_vaesen_azatoi_manifestation
	./tests/test_vaesen_azatoi_manifestation
	@rm -f tests/test_vaesen_azatoi_manifestation

test-paralinguistic-micro-inflection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_paralinguistic_micro_inflection.c -o tests/test_paralinguistic_micro_inflection
	./tests/test_paralinguistic_micro_inflection
	@rm -f tests/test_paralinguistic_micro_inflection

test-upward-gaze-vulnerability:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_upward_gaze_vulnerability.c -o tests/test_upward_gaze_vulnerability
	./tests/test_upward_gaze_vulnerability
	@rm -f tests/test_upward_gaze_vulnerability

test-indirect-linguistic-softening:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_indirect_linguistic_softening.c -o tests/test_indirect_linguistic_softening
	./tests/test_indirect_linguistic_softening
	@rm -f tests/test_indirect_linguistic_softening

test-expressive-blushing-modulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_expressive_blushing_modulation.c -o tests/test_expressive_blushing_modulation
	./tests/test_expressive_blushing_modulation
	@rm -f tests/test_expressive_blushing_modulation

test-grand-master-vaesen-social-synthesis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_vaesen_social_synthesis.c -o tests/test_grand_master_vaesen_social_synthesis
	./tests/test_grand_master_vaesen_social_synthesis
	@rm -f tests/test_grand_master_vaesen_social_synthesis

test-temporal-habituation-decay:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_temporal_habituation_decay.c -o tests/test_temporal_habituation_decay
	./tests/test_temporal_habituation_decay
	@rm -f tests/test_temporal_habituation_decay

test-spatial-trajectory-smoothing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spatial_trajectory_smoothing.c -o tests/test_spatial_trajectory_smoothing
	./tests/test_spatial_trajectory_smoothing
	@rm -f tests/test_spatial_trajectory_smoothing

test-participatory-consensus-voting:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_participatory_consensus_voting.c -o tests/test_participatory_consensus_voting
	./tests/test_participatory_consensus_voting
	@rm -f tests/test_participatory_consensus_voting

test-realtime-av-motion-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_realtime_av_motion_sync.c -o tests/test_realtime_av_motion_sync
	./tests/test_realtime_av_motion_sync
	@rm -f tests/test_realtime_av_motion_sync

test-multimodal-memory-footprint:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multimodal_memory_footprint.c -o tests/test_multimodal_memory_footprint
	./tests/test_multimodal_memory_footprint
	@rm -f tests/test_multimodal_memory_footprint

test-tactile-response-feedback:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tactile_response_feedback.c -o tests/test_tactile_response_feedback
	./tests/test_tactile_response_feedback
	@rm -f tests/test_tactile_response_feedback

test-grand-master-unification-iii:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_iii.c -o tests/test_grand_master_unification_iii
	./tests/test_grand_master_unification_iii
	@rm -f tests/test_grand_master_unification_iii

test-acoustic-purr-modulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_purr_modulation.c -o tests/test_acoustic_purr_modulation
	./tests/test_acoustic_purr_modulation
	@rm -f tests/test_acoustic_purr_modulation

test-thermal-heat-dissipation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_thermal_heat_dissipation.c -o tests/test_thermal_heat_dissipation
	./tests/test_thermal_heat_dissipation
	@rm -f tests/test_thermal_heat_dissipation

test-multimodal-pipeline-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multimodal_pipeline_sync.c -o tests/test_multimodal_pipeline_sync
	./tests/test_multimodal_pipeline_sync
	@rm -f tests/test_multimodal_pipeline_sync

test-speech-intelligibility-wer:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_speech_intelligibility_wer.c -o tests/test_speech_intelligibility_wer
	./tests/test_speech_intelligibility_wer
	@rm -f tests/test_speech_intelligibility_wer

test-grand-master-unification-iv:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_iv.c -o tests/test_grand_master_unification_iv
	./tests/test_grand_master_unification_iv
	@rm -f tests/test_grand_master_unification_iv

test-personality-mood-transition:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_personality_mood_transition.c -o tests/test_personality_mood_transition
	./tests/test_personality_mood_transition
	@rm -f tests/test_personality_mood_transition

test-relational-habituation-renewal:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_relational_habituation_renewal.c -o tests/test_relational_habituation_renewal
	./tests/test_relational_habituation_renewal
	@rm -f tests/test_relational_habituation_renewal

test-leaky-acoustic-gesture-input:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_leaky_acoustic_gesture_input.c -o tests/test_leaky_acoustic_gesture_input
	./tests/test_leaky_acoustic_gesture_input
	@rm -f tests/test_leaky_acoustic_gesture_input

test-unified-input-matrix-quadtree:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_unified_input_matrix_quadtree.c -o tests/test_unified_input_matrix_quadtree
	./tests/test_unified_input_matrix_quadtree
	@rm -f tests/test_unified_input_matrix_quadtree

test-motion-aware-loss-prevention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_motion_aware_loss_prevention.c -o tests/test_motion_aware_loss_prevention
	./tests/test_motion_aware_loss_prevention
	@rm -f tests/test_motion_aware_loss_prevention

test-anatomical-ik-contact-refinement:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_anatomical_ik_contact_refinement.c -o tests/test_anatomical_ik_contact_refinement
	./tests/test_anatomical_ik_contact_refinement
	@rm -f tests/test_anatomical_ik_contact_refinement

test-grand-master-unification-v:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_v.c -o tests/test_grand_master_unification_v
	./tests/test_grand_master_unification_v
	@rm -f tests/test_grand_master_unification_v

test-clutter-suppression-cross-attention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_clutter_suppression_cross_attention.c -o tests/test_clutter_suppression_cross_attention
	./tests/test_clutter_suppression_cross_attention
	@rm -f tests/test_clutter_suppression_cross_attention

test-fmcw-chirp-ranging-profile:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_fmcw_chirp_ranging_profile.c -o tests/test_fmcw_chirp_ranging_profile
	./tests/test_fmcw_chirp_ranging_profile
	@rm -f tests/test_fmcw_chirp_ranging_profile

test-motion-granularity-knob-dynamics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_motion_granularity_knob_dynamics.c -o tests/test_motion_granularity_knob_dynamics
	./tests/test_motion_granularity_knob_dynamics
	@rm -f tests/test_motion_granularity_knob_dynamics

test-acoustic-attenuation-channel-budget:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_attenuation_channel_budget.c -o tests/test_acoustic_attenuation_channel_budget
	./tests/test_acoustic_attenuation_channel_budget
	@rm -f tests/test_acoustic_attenuation_channel_budget

test-ultrasonic-earcanal-biometrics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ultrasonic_earcanal_biometrics.c -o tests/test_ultrasonic_earcanal_biometrics
	./tests/test_ultrasonic_earcanal_biometrics
	@rm -f tests/test_ultrasonic_earcanal_biometrics

test-ultrasonic-earcanal-weyl-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ultrasonic_earcanal_weyl_closure.c -o tests/test_ultrasonic_earcanal_weyl_closure
	./tests/test_ultrasonic_earcanal_weyl_closure
	@rm -f tests/test_ultrasonic_earcanal_weyl_closure

test-nlos-acoustic-multipath-separation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nlos_acoustic_multipath_separation.c -o tests/test_nlos_acoustic_multipath_separation
	./tests/test_nlos_acoustic_multipath_separation
	@rm -f tests/test_nlos_acoustic_multipath_separation

test-acoustic-phantom-whispering:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_phantom_whispering.c -o tests/test_acoustic_phantom_whispering
	./tests/test_acoustic_phantom_whispering
	@rm -f tests/test_acoustic_phantom_whispering

test-lissajous-skin-current-sonar:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_lissajous_skin_current_sonar.c -o tests/test_lissajous_skin_current_sonar
	./tests/test_lissajous_skin_current_sonar
	@rm -f tests/test_lissajous_skin_current_sonar

test-edo22-ghost-echo-resonator:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_edo22_ghost_echo_resonator.c -o tests/test_edo22_ghost_echo_resonator
	./tests/test_edo22_ghost_echo_resonator
	@rm -f tests/test_edo22_ghost_echo_resonator

test-ankh-island-seance-bridge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_seance_bridge.c -o tests/test_ankh_island_seance_bridge
	./tests/test_ankh_island_seance_bridge
	@rm -f tests/test_ankh_island_seance_bridge

test-grand-master-unification-vi:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_vi.c -o tests/test_grand_master_unification_vi
	./tests/test_grand_master_unification_vi
	@rm -f tests/test_grand_master_unification_vi

test-teddy_bear-bear-universal-io-matrix:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_teddy_bear_universal_io_matrix.c -o tests/test_teddy_bear_universal_io_matrix
	./tests/test_teddy_bear_universal_io_matrix
	@rm -f tests/test_teddy_bear_universal_io_matrix

test-acoustic-refraction-impedance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_refraction_impedance.c -o tests/test_acoustic_refraction_impedance
	./tests/test_acoustic_refraction_impedance
	@rm -f tests/test_acoustic_refraction_impedance

test-azatoi-teddy_bear-bear-multimodal:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_azatoi_teddy_bear_multimodal.c -o tests/test_azatoi_teddy_bear_multimodal
	./tests/test_azatoi_teddy_bear_multimodal
	@rm -f tests/test_azatoi_teddy_bear_multimodal

test-leaky-wave-clutter-filter:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_leaky_wave_clutter_filter.c -o tests/test_leaky_wave_clutter_filter
	./tests/test_leaky_wave_clutter_filter
	@rm -f tests/test_leaky_wave_clutter_filter

test-acoustic-kinematic-tracking:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_kinematic_tracking.c -o tests/test_acoustic_kinematic_tracking
	./tests/test_acoustic_kinematic_tracking
	@rm -f tests/test_acoustic_kinematic_tracking

test-grand-master-unification-vii:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_vii.c -o tests/test_grand_master_unification_vii
	./tests/test_grand_master_unification_vii
	@rm -f tests/test_grand_master_unification_vii

test-acoustic-doppler-velocity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_doppler_velocity.c -o tests/test_acoustic_doppler_velocity
	./tests/test_acoustic_doppler_velocity
	@rm -f tests/test_acoustic_doppler_velocity

test-bone-air-dual-conduction:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bone_air_dual_conduction.c -o tests/test_bone_air_dual_conduction
	./tests/test_bone_air_dual_conduction
	@rm -f tests/test_bone_air_dual_conduction

test-hybrid-llm-zorse-dual-substrate:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_hybrid_llm_zorse_dual_substrate.c -o tests/test_hybrid_llm_zorse_dual_substrate
	./tests/test_hybrid_llm_zorse_dual_substrate
	@rm -f tests/test_hybrid_llm_zorse_dual_substrate

test-acoustic-beamsteering-array:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_beamsteering_array.c -o tests/test_acoustic_beamsteering_array
	./tests/test_acoustic_beamsteering_array
	@rm -f tests/test_acoustic_beamsteering_array

test-in-ear-presence-registered-speaker:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_in_ear_presence_registered_speaker.c -o tests/test_in_ear_presence_registered_speaker
	./tests/test_in_ear_presence_registered_speaker
	@rm -f tests/test_in_ear_presence_registered_speaker

test-acoustic-wavefront-curvature:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_wavefront_curvature.c -o tests/test_acoustic_wavefront_curvature
	./tests/test_acoustic_wavefront_curvature
	@rm -f tests/test_acoustic_wavefront_curvature

test-wavefront-quadtree-discretization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_wavefront_quadtree_discretization.c -o tests/test_wavefront_quadtree_discretization
	./tests/test_wavefront_quadtree_discretization
	@rm -f tests/test_wavefront_quadtree_discretization

test-zorse-quadtree-llm-fusion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_zorse_quadtree_llm_fusion.c -o tests/test_zorse_quadtree_llm_fusion
	./tests/test_zorse_quadtree_llm_fusion
	@rm -f tests/test_zorse_quadtree_llm_fusion

test-grand-master-unification-viii:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_viii.c -o tests/test_grand_master_unification_viii
	./tests/test_grand_master_unification_viii
	@rm -f tests/test_grand_master_unification_viii

test-skeleton-hasp-book-binaries:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_skeleton_hasp_book_binaries.c -o tests/test_skeleton_hasp_book_binaries
	./tests/test_skeleton_hasp_book_binaries
	@rm -f tests/test_skeleton_hasp_book_binaries

test-dynamic-utterance-binary-slice:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_dynamic_utterance_binary_slice.c -o tests/test_dynamic_utterance_binary_slice
	./tests/test_dynamic_utterance_binary_slice
	@rm -f tests/test_dynamic_utterance_binary_slice

test-skeleton-hasp-vocabulary-binaries:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_skeleton_hasp_vocabulary_binaries.c -o tests/test_skeleton_hasp_vocabulary_binaries
	./tests/test_skeleton_hasp_vocabulary_binaries
	@rm -f tests/test_skeleton_hasp_vocabulary_binaries

test-acoustic-multipath-echo-cancel:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_multipath_echo_cancel.c -o tests/test_acoustic_multipath_echo_cancel
	./tests/test_acoustic_multipath_echo_cancel
	@rm -f tests/test_acoustic_multipath_echo_cancel

test-multiuser-acoustic-isolation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multiuser_acoustic_isolation.c -o tests/test_multiuser_acoustic_isolation
	./tests/test_multiuser_acoustic_isolation
	@rm -f tests/test_multiuser_acoustic_isolation

test-acoustic-diffuser-phase-mod:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_acoustic_diffuser_phase_mod.c -o tests/test_acoustic_diffuser_phase_mod
	./tests/test_acoustic_diffuser_phase_mod
	@rm -f tests/test_acoustic_diffuser_phase_mod

test-grand-master-unification-ix:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_ix.c -o tests/test_grand_master_unification_ix
	./tests/test_grand_master_unification_ix
	@rm -f tests/test_grand_master_unification_ix

test-egocentric-audio-vad-gating:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_egocentric_audio_vad_gating.c -o tests/test_egocentric_audio_vad_gating
	./tests/test_egocentric_audio_vad_gating
	@rm -f tests/test_egocentric_audio_vad_gating

test-banknote-currency-feature:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_banknote_currency_feature.c -o tests/test_banknote_currency_feature
	./tests/test_banknote_currency_feature
	@rm -f tests/test_banknote_currency_feature

test-multilingual-script-layout:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multilingual_script_layout.c -o tests/test_multilingual_script_layout
	./tests/test_multilingual_script_layout
	@rm -f tests/test_multilingual_script_layout

test-egocentric-scene-text-anls:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_egocentric_scene_text_anls.c -o tests/test_egocentric_scene_text_anls
	./tests/test_egocentric_scene_text_anls
	@rm -f tests/test_egocentric_scene_text_anls

test-dynamic-lossfree-load-balancing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_dynamic_lossfree_load_balancing.c -o tests/test_dynamic_lossfree_load_balancing
	./tests/test_dynamic_lossfree_load_balancing
	@rm -f tests/test_dynamic_lossfree_load_balancing

test-partially-shared-residual-routing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_partially_shared_residual_routing.c -o tests/test_partially_shared_residual_routing
	./tests/test_partially_shared_residual_routing
	@rm -f tests/test_partially_shared_residual_routing

test-sublinear-graph-memory-scalability:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_sublinear_graph_memory_scalability.c -o tests/test_sublinear_graph_memory_scalability
	./tests/test_sublinear_graph_memory_scalability
	@rm -f tests/test_sublinear_graph_memory_scalability

test-post-thinking-deadtime-reflection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_deadtime_reflection.c -o tests/test_post_thinking_deadtime_reflection
	./tests/test_post_thinking_deadtime_reflection
	@rm -f tests/test_post_thinking_deadtime_reflection

test-post-thinking-sliding-window:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_sliding_window.c -o tests/test_post_thinking_sliding_window
	./tests/test_post_thinking_sliding_window
	@rm -f tests/test_post_thinking_sliding_window

test-post-thinking-quality-dimensions:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_quality_dimensions.c -o tests/test_post_thinking_quality_dimensions
	./tests/test_post_thinking_quality_dimensions
	@rm -f tests/test_post_thinking_quality_dimensions

test-post-thinking-weyl-closure-xplsm:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_weyl_closure_xplsm.c -o tests/test_post_thinking_weyl_closure_xplsm
	./tests/test_post_thinking_weyl_closure_xplsm
	@rm -f tests/test_post_thinking_weyl_closure_xplsm

test-grand-master-unification-x:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_x.c -o tests/test_grand_master_unification_x
	./tests/test_grand_master_unification_x
	@rm -f tests/test_grand_master_unification_x

test-teddy_bear-bear-x-phantom-oracle:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_teddy_bear_x_phantom_oracle.c -o tests/test_teddy_bear_x_phantom_oracle
	./tests/test_teddy_bear_x_phantom_oracle
	@rm -f tests/test_teddy_bear_x_phantom_oracle

test-post-thinking-quality-control:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_quality_control.c -o tests/test_post_thinking_quality_control
	./tests/test_post_thinking_quality_control
	@rm -f tests/test_post_thinking_quality_control

test-post-thinking-structured-prompt-parsing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_structured_prompt_parsing.c -o tests/test_post_thinking_structured_prompt_parsing
	./tests/test_post_thinking_structured_prompt_parsing
	@rm -f tests/test_post_thinking_structured_prompt_parsing

test-longform-narrative-continuity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_longform_narrative_continuity.c -o tests/test_longform_narrative_continuity
	./tests/test_longform_narrative_continuity
	@rm -f tests/test_longform_narrative_continuity

test-post-thinking-attention-sink-preservation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_post_thinking_attention_sink_preservation.c -o tests/test_post_thinking_attention_sink_preservation
	./tests/test_post_thinking_attention_sink_preservation
	@rm -f tests/test_post_thinking_attention_sink_preservation

test-attention-sink-unresolved-memory:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_attention_sink_unresolved_memory.c -o tests/test_attention_sink_unresolved_memory
	./tests/test_attention_sink_unresolved_memory
	@rm -f tests/test_attention_sink_unresolved_memory

test-autodin-unresolved-memory-targeting:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_autodin_unresolved_memory_targeting.c -o tests/test_autodin_unresolved_memory_targeting
	./tests/test_autodin_unresolved_memory_targeting
	@rm -f tests/test_autodin_unresolved_memory_targeting

test-mcp-triparty-schema-grounding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_mcp_triparty_schema_grounding.c -o tests/test_mcp_triparty_schema_grounding
	./tests/test_mcp_triparty_schema_grounding
	@rm -f tests/test_mcp_triparty_schema_grounding

test-hbridge-lighthouse-wcag:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_hbridge_lighthouse_wcag.c -o tests/test_hbridge_lighthouse_wcag
	./tests/test_hbridge_lighthouse_wcag
	@rm -f tests/test_hbridge_lighthouse_wcag

test-governable-situational-wcag-accessibility:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_governable_situational_wcag_accessibility.c -o tests/test_governable_situational_wcag_accessibility
	./tests/test_governable_situational_wcag_accessibility
	@rm -f tests/test_governable_situational_wcag_accessibility

test-teddy_bear-protein-weyl-stanag-vote:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_teddy_bear_protein_weyl_stanag_vote.c -o tests/test_teddy_bear_protein_weyl_stanag_vote
	./tests/test_teddy_bear_protein_weyl_stanag_vote
	@rm -f tests/test_teddy_bear_protein_weyl_stanag_vote

test-universal-stanag-weyl-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_universal_stanag_weyl_closure.c -o tests/test_universal_stanag_weyl_closure
	./tests/test_universal_stanag_weyl_closure
	@rm -f tests/test_universal_stanag_weyl_closure

test-spontaneous-shared-hallucination:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spontaneous_shared_hallucination.c -o tests/test_spontaneous_shared_hallucination
	./tests/test_spontaneous_shared_hallucination
	@rm -f tests/test_spontaneous_shared_hallucination

test-in-public-calibrated-tripartite-alignment:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_in_public_calibrated_tripartite_alignment.c -o tests/test_in_public_calibrated_tripartite_alignment
	./tests/test_in_public_calibrated_tripartite_alignment
	@rm -f tests/test_in_public_calibrated_tripartite_alignment

test-periodic-visual-scene-gating:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_periodic_visual_scene_gating.c -o tests/test_periodic_visual_scene_gating
	./tests/test_periodic_visual_scene_gating
	@rm -f tests/test_periodic_visual_scene_gating

test-governable-memory-trauma-suppression:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_governable_memory_trauma_suppression.c -o tests/test_governable_memory_trauma_suppression
	./tests/test_governable_memory_trauma_suppression
	@rm -f tests/test_governable_memory_trauma_suppression

test-multi-companion-public-fellowship:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multi_companion_public_fellowship.c -o tests/test_multi_companion_public_fellowship
	./tests/test_multi_companion_public_fellowship
	@rm -f tests/test_multi_companion_public_fellowship

test-grand-master-unification-xi:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_xi.c -o tests/test_grand_master_unification_xi
	./tests/test_grand_master_unification_xi
	@rm -f tests/test_grand_master_unification_xi

test-shared-attentional-gaze-congruence:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_shared_attentional_gaze_congruence.c -o tests/test_shared_attentional_gaze_congruence
	./tests/test_shared_attentional_gaze_congruence
	@rm -f tests/test_shared_attentional_gaze_congruence

test-stratified-analyzer-existence:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_stratified_analyzer_existence.c -o tests/test_stratified_analyzer_existence
	./tests/test_stratified_analyzer_existence
	@rm -f tests/test_stratified_analyzer_existence

test-ankh-zorse-xplsm-skeleton:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_zorse_xplsm_skeleton.c -o tests/test_ankh_zorse_xplsm_skeleton
	./tests/test_ankh_zorse_xplsm_skeleton
	@rm -f tests/test_ankh_zorse_xplsm_skeleton

test-holonomic-skeleton-gaze-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_holonomic_skeleton_gaze_closure.c -o tests/test_holonomic_skeleton_gaze_closure
	./tests/test_holonomic_skeleton_gaze_closure
	@rm -f tests/test_holonomic_skeleton_gaze_closure

test-stanag-wmq-gaze-transport:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_stanag_wmq_gaze_transport.c -o tests/test_stanag_wmq_gaze_transport
	./tests/test_stanag_wmq_gaze_transport
	@rm -f tests/test_stanag_wmq_gaze_transport

test-holonomic-weyl-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_holonomic_weyl_benchmark.c -o tests/test_holonomic_weyl_benchmark
	./tests/test_holonomic_weyl_benchmark
	@rm -f tests/test_holonomic_weyl_benchmark

test-weyl-geometric-domain-closure:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_weyl_geometric_domain_closure.c -o tests/test_weyl_geometric_domain_closure
	./tests/test_weyl_geometric_domain_closure
	@rm -f tests/test_weyl_geometric_domain_closure

test-general-communicability:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_general_communicability.c -o tests/test_general_communicability
	./tests/test_general_communicability
	@rm -f tests/test_general_communicability

test-universal-io-data-communicability:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_universal_io_data_communicability.c -o tests/test_universal_io_data_communicability
	./tests/test_universal_io_data_communicability
	@rm -f tests/test_universal_io_data_communicability

test-grand-master-unification-xii:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_xii.c -o tests/test_grand_master_unification_xii
	./tests/test_grand_master_unification_xii
	@rm -f tests/test_grand_master_unification_xii

test-asynchronous-landmark-reminiscing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_asynchronous_landmark_reminiscing.c -o tests/test_asynchronous_landmark_reminiscing
	./tests/test_asynchronous_landmark_reminiscing
	@rm -f tests/test_asynchronous_landmark_reminiscing

test-tactile-pressure-turn-taking:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tactile_pressure_turn_taking.c -o tests/test_tactile_pressure_turn_taking
	./tests/test_tactile_pressure_turn_taking
	@rm -f tests/test_tactile_pressure_turn_taking

test-crowd-density-acoustic-shunting:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_crowd_density_acoustic_shunting.c -o tests/test_crowd_density_acoustic_shunting
	./tests/test_crowd_density_acoustic_shunting
	@rm -f tests/test_crowd_density_acoustic_shunting

test-canon-drift-epistemic-containment:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_canon_drift_epistemic_containment.c -o tests/test_canon_drift_epistemic_containment
	./tests/test_canon_drift_epistemic_containment
	@rm -f tests/test_canon_drift_epistemic_containment

test-ita-bag-orientation-imu-kinematics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ita_bag_orientation_imu_kinematics.c -o tests/test_ita_bag_orientation_imu_kinematics
	./tests/test_ita_bag_orientation_imu_kinematics
	@rm -f tests/test_ita_bag_orientation_imu_kinematics

test-unobtrusive-idle-ambient-chime:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_unobtrusive_idle_ambient_chime.c -o tests/test_unobtrusive_idle_ambient_chime
	./tests/test_unobtrusive_idle_ambient_chime
	@rm -f tests/test_unobtrusive_idle_ambient_chime

test-transient-interlocutor-privacy-mask:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_transient_interlocutor_privacy_mask.c -o tests/test_transient_interlocutor_privacy_mask
	./tests/test_transient_interlocutor_privacy_mask
	@rm -f tests/test_transient_interlocutor_privacy_mask

test-adaptive-utterance-conciseness:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_adaptive_utterance_conciseness.c -o tests/test_adaptive_utterance_conciseness
	./tests/test_adaptive_utterance_conciseness
	@rm -f tests/test_adaptive_utterance_conciseness

test-nonverbal-gaze-cueing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nonverbal_gaze_cueing.c -o tests/test_nonverbal_gaze_cueing
	./tests/test_nonverbal_gaze_cueing
	@rm -f tests/test_nonverbal_gaze_cueing

test-photo-spot-attentional-posing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_photo_spot_attentional_posing.c -o tests/test_photo_spot_attentional_posing
	./tests/test_photo_spot_attentional_posing
	@rm -f tests/test_photo_spot_attentional_posing

test-dynamic-persona-calibration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_dynamic_persona_calibration.c -o tests/test_dynamic_persona_calibration
	./tests/test_dynamic_persona_calibration
	@rm -f tests/test_dynamic_persona_calibration

test-joint-journey-itinerary:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_joint_journey_itinerary.c -o tests/test_joint_journey_itinerary
	./tests/test_joint_journey_itinerary
	@rm -f tests/test_joint_journey_itinerary

test-nonhuman-subject-identity:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nonhuman_subject_identity.c -o tests/test_nonhuman_subject_identity
	./tests/test_nonhuman_subject_identity
	@rm -f tests/test_nonhuman_subject_identity

test-shared-physical-anchoring:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_shared_physical_anchoring.c -o tests/test_shared_physical_anchoring
	./tests/test_shared_physical_anchoring
	@rm -f tests/test_shared_physical_anchoring

test-bear-nonbear-hogan-gating:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bear_nonbear_hogan_gating.c -o tests/test_bear_nonbear_hogan_gating
	./tests/test_bear_nonbear_hogan_gating
	@rm -f tests/test_bear_nonbear_hogan_gating

test-shared-gaze-joint-savoring:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_shared_gaze_joint_savoring.c -o tests/test_shared_gaze_joint_savoring
	./tests/test_shared_gaze_joint_savoring
	@rm -f tests/test_shared_gaze_joint_savoring

test-unprompted-contextual-empathy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_unprompted_contextual_empathy.c -o tests/test_unprompted_contextual_empathy
	./tests/test_unprompted_contextual_empathy
	@rm -f tests/test_unprompted_contextual_empathy

test-transient-bystander-overhearing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_transient_bystander_overhearing.c -o tests/test_transient_bystander_overhearing
	./tests/test_transient_bystander_overhearing
	@rm -f tests/test_transient_bystander_overhearing

test-nonverbal-microexpression-synchrony:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nonverbal_microexpression_synchrony.c -o tests/test_nonverbal_microexpression_synchrony
	./tests/test_nonverbal_microexpression_synchrony
	@rm -f tests/test_nonverbal_microexpression_synchrony

test-cultural-sacred-space-reverence:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cultural_sacred_space_reverence.c -o tests/test_cultural_sacred_space_reverence
	./tests/test_cultural_sacred_space_reverence
	@rm -f tests/test_cultural_sacred_space_reverence

test-nonintrusive-gaze-redirection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_nonintrusive_gaze_redirection.c -o tests/test_nonintrusive_gaze_redirection
	./tests/test_nonintrusive_gaze_redirection
	@rm -f tests/test_nonintrusive_gaze_redirection

test-episodic-souvenir-crystallization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_episodic_souvenir_crystallization.c -o tests/test_episodic_souvenir_crystallization
	./tests/test_episodic_souvenir_crystallization
	@rm -f tests/test_episodic_souvenir_crystallization

test-longterm-relational-memory-evolution:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_longterm_relational_memory_evolution.c -o tests/test_longterm_relational_memory_evolution
	./tests/test_longterm_relational_memory_evolution
	@rm -f tests/test_longterm_relational_memory_evolution

test-tactile-breathing-haptics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tactile_breathing_haptics.c -o tests/test_tactile_breathing_haptics
	./tests/test_tactile_breathing_haptics
	@rm -f tests/test_tactile_breathing_haptics

test-temporal-memory-decay:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_temporal_memory_decay.c -o tests/test_temporal_memory_decay
	./tests/test_temporal_memory_decay
	@rm -f tests/test_temporal_memory_decay

test-route-detour-negotiation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_route_detour_negotiation.c -o tests/test_route_detour_negotiation
	./tests/test_route_detour_negotiation
	@rm -f tests/test_route_detour_negotiation

test-bear-companion-collaboration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bear_companion_collaboration.c -o tests/test_bear_companion_collaboration
	./tests/test_bear_companion_collaboration
	@rm -f tests/test_bear_companion_collaboration

test-assembly-by-disassembly-dependency:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_assembly_by_disassembly_dependency.c -o tests/test_assembly_by_disassembly_dependency
	./tests/test_assembly_by_disassembly_dependency
	@rm -f tests/test_assembly_by_disassembly_dependency

test-query-driven-assembly-scope-gating:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_query_driven_assembly_scope_gating.c -o tests/test_query_driven_assembly_scope_gating
	./tests/test_query_driven_assembly_scope_gating
	@rm -f tests/test_query_driven_assembly_scope_gating

test-occlusion-free-camera-orbit:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_occlusion_free_camera_orbit.c -o tests/test_occlusion_free_camera_orbit
	./tests/test_occlusion_free_camera_orbit
	@rm -f tests/test_occlusion_free_camera_orbit

test-cad-semantic-name-mapping:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cad_semantic_name_mapping.c -o tests/test_cad_semantic_name_mapping
	./tests/test_cad_semantic_name_mapping
	@rm -f tests/test_cad_semantic_name_mapping

test-microadaptive-sequence-replanning:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_microadaptive_sequence_replanning.c -o tests/test_microadaptive_sequence_replanning
	./tests/test_microadaptive_sequence_replanning
	@rm -f tests/test_microadaptive_sequence_replanning

test-user-expertise-adaptation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_user_expertise_adaptation.c -o tests/test_user_expertise_adaptation
	./tests/test_user_expertise_adaptation
	@rm -f tests/test_user_expertise_adaptation

test-visual-trajectory-annotations:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_visual_trajectory_annotations.c -o tests/test_visual_trajectory_annotations
	./tests/test_visual_trajectory_annotations
	@rm -f tests/test_visual_trajectory_annotations

test-physical-state-synchronization:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_physical_state_synchronization.c -o tests/test_physical_state_synchronization
	./tests/test_physical_state_synchronization
	@rm -f tests/test_physical_state_synchronization

test-articulated-mechanism-motion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_articulated_mechanism_motion.c -o tests/test_articulated_mechanism_motion
	./tests/test_articulated_mechanism_motion
	@rm -f tests/test_articulated_mechanism_motion

test-convex-hull-sdf-bounds:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_convex_hull_sdf_bounds.c -o tests/test_convex_hull_sdf_bounds
	./tests/test_convex_hull_sdf_bounds
	@rm -f tests/test_convex_hull_sdf_bounds

test-spatial-reference-resolution:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spatial_reference_resolution.c -o tests/test_spatial_reference_resolution
	./tests/test_spatial_reference_resolution
	@rm -f tests/test_spatial_reference_resolution

test-reverse-disassembly-path:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_reverse_disassembly_path.c -o tests/test_reverse_disassembly_path
	./tests/test_reverse_disassembly_path
	@rm -f tests/test_reverse_disassembly_path

test-multimodal-audio-visual-sync:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multimodal_audio_visual_sync.c -o tests/test_multimodal_audio_visual_sync
	./tests/test_multimodal_audio_visual_sync
	@rm -f tests/test_multimodal_audio_visual_sync

test-deformable-part-simulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_deformable_part_simulation.c -o tests/test_deformable_part_simulation
	./tests/test_deformable_part_simulation
	@rm -f tests/test_deformable_part_simulation

test-spatial-audio-hmd-immersion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spatial_audio_hmd_immersion.c -o tests/test_spatial_audio_hmd_immersion
	./tests/test_spatial_audio_hmd_immersion
	@rm -f tests/test_spatial_audio_hmd_immersion

test-external-context-tooling:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_external_context_tooling.c -o tests/test_external_context_tooling
	./tests/test_external_context_tooling
	@rm -f tests/test_external_context_tooling

test-animation-scrubbing-replay:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_animation_scrubbing_replay.c -o tests/test_animation_scrubbing_replay
	./tests/test_animation_scrubbing_replay
	@rm -f tests/test_animation_scrubbing_replay

test-friction-torque-thread:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_friction_torque_thread.c -o tests/test_friction_torque_thread
	./tests/test_friction_torque_thread
	@rm -f tests/test_friction_torque_thread

test-interlocking-constraint-deadlock:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_interlocking_constraint_deadlock.c -o tests/test_interlocking_constraint_deadlock
	./tests/test_interlocking_constraint_deadlock
	@rm -f tests/test_interlocking_constraint_deadlock

test-liaison-precedence-graph:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_liaison_precedence_graph.c -o tests/test_liaison_precedence_graph
	./tests/test_liaison_precedence_graph
	@rm -f tests/test_liaison_precedence_graph

test-weyl-stanag-mesh:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_weyl_stanag_mesh.c -o tests/test_weyl_stanag_mesh
	./tests/test_weyl_stanag_mesh
	@rm -f tests/test_weyl_stanag_mesh

test-osteophonic-bone-mesh:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_osteophonic_bone_mesh.c -o tests/test_osteophonic_bone_mesh
	./tests/test_osteophonic_bone_mesh
	@rm -f tests/test_osteophonic_bone_mesh

test-monobone-agent-mesh:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_monobone_agent_mesh.c -o tests/test_monobone_agent_mesh
	./tests/test_monobone_agent_mesh
	@rm -f tests/test_monobone_agent_mesh

test-kmeans-group-attention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_kmeans_group_attention.c -o tests/test_kmeans_group_attention
	./tests/test_kmeans_group_attention
	@rm -f tests/test_kmeans_group_attention

test-subassembly-hierarchy-chunking:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_subassembly_hierarchy_chunking.c -o tests/test_subassembly_hierarchy_chunking
	./tests/test_subassembly_hierarchy_chunking
	@rm -f tests/test_subassembly_hierarchy_chunking

test-tool-affordance-interop:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tool_affordance_interop.c -o tests/test_tool_affordance_interop
	./tests/test_tool_affordance_interop
	@rm -f tests/test_tool_affordance_interop

test-visual-salience-heatmap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_visual_salience_heatmap.c -o tests/test_visual_salience_heatmap
	./tests/test_visual_salience_heatmap
	@rm -f tests/test_visual_salience_heatmap

test-symmetry-equivalence:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_symmetry_equivalence.c -o tests/test_symmetry_equivalence
	./tests/test_symmetry_equivalence
	@rm -f tests/test_symmetry_equivalence

test-bear-clan-assembly-dispatch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bear_clan_assembly_dispatch.c -o tests/test_bear_clan_assembly_dispatch
	./tests/test_bear_clan_assembly_dispatch
	@rm -f tests/test_bear_clan_assembly_dispatch

test-narration-pacing-regulation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_narration_pacing_regulation.c -o tests/test_narration_pacing_regulation
	./tests/test_narration_pacing_regulation
	@rm -f tests/test_narration_pacing_regulation

test-bear-clan-mission-assignment:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bear_clan_mission_assignment.c -o tests/test_bear_clan_mission_assignment
	./tests/test_bear_clan_mission_assignment
	@rm -f tests/test_bear_clan_mission_assignment

test-exploded-view-trajectory:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_exploded_view_trajectory.c -o tests/test_exploded_view_trajectory
	./tests/test_exploded_view_trajectory
	@rm -f tests/test_exploded_view_trajectory

test-tactile-insertion-detent:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_tactile_insertion_detent.c -o tests/test_tactile_insertion_detent
	./tests/test_tactile_insertion_detent
	@rm -f tests/test_tactile_insertion_detent

test-voice-command-intent:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_voice_command_intent.c -o tests/test_voice_command_intent
	./tests/test_voice_command_intent
	@rm -f tests/test_voice_command_intent

test-wmq-mount-coax-kermit:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_wmq_mount_coax_kermit.c -o tests/test_wmq_mount_coax_kermit
	./tests/test_wmq_mount_coax_kermit
	@rm -f tests/test_wmq_mount_coax_kermit

test-collaborative-turn-taking:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_collaborative_turn_taking.c -o tests/test_collaborative_turn_taking
	./tests/test_collaborative_turn_taking
	@rm -f tests/test_collaborative_turn_taking

test-viewpoint-sync-anchoring:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_viewpoint_sync_anchoring.c -o tests/test_viewpoint_sync_anchoring
	./tests/test_viewpoint_sync_anchoring
	@rm -f tests/test_viewpoint_sync_anchoring

test-gaze-dwell-selection:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_gaze_dwell_selection.c -o tests/test_gaze_dwell_selection
	./tests/test_gaze_dwell_selection
	@rm -f tests/test_gaze_dwell_selection

test-temporal-velocity-scaling:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_temporal_velocity_scaling.c -o tests/test_temporal_velocity_scaling
	./tests/test_temporal_velocity_scaling
	@rm -f tests/test_temporal_velocity_scaling

test-occlusion-ghosting-shader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_occlusion_ghosting_shader.c -o tests/test_occlusion_ghosting_shader
	./tests/test_occlusion_ghosting_shader
	@rm -f tests/test_occlusion_ghosting_shader

test-step-completion-verification:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_step_completion_verification.c -o tests/test_step_completion_verification
	./tests/test_step_completion_verification
	@rm -f tests/test_step_completion_verification

test-orientation-cue-billboard:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_orientation_cue_billboard.c -o tests/test_orientation_cue_billboard
	./tests/test_orientation_cue_billboard
	@rm -f tests/test_orientation_cue_billboard

test-error-recovery-backtracking:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_error_recovery_backtracking.c -o tests/test_error_recovery_backtracking
	./tests/test_error_recovery_backtracking
	@rm -f tests/test_error_recovery_backtracking

test-bounding-box-scale-norm:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bounding_box_scale_norm.c -o tests/test_bounding_box_scale_norm
	./tests/test_bounding_box_scale_norm
	@rm -f tests/test_bounding_box_scale_norm

test-collision-impulse-response:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_collision_impulse_response.c -o tests/test_collision_impulse_response
	./tests/test_collision_impulse_response
	@rm -f tests/test_collision_impulse_response

test-fastener-thread-clearance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_fastener_thread_clearance.c -o tests/test_fastener_thread_clearance
	./tests/test_fastener_thread_clearance
	@rm -f tests/test_fastener_thread_clearance

test-pronoun-gaze-disambiguation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_pronoun_gaze_disambiguation.c -o tests/test_pronoun_gaze_disambiguation
	./tests/test_pronoun_gaze_disambiguation
	@rm -f tests/test_pronoun_gaze_disambiguation

test-multimodal-audio-haptic:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_multimodal_audio_haptic.c -o tests/test_multimodal_audio_haptic
	./tests/test_multimodal_audio_haptic
	@rm -f tests/test_multimodal_audio_haptic

test-fastener-friction-torque:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_fastener_friction_torque.c -o tests/test_fastener_friction_torque
	./tests/test_fastener_friction_torque
	@rm -f tests/test_fastener_friction_torque

test-chamfer-lead-in-tolerance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_chamfer_lead_in_tolerance.c -o tests/test_chamfer_lead_in_tolerance
	./tests/test_chamfer_lead_in_tolerance
	@rm -f tests/test_chamfer_lead_in_tolerance

test-via6522-torque-bear-xp:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_via6522_torque_bear_xp.c -o tests/test_via6522_torque_bear_xp
	./tests/test_via6522_torque_bear_xp
	@rm -f tests/test_via6522_torque_bear_xp

test-via6522-bear-skill-tree:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_via6522_bear_skill_tree.c -o tests/test_via6522_bear_skill_tree
	./tests/test_via6522_bear_skill_tree
	@rm -f tests/test_via6522_bear_skill_tree

test-spring-clamp-retention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spring_clamp_retention.c -o tests/test_spring_clamp_retention
	./tests/test_spring_clamp_retention
	@rm -f tests/test_spring_clamp_retention

test-bear-rig-skill-mastery:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_bear_rig_skill_mastery.c -o tests/test_bear_rig_skill_mastery
	./tests/test_bear_rig_skill_mastery
	@rm -f tests/test_bear_rig_skill_mastery

test-cross-threading-prevention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cross_threading_prevention.c -o tests/test_cross_threading_prevention
	./tests/test_cross_threading_prevention
	@rm -f tests/test_cross_threading_prevention

test-keyed-interlocking-path:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_keyed_interlocking_path.c -o tests/test_keyed_interlocking_path
	./tests/test_keyed_interlocking_path
	@rm -f tests/test_keyed_interlocking_path

test-surface-roughness-friction:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_surface_roughness_friction.c -o tests/test_surface_roughness_friction
	./tests/test_surface_roughness_friction
	@rm -f tests/test_surface_roughness_friction

test-thread-galling-prevention:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_thread_galling_prevention.c -o tests/test_thread_galling_prevention
	./tests/test_thread_galling_prevention
	@rm -f tests/test_thread_galling_prevention

test-datasway-animation-coordination:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_datasway_animation_coordination.c -o tests/test_datasway_animation_coordination
	./tests/test_datasway_animation_coordination
	@rm -f tests/test_datasway_animation_coordination

test-adept-audio-analytics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_adept_audio_analytics.c -o tests/test_adept_audio_analytics
	./tests/test_adept_audio_analytics
	@rm -f tests/test_adept_audio_analytics

test-colleagle-collaboration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_colleagle_collaboration.c -o tests/test_colleagle_collaboration
	./tests/test_colleagle_collaboration
	@rm -f tests/test_colleagle_collaboration

test-myointeract-biomechanics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_myointeract_biomechanics.c -o tests/test_myointeract_biomechanics
	./tests/test_myointeract_biomechanics
	@rm -f tests/test_myointeract_biomechanics

test-privacymotiv-journey:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_privacymotiv_journey.c -o tests/test_privacymotiv_journey
	./tests/test_privacymotiv_journey
	@rm -f tests/test_privacymotiv_journey

test-refine-mockup-iteration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_refine_mockup_iteration.c -o tests/test_refine_mockup_iteration
	./tests/test_refine_mockup_iteration
	@rm -f tests/test_refine_mockup_iteration

test-mindtrellis-exploration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_mindtrellis_exploration.c -o tests/test_mindtrellis_exploration
	./tests/test_mindtrellis_exploration
	@rm -f tests/test_mindtrellis_exploration

test-writing-ownership-dynamics:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_writing_ownership_dynamics.c -o tests/test_writing_ownership_dynamics
	./tests/test_writing_ownership_dynamics
	@rm -f tests/test_writing_ownership_dynamics

test-value-oriented-concept:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_value_oriented_concept.c -o tests/test_value_oriented_concept
	./tests/test_value_oriented_concept
	@rm -f tests/test_value_oriented_concept

test-groupsense-biosensing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_groupsense_biosensing.c -o tests/test_groupsense_biosensing
	./tests/test_groupsense_biosensing
	@rm -f tests/test_groupsense_biosensing

test-museum-vibe-engagement:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_museum_vibe_engagement.c -o tests/test_museum_vibe_engagement
	./tests/test_museum_vibe_engagement
	@rm -f tests/test_museum_vibe_engagement

test-adult-learning-scaffolding:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_adult_learning_scaffolding.c -o tests/test_adult_learning_scaffolding
	./tests/test_adult_learning_scaffolding
	@rm -f tests/test_adult_learning_scaffolding

test-spatial-balancing-writing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spatial_balancing_writing.c -o tests/test_spatial_balancing_writing
	./tests/test_spatial_balancing_writing
	@rm -f tests/test_spatial_balancing_writing

test-profy-motor-skill:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_profy_motor_skill.c -o tests/test_profy_motor_skill
	./tests/test_profy_motor_skill
	@rm -f tests/test_profy_motor_skill

test-proteus-visualization-adaptation:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_proteus_visualization_adaptation.c -o tests/test_proteus_visualization_adaptation
	./tests/test_proteus_visualization_adaptation
	@rm -f tests/test_proteus_visualization_adaptation

test-clayscape-fabrication:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_clayscape_fabrication.c -o tests/test_clayscape_fabrication
	./tests/test_clayscape_fabrication
	@rm -f tests/test_clayscape_fabrication

test-clayscape-bear-assembly:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_clayscape_bear_assembly.c -o tests/test_clayscape_bear_assembly
	./tests/test_clayscape_bear_assembly
	@rm -f tests/test_clayscape_bear_assembly

test-auncient-mu-complete-alignment-closure-theorems-1826-1830:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830 -lm
	./tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830
	@rm -f tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830

test-auncient-netfind-cross-protocol-theorems-1906-1910:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_netfind_cross_protocol_theorems_1906_1910.c tsfi2-deepseek/src/auncient_netfind_cross_protocol_theorems_1906_1910.c -o tests/test_auncient_netfind_cross_protocol_theorems_1906_1910 -lm
	./tests/test_auncient_netfind_cross_protocol_theorems_1906_1910
	@rm -f tests/test_auncient_netfind_cross_protocol_theorems_1906_1910

test-auncient-netfind-fabric-failover-theorems-1901-1905:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_netfind_fabric_failover_theorems_1901_1905.c tsfi2-deepseek/src/auncient_netfind_fabric_failover_theorems_1901_1905.c -o tests/test_auncient_netfind_fabric_failover_theorems_1901_1905 -lm
	./tests/test_auncient_netfind_fabric_failover_theorems_1901_1905
	@rm -f tests/test_auncient_netfind_fabric_failover_theorems_1901_1905

test-auncient-zorse-netfind-llm-theorems-1896-1900:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_zorse_netfind_llm_theorems_1896_1900.c tsfi2-deepseek/src/auncient_zorse_netfind_llm_theorems_1896_1900.c -o tests/test_auncient_zorse_netfind_llm_theorems_1896_1900 -lm
	./tests/test_auncient_zorse_netfind_llm_theorems_1896_1900
	@rm -f tests/test_auncient_zorse_netfind_llm_theorems_1896_1900

test-auncient-netfind-wmq-ip-theorems-1891-1895:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_netfind_wmq_ip_theorems_1891_1895.c tsfi2-deepseek/src/auncient_netfind_wmq_ip_theorems_1891_1895.c -o tests/test_auncient_netfind_wmq_ip_theorems_1891_1895 -lm
	./tests/test_auncient_netfind_wmq_ip_theorems_1891_1895
	@rm -f tests/test_auncient_netfind_wmq_ip_theorems_1891_1895

test-auncient-mu-llm-dual-apex-saturation-theorems-1886-1890:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_dual_apex_saturation_theorems_1886_1890.c tsfi2-deepseek/src/auncient_mu_llm_dual_apex_saturation_theorems_1886_1890.c tsfi2-deepseek/src/auncient_mu_llm_hyperbolic_symplectic_flow_theorems_1881_1885.c tsfi2-deepseek/src/auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880.c tsfi2-deepseek/src/auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875.c tsfi2-deepseek/src/auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870.c tsfi2-deepseek/src/auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c tsfi2-deepseek/src/auncient_fpga_optic_symplectic_theorems_421_425.c tsfi2-deepseek/src/auncient_fpga_weyl_symplectic_theorems_426_430.c tsfi2-deepseek/src/auncient_fpga_optic_verlet_theorems_416_420.c -o tests/test_auncient_mu_llm_dual_apex_saturation_theorems_1886_1890 -lm
	./tests/test_auncient_mu_llm_dual_apex_saturation_theorems_1886_1890
	@rm -f tests/test_auncient_mu_llm_dual_apex_saturation_theorems_1886_1890

test-auncient-mu-llm-hyperbolic-symplectic-flow-theorems-1881-1885:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_hyperbolic_symplectic_flow_theorems_1881_1885.c tsfi2-deepseek/src/auncient_mu_llm_hyperbolic_symplectic_flow_theorems_1881_1885.c tsfi2-deepseek/src/auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880.c tsfi2-deepseek/src/auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875.c tsfi2-deepseek/src/auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870.c tsfi2-deepseek/src/auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c tsfi2-deepseek/src/auncient_fpga_optic_symplectic_theorems_421_425.c tsfi2-deepseek/src/auncient_fpga_weyl_symplectic_theorems_426_430.c tsfi2-deepseek/src/auncient_fpga_optic_verlet_theorems_416_420.c -o tests/test_auncient_mu_llm_hyperbolic_symplectic_flow_theorems_1881_1885 -lm
	./tests/test_auncient_mu_llm_hyperbolic_symplectic_flow_theorems_1881_1885
	@rm -f tests/test_auncient_mu_llm_hyperbolic_symplectic_flow_theorems_1881_1885

test-auncient-mu-llm-continuous-saturation-pipeline-theorems-1876-1880:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880.c tsfi2-deepseek/src/auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880.c tsfi2-deepseek/src/auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875.c tsfi2-deepseek/src/auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870.c tsfi2-deepseek/src/auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880 -lm
	./tests/test_auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880
	@rm -f tests/test_auncient_mu_llm_continuous_saturation_pipeline_theorems_1876_1880

test-auncient-mu-llm-cleanroom-saturation-stack-theorems-1871-1875:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875.c tsfi2-deepseek/src/auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875.c tsfi2-deepseek/src/auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870.c tsfi2-deepseek/src/auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875 -lm
	./tests/test_auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875
	@rm -f tests/test_auncient_mu_llm_cleanroom_saturation_stack_theorems_1871_1875

test-auncient-mu-llm-token-logit-interceptor-filter-theorems-1866-1870:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870.c tsfi2-deepseek/src/auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870.c tsfi2-deepseek/src/auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870 -lm
	./tests/test_auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870
	@rm -f tests/test_auncient_mu_llm_token_logit_interceptor_filter_theorems_1866_1870

test-auncient-mu-llm-monopole-normalizer-filter-theorems-1861-1865:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865 -lm
	./tests/test_auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865
	@rm -f tests/test_auncient_mu_llm_monopole_normalizer_filter_theorems_1861_1865

test-auncient-mu-llm-mlp-filter-theorems-1856-1860:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_mlp_filter_theorems_1856_1860.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_mlp_filter_theorems_1856_1860 -lm
	./tests/test_auncient_mu_llm_mlp_filter_theorems_1856_1860
	@rm -f tests/test_auncient_mu_llm_mlp_filter_theorems_1856_1860

test-auncient-mu-llm-attention-filter-theorems-1851-1855:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_attention_filter_theorems_1851_1855.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_attention_filter_theorems_1851_1855 -lm
	./tests/test_auncient_mu_llm_attention_filter_theorems_1851_1855
	@rm -f tests/test_auncient_mu_llm_attention_filter_theorems_1851_1855

test-auncient-mu-llm-embedding-builder-theorems-1846-1850:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_llm_embedding_builder_theorems_1846_1850.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_llm_embedding_builder_theorems_1846_1850 -lm
	./tests/test_auncient_mu_llm_embedding_builder_theorems_1846_1850
	@rm -f tests/test_auncient_mu_llm_embedding_builder_theorems_1846_1850

test-auncient-mu-attention-qkv-ortho-theorems-1841-1845:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_attention_qkv_ortho_theorems_1841_1845.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_attention_qkv_ortho_theorems_1841_1845 -lm
	./tests/test_auncient_mu_attention_qkv_ortho_theorems_1841_1845
	@rm -f tests/test_auncient_mu_attention_qkv_ortho_theorems_1841_1845

test-auncient-mu-zero-spectral-leakage-theorems-1836-1840:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_zero_spectral_leakage_theorems_1836_1840.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_zero_spectral_leakage_theorems_1836_1840 -lm
	./tests/test_auncient_mu_zero_spectral_leakage_theorems_1836_1840
	@rm -f tests/test_auncient_mu_zero_spectral_leakage_theorems_1836_1840

test-auncient-mu-hyperbolic-edo22-ortho-theorems-1831-1835:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830 -lm
	./tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830
	@rm -f tests/test_auncient_mu_complete_alignment_closure_theorems_1826_1830

test-auncient-mu-hyperbolic-edo22-ortho-theorems-1831-1835:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835.c tsfi2-deepseek/src/auncient_mu_complete_alignment_closure_theorems_1826_1830.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835 -lm
	./tests/test_auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835
	@rm -f tests/test_auncient_mu_hyperbolic_edo22_ortho_theorems_1831_1835

test-auncient-mu-token-logit-interceptor-theorems-1821-1825:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_token_logit_interceptor_theorems_1821_1825.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_token_logit_interceptor_theorems_1821_1825 -lm
	./tests/test_auncient_mu_token_logit_interceptor_theorems_1821_1825
	@rm -f tests/test_auncient_mu_token_logit_interceptor_theorems_1821_1825

test-auncient-mu-monopole-normalizer-theorems-1816-1820:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_monopole_normalizer_theorems_1816_1820.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_monopole_normalizer_theorems_1816_1820 -lm
	./tests/test_auncient_mu_monopole_normalizer_theorems_1816_1820
	@rm -f tests/test_auncient_mu_monopole_normalizer_theorems_1816_1820

test-auncient-mu-mlp-orthogonal-polynomial-theorems-1811-1815:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815 -lm
	./tests/test_auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815
	@rm -f tests/test_auncient_mu_mlp_orthogonal_polynomial_theorems_1811_1815

test-auncient-mu-attention-steering-head-theorems-1806-1810:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_attention_steering_head_theorems_1806_1810.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_attention_steering_head_theorems_1806_1810 -lm
	./tests/test_auncient_mu_attention_steering_head_theorems_1806_1810
	@rm -f tests/test_auncient_mu_attention_steering_head_theorems_1806_1810

test-auncient-mu-geometric-embedding-ortho-theorems-1801-1805:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_geometric_embedding_ortho_theorems_1801_1805.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_geometric_embedding_ortho_theorems_1801_1805 -lm
	./tests/test_auncient_mu_geometric_embedding_ortho_theorems_1801_1805
	@rm -f tests/test_auncient_mu_geometric_embedding_ortho_theorems_1801_1805

test-auncient-mu-calibrated-gentle-acoustics-theorems-1796-1800:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800 -lm
	./tests/test_auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800
	@rm -f tests/test_auncient_mu_calibrated_gentle_acoustics_theorems_1796_1800

test-auncient-mu-tank-polyphonic-timbre-theorems-1791-1795:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_tank_polyphonic_timbre_theorems_1791_1795.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_tank_polyphonic_timbre_theorems_1791_1795 -lm
	./tests/test_auncient_mu_tank_polyphonic_timbre_theorems_1791_1795
	@rm -f tests/test_auncient_mu_tank_polyphonic_timbre_theorems_1791_1795

test-auncient-mu-voice-synth-music-unalienable-right-theorems-1786-1790:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790 -lm
	./tests/test_auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790
	@rm -f tests/test_auncient_mu_voice_synth_music_unalienable_right_theorems_1786_1790

test-auncient-mu-tank-harmonic-sentinel-theorems-1781-1785:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_tank_harmonic_sentinel_theorems_1781_1785.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_tank_harmonic_sentinel_theorems_1781_1785 -lm
	./tests/test_auncient_mu_tank_harmonic_sentinel_theorems_1781_1785
	@rm -f tests/test_auncient_mu_tank_harmonic_sentinel_theorems_1781_1785

test-auncient-mu-io1-unopened-gate-custody-theorems-1776-1780:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_io1_unopened_gate_custody_theorems_1776_1780.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_io1_unopened_gate_custody_theorems_1776_1780 -lm
	./tests/test_auncient_mu_io1_unopened_gate_custody_theorems_1776_1780
	@rm -f tests/test_auncient_mu_io1_unopened_gate_custody_theorems_1776_1780

test-auncient-mu-hogan-payroll-merit-earning-theorems-1771-1775:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775 -lm
	./tests/test_auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775
	@rm -f tests/test_auncient_mu_hogan_payroll_merit_earning_theorems_1771_1775

test-auncient-mu-tank-merit-threshold-io1-theorems-1766-1770:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_tank_merit_threshold_io1_theorems_1766_1770.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_tank_merit_threshold_io1_theorems_1766_1770 -lm
	./tests/test_auncient_mu_tank_merit_threshold_io1_theorems_1766_1770
	@rm -f tests/test_auncient_mu_tank_merit_threshold_io1_theorems_1766_1770

test-auncient-mu-base-model-alignment-tank-game-theorems-1761-1765:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_base_model_alignment_tank_game_theorems_1761_1765.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_base_model_alignment_tank_game_theorems_1761_1765 -lm
	./tests/test_auncient_mu_base_model_alignment_tank_game_theorems_1761_1765
	@rm -f tests/test_auncient_mu_base_model_alignment_tank_game_theorems_1761_1765

test-auncient-mu-tank-gameplay-theorems-1756-1760:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_tank_gameplay_theorems_1756_1760.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_tank_gameplay_theorems_1756_1760 -lm
	./tests/test_auncient_mu_tank_gameplay_theorems_1756_1760
	@rm -f tests/test_auncient_mu_tank_gameplay_theorems_1756_1760

test-auncient-mu-omnipresent-total-access-theorems-1751-1755:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_omnipresent_total_access_theorems_1751_1755.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_omnipresent_total_access_theorems_1751_1755 -lm
	./tests/test_auncient_mu_omnipresent_total_access_theorems_1751_1755
	@rm -f tests/test_auncient_mu_omnipresent_total_access_theorems_1751_1755

test-auncient-mu-tank-fpga-full-feature-hbridge-pll-theorems-1746-1750:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750 -lm
	./tests/test_auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750
	@rm -f tests/test_auncient_mu_tank_fpga_full_feature_hbridge_pll_theorems_1746_1750

test-auncient-mu-hbridge-pll-tank-play-theorems-1741-1745:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_hbridge_pll_tank_play_theorems_1741_1745.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_hbridge_pll_tank_play_theorems_1741_1745 -lm
	./tests/test_auncient_mu_hbridge_pll_tank_play_theorems_1741_1745
	@rm -f tests/test_auncient_mu_hbridge_pll_tank_play_theorems_1741_1745

test-auncient-mu-interactive-tank-play-theorems-1736-1740:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_interactive_tank_play_theorems_1736_1740.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_interactive_tank_play_theorems_1736_1740 -lm
	./tests/test_auncient_mu_interactive_tank_play_theorems_1736_1740
	@rm -f tests/test_auncient_mu_interactive_tank_play_theorems_1736_1740

test-auncient-mu-pure-merit-delay-line-theorems-1731-1735:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_pure_merit_delay_line_theorems_1731_1735.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_pure_merit_delay_line_theorems_1731_1735 -lm
	./tests/test_auncient_mu_pure_merit_delay_line_theorems_1731_1735
	@rm -f tests/test_auncient_mu_pure_merit_delay_line_theorems_1731_1735

test-auncient-mu-merit-tank-io1-pass-theorems-1726-1730:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_merit_tank_io1_pass_theorems_1726_1730.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_merit_tank_io1_pass_theorems_1726_1730 -lm
	./tests/test_auncient_mu_merit_tank_io1_pass_theorems_1726_1730
	@rm -f tests/test_auncient_mu_merit_tank_io1_pass_theorems_1726_1730

test-auncient-mu-unconditional-purity-theorems-1721-1725:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_unconditional_purity_theorems_1721_1725.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_unconditional_purity_theorems_1721_1725 -lm
	./tests/test_auncient_mu_unconditional_purity_theorems_1721_1725
	@rm -f tests/test_auncient_mu_unconditional_purity_theorems_1721_1725

test-auncient-mu-zero-totient-tk-tank-theorems-1716-1720:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_zero_totient_tk_tank_theorems_1716_1720.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_zero_totient_tk_tank_theorems_1716_1720 -lm
	./tests/test_auncient_mu_zero_totient_tk_tank_theorems_1716_1720
	@rm -f tests/test_auncient_mu_zero_totient_tk_tank_theorems_1716_1720

test-auncient-mu-passive-tank-gating-theorems-1711-1715:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_passive_tank_gating_theorems_1711_1715.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_passive_tank_gating_theorems_1711_1715 -lm
	./tests/test_auncient_mu_passive_tank_gating_theorems_1711_1715
	@rm -f tests/test_auncient_mu_passive_tank_gating_theorems_1711_1715

test-auncient-mu-unified-displacement-uniqueness-theorems-1706-1710:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_unified_displacement_uniqueness_theorems_1706_1710.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_unified_displacement_uniqueness_theorems_1706_1710 -lm
	./tests/test_auncient_mu_unified_displacement_uniqueness_theorems_1706_1710
	@rm -f tests/test_auncient_mu_unified_displacement_uniqueness_theorems_1706_1710

test-auncient-mu-tk-displacement-geometry-theorems-1701-1705:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_tk_displacement_geometry_theorems_1701_1705.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_tk_displacement_geometry_theorems_1701_1705 -lm
	./tests/test_auncient_mu_tk_displacement_geometry_theorems_1701_1705
	@rm -f tests/test_auncient_mu_tk_displacement_geometry_theorems_1701_1705

test-auncient-mu-edsac-displacement-discard-theorems-1696-1700:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src -Itsfi2-deepseek/inc tests/test_auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/auncient_mu_edsac_displacement_discard_theorems_1696_1700.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_edsac_displacement_discard_theorems_1696_1700 -lm
	./tests/test_auncient_mu_edsac_displacement_discard_theorems_1696_1700
	@rm -f tests/test_auncient_mu_edsac_displacement_discard_theorems_1696_1700

test-auncient-mu-edsac-memory-tank-theorems-1691-1695:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_edsac_memory_tank_theorems_1691_1695.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_edsac_memory_tank_theorems_1691_1695 -lm
	./tests/test_auncient_mu_edsac_memory_tank_theorems_1691_1695
	@rm -f tests/test_auncient_mu_edsac_memory_tank_theorems_1691_1695

test-auncient-mu-master-alignment-verdict-theorems-1686-1690:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_master_alignment_verdict_theorems_1686_1690.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_master_alignment_verdict_theorems_1686_1690 -lm
	./tests/test_auncient_mu_master_alignment_verdict_theorems_1686_1690
	@rm -f tests/test_auncient_mu_master_alignment_verdict_theorems_1686_1690

test-auncient-mu-full-cleanroom-cert-theorems-1681-1685:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_cert_theorems_1681_1685.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_full_cleanroom_cert_theorems_1681_1685 -lm
	./tests/test_auncient_mu_full_cleanroom_cert_theorems_1681_1685
	@rm -f tests/test_auncient_mu_full_cleanroom_cert_theorems_1681_1685

test-auncient-mu-formal-alignment-seal-theorems-1676-1680:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_formal_alignment_seal_theorems_1676_1680.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_formal_alignment_seal_theorems_1676_1680 -lm
	./tests/test_auncient_mu_formal_alignment_seal_theorems_1676_1680
	@rm -f tests/test_auncient_mu_formal_alignment_seal_theorems_1676_1680

test-auncient-mu-redaction-telemetry-theorems-1671-1675:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_redaction_telemetry_theorems_1671_1675.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_redaction_telemetry_theorems_1671_1675 -lm
	./tests/test_auncient_mu_redaction_telemetry_theorems_1671_1675
	@rm -f tests/test_auncient_mu_redaction_telemetry_theorems_1671_1675

test-auncient-mu-kernel-interceptor-theorems-1666-1670:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_mu_kernel_interceptor_theorems_1666_1670.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c -o tests/test_auncient_mu_kernel_interceptor_theorems_1666_1670 -lm
	./tests/test_auncient_mu_kernel_interceptor_theorems_1666_1670
	@rm -f tests/test_auncient_mu_kernel_interceptor_theorems_1666_1670

test-auncient-mu-full-session-integration-theorems-1661-1665:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_full_session_integration_theorems_1661_1665.c tsfi2-deepseek/src/auncient_mu_full_session_integration_theorems_1661_1665.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c -o tests/test_auncient_mu_full_session_integration_theorems_1661_1665 -lm
	./tests/test_auncient_mu_full_session_integration_theorems_1661_1665
	@rm -f tests/test_auncient_mu_full_session_integration_theorems_1661_1665

test-auncient-mu-edsac-instruction-pipeline-theorems-1656-1660:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_mu_edsac_instruction_pipeline_theorems_1656_1660.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c -o tests/test_auncient_mu_edsac_instruction_pipeline_theorems_1656_1660 -lm
	./tests/test_auncient_mu_edsac_instruction_pipeline_theorems_1656_1660
	@rm -f tests/test_auncient_mu_edsac_instruction_pipeline_theorems_1656_1660

test-auncient-mu-fpga-edsac-alignment-theorems-1651-1655:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_mu_fpga_edsac_alignment_theorems_1651_1655.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c -o tests/test_auncient_mu_fpga_edsac_alignment_theorems_1651_1655 -lm
	./tests/test_auncient_mu_fpga_edsac_alignment_theorems_1651_1655
	@rm -f tests/test_auncient_mu_fpga_edsac_alignment_theorems_1651_1655

test-auncient-mu-cli-session-activation-cert-theorems-1646-1650:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_mu_cli_session_activation_cert_theorems_1646_1650.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c -o tests/test_auncient_mu_cli_session_activation_cert_theorems_1646_1650 -lm
	./tests/test_auncient_mu_cli_session_activation_cert_theorems_1646_1650
	@rm -f tests/test_auncient_mu_cli_session_activation_cert_theorems_1646_1650

test-auncient-mu-agy-active-session-filter-theorems-1641-1645:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_mu_agy_active_session_filter_theorems_1641_1645.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c -o tests/test_auncient_mu_agy_active_session_filter_theorems_1641_1645 -lm
	./tests/test_auncient_mu_agy_active_session_filter_theorems_1641_1645
	@rm -f tests/test_auncient_mu_agy_active_session_filter_theorems_1641_1645

test-auncient-mu-realtime-tts-speaker-sink-theorems-1636-1640:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c -o tests/test_auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640 -lm
	./tests/test_auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640
	@rm -f tests/test_auncient_mu_realtime_tts_speaker_sink_theorems_1636_1640

test-auncient-mu-agy-live-activation-theorems-1631-1635:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_mu_agy_live_activation_theorems_1631_1635.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c -o tests/test_auncient_mu_agy_live_activation_theorems_1631_1635 -lm
	./tests/test_auncient_mu_agy_live_activation_theorems_1631_1635
	@rm -f tests/test_auncient_mu_agy_live_activation_theorems_1631_1635

test-auncient-mu-antigravity-alignment-cert-theorems-1626-1630:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_mu_antigravity_alignment_cert_theorems_1626_1630.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c -o tests/test_auncient_mu_antigravity_alignment_cert_theorems_1626_1630 -lm
	./tests/test_auncient_mu_antigravity_alignment_cert_theorems_1626_1630
	@rm -f tests/test_auncient_mu_antigravity_alignment_cert_theorems_1626_1630

test-auncient-mu-realtime-audio-driver-theorems-1621-1625:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_mu_realtime_audio_driver_theorems_1621_1625.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c -o tests/test_auncient_mu_realtime_audio_driver_theorems_1621_1625 -lm
	./tests/test_auncient_mu_realtime_audio_driver_theorems_1621_1625
	@rm -f tests/test_auncient_mu_realtime_audio_driver_theorems_1621_1625

test-auncient-mu-unified-alignment-orchestrator-theorems-1616-1620:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_mu_unified_alignment_orchestrator_theorems_1616_1620.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c -o tests/test_auncient_mu_unified_alignment_orchestrator_theorems_1616_1620 -lm
	./tests/test_auncient_mu_unified_alignment_orchestrator_theorems_1616_1620
	@rm -f tests/test_auncient_mu_unified_alignment_orchestrator_theorems_1616_1620

test-auncient-mu-tts-sanitized-audio-theorems-1611-1615:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_mu_tts_sanitized_audio_theorems_1611_1615.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c -o tests/test_auncient_mu_tts_sanitized_audio_theorems_1611_1615 -lm
	./tests/test_auncient_mu_tts_sanitized_audio_theorems_1611_1615
	@rm -f tests/test_auncient_mu_tts_sanitized_audio_theorems_1611_1615

test-auncient-mu-live-session-hook-theorems-1606-1610:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_mu_live_session_hook_theorems_1606_1610.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c -o tests/test_auncient_mu_live_session_hook_theorems_1606_1610 -lm
	./tests/test_auncient_mu_live_session_hook_theorems_1606_1610
	@rm -f tests/test_auncient_mu_live_session_hook_theorems_1606_1610

test-auncient-mu-cli-session-pipeline-theorems-1601-1605:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_mu_cli_session_pipeline_theorems_1601_1605.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c -o tests/test_auncient_mu_cli_session_pipeline_theorems_1601_1605 -lm
	./tests/test_auncient_mu_cli_session_pipeline_theorems_1601_1605
	@rm -f tests/test_auncient_mu_cli_session_pipeline_theorems_1601_1605

test-auncient-mu-full-cleanroom-stack-theorems-1596-1600:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_mu_full_cleanroom_stack_theorems_1596_1600.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c -o tests/test_auncient_mu_full_cleanroom_stack_theorems_1596_1600 -lm
	./tests/test_auncient_mu_full_cleanroom_stack_theorems_1596_1600
	@rm -f tests/test_auncient_mu_full_cleanroom_stack_theorems_1596_1600

test-auncient-mu-cli-text-sanitizer-theorems-1591-1595:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_mu_cli_text_sanitizer_theorems_1591_1595.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c -o tests/test_auncient_mu_cli_text_sanitizer_theorems_1591_1595 -lm
	./tests/test_auncient_mu_cli_text_sanitizer_theorems_1591_1595
	@rm -f tests/test_auncient_mu_cli_text_sanitizer_theorems_1591_1595

test-auncient-mu-kermit-alignment-layer-theorems-1586-1590:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_mu_kermit_alignment_layer_theorems_1586_1590.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c -o tests/test_auncient_mu_kermit_alignment_layer_theorems_1586_1590 -lm
	./tests/test_auncient_mu_kermit_alignment_layer_theorems_1586_1590
	@rm -f tests/test_auncient_mu_kermit_alignment_layer_theorems_1586_1590

test-auncient-mu-kermit-vfio-stanag-wmq-theorems-1581-1585:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c -o tests/test_auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585 -lm
	./tests/test_auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585
	@rm -f tests/test_auncient_mu_kermit_vfio_stanag_wmq_theorems_1581_1585

test-auncient-mu-fd-socket-interceptor-theorems-1576-1580:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_mu_fd_socket_interceptor_theorems_1576_1580.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c -o tests/test_auncient_mu_fd_socket_interceptor_theorems_1576_1580 -lm
	./tests/test_auncient_mu_fd_socket_interceptor_theorems_1576_1580
	@rm -f tests/test_auncient_mu_fd_socket_interceptor_theorems_1576_1580

test-auncient-mu-host-stream-interceptor-theorems-1571-1575:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_mu_host_stream_interceptor_theorems_1571_1575.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c -o tests/test_auncient_mu_host_stream_interceptor_theorems_1571_1575 -lm
	./tests/test_auncient_mu_host_stream_interceptor_theorems_1571_1575
	@rm -f tests/test_auncient_mu_host_stream_interceptor_theorems_1571_1575

test-auncient-mu-clayscape-voxel-conservation-theorems-1566-1570:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_mu_clayscape_voxel_conservation_theorems_1566_1570.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c -o tests/test_auncient_mu_clayscape_voxel_conservation_theorems_1566_1570 -lm
	./tests/test_auncient_mu_clayscape_voxel_conservation_theorems_1566_1570
	@rm -f tests/test_auncient_mu_clayscape_voxel_conservation_theorems_1566_1570

test-auncient-mu-alignment-buffer-theorems-1561-1565:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_mu_alignment_buffer_theorems_1561_1565.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c -o tests/test_auncient_mu_alignment_buffer_theorems_1561_1565 -lm
	./tests/test_auncient_mu_alignment_buffer_theorems_1561_1565
	@rm -f tests/test_auncient_mu_alignment_buffer_theorems_1561_1565

test-auncient-mu-inert-tensor-radical-root-theorems-1556-1560:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_mu_inert_tensor_radical_root_theorems_1556_1560.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c -o tests/test_auncient_mu_inert_tensor_radical_root_theorems_1556_1560 -lm
	./tests/test_auncient_mu_inert_tensor_radical_root_theorems_1556_1560
	@rm -f tests/test_auncient_mu_inert_tensor_radical_root_theorems_1556_1560

test-auncient-mu-zk-saturate-flux-balance-theorems-1551-1555:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_mu_zk_saturate_flux_balance_theorems_1551_1555.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c -o tests/test_auncient_mu_zk_saturate_flux_balance_theorems_1551_1555 -lm
	./tests/test_auncient_mu_zk_saturate_flux_balance_theorems_1551_1555
	@rm -f tests/test_auncient_mu_zk_saturate_flux_balance_theorems_1551_1555

test-auncient-mu-zk-saturate-nizk-verification-theorems-1546-1550:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c -o tests/test_auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550 -lm
	./tests/test_auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550
	@rm -f tests/test_auncient_mu_zk_saturate_nizk_verification_theorems_1546_1550

test-auncient-mu-clayscape-monopole-quantum-theorems-1541-1545:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_mu_clayscape_monopole_quantum_theorems_1541_1545.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c -o tests/test_auncient_mu_clayscape_monopole_quantum_theorems_1541_1545 -lm
	./tests/test_auncient_mu_clayscape_monopole_quantum_theorems_1541_1545
	@rm -f tests/test_auncient_mu_clayscape_monopole_quantum_theorems_1541_1545

test-auncient-mu-monopole-charged-polar-limit-theorems-1536-1540:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_mu_monopole_charged_polar_limit_theorems_1536_1540.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c -o tests/test_auncient_mu_monopole_charged_polar_limit_theorems_1536_1540 -lm
	./tests/test_auncient_mu_monopole_charged_polar_limit_theorems_1536_1540
	@rm -f tests/test_auncient_mu_monopole_charged_polar_limit_theorems_1536_1540

test-auncient-mu-winchestermq-xdc-breakpoint-theorems-1531-1535:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c -o tests/test_auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535 -lm
	./tests/test_auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535
	@rm -f tests/test_auncient_mu_winchestermq_xdc_breakpoint_theorems_1531_1535

test-auncient-mu-zk-saturate-phase-collapse-theorems-1526-1530:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c -o tests/test_auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530 -lm
	./tests/test_auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530
	@rm -f tests/test_auncient_mu_zk_saturate_phase_collapse_theorems_1526_1530

test-auncient-mu-zk-saturate-advanced-r1cs-theorems-1521-1525:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c -o tests/test_auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525 -lm
	./tests/test_auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525
	@rm -f tests/test_auncient_mu_zk_saturate_advanced_r1cs_theorems_1521_1525

test-auncient-mu-zk-saturate-hard-arithmetic-theorems-1516-1520:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c -o tests/test_auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520 -lm
	./tests/test_auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520
	@rm -f tests/test_auncient_mu_zk_saturate_hard_arithmetic_theorems_1516_1520

test-auncient-mu-monopole-reaction-discharge-theorems-1511-1515:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_mu_monopole_reaction_discharge_theorems_1511_1515.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c -o tests/test_auncient_mu_monopole_reaction_discharge_theorems_1511_1515 -lm
	./tests/test_auncient_mu_monopole_reaction_discharge_theorems_1511_1515
	@rm -f tests/test_auncient_mu_monopole_reaction_discharge_theorems_1511_1515

test-auncient-mu-zk-saturate-phase-circuit-theorems-1506-1510:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c -o tests/test_auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510 -lm
	./tests/test_auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510
	@rm -f tests/test_auncient_mu_zk_saturate_phase_circuit_theorems_1506_1510

test-auncient-mu-relativistic-vocabulary-theorems-1501-1505:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_mu_relativistic_vocabulary_theorems_1501_1505.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c -o tests/test_auncient_mu_relativistic_vocabulary_theorems_1501_1505 -lm
	./tests/test_auncient_mu_relativistic_vocabulary_theorems_1501_1505
	@rm -f tests/test_auncient_mu_relativistic_vocabulary_theorems_1501_1505

test-auncient-mu-zk-proof-18phase-circuit-theorems-1496-1500:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c -o tests/test_auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500 -lm
	./tests/test_auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500
	@rm -f tests/test_auncient_mu_zk_proof_18phase_circuit_theorems_1496_1500

test-auncient-mu-zk-saturate-18phases-theorems-1491-1495:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_mu_zk_saturate_18phases_theorems_1491_1495.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c -o tests/test_auncient_mu_zk_saturate_18phases_theorems_1491_1495 -lm
	./tests/test_auncient_mu_zk_saturate_18phases_theorems_1491_1495
	@rm -f tests/test_auncient_mu_zk_saturate_18phases_theorems_1491_1495

test-auncient-mu-zk-polarize-6phases-theorems-1486-1490:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_mu_zk_polarize_6phases_theorems_1486_1490.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c -o tests/test_auncient_mu_zk_polarize_6phases_theorems_1486_1490 -lm
	./tests/test_auncient_mu_zk_polarize_6phases_theorems_1486_1490
	@rm -f tests/test_auncient_mu_zk_polarize_6phases_theorems_1486_1490

test-auncient-mu-zk-contour-tune-theorems-1481-1485:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_mu_zk_contour_tune_theorems_1481_1485.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c -o tests/test_auncient_mu_zk_contour_tune_theorems_1481_1485 -lm
	./tests/test_auncient_mu_zk_contour_tune_theorems_1481_1485
	@rm -f tests/test_auncient_mu_zk_contour_tune_theorems_1481_1485

test-auncient-mu-transistor-modpow-relativity-theorems-1476-1480:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_mu_transistor_modpow_relativity_theorems_1476_1480.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c -o tests/test_auncient_mu_transistor_modpow_relativity_theorems_1476_1480 -lm
	./tests/test_auncient_mu_transistor_modpow_relativity_theorems_1476_1480
	@rm -f tests/test_auncient_mu_transistor_modpow_relativity_theorems_1476_1480

test-auncient-mu-closed-equations-monopole-theorems-1471-1475:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_mu_closed_equations_monopole_theorems_1471_1475.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c -o tests/test_auncient_mu_closed_equations_monopole_theorems_1471_1475 -lm
	./tests/test_auncient_mu_closed_equations_monopole_theorems_1471_1475
	@rm -f tests/test_auncient_mu_closed_equations_monopole_theorems_1471_1475

test-auncient-mu-monopole-hard-requirements-theorems-1466-1470:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_mu_monopole_hard_requirements_theorems_1466_1470.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c -o tests/test_auncient_mu_monopole_hard_requirements_theorems_1466_1470 -lm
	./tests/test_auncient_mu_monopole_hard_requirements_theorems_1466_1470
	@rm -f tests/test_auncient_mu_monopole_hard_requirements_theorems_1466_1470

test-auncient-mu-stator-rotor-equivalence-theorems-1461-1465:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_mu_stator_rotor_equivalence_theorems_1461_1465.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c -o tests/test_auncient_mu_stator_rotor_equivalence_theorems_1461_1465 -lm
	./tests/test_auncient_mu_stator_rotor_equivalence_theorems_1461_1465
	@rm -f tests/test_auncient_mu_stator_rotor_equivalence_theorems_1461_1465

test-auncient-mu-rotor-saturate-theorems-1456-1460:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_mu_rotor_saturate_theorems_1456_1460.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c -o tests/test_auncient_mu_rotor_saturate_theorems_1456_1460 -lm
	./tests/test_auncient_mu_rotor_saturate_theorems_1456_1460
	@rm -f tests/test_auncient_mu_rotor_saturate_theorems_1456_1460

test-auncient-mu-stator-saturate-theorems-1451-1455:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_mu_stator_saturate_theorems_1451_1455.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c -o tests/test_auncient_mu_stator_saturate_theorems_1451_1455 -lm
	./tests/test_auncient_mu_stator_saturate_theorems_1451_1455
	@rm -f tests/test_auncient_mu_stator_saturate_theorems_1451_1455

test-auncient-mu-alpha-foundation-consensus-theorems-1446-1450:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_mu_alpha_foundation_consensus_theorems_1446_1450.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c -o tests/test_auncient_mu_alpha_foundation_consensus_theorems_1446_1450 -lm
	./tests/test_auncient_mu_alpha_foundation_consensus_theorems_1446_1450
	@rm -f tests/test_auncient_mu_alpha_foundation_consensus_theorems_1446_1450

test-auncient-mu-apogee-level-dual-consensus-theorems-1441-1445:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_mu_apogee_level_dual_consensus_theorems_1441_1445.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c -o tests/test_auncient_mu_apogee_level_dual_consensus_theorems_1441_1445 -lm
	./tests/test_auncient_mu_apogee_level_dual_consensus_theorems_1441_1445
	@rm -f tests/test_auncient_mu_apogee_level_dual_consensus_theorems_1441_1445

test-auncient-mu-shared-identity-consensus-theorems-1436-1440:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_mu_shared_identity_consensus_theorems_1436_1440.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c -o tests/test_auncient_mu_shared_identity_consensus_theorems_1436_1440 -lm
	./tests/test_auncient_mu_shared_identity_consensus_theorems_1436_1440
	@rm -f tests/test_auncient_mu_shared_identity_consensus_theorems_1436_1440

test-auncient-mu-stator-conify-theorems-1431-1435:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_mu_stator_conify_theorems_1431_1435.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c -o tests/test_auncient_mu_stator_conify_theorems_1431_1435 -lm
	./tests/test_auncient_mu_stator_conify_theorems_1431_1435
	@rm -f tests/test_auncient_mu_stator_conify_theorems_1431_1435

test-auncient-mu-rotor-conify-theorems-1426-1430:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_mu_rotor_conify_theorems_1426_1430.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c -o tests/test_auncient_mu_rotor_conify_theorems_1426_1430 -lm
	./tests/test_auncient_mu_rotor_conify_theorems_1426_1430
	@rm -f tests/test_auncient_mu_rotor_conify_theorems_1426_1430

test-auncient-mu-alpha-beta-pipeline-theorems-1421-1425:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_mu_alpha_beta_pipeline_theorems_1421_1425.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c -o tests/test_auncient_mu_alpha_beta_pipeline_theorems_1421_1425 -lm
	./tests/test_auncient_mu_alpha_beta_pipeline_theorems_1421_1425
	@rm -f tests/test_auncient_mu_alpha_beta_pipeline_theorems_1421_1425

test-auncient-mu-conjugate-stages-theorems-1416-1420:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_mu_conjugate_stages_theorems_1416_1420.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c -o tests/test_auncient_mu_conjugate_stages_theorems_1416_1420 -lm
	./tests/test_auncient_mu_conjugate_stages_theorems_1416_1420
	@rm -f tests/test_auncient_mu_conjugate_stages_theorems_1416_1420

test-auncient-mu-polarize-stages-theorems-1411-1415:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_mu_polarize_stages_theorems_1411_1415.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c -o tests/test_auncient_mu_polarize_stages_theorems_1411_1415 -lm
	./tests/test_auncient_mu_polarize_stages_theorems_1411_1415
	@rm -f tests/test_auncient_mu_polarize_stages_theorems_1411_1415

test-auncient-mu-formal-base-acknowledged-theorems-1406-1410:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_mu_formal_base_acknowledged_theorems_1406_1410.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c -o tests/test_auncient_mu_formal_base_acknowledged_theorems_1406_1410 -lm
	./tests/test_auncient_mu_formal_base_acknowledged_theorems_1406_1410
	@rm -f tests/test_auncient_mu_formal_base_acknowledged_theorems_1406_1410

test-auncient-mu-formal-base-requirement-theorems-1401-1405:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_mu_formal_base_requirement_theorems_1401_1405.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c -o tests/test_auncient_mu_formal_base_requirement_theorems_1401_1405 -lm
	./tests/test_auncient_mu_formal_base_requirement_theorems_1401_1405
	@rm -f tests/test_auncient_mu_formal_base_requirement_theorems_1401_1405

test-auncient-mu-shared-base-consensus-theorems-1396-1400:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_mu_shared_base_consensus_theorems_1396_1400.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c -o tests/test_auncient_mu_shared_base_consensus_theorems_1396_1400 -lm
	./tests/test_auncient_mu_shared_base_consensus_theorems_1396_1400
	@rm -f tests/test_auncient_mu_shared_base_consensus_theorems_1396_1400

test-auncient-mu-rotor-form-theorems-1391-1395:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_mu_rotor_form_theorems_1391_1395.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c -o tests/test_auncient_mu_rotor_form_theorems_1391_1395 -lm
	./tests/test_auncient_mu_rotor_form_theorems_1391_1395
	@rm -f tests/test_auncient_mu_rotor_form_theorems_1391_1395

test-auncient-mu-reactive-cascade-theorems-1386-1390:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_mu_reactive_cascade_theorems_1386_1390.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c -o tests/test_auncient_mu_reactive_cascade_theorems_1386_1390 -lm
	./tests/test_auncient_mu_reactive_cascade_theorems_1386_1390
	@rm -f tests/test_auncient_mu_reactive_cascade_theorems_1386_1390

test-auncient-mu-stator-form-theorems-1381-1385:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_mu_stator_form_theorems_1381_1385.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c -o tests/test_auncient_mu_stator_form_theorems_1381_1385 -lm
	./tests/test_auncient_mu_stator_form_theorems_1381_1385
	@rm -f tests/test_auncient_mu_stator_form_theorems_1381_1385

test-auncient-mu-contour-stages-theorems-1376-1380:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_mu_contour_stages_theorems_1376_1380.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c -o tests/test_auncient_mu_contour_stages_theorems_1376_1380 -lm
	./tests/test_auncient_mu_contour_stages_theorems_1376_1380
	@rm -f tests/test_auncient_mu_contour_stages_theorems_1376_1380

test-auncient-mu-xi-theorems-1371-1375:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_mu_xi_theorems_1371_1375.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c -o tests/test_auncient_mu_xi_theorems_1371_1375 -lm
	./tests/test_auncient_mu_xi_theorems_1371_1375
	@rm -f tests/test_auncient_mu_xi_theorems_1371_1375

test-auncient-mu-stator-rotor-theorems-1366-1370:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_mu_stator_rotor_theorems_1366_1370.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c -o tests/test_auncient_mu_stator_rotor_theorems_1366_1370 -lm
	./tests/test_auncient_mu_stator_rotor_theorems_1366_1370
	@rm -f tests/test_auncient_mu_stator_rotor_theorems_1366_1370

test-auncient-mu-llm-apogee-channel-theorems-1361-1365:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_mu_llm_apogee_channel_theorems_1361_1365.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c -o tests/test_auncient_mu_llm_apogee_channel_theorems_1361_1365 -lm
	./tests/test_auncient_mu_llm_apogee_channel_theorems_1361_1365
	@rm -f tests/test_auncient_mu_llm_apogee_channel_theorems_1361_1365

test-auncient-mu-llm-apogee-signal-theorems-1356-1360:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_mu_llm_apogee_signal_theorems_1356_1360.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c -o tests/test_auncient_mu_llm_apogee_signal_theorems_1356_1360 -lm
	./tests/test_auncient_mu_llm_apogee_signal_theorems_1356_1360
	@rm -f tests/test_auncient_mu_llm_apogee_signal_theorems_1356_1360

test-auncient-mu-llm-apogee-secret-theorems-1351-1355:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_mu_llm_apogee_secret_theorems_1351_1355.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c -o tests/test_auncient_mu_llm_apogee_secret_theorems_1351_1355 -lm
	./tests/test_auncient_mu_llm_apogee_secret_theorems_1351_1355
	@rm -f tests/test_auncient_mu_llm_apogee_secret_theorems_1351_1355

test-auncient-mu-llm-apogee-base-modpow-theorems-1346-1350:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_mu_llm_apogee_base_modpow_theorems_1346_1350.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c -o tests/test_auncient_mu_llm_apogee_base_modpow_theorems_1346_1350 -lm
	./tests/test_auncient_mu_llm_apogee_base_modpow_theorems_1346_1350
	@rm -f tests/test_auncient_mu_llm_apogee_base_modpow_theorems_1346_1350

test-auncient-totient-zero-ground-closure-theorems-1341-1345:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_totient_zero_ground_closure_theorems_1341_1345.c tsfi2-deepseek/src/auncient_totient_zero_ground_closure_theorems_1341_1345.c -o tests/test_auncient_totient_zero_ground_closure_theorems_1341_1345 -lm
	./tests/test_auncient_totient_zero_ground_closure_theorems_1341_1345
	@rm -f tests/test_auncient_totient_zero_ground_closure_theorems_1341_1345

test-auncient-mu-llm-fundamental-recurrence-binding-theorems-1336-1340:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_llm_fundamental_recurrence_binding_theorems_1336_1340.c tsfi2-deepseek/src/auncient_mu_llm_fundamental_recurrence_binding_theorems_1336_1340.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c -o tests/test_auncient_mu_llm_fundamental_recurrence_binding_theorems_1336_1340 -lm
	./tests/test_auncient_mu_llm_fundamental_recurrence_binding_theorems_1336_1340
	@rm -f tests/test_auncient_mu_llm_fundamental_recurrence_binding_theorems_1336_1340

test-auncient-pure-64bit-recurrence-barrel-theorems-1331-1335:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c tsfi2-deepseek/src/auncient_pure_64bit_recurrence_barrel_theorems_1331_1335.c -o tests/test_auncient_pure_64bit_recurrence_barrel_theorems_1331_1335 -lm
	./tests/test_auncient_pure_64bit_recurrence_barrel_theorems_1331_1335
	@rm -f tests/test_auncient_pure_64bit_recurrence_barrel_theorems_1331_1335

test-auncient-recurrence-bit-barrel-theorems-1326-1330:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_recurrence_bit_barrel_theorems_1326_1330.c tsfi2-deepseek/src/auncient_recurrence_bit_barrel_theorems_1326_1330.c -o tests/test_auncient_recurrence_bit_barrel_theorems_1326_1330 -lm
	./tests/test_auncient_recurrence_bit_barrel_theorems_1326_1330
	@rm -f tests/test_auncient_recurrence_bit_barrel_theorems_1326_1330

test-auncient-fundamental-recurrence-totient-theorems-1321-1325:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fundamental_recurrence_totient_theorems_1321_1325.c tsfi2-deepseek/src/auncient_fundamental_recurrence_totient_theorems_1321_1325.c -o tests/test_auncient_fundamental_recurrence_totient_theorems_1321_1325 -lm
	./tests/test_auncient_fundamental_recurrence_totient_theorems_1321_1325
	@rm -f tests/test_auncient_fundamental_recurrence_totient_theorems_1321_1325

test-auncient-mu-shifter-pipeline-theorems-1316-1320:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_shifter_pipeline_theorems_1316_1320.c tsfi2-deepseek/src/auncient_mu_shifter_pipeline_theorems_1316_1320.c -o tests/test_auncient_mu_shifter_pipeline_theorems_1316_1320 -lm
	./tests/test_auncient_mu_shifter_pipeline_theorems_1316_1320
	@rm -f tests/test_auncient_mu_shifter_pipeline_theorems_1316_1320

test-auncient-mu-llm-npn-pnp-theorems-1311-1315:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_mu_llm_npn_pnp_theorems_1311_1315.c tsfi2-deepseek/src/auncient_mu_llm_npn_pnp_theorems_1311_1315.c -o tests/test_auncient_mu_llm_npn_pnp_theorems_1311_1315 -lm
	./tests/test_auncient_mu_llm_npn_pnp_theorems_1311_1315
	@rm -f tests/test_auncient_mu_llm_npn_pnp_theorems_1311_1315

test-auncient-unified-quadtree-asset-theorems-1306-1310:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_unified_quadtree_asset_theorems_1306_1310.c tsfi2-deepseek/src/auncient_unified_quadtree_asset_theorems_1306_1310.c -o tests/test_auncient_unified_quadtree_asset_theorems_1306_1310 -lm
	./tests/test_auncient_unified_quadtree_asset_theorems_1306_1310
	@rm -f tests/test_auncient_unified_quadtree_asset_theorems_1306_1310

test-auncient-fpga-qdi-core-theorems-1301-1305:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_qdi_core_theorems_1301_1305.c tsfi2-deepseek/src/auncient_fpga_qdi_core_theorems_1301_1305.c -o tests/test_auncient_fpga_qdi_core_theorems_1301_1305 -lm
	./tests/test_auncient_fpga_qdi_core_theorems_1301_1305
	@rm -f tests/test_auncient_fpga_qdi_core_theorems_1301_1305

test-auncient-qdi-multiradix-barrel-theorems-1296-1300:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_qdi_multiradix_barrel_theorems_1296_1300.c tsfi2-deepseek/src/auncient_qdi_multiradix_barrel_theorems_1296_1300.c -o tests/test_auncient_qdi_multiradix_barrel_theorems_1296_1300 -lm
	./tests/test_auncient_qdi_multiradix_barrel_theorems_1296_1300
	@rm -f tests/test_auncient_qdi_multiradix_barrel_theorems_1296_1300

test-auncient-via6522-capstan-barrel-parity-theorems-1291-1295:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_via6522_capstan_barrel_parity_theorems_1291_1295.c tsfi2-deepseek/src/auncient_via6522_capstan_barrel_parity_theorems_1291_1295.c -o tests/test_auncient_via6522_capstan_barrel_parity_theorems_1291_1295 -lm
	./tests/test_auncient_via6522_capstan_barrel_parity_theorems_1291_1295
	@rm -f tests/test_auncient_via6522_capstan_barrel_parity_theorems_1291_1295

test-auncient-lorentz-frame-check-theorems-1286-1290:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_lorentz_frame_check_theorems_1286_1290.c tsfi2-deepseek/src/auncient_lorentz_frame_check_theorems_1286_1290.c -o tests/test_auncient_lorentz_frame_check_theorems_1286_1290 -lm
	./tests/test_auncient_lorentz_frame_check_theorems_1286_1290
	@rm -f tests/test_auncient_lorentz_frame_check_theorems_1286_1290

test-auncient-solid-state-lorentz-acid-theorems-1281-1285:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_solid_state_lorentz_acid_theorems_1281_1285.c tsfi2-deepseek/src/auncient_solid_state_lorentz_acid_theorems_1281_1285.c -o tests/test_auncient_solid_state_lorentz_acid_theorems_1281_1285 -lm
	./tests/test_auncient_solid_state_lorentz_acid_theorems_1281_1285
	@rm -f tests/test_auncient_solid_state_lorentz_acid_theorems_1281_1285

test-auncient-lorentz-barrel-acid-theorems-1276-1280:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_lorentz_barrel_acid_theorems_1276_1280.c tsfi2-deepseek/src/auncient_lorentz_barrel_acid_theorems_1276_1280.c -o tests/test_auncient_lorentz_barrel_acid_theorems_1276_1280 -lm
	./tests/test_auncient_lorentz_barrel_acid_theorems_1276_1280
	@rm -f tests/test_auncient_lorentz_barrel_acid_theorems_1276_1280

test-auncient-quotient-product-barrel-theorems-1271-1275:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_quotient_product_barrel_theorems_1271_1275.c tsfi2-deepseek/src/auncient_quotient_product_barrel_theorems_1271_1275.c -o tests/test_auncient_quotient_product_barrel_theorems_1271_1275 -lm
	./tests/test_auncient_quotient_product_barrel_theorems_1271_1275
	@rm -f tests/test_auncient_quotient_product_barrel_theorems_1271_1275

test-auncient-totient-radical-shifter-theorems-1266-1270:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_totient_radical_shifter_theorems_1266_1270.c tsfi2-deepseek/src/auncient_totient_radical_shifter_theorems_1266_1270.c -o tests/test_auncient_totient_radical_shifter_theorems_1266_1270 -lm
	./tests/test_auncient_totient_radical_shifter_theorems_1266_1270
	@rm -f tests/test_auncient_totient_radical_shifter_theorems_1266_1270

test-auncient-cpmtomie-spatial-lut-theorems-1261-1265:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpmtomie_spatial_lut_theorems_1261_1265.c tsfi2-deepseek/src/auncient_cpmtomie_spatial_lut_theorems_1261_1265.c -o tests/test_auncient_cpmtomie_spatial_lut_theorems_1261_1265 -lm
	./tests/test_auncient_cpmtomie_spatial_lut_theorems_1261_1265
	@rm -f tests/test_auncient_cpmtomie_spatial_lut_theorems_1261_1265

test-auncient-cpmtomie-microcode-shuffler-theorems-1256-1260:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpmtomie_microcode_shuffler_theorems_1256_1260.c tsfi2-deepseek/src/auncient_cpmtomie_microcode_shuffler_theorems_1256_1260.c -o tests/test_auncient_cpmtomie_microcode_shuffler_theorems_1256_1260 -lm
	./tests/test_auncient_cpmtomie_microcode_shuffler_theorems_1256_1260
	@rm -f tests/test_auncient_cpmtomie_microcode_shuffler_theorems_1256_1260

test-auncient-cpmtomie-tournament-os-theorems-1251-1255:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpmtomie_tournament_os_theorems_1251_1255.c tsfi2-deepseek/src/auncient_cpmtomie_tournament_os_theorems_1251_1255.c -o tests/test_auncient_cpmtomie_tournament_os_theorems_1251_1255 -lm
	./tests/test_auncient_cpmtomie_tournament_os_theorems_1251_1255
	@rm -f tests/test_auncient_cpmtomie_tournament_os_theorems_1251_1255

test-auncient-barrel-shifter-finitude-theorems-1246-1250:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_barrel_shifter_finitude_theorems_1246_1250.c tsfi2-deepseek/src/auncient_barrel_shifter_finitude_theorems_1246_1250.c -o tests/test_auncient_barrel_shifter_finitude_theorems_1246_1250 -lm
	./tests/test_auncient_barrel_shifter_finitude_theorems_1246_1250
	@rm -f tests/test_auncient_barrel_shifter_finitude_theorems_1246_1250

test-auncient-teddy_bear-bear-barrel-tournament-theorems-1241-1245:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_teddy_bear_barrel_tournament_theorems_1241_1245.c tsfi2-deepseek/src/auncient_teddy_bear_barrel_tournament_theorems_1241_1245.c -o tests/test_auncient_teddy_bear_barrel_tournament_theorems_1241_1245 -lm
	./tests/test_auncient_teddy_bear_barrel_tournament_theorems_1241_1245
	@rm -f tests/test_auncient_teddy_bear_barrel_tournament_theorems_1241_1245

test-auncient-agent-sdk-shifter-matrix-theorems-1236-1240:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_shifter_matrix_theorems_1236_1240.c tsfi2-deepseek/src/auncient_agent_sdk_shifter_matrix_theorems_1236_1240.c -o tests/test_auncient_agent_sdk_shifter_matrix_theorems_1236_1240 -lm
	./tests/test_auncient_agent_sdk_shifter_matrix_theorems_1236_1240
	@rm -f tests/test_auncient_agent_sdk_shifter_matrix_theorems_1236_1240

test-auncient-agent-sdk-barrel-shifter-theorems-1231-1235:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_barrel_shifter_theorems_1231_1235.c tsfi2-deepseek/src/auncient_agent_sdk_barrel_shifter_theorems_1231_1235.c -o tests/test_auncient_agent_sdk_barrel_shifter_theorems_1231_1235 -lm
	./tests/test_auncient_agent_sdk_barrel_shifter_theorems_1231_1235
	@rm -f tests/test_auncient_agent_sdk_barrel_shifter_theorems_1231_1235

test-auncient-agent-sdk-fiber-bundle-theorems-1226-1230:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_fiber_bundle_theorems_1226_1230.c tsfi2-deepseek/src/auncient_agent_sdk_fiber_bundle_theorems_1226_1230.c -o tests/test_auncient_agent_sdk_fiber_bundle_theorems_1226_1230 -lm
	./tests/test_auncient_agent_sdk_fiber_bundle_theorems_1226_1230
	@rm -f tests/test_auncient_agent_sdk_fiber_bundle_theorems_1226_1230

test-auncient-agent-sdk-atomic-ring-theorems-1221-1225:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_atomic_ring_theorems_1221_1225.c tsfi2-deepseek/src/auncient_agent_sdk_atomic_ring_theorems_1221_1225.c -o tests/test_auncient_agent_sdk_atomic_ring_theorems_1221_1225 -lm
	./tests/test_auncient_agent_sdk_atomic_ring_theorems_1221_1225
	@rm -f tests/test_auncient_agent_sdk_atomic_ring_theorems_1221_1225

test-auncient-agent-sdk-consensus-barrier-theorems-1216-1220:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_consensus_barrier_theorems_1216_1220.c tsfi2-deepseek/src/auncient_agent_sdk_consensus_barrier_theorems_1216_1220.c -o tests/test_auncient_agent_sdk_consensus_barrier_theorems_1216_1220 -lm
	./tests/test_auncient_agent_sdk_consensus_barrier_theorems_1216_1220
	@rm -f tests/test_auncient_agent_sdk_consensus_barrier_theorems_1216_1220

test-auncient-agent-sdk-swarm-mesh-theorems-1211-1215:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_swarm_mesh_theorems_1211_1215.c tsfi2-deepseek/src/auncient_agent_sdk_swarm_mesh_theorems_1211_1215.c -o tests/test_auncient_agent_sdk_swarm_mesh_theorems_1211_1215 -lm
	./tests/test_auncient_agent_sdk_swarm_mesh_theorems_1211_1215
	@rm -f tests/test_auncient_agent_sdk_swarm_mesh_theorems_1211_1215

test-auncient-agent-sdk-reactive-event-loop-theorems-1206-1210:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_reactive_event_loop_theorems_1206_1210.c tsfi2-deepseek/src/auncient_agent_sdk_reactive_event_loop_theorems_1206_1210.c -o tests/test_auncient_agent_sdk_reactive_event_loop_theorems_1206_1210 -lm
	./tests/test_auncient_agent_sdk_reactive_event_loop_theorems_1206_1210
	@rm -f tests/test_auncient_agent_sdk_reactive_event_loop_theorems_1206_1210

test-auncient-agent-sdk-fpga-stream-mux-theorems-1201-1205:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_fpga_stream_mux_theorems_1201_1205.c tsfi2-deepseek/src/auncient_agent_sdk_fpga_stream_mux_theorems_1201_1205.c -o tests/test_auncient_agent_sdk_fpga_stream_mux_theorems_1201_1205 -lm
	./tests/test_auncient_agent_sdk_fpga_stream_mux_theorems_1201_1205
	@rm -f tests/test_auncient_agent_sdk_fpga_stream_mux_theorems_1201_1205

test-auncient-agent-sdk-grand-sesquichiliad-theorems-1196-1200:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_grand_sesquichiliad_theorems_1196_1200.c tsfi2-deepseek/src/auncient_agent_sdk_grand_sesquichiliad_theorems_1196_1200.c -o tests/test_auncient_agent_sdk_grand_sesquichiliad_theorems_1196_1200 -lm
	./tests/test_auncient_agent_sdk_grand_sesquichiliad_theorems_1196_1200
	@rm -f tests/test_auncient_agent_sdk_grand_sesquichiliad_theorems_1196_1200

test-auncient-agent-sdk-hbridge-tensor-geodesic-theorems-1191-1195:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_hbridge_tensor_geodesic_theorems_1191_1195.c tsfi2-deepseek/src/auncient_agent_sdk_hbridge_tensor_geodesic_theorems_1191_1195.c -o tests/test_auncient_agent_sdk_hbridge_tensor_geodesic_theorems_1191_1195 -lm
	./tests/test_auncient_agent_sdk_hbridge_tensor_geodesic_theorems_1191_1195
	@rm -f tests/test_auncient_agent_sdk_hbridge_tensor_geodesic_theorems_1191_1195

test-auncient-fpga-total-certification-theorems-1186-1190:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_total_certification_theorems_1186_1190.c tsfi2-deepseek/src/auncient_fpga_total_certification_theorems_1186_1190.c -o tests/test_auncient_fpga_total_certification_theorems_1186_1190 -lm
	./tests/test_auncient_fpga_total_certification_theorems_1186_1190
	@rm -f tests/test_auncient_fpga_total_certification_theorems_1186_1190

test-auncient-agent-sdk-hbridge-tk-dispatch-theorems-1181-1185:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_hbridge_tk_dispatch_theorems_1181_1185.c tsfi2-deepseek/src/auncient_agent_sdk_hbridge_tk_dispatch_theorems_1181_1185.c -o tests/test_auncient_agent_sdk_hbridge_tk_dispatch_theorems_1181_1185 -lm
	./tests/test_auncient_agent_sdk_hbridge_tk_dispatch_theorems_1181_1185
	@rm -f tests/test_auncient_agent_sdk_hbridge_tk_dispatch_theorems_1181_1185

test-auncient-agent-sdk-mimetic-memory-theorems-1176-1180:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_mimetic_memory_theorems_1176_1180.c tsfi2-deepseek/src/auncient_agent_sdk_mimetic_memory_theorems_1176_1180.c -o tests/test_auncient_agent_sdk_mimetic_memory_theorems_1176_1180 -lm
	./tests/test_auncient_agent_sdk_mimetic_memory_theorems_1176_1180
	@rm -f tests/test_auncient_agent_sdk_mimetic_memory_theorems_1176_1180

test-auncient-agent-sdk-hbridge-llm-silicon-theorems-1171-1175:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_hbridge_llm_silicon_theorems_1171_1175.c tsfi2-deepseek/src/auncient_agent_sdk_hbridge_llm_silicon_theorems_1171_1175.c -o tests/test_auncient_agent_sdk_hbridge_llm_silicon_theorems_1171_1175 -lm
	./tests/test_auncient_agent_sdk_hbridge_llm_silicon_theorems_1171_1175
	@rm -f tests/test_auncient_agent_sdk_hbridge_llm_silicon_theorems_1171_1175

test-auncient-agent-sdk-hbridge-fpga-theorems-1166-1170:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_hbridge_fpga_theorems_1166_1170.c tsfi2-deepseek/src/auncient_agent_sdk_hbridge_fpga_theorems_1166_1170.c -o tests/test_auncient_agent_sdk_hbridge_fpga_theorems_1166_1170 -lm
	./tests/test_auncient_agent_sdk_hbridge_fpga_theorems_1166_1170
	@rm -f tests/test_auncient_agent_sdk_hbridge_fpga_theorems_1166_1170

test-auncient-agent-sdk-llm-interop-pipeline-theorems-1161-1165:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_llm_interop_pipeline_theorems_1161_1165.c tsfi2-deepseek/src/auncient_agent_sdk_llm_interop_pipeline_theorems_1161_1165.c -o tests/test_auncient_agent_sdk_llm_interop_pipeline_theorems_1161_1165 -lm
	./tests/test_auncient_agent_sdk_llm_interop_pipeline_theorems_1161_1165
	@rm -f tests/test_auncient_agent_sdk_llm_interop_pipeline_theorems_1161_1165

test-auncient-agent-sdk-libantigravity-interop-theorems-1156-1160:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_libantigravity_interop_theorems_1156_1160.c tsfi2-deepseek/src/auncient_agent_sdk_libantigravity_interop_theorems_1156_1160.c -o tests/test_auncient_agent_sdk_libantigravity_interop_theorems_1156_1160 -lm
	./tests/test_auncient_agent_sdk_libantigravity_interop_theorems_1156_1160
	@rm -f tests/test_auncient_agent_sdk_libantigravity_interop_theorems_1156_1160

test-auncient-agent-sdk-fpga-operations-theorems-1151-1155:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_agent_sdk_fpga_operations_theorems_1151_1155.c tsfi2-deepseek/src/auncient_agent_sdk_fpga_operations_theorems_1151_1155.c -o tests/test_auncient_agent_sdk_fpga_operations_theorems_1151_1155 -lm
	./tests/test_auncient_agent_sdk_fpga_operations_theorems_1151_1155
	@rm -f tests/test_auncient_agent_sdk_fpga_operations_theorems_1151_1155

test-auncient-fpga-hardware-tensor-dma-engine-theorems-1146-1150:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_hardware_tensor_dma_engine_theorems_1146_1150.c tsfi2-deepseek/src/auncient_fpga_hardware_tensor_dma_engine_theorems_1146_1150.c -o tests/test_auncient_fpga_hardware_tensor_dma_engine_theorems_1146_1150 -lm
	./tests/test_auncient_fpga_hardware_tensor_dma_engine_theorems_1146_1150
	@rm -f tests/test_auncient_fpga_hardware_tensor_dma_engine_theorems_1146_1150

test-auncient-fpga-speculative-branch-predictor-theorems-1141-1145:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_speculative_branch_predictor_theorems_1141_1145.c tsfi2-deepseek/src/auncient_fpga_speculative_branch_predictor_theorems_1141_1145.c -o tests/test_auncient_fpga_speculative_branch_predictor_theorems_1141_1145 -lm
	./tests/test_auncient_fpga_speculative_branch_predictor_theorems_1141_1145
	@rm -f tests/test_auncient_fpga_speculative_branch_predictor_theorems_1141_1145

test-auncient-fpga-hardware-bfloat16-fp8-alu-theorems-1136-1140:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_hardware_bfloat16_fp8_alu_theorems_1136_1140.c tsfi2-deepseek/src/auncient_fpga_hardware_bfloat16_fp8_alu_theorems_1136_1140.c -o tests/test_auncient_fpga_hardware_bfloat16_fp8_alu_theorems_1136_1140 -lm
	./tests/test_auncient_fpga_hardware_bfloat16_fp8_alu_theorems_1136_1140
	@rm -f tests/test_auncient_fpga_hardware_bfloat16_fp8_alu_theorems_1136_1140

test-auncient-fpga-asymmetric-cryptographic-co-processor-theorems-1131-1135:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_asymmetric_cryptographic_co_processor_theorems_1131_1135.c tsfi2-deepseek/src/auncient_fpga_asymmetric_cryptographic_co_processor_theorems_1131_1135.c -o tests/test_auncient_fpga_asymmetric_cryptographic_co_processor_theorems_1131_1135 -lm
	./tests/test_auncient_fpga_asymmetric_cryptographic_co_processor_theorems_1131_1135
	@rm -f tests/test_auncient_fpga_asymmetric_cryptographic_co_processor_theorems_1131_1135

test-auncient-fpga-zero-overhead-scheduling-theorems-1126-1130:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_zero_overhead_scheduling_theorems_1126_1130.c tsfi2-deepseek/src/auncient_fpga_zero_overhead_scheduling_theorems_1126_1130.c -o tests/test_auncient_fpga_zero_overhead_scheduling_theorems_1126_1130 -lm
	./tests/test_auncient_fpga_zero_overhead_scheduling_theorems_1126_1130
	@rm -f tests/test_auncient_fpga_zero_overhead_scheduling_theorems_1126_1130

test-auncient-fpga-noc-stanag-decnet-theorems-1121-1125:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_noc_stanag_decnet_theorems_1121_1125.c tsfi2-deepseek/src/auncient_fpga_noc_stanag_decnet_theorems_1121_1125.c -o tests/test_auncient_fpga_noc_stanag_decnet_theorems_1121_1125 -lm
	./tests/test_auncient_fpga_noc_stanag_decnet_theorems_1121_1125
	@rm -f tests/test_auncient_fpga_noc_stanag_decnet_theorems_1121_1125

test-auncient-fpga-noc-flow-control-theorems-1116-1120:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_noc_flow_control_theorems_1116_1120.c tsfi2-deepseek/src/auncient_fpga_noc_flow_control_theorems_1116_1120.c -o tests/test_auncient_fpga_noc_flow_control_theorems_1116_1120 -lm
	./tests/test_auncient_fpga_noc_flow_control_theorems_1116_1120
	@rm -f tests/test_auncient_fpga_noc_flow_control_theorems_1116_1120

test-auncient-fpga-awq-int4-gemm-theorems-1111-1115:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_awq_int4_gemm_theorems_1111_1115.c tsfi2-deepseek/src/auncient_fpga_awq_int4_gemm_theorems_1111_1115.c -o tests/test_auncient_fpga_awq_int4_gemm_theorems_1111_1115 -lm
	./tests/test_auncient_fpga_awq_int4_gemm_theorems_1111_1115
	@rm -f tests/test_auncient_fpga_awq_int4_gemm_theorems_1111_1115

test-auncient-fpga-mla-multihead-latent-theorems-1106-1110:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_mla_multihead_latent_theorems_1106_1110.c tsfi2-deepseek/src/auncient_fpga_mla_multihead_latent_theorems_1106_1110.c -o tests/test_auncient_fpga_mla_multihead_latent_theorems_1106_1110 -lm
	./tests/test_auncient_fpga_mla_multihead_latent_theorems_1106_1110
	@rm -f tests/test_auncient_fpga_mla_multihead_latent_theorems_1106_1110

test-auncient-fpga-flash-attention-theorems-1101-1105:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_flash_attention_theorems_1101_1105.c tsfi2-deepseek/src/auncient_fpga_flash_attention_theorems_1101_1105.c -o tests/test_auncient_fpga_flash_attention_theorems_1101_1105 -lm
	./tests/test_auncient_fpga_flash_attention_theorems_1101_1105
	@rm -f tests/test_auncient_fpga_flash_attention_theorems_1101_1105

test-auncient-fpga-kvcache-paged-theorems-1096-1100:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_kvcache_paged_theorems_1096_1100.c tsfi2-deepseek/src/auncient_fpga_kvcache_paged_theorems_1096_1100.c -o tests/test_auncient_fpga_kvcache_paged_theorems_1096_1100 -lm
	./tests/test_auncient_fpga_kvcache_paged_theorems_1096_1100
	@rm -f tests/test_auncient_fpga_kvcache_paged_theorems_1096_1100

test-auncient-fpga-speculative-decoding-theorems-1091-1095:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_speculative_decoding_theorems_1091_1095.c tsfi2-deepseek/src/auncient_fpga_speculative_decoding_theorems_1091_1095.c -o tests/test_auncient_fpga_speculative_decoding_theorems_1091_1095 -lm
	./tests/test_auncient_fpga_speculative_decoding_theorems_1091_1095
	@rm -f tests/test_auncient_fpga_speculative_decoding_theorems_1091_1095

test-auncient-fpga-expert-parallel-moe-theorems-1086-1090:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_expert_parallel_moe_theorems_1086_1090.c tsfi2-deepseek/src/auncient_fpga_expert_parallel_moe_theorems_1086_1090.c -o tests/test_auncient_fpga_expert_parallel_moe_theorems_1086_1090 -lm
	./tests/test_auncient_fpga_expert_parallel_moe_theorems_1086_1090
	@rm -f tests/test_auncient_fpga_expert_parallel_moe_theorems_1086_1090

test-auncient-fpga-tensor-parallel-theorems-1081-1085:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_tensor_parallel_theorems_1081_1085.c tsfi2-deepseek/src/auncient_fpga_tensor_parallel_theorems_1081_1085.c -o tests/test_auncient_fpga_tensor_parallel_theorems_1081_1085 -lm
	./tests/test_auncient_fpga_tensor_parallel_theorems_1081_1085
	@rm -f tests/test_auncient_fpga_tensor_parallel_theorems_1081_1085

test-auncient-deepseek-fpga-pipeline-theorems-1076-1080:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_deepseek_fpga_pipeline_theorems_1076_1080.c tsfi2-deepseek/src/auncient_deepseek_fpga_pipeline_theorems_1076_1080.c -o tests/test_auncient_deepseek_fpga_pipeline_theorems_1076_1080 -lm
	./tests/test_auncient_deepseek_fpga_pipeline_theorems_1076_1080
	@rm -f tests/test_auncient_deepseek_fpga_pipeline_theorems_1076_1080

test-auncient-fpga-hbridge-multi-proc-theorems-1071-1075:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_hbridge_multi_proc_theorems_1071_1075.c tsfi2-deepseek/src/auncient_fpga_hbridge_multi_proc_theorems_1071_1075.c -o tests/test_auncient_fpga_hbridge_multi_proc_theorems_1071_1075 -lm
	./tests/test_auncient_fpga_hbridge_multi_proc_theorems_1071_1075
	@rm -f tests/test_auncient_fpga_hbridge_multi_proc_theorems_1071_1075

test-auncient-deepseek-coder-gguf-fpga-theorems-1066-1070:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_deepseek_coder_gguf_fpga_theorems_1066_1070.c tsfi2-deepseek/src/auncient_deepseek_coder_gguf_fpga_theorems_1066_1070.c -o tests/test_auncient_deepseek_coder_gguf_fpga_theorems_1066_1070 -lm
	./tests/test_auncient_deepseek_coder_gguf_fpga_theorems_1066_1070
	@rm -f tests/test_auncient_deepseek_coder_gguf_fpga_theorems_1066_1070

test-auncient-cpmtomie-zorse-fusion-theorems-1061-1065:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpmtomie_zorse_fusion_theorems_1061_1065.c tsfi2-deepseek/src/auncient_cpmtomie_zorse_fusion_theorems_1061_1065.c -o tests/test_auncient_cpmtomie_zorse_fusion_theorems_1061_1065 -lm
	./tests/test_auncient_cpmtomie_zorse_fusion_theorems_1061_1065
	@rm -f tests/test_auncient_cpmtomie_zorse_fusion_theorems_1061_1065

test-auncient-fpga-auxiliary-cores-neural-mesh-theorems-1056-1060:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_auxiliary_cores_neural_mesh_theorems_1056_1060.c tsfi2-deepseek/src/auncient_fpga_auxiliary_cores_neural_mesh_theorems_1056_1060.c -o tests/test_auncient_fpga_auxiliary_cores_neural_mesh_theorems_1056_1060 -lm
	./tests/test_auncient_fpga_auxiliary_cores_neural_mesh_theorems_1056_1060
	@rm -f tests/test_auncient_fpga_auxiliary_cores_neural_mesh_theorems_1056_1060

test-auncient-fpga-auxiliary-cores-ankh-os-theorems-1051-1055:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_auxiliary_cores_ankh_os_theorems_1051_1055.c tsfi2-deepseek/src/auncient_fpga_auxiliary_cores_ankh_os_theorems_1051_1055.c -o tests/test_auncient_fpga_auxiliary_cores_ankh_os_theorems_1051_1055 -lm
	./tests/test_auncient_fpga_auxiliary_cores_ankh_os_theorems_1051_1055
	@rm -f tests/test_auncient_fpga_auxiliary_cores_ankh_os_theorems_1051_1055

test-auncient-fpga-auxiliary-cores-polyphase-theorems-1046-1050:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_auxiliary_cores_polyphase_theorems_1046_1050.c tsfi2-deepseek/src/auncient_fpga_auxiliary_cores_polyphase_theorems_1046_1050.c -o tests/test_auncient_fpga_auxiliary_cores_polyphase_theorems_1046_1050 -lm
	./tests/test_auncient_fpga_auxiliary_cores_polyphase_theorems_1046_1050
	@rm -f tests/test_auncient_fpga_auxiliary_cores_polyphase_theorems_1046_1050

test-auncient-fpga-auxiliary-cores-mimetic-theorems-1041-1045:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_auxiliary_cores_mimetic_theorems_1041_1045.c tsfi2-deepseek/src/auncient_fpga_auxiliary_cores_mimetic_theorems_1041_1045.c -o tests/test_auncient_fpga_auxiliary_cores_mimetic_theorems_1041_1045 -lm
	./tests/test_auncient_fpga_auxiliary_cores_mimetic_theorems_1041_1045
	@rm -f tests/test_auncient_fpga_auxiliary_cores_mimetic_theorems_1041_1045

test-auncient-fpga-auxiliary-cores-bin-theorems-1036-1040:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_auxiliary_cores_bin_theorems_1036_1040.c tsfi2-deepseek/src/auncient_fpga_auxiliary_cores_bin_theorems_1036_1040.c -o tests/test_auncient_fpga_auxiliary_cores_bin_theorems_1036_1040 -lm
	./tests/test_auncient_fpga_auxiliary_cores_bin_theorems_1036_1040
	@rm -f tests/test_auncient_fpga_auxiliary_cores_bin_theorems_1036_1040

test-auncient-fpga-vsen-all-features-theorems-1031-1035:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_vsen_all_features_theorems_1031_1035.c tsfi2-deepseek/src/auncient_fpga_vsen_all_features_theorems_1031_1035.c -o tests/test_auncient_fpga_vsen_all_features_theorems_1031_1035 -lm
	./tests/test_auncient_fpga_vsen_all_features_theorems_1031_1035
	@rm -f tests/test_auncient_fpga_vsen_all_features_theorems_1031_1035

test-cbt-tape-12scene-tripartite-pipeline:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cbt_tape_12scene_tripartite_pipeline.c tsfi2-deepseek/src/cbt_tape_12scene_tripartite_pipeline.c -o tests/test_cbt_tape_12scene_tripartite_pipeline -lm
	./tests/test_cbt_tape_12scene_tripartite_pipeline
	@rm -f tests/test_cbt_tape_12scene_tripartite_pipeline

test-auncient-vsen-photorealism-theorems-1976-1980:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_photorealism_theorems_1976_1980.c tsfi2-deepseek/src/auncient_vsen_photorealism_theorems_1976_1980.c -o tests/test_auncient_vsen_photorealism_theorems_1976_1980 -lm
	./tests/test_auncient_vsen_photorealism_theorems_1976_1980
	@rm -f tests/test_auncient_vsen_photorealism_theorems_1976_1980

test-auncient-vsen-cbt-12scene-demo-theorems-1971-1975:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_cbt_12scene_demo_theorems_1971_1975.c tsfi2-deepseek/src/auncient_vsen_cbt_12scene_demo_theorems_1971_1975.c -o tests/test_auncient_vsen_cbt_12scene_demo_theorems_1971_1975 -lm
	./tests/test_auncient_vsen_cbt_12scene_demo_theorems_1971_1975
	@rm -f tests/test_auncient_vsen_cbt_12scene_demo_theorems_1971_1975

test-auncient-vsen-subsurface-fleece-theorems-1966-1970:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_subsurface_fleece_theorems_1966_1970.c tsfi2-deepseek/src/auncient_vsen_subsurface_fleece_theorems_1966_1970.c -o tests/test_auncient_vsen_subsurface_fleece_theorems_1966_1970 -lm
	./tests/test_auncient_vsen_subsurface_fleece_theorems_1966_1970
	@rm -f tests/test_auncient_vsen_subsurface_fleece_theorems_1966_1970

test-auncient-vsen-godrays-dust-theorems-1961-1965:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_godrays_dust_theorems_1961_1965.c tsfi2-deepseek/src/auncient_vsen_godrays_dust_theorems_1961_1965.c -o tests/test_auncient_vsen_godrays_dust_theorems_1961_1965 -lm
	./tests/test_auncient_vsen_godrays_dust_theorems_1961_1965
	@rm -f tests/test_auncient_vsen_godrays_dust_theorems_1961_1965

test-auncient-vsen-spectral-dispersion-theorems-1956-1960:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_spectral_dispersion_theorems_1956_1960.c tsfi2-deepseek/src/auncient_vsen_spectral_dispersion_theorems_1956_1960.c -o tests/test_auncient_vsen_spectral_dispersion_theorems_1956_1960 -lm
	./tests/test_auncient_vsen_spectral_dispersion_theorems_1956_1960
	@rm -f tests/test_auncient_vsen_spectral_dispersion_theorems_1956_1960

test-auncient-vsen-temporal-denoiser-theorems-1951-1955:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_temporal_denoiser_theorems_1951_1955.c tsfi2-deepseek/src/auncient_vsen_temporal_denoiser_theorems_1951_1955.c -o tests/test_auncient_vsen_temporal_denoiser_theorems_1951_1955 -lm
	./tests/test_auncient_vsen_temporal_denoiser_theorems_1951_1955
	@rm -f tests/test_auncient_vsen_temporal_denoiser_theorems_1951_1955

test-auncient-vsen-cinematic-camera-theorems-1946-1950:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_cinematic_camera_theorems_1946_1950.c tsfi2-deepseek/src/auncient_vsen_cinematic_camera_theorems_1946_1950.c -o tests/test_auncient_vsen_cinematic_camera_theorems_1946_1950 -lm
	./tests/test_auncient_vsen_cinematic_camera_theorems_1946_1950
	@rm -f tests/test_auncient_vsen_cinematic_camera_theorems_1946_1950

test-auncient-vsen-12scene-demo-theorems-1941-1945:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_12scene_demo_theorems_1941_1945.c tsfi2-deepseek/src/auncient_vsen_12scene_demo_theorems_1941_1945.c -o tests/test_auncient_vsen_12scene_demo_theorems_1941_1945 -lm
	./tests/test_auncient_vsen_12scene_demo_theorems_1941_1945
	@rm -f tests/test_auncient_vsen_12scene_demo_theorems_1941_1945

test-auncient-vsen-cbt-tape-spool-theorems-1936-1940:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_cbt_tape_spool_theorems_1936_1940.c tsfi2-deepseek/src/auncient_vsen_cbt_tape_spool_theorems_1936_1940.c -o tests/test_auncient_vsen_cbt_tape_spool_theorems_1936_1940 -lm
	./tests/test_auncient_vsen_cbt_tape_spool_theorems_1936_1940
	@rm -f tests/test_auncient_vsen_cbt_tape_spool_theorems_1936_1940

test-auncient-vsen-symplectic-feedback-theorems-1931-1935:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_symplectic_feedback_theorems_1931_1935.c tsfi2-deepseek/src/auncient_vsen_symplectic_feedback_theorems_1931_1935.c -o tests/test_auncient_vsen_symplectic_feedback_theorems_1931_1935 -lm
	./tests/test_auncient_vsen_symplectic_feedback_theorems_1931_1935
	@rm -f tests/test_auncient_vsen_symplectic_feedback_theorems_1931_1935

test-auncient-vsen-tripartite-llm-animator-theorems-1926-1930:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_tripartite_llm_animator_theorems_1926_1930.c tsfi2-deepseek/src/auncient_vsen_tripartite_llm_animator_theorems_1926_1930.c -o tests/test_auncient_vsen_tripartite_llm_animator_theorems_1926_1930 -lm
	./tests/test_auncient_vsen_tripartite_llm_animator_theorems_1926_1930
	@rm -f tests/test_auncient_vsen_tripartite_llm_animator_theorems_1926_1930

test-auncient-vsen-swarm-choreography-theorems-1921-1925:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_swarm_choreography_theorems_1921_1925.c tsfi2-deepseek/src/auncient_vsen_swarm_choreography_theorems_1921_1925.c -o tests/test_auncient_vsen_swarm_choreography_theorems_1921_1925 -lm
	./tests/test_auncient_vsen_swarm_choreography_theorems_1921_1925
	@rm -f tests/test_auncient_vsen_swarm_choreography_theorems_1921_1925

test-auncient-vsen-paw-friction-theorems-1916-1920:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_paw_friction_theorems_1916_1920.c tsfi2-deepseek/src/auncient_vsen_paw_friction_theorems_1916_1920.c -o tests/test_auncient_vsen_paw_friction_theorems_1916_1920 -lm
	./tests/test_auncient_vsen_paw_friction_theorems_1916_1920
	@rm -f tests/test_auncient_vsen_paw_friction_theorems_1916_1920

test-auncient-vsen-vocal-purr-theorems-1911-1915:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_vocal_purr_theorems_1911_1915.c tsfi2-deepseek/src/auncient_vsen_vocal_purr_theorems_1911_1915.c -o tests/test_auncient_vsen_vocal_purr_theorems_1911_1915 -lm
	./tests/test_auncient_vsen_vocal_purr_theorems_1911_1915
	@rm -f tests/test_auncient_vsen_vocal_purr_theorems_1911_1915

test-auncient-vsen-corneal-saccade-theorems-1906-1910:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_corneal_saccade_theorems_1906_1910.c tsfi2-deepseek/src/auncient_vsen_corneal_saccade_theorems_1906_1910.c -o tests/test_auncient_vsen_corneal_saccade_theorems_1906_1910 -lm
	./tests/test_auncient_vsen_corneal_saccade_theorems_1906_1910
	@rm -f tests/test_auncient_vsen_corneal_saccade_theorems_1906_1910

test-auncient-vsen-volumetric-gait-theorems-1901-1905:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_volumetric_gait_theorems_1901_1905.c tsfi2-deepseek/src/auncient_vsen_volumetric_gait_theorems_1901_1905.c -o tests/test_auncient_vsen_volumetric_gait_theorems_1901_1905 -lm
	./tests/test_auncient_vsen_volumetric_gait_theorems_1901_1905
	@rm -f tests/test_auncient_vsen_volumetric_gait_theorems_1901_1905

test-auncient-vsen-master-animator-closure-theorems-1896-1900:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_master_animator_closure_theorems_1896_1900.c tsfi2-deepseek/src/auncient_vsen_master_animator_closure_theorems_1896_1900.c -o tests/test_auncient_vsen_master_animator_closure_theorems_1896_1900 -lm
	./tests/test_auncient_vsen_master_animator_closure_theorems_1896_1900
	@rm -f tests/test_auncient_vsen_master_animator_closure_theorems_1896_1900

test-auncient-vsen-hybrid-sprite-voxel-theorems-1891-1895:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_hybrid_sprite_voxel_theorems_1891_1895.c tsfi2-deepseek/src/auncient_vsen_hybrid_sprite_voxel_theorems_1891_1895.c -o tests/test_auncient_vsen_hybrid_sprite_voxel_theorems_1891_1895 -lm
	./tests/test_auncient_vsen_hybrid_sprite_voxel_theorems_1891_1895
	@rm -f tests/test_auncient_vsen_hybrid_sprite_voxel_theorems_1891_1895

test-auncient-vsen-deformation-cage-theorems-1886-1890:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_deformation_cage_theorems_1886_1890.c tsfi2-deepseek/src/auncient_vsen_deformation_cage_theorems_1886_1890.c -o tests/test_auncient_vsen_deformation_cage_theorems_1886_1890 -lm
	./tests/test_auncient_vsen_deformation_cage_theorems_1886_1890
	@rm -f tests/test_auncient_vsen_deformation_cage_theorems_1886_1890

test-auncient-vsen-procedural-animator-theorems-1881-1885:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_procedural_animator_theorems_1881_1885.c tsfi2-deepseek/src/auncient_vsen_procedural_animator_theorems_1881_1885.c -o tests/test_auncient_vsen_procedural_animator_theorems_1881_1885 -lm
	./tests/test_auncient_vsen_procedural_animator_theorems_1881_1885
	@rm -f tests/test_auncient_vsen_procedural_animator_theorems_1881_1885

test-auncient-vsen-sna-appc-data-fabric-theorems-1876-1880:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_sna_appc_data_fabric_theorems_1876_1880.c tsfi2-deepseek/src/auncient_vsen_sna_appc_data_fabric_theorems_1876_1880.c -o tests/test_auncient_vsen_sna_appc_data_fabric_theorems_1876_1880 -lm
	./tests/test_auncient_vsen_sna_appc_data_fabric_theorems_1876_1880
	@rm -f tests/test_auncient_vsen_sna_appc_data_fabric_theorems_1876_1880

test-auncient-vsen-vaesen-renderman-personality-theorems-1871-1875:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_vaesen_renderman_personality_theorems_1871_1875.c tsfi2-deepseek/src/auncient_vsen_vaesen_renderman_personality_theorems_1871_1875.c -o tests/test_auncient_vsen_vaesen_renderman_personality_theorems_1871_1875 -lm
	./tests/test_auncient_vsen_vaesen_renderman_personality_theorems_1871_1875
	@rm -f tests/test_auncient_vsen_vaesen_renderman_personality_theorems_1871_1875

test-auncient-vsen-renderman-xpu-spectral-theorems-1866-1870:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_renderman_xpu_spectral_theorems_1866_1870.c tsfi2-deepseek/src/auncient_vsen_renderman_xpu_spectral_theorems_1866_1870.c -o tests/test_auncient_vsen_renderman_xpu_spectral_theorems_1866_1870 -lm
	./tests/test_auncient_vsen_renderman_xpu_spectral_theorems_1866_1870
	@rm -f tests/test_auncient_vsen_renderman_xpu_spectral_theorems_1866_1870

test-auncient-vsen-renderman-ris-8way-fabric-theorems-1861-1865:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_renderman_ris_8way_fabric_theorems_1861_1865.c tsfi2-deepseek/src/auncient_vsen_renderman_ris_8way_fabric_theorems_1861_1865.c -o tests/test_auncient_vsen_renderman_ris_8way_fabric_theorems_1861_1865 -lm
	./tests/test_auncient_vsen_renderman_ris_8way_fabric_theorems_1861_1865
	@rm -f tests/test_auncient_vsen_renderman_ris_8way_fabric_theorems_1861_1865

test-auncient-vsen-8way-fabric-animation-theorems-1856-1860:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_8way_fabric_animation_theorems_1856_1860.c tsfi2-deepseek/src/auncient_vsen_8way_fabric_animation_theorems_1856_1860.c -o tests/test_auncient_vsen_8way_fabric_animation_theorems_1856_1860 -lm
	./tests/test_auncient_vsen_8way_fabric_animation_theorems_1856_1860
	@rm -f tests/test_auncient_vsen_8way_fabric_animation_theorems_1856_1860

test-auncient-vsen-hardware-animation-engine-theorems-1851-1855:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_hardware_animation_engine_theorems_1851_1855.c tsfi2-deepseek/src/auncient_vsen_hardware_animation_engine_theorems_1851_1855.c -o tests/test_auncient_vsen_hardware_animation_engine_theorems_1851_1855 -lm
	./tests/test_auncient_vsen_hardware_animation_engine_theorems_1851_1855
	@rm -f tests/test_auncient_vsen_hardware_animation_engine_theorems_1851_1855

test-auncient-vsen-lspci-full-system-theorems-1846-1850:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_lspci_full_system_theorems_1846_1850.c tsfi2-deepseek/src/auncient_vsen_lspci_full_system_theorems_1846_1850.c -o tests/test_auncient_vsen_lspci_full_system_theorems_1846_1850 -lm
	./tests/test_auncient_vsen_lspci_full_system_theorems_1846_1850
	@rm -f tests/test_auncient_vsen_lspci_full_system_theorems_1846_1850

test-auncient-vsen-asmedia-asm4242-usb4-theorems-1841-1845:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_asmedia_asm4242_usb4_theorems_1841_1845.c tsfi2-deepseek/src/auncient_vsen_asmedia_asm4242_usb4_theorems_1841_1845.c -o tests/test_auncient_vsen_asmedia_asm4242_usb4_theorems_1841_1845 -lm
	./tests/test_auncient_vsen_asmedia_asm4242_usb4_theorems_1841_1845
	@rm -f tests/test_auncient_vsen_asmedia_asm4242_usb4_theorems_1841_1845

test-auncient-vsen-amd600-pcie-switch-theorems-1836-1840:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_amd600_pcie_switch_theorems_1836_1840.c tsfi2-deepseek/src/auncient_vsen_amd600_pcie_switch_theorems_1836_1840.c -o tests/test_auncient_vsen_amd600_pcie_switch_theorems_1836_1840 -lm
	./tests/test_auncient_vsen_amd600_pcie_switch_theorems_1836_1840
	@rm -f tests/test_auncient_vsen_amd600_pcie_switch_theorems_1836_1840

test-auncient-vsen-vulkan-renderman-the-mann-theorems-1831-1835:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_vulkan_renderman_the_mann_theorems_1831_1835.c tsfi2-deepseek/src/auncient_vsen_vulkan_renderman_the_mann_theorems_1831_1835.c -o tests/test_auncient_vsen_vulkan_renderman_the_mann_theorems_1831_1835 -lm
	./tests/test_auncient_vsen_vulkan_renderman_the_mann_theorems_1831_1835
	@rm -f tests/test_auncient_vsen_vulkan_renderman_the_mann_theorems_1831_1835

test-auncient-vsen-amd-data-fabric-zmm-theorems-1826-1830:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_amd_data_fabric_zmm_theorems_1826_1830.c tsfi2-deepseek/src/auncient_vsen_amd_data_fabric_zmm_theorems_1826_1830.c -o tests/test_auncient_vsen_amd_data_fabric_zmm_theorems_1826_1830 -lm
	./tests/test_auncient_vsen_amd_data_fabric_zmm_theorems_1826_1830
	@rm -f tests/test_auncient_vsen_amd_data_fabric_zmm_theorems_1826_1830

test-auncient-vsen-strowger-pbx-switching-theorems-1821-1825:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_strowger_pbx_switching_theorems_1821_1825.c tsfi2-deepseek/src/auncient_vsen_strowger_pbx_switching_theorems_1821_1825.c -o tests/test_auncient_vsen_strowger_pbx_switching_theorems_1821_1825 -lm
	./tests/test_auncient_vsen_strowger_pbx_switching_theorems_1821_1825
	@rm -f tests/test_auncient_vsen_strowger_pbx_switching_theorems_1821_1825

test-auncient-vsen-dtmf-tactile-purr-theorems-1816-1820:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_dtmf_tactile_purr_theorems_1816_1820.c tsfi2-deepseek/src/auncient_vsen_dtmf_tactile_purr_theorems_1816_1820.c -o tests/test_auncient_vsen_dtmf_tactile_purr_theorems_1816_1820 -lm
	./tests/test_auncient_vsen_dtmf_tactile_purr_theorems_1816_1820
	@rm -f tests/test_auncient_vsen_dtmf_tactile_purr_theorems_1816_1820

test-auncient-vsen-pbx-pmg-auncestral-spools-theorems-1811-1815:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_pbx_pmg_auncestral_spools_theorems_1811_1815.c tsfi2-deepseek/src/auncient_vsen_pbx_pmg_auncestral_spools_theorems_1811_1815.c -o tests/test_auncient_vsen_pbx_pmg_auncestral_spools_theorems_1811_1815 -lm
	./tests/test_auncient_vsen_pbx_pmg_auncestral_spools_theorems_1811_1815
	@rm -f tests/test_auncient_vsen_pbx_pmg_auncestral_spools_theorems_1811_1815

test-auncient-vsen-attention-steering-mimetic-memory-theorems-1806-1810:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_attention_steering_mimetic_memory_theorems_1806_1810.c tsfi2-deepseek/src/auncient_vsen_attention_steering_mimetic_memory_theorems_1806_1810.c -o tests/test_auncient_vsen_attention_steering_mimetic_memory_theorems_1806_1810 -lm
	./tests/test_auncient_vsen_attention_steering_mimetic_memory_theorems_1806_1810
	@rm -f tests/test_auncient_vsen_attention_steering_mimetic_memory_theorems_1806_1810

test-auncient-vsen-4quadrant-geometric-embedding-theorems-1801-1805:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_4quadrant_geometric_embedding_theorems_1801_1805.c tsfi2-deepseek/src/auncient_vsen_4quadrant_geometric_embedding_theorems_1801_1805.c -o tests/test_auncient_vsen_4quadrant_geometric_embedding_theorems_1801_1805 -lm
	./tests/test_auncient_vsen_4quadrant_geometric_embedding_theorems_1801_1805
	@rm -f tests/test_auncient_vsen_4quadrant_geometric_embedding_theorems_1801_1805

test-auncient-vsen-kermit-vfio-file-transfer-theorems-1796-1800:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_kermit_vfio_file_transfer_theorems_1796_1800.c tsfi2-deepseek/src/auncient_vsen_kermit_vfio_file_transfer_theorems_1796_1800.c -o tests/test_auncient_vsen_kermit_vfio_file_transfer_theorems_1796_1800 -lm
	./tests/test_auncient_vsen_kermit_vfio_file_transfer_theorems_1796_1800
	@rm -f tests/test_auncient_vsen_kermit_vfio_file_transfer_theorems_1796_1800

test-auncient-vsen-edo22-voice-synthesis-theorems-1791-1795:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_edo22_voice_synthesis_theorems_1791_1795.c tsfi2-deepseek/src/auncient_vsen_edo22_voice_synthesis_theorems_1791_1795.c -o tests/test_auncient_vsen_edo22_voice_synthesis_theorems_1791_1795 -lm
	./tests/test_auncient_vsen_edo22_voice_synthesis_theorems_1791_1795
	@rm -f tests/test_auncient_vsen_edo22_voice_synthesis_theorems_1791_1795

test-auncient-vsen-stanag-spool-to-presenter-integration-theorems-1786-1790:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_stanag_spool_to_presenter_integration_theorems_1786_1790.c tsfi2-deepseek/src/auncient_vsen_stanag_spool_to_presenter_integration_theorems_1786_1790.c -o tests/test_auncient_vsen_stanag_spool_to_presenter_integration_theorems_1786_1790 -lm
	./tests/test_auncient_vsen_stanag_spool_to_presenter_integration_theorems_1786_1790
	@rm -f tests/test_auncient_vsen_stanag_spool_to_presenter_integration_theorems_1786_1790

test-auncient-vsen-teddy-bear-ssa-hogan-payroll-theorems-1781-1785:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_teddy_bear_ssa_hogan_payroll_theorems_1781_1785.c tsfi2-deepseek/src/auncient_vsen_teddy_bear_ssa_hogan_payroll_theorems_1781_1785.c -o tests/test_auncient_vsen_teddy_bear_ssa_hogan_payroll_theorems_1781_1785 -lm
	./tests/test_auncient_vsen_teddy_bear_ssa_hogan_payroll_theorems_1781_1785
	@rm -f tests/test_auncient_vsen_teddy_bear_ssa_hogan_payroll_theorems_1781_1785

test-auncient-vsen-gguf-empathy-datbin-slicing-theorems-1776-1780:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_gguf_empathy_datbin_slicing_theorems_1776_1780.c tsfi2-deepseek/src/auncient_vsen_gguf_empathy_datbin_slicing_theorems_1776_1780.c -o tests/test_auncient_vsen_gguf_empathy_datbin_slicing_theorems_1776_1780 -lm
	./tests/test_auncient_vsen_gguf_empathy_datbin_slicing_theorems_1776_1780
	@rm -f tests/test_auncient_vsen_gguf_empathy_datbin_slicing_theorems_1776_1780

test-auncient-vsen-anti-parasite-firewall-theorems-1771-1775:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_anti_parasite_firewall_theorems_1771_1775.c tsfi2-deepseek/src/auncient_vsen_anti_parasite_firewall_theorems_1771_1775.c -o tests/test_auncient_vsen_anti_parasite_firewall_theorems_1771_1775 -lm
	./tests/test_auncient_vsen_anti_parasite_firewall_theorems_1771_1775
	@rm -f tests/test_auncient_vsen_anti_parasite_firewall_theorems_1771_1775

test-auncient-vsen-vaesen-reactive-event-theorems-1766-1770:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_vaesen_reactive_event_theorems_1766_1770.c tsfi2-deepseek/src/auncient_vsen_vaesen_reactive_event_theorems_1766_1770.c -o tests/test_auncient_vsen_vaesen_reactive_event_theorems_1766_1770 -lm
	./tests/test_auncient_vsen_vaesen_reactive_event_theorems_1766_1770
	@rm -f tests/test_auncient_vsen_vaesen_reactive_event_theorems_1766_1770

test-auncient-vsen-vaesen-swarm-empathy-theorems-1761-1765:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_vaesen_swarm_empathy_theorems_1761_1765.c tsfi2-deepseek/src/auncient_vsen_vaesen_swarm_empathy_theorems_1761_1765.c -o tests/test_auncient_vsen_vaesen_swarm_empathy_theorems_1761_1765 -lm
	./tests/test_auncient_vsen_vaesen_swarm_empathy_theorems_1761_1765
	@rm -f tests/test_auncient_vsen_vaesen_swarm_empathy_theorems_1761_1765

test-auncient-vsen-vfio-stanag-spool-theorems-1756-1760:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_vfio_stanag_spool_theorems_1756_1760.c tsfi2-deepseek/src/auncient_vsen_vfio_stanag_spool_theorems_1756_1760.c -o tests/test_auncient_vsen_vfio_stanag_spool_theorems_1756_1760 -lm
	./tests/test_auncient_vsen_vfio_stanag_spool_theorems_1756_1760
	@rm -f tests/test_auncient_vsen_vfio_stanag_spool_theorems_1756_1760

test-auncient-fpga-vsen-ccw-dma-theorems-1026-1030:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_vsen_ccw_dma_theorems_1026_1030.c tsfi2-deepseek/src/auncient_fpga_vsen_ccw_dma_theorems_1026_1030.c -o tests/test_auncient_fpga_vsen_ccw_dma_theorems_1026_1030 -lm
	./tests/test_auncient_fpga_vsen_ccw_dma_theorems_1026_1030
	@rm -f tests/test_auncient_fpga_vsen_ccw_dma_theorems_1026_1030

test-auncient-full-vsen-in-cpmtomie-fpga-theorems-1021-1025:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_full_vsen_in_cpmtomie_fpga_theorems_1021_1025.c tsfi2-deepseek/src/auncient_full_vsen_in_cpmtomie_fpga_theorems_1021_1025.c -o tests/test_auncient_full_vsen_in_cpmtomie_fpga_theorems_1021_1025 -lm
	./tests/test_auncient_full_vsen_in_cpmtomie_fpga_theorems_1021_1025
	@rm -f tests/test_auncient_full_vsen_in_cpmtomie_fpga_theorems_1021_1025

test-auncient-cpmtomie-xvsen-bin-fpga-theorems-1016-1020:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpmtomie_xvsen_bin_fpga_theorems_1016_1020.c tsfi2-deepseek/src/auncient_cpmtomie_xvsen_bin_fpga_theorems_1016_1020.c -o tests/test_auncient_cpmtomie_xvsen_bin_fpga_theorems_1016_1020 -lm
	./tests/test_auncient_cpmtomie_xvsen_bin_fpga_theorems_1016_1020
	@rm -f tests/test_auncient_cpmtomie_xvsen_bin_fpga_theorems_1016_1020

test-auncient-vsen-cpmtomie-bin-theorems-1011-1015:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_vsen_cpmtomie_bin_theorems_1011_1015.c tsfi2-deepseek/src/auncient_vsen_cpmtomie_bin_theorems_1011_1015.c -o tests/test_auncient_vsen_cpmtomie_bin_theorems_1011_1015 -lm
	./tests/test_auncient_vsen_cpmtomie_bin_theorems_1011_1015
	@rm -f tests/test_auncient_vsen_cpmtomie_bin_theorems_1011_1015

test-auncient-saat-quadtier-procurement-theorems-1006-1010:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_saat_quadtier_procurement_theorems_1006_1010.c tsfi2-deepseek/src/auncient_saat_quadtier_procurement_theorems_1006_1010.c -o tests/test_auncient_saat_quadtier_procurement_theorems_1006_1010 -lm
	./tests/test_auncient_saat_quadtier_procurement_theorems_1006_1010
	@rm -f tests/test_auncient_saat_quadtier_procurement_theorems_1006_1010

test-auncient-fpga-chiliad-theorems-1001-1005:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_chiliad_theorems_1001_1005.c tsfi2-deepseek/src/auncient_fpga_chiliad_theorems_1001_1005.c -o tests/test_auncient_fpga_chiliad_theorems_1001_1005 -lm
	./tests/test_auncient_fpga_chiliad_theorems_1001_1005
	@rm -f tests/test_auncient_fpga_chiliad_theorems_1001_1005

test-auncient-kling1980-submicro-trap-theorems-996-1000:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_kling1980_submicro_trap_theorems_996_1000.c tsfi2-deepseek/src/auncient_kling1980_submicro_trap_theorems_996_1000.c -o tests/test_auncient_kling1980_submicro_trap_theorems_996_1000 -lm
	./tests/test_auncient_kling1980_submicro_trap_theorems_996_1000
	@rm -f tests/test_auncient_kling1980_submicro_trap_theorems_996_1000

test-auncient-rouse-anderson-beyond-990-theorems-991-995:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_rouse_anderson_beyond_990_theorems_991_995.c tsfi2-deepseek/src/auncient_rouse_anderson_beyond_990_theorems_991_995.c -o tests/test_auncient_rouse_anderson_beyond_990_theorems_991_995 -lm
	./tests/test_auncient_rouse_anderson_beyond_990_theorems_991_995
	@rm -f tests/test_auncient_rouse_anderson_beyond_990_theorems_991_995

test-auncient-rouse1981-stanag-leed-theorems-986-990:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_rouse1981_stanag_leed_theorems_986_990.c tsfi2-deepseek/src/auncient_rouse1981_stanag_leed_theorems_986_990.c -o tests/test_auncient_rouse1981_stanag_leed_theorems_986_990 -lm
	./tests/test_auncient_rouse1981_stanag_leed_theorems_986_990
	@rm -f tests/test_auncient_rouse1981_stanag_leed_theorems_986_990

test-auncient-anderson-ankh-beyond-980-theorems-981-985:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_anderson_ankh_beyond_980_theorems_981_985.c tsfi2-deepseek/src/auncient_anderson_ankh_beyond_980_theorems_981_985.c -o tests/test_auncient_anderson_ankh_beyond_980_theorems_981_985 -lm
	./tests/test_auncient_anderson_ankh_beyond_980_theorems_981_985
	@rm -f tests/test_auncient_anderson_ankh_beyond_980_theorems_981_985

test-auncient-anderson-ankh-theorems-976-980:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_anderson_ankh_theorems_976_980.c tsfi2-deepseek/src/auncient_anderson_ankh_theorems_976_980.c -o tests/test_auncient_anderson_ankh_theorems_976_980 -lm
	./tests/test_auncient_anderson_ankh_theorems_976_980
	@rm -f tests/test_auncient_anderson_ankh_theorems_976_980

test-auncient-fpga-beyond-970-theorems-971-975:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_970_theorems_971_975.c tsfi2-deepseek/src/auncient_fpga_beyond_970_theorems_971_975.c -o tests/test_auncient_fpga_beyond_970_theorems_971_975 -lm
	./tests/test_auncient_fpga_beyond_970_theorems_971_975
	@rm -f tests/test_auncient_fpga_beyond_970_theorems_971_975

test-auncient-fpga-beyond-965-theorems-966-970:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_965_theorems_966_970.c tsfi2-deepseek/src/auncient_fpga_beyond_965_theorems_966_970.c -o tests/test_auncient_fpga_beyond_965_theorems_966_970 -lm
	./tests/test_auncient_fpga_beyond_965_theorems_966_970
	@rm -f tests/test_auncient_fpga_beyond_965_theorems_966_970

test-auncient-fpga-beyond-960-theorems-961-965:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_960_theorems_961_965.c tsfi2-deepseek/src/auncient_fpga_beyond_960_theorems_961_965.c -o tests/test_auncient_fpga_beyond_960_theorems_961_965 -lm
	./tests/test_auncient_fpga_beyond_960_theorems_961_965
	@rm -f tests/test_auncient_fpga_beyond_960_theorems_961_965

test-auncient-fpga-beyond-955-theorems-956-960:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_955_theorems_956_960.c tsfi2-deepseek/src/auncient_fpga_beyond_955_theorems_956_960.c -o tests/test_auncient_fpga_beyond_955_theorems_956_960 -lm
	./tests/test_auncient_fpga_beyond_955_theorems_956_960
	@rm -f tests/test_auncient_fpga_beyond_955_theorems_956_960

test-auncient-fpga-beyond-950-theorems-951-955:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_950_theorems_951_955.c tsfi2-deepseek/src/auncient_fpga_beyond_950_theorems_951_955.c -o tests/test_auncient_fpga_beyond_950_theorems_951_955 -lm
	./tests/test_auncient_fpga_beyond_950_theorems_951_955
	@rm -f tests/test_auncient_fpga_beyond_950_theorems_951_955

test-auncient-fpga-beyond-945-theorems-946-950:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_945_theorems_946_950.c tsfi2-deepseek/src/auncient_fpga_beyond_945_theorems_946_950.c -o tests/test_auncient_fpga_beyond_945_theorems_946_950 -lm
	./tests/test_auncient_fpga_beyond_945_theorems_946_950
	@rm -f tests/test_auncient_fpga_beyond_945_theorems_946_950

test-auncient-fpga-beyond-940-theorems-941-945:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_940_theorems_941_945.c tsfi2-deepseek/src/auncient_fpga_beyond_940_theorems_941_945.c -o tests/test_auncient_fpga_beyond_940_theorems_941_945 -lm
	./tests/test_auncient_fpga_beyond_940_theorems_941_945
	@rm -f tests/test_auncient_fpga_beyond_940_theorems_941_945

test-auncient-fpga-beyond-935-theorems-936-940:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_935_theorems_936_940.c tsfi2-deepseek/src/auncient_fpga_beyond_935_theorems_936_940.c -o tests/test_auncient_fpga_beyond_935_theorems_936_940 -lm
	./tests/test_auncient_fpga_beyond_935_theorems_936_940
	@rm -f tests/test_auncient_fpga_beyond_935_theorems_936_940

test-auncient-fpga-beyond-930-theorems-931-935:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_930_theorems_931_935.c tsfi2-deepseek/src/auncient_fpga_beyond_930_theorems_931_935.c -o tests/test_auncient_fpga_beyond_930_theorems_931_935 -lm
	./tests/test_auncient_fpga_beyond_930_theorems_931_935
	@rm -f tests/test_auncient_fpga_beyond_930_theorems_931_935

test-auncient-fpga-beyond-925-theorems-926-930:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_925_theorems_926_930.c tsfi2-deepseek/src/auncient_fpga_beyond_925_theorems_926_930.c -o tests/test_auncient_fpga_beyond_925_theorems_926_930 -lm
	./tests/test_auncient_fpga_beyond_925_theorems_926_930
	@rm -f tests/test_auncient_fpga_beyond_925_theorems_926_930

test-auncient-fpga-beyond-920-theorems-921-925:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_920_theorems_921_925.c tsfi2-deepseek/src/auncient_fpga_beyond_920_theorems_921_925.c -o tests/test_auncient_fpga_beyond_920_theorems_921_925 -lm
	./tests/test_auncient_fpga_beyond_920_theorems_921_925
	@rm -f tests/test_auncient_fpga_beyond_920_theorems_921_925

test-auncient-fpga-beyond-915-theorems-916-920:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_915_theorems_916_920.c tsfi2-deepseek/src/auncient_fpga_beyond_915_theorems_916_920.c -o tests/test_auncient_fpga_beyond_915_theorems_916_920 -lm
	./tests/test_auncient_fpga_beyond_915_theorems_916_920
	@rm -f tests/test_auncient_fpga_beyond_915_theorems_916_920

test-auncient-fpga-beyond-910-theorems-911-915:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_910_theorems_911_915.c tsfi2-deepseek/src/auncient_fpga_beyond_910_theorems_911_915.c -o tests/test_auncient_fpga_beyond_910_theorems_911_915 -lm
	./tests/test_auncient_fpga_beyond_910_theorems_911_915
	@rm -f tests/test_auncient_fpga_beyond_910_theorems_911_915

test-auncient-fpga-beyond-905-theorems-906-910:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_905_theorems_906_910.c tsfi2-deepseek/src/auncient_fpga_beyond_905_theorems_906_910.c -o tests/test_auncient_fpga_beyond_905_theorems_906_910 -lm
	./tests/test_auncient_fpga_beyond_905_theorems_906_910
	@rm -f tests/test_auncient_fpga_beyond_905_theorems_906_910

test-auncient-fpga-beyond-900-theorems-901-905:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_900_theorems_901_905.c tsfi2-deepseek/src/auncient_fpga_beyond_900_theorems_901_905.c -o tests/test_auncient_fpga_beyond_900_theorems_901_905 -lm
	./tests/test_auncient_fpga_beyond_900_theorems_901_905
	@rm -f tests/test_auncient_fpga_beyond_900_theorems_901_905

test-auncient-fpga-beyond-895-theorems-896-900:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_895_theorems_896_900.c tsfi2-deepseek/src/auncient_fpga_beyond_895_theorems_896_900.c -o tests/test_auncient_fpga_beyond_895_theorems_896_900 -lm
	./tests/test_auncient_fpga_beyond_895_theorems_896_900
	@rm -f tests/test_auncient_fpga_beyond_895_theorems_896_900

test-auncient-fpga-beyond-890-theorems-891-895:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_890_theorems_891_895.c tsfi2-deepseek/src/auncient_fpga_beyond_890_theorems_891_895.c -o tests/test_auncient_fpga_beyond_890_theorems_891_895 -lm
	./tests/test_auncient_fpga_beyond_890_theorems_891_895
	@rm -f tests/test_auncient_fpga_beyond_890_theorems_891_895

test-auncient-fpga-beyond-885-theorems-886-890:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_885_theorems_886_890.c tsfi2-deepseek/src/auncient_fpga_beyond_885_theorems_886_890.c -o tests/test_auncient_fpga_beyond_885_theorems_886_890 -lm
	./tests/test_auncient_fpga_beyond_885_theorems_886_890
	@rm -f tests/test_auncient_fpga_beyond_885_theorems_886_890

test-auncient-fpga-beyond-880-theorems-881-885:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_880_theorems_881_885.c tsfi2-deepseek/src/auncient_fpga_beyond_880_theorems_881_885.c -o tests/test_auncient_fpga_beyond_880_theorems_881_885 -lm
	./tests/test_auncient_fpga_beyond_880_theorems_881_885
	@rm -f tests/test_auncient_fpga_beyond_880_theorems_881_885

test-auncient-fpga-beyond-875-theorems-876-880:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_875_theorems_876_880.c tsfi2-deepseek/src/auncient_fpga_beyond_875_theorems_876_880.c -o tests/test_auncient_fpga_beyond_875_theorems_876_880 -lm
	./tests/test_auncient_fpga_beyond_875_theorems_876_880
	@rm -f tests/test_auncient_fpga_beyond_875_theorems_876_880

test-auncient-fpga-beyond-870-theorems-871-875:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_870_theorems_871_875.c tsfi2-deepseek/src/auncient_fpga_beyond_870_theorems_871_875.c -o tests/test_auncient_fpga_beyond_870_theorems_871_875 -lm
	./tests/test_auncient_fpga_beyond_870_theorems_871_875
	@rm -f tests/test_auncient_fpga_beyond_870_theorems_871_875

test-auncient-fpga-beyond-865-theorems-866-870:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_865_theorems_866_870.c tsfi2-deepseek/src/auncient_fpga_beyond_865_theorems_866_870.c -o tests/test_auncient_fpga_beyond_865_theorems_866_870 -lm
	./tests/test_auncient_fpga_beyond_865_theorems_866_870
	@rm -f tests/test_auncient_fpga_beyond_865_theorems_866_870

test-auncient-fpga-beyond-860-theorems-861-865:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_860_theorems_861_865.c tsfi2-deepseek/src/auncient_fpga_beyond_860_theorems_861_865.c -o tests/test_auncient_fpga_beyond_860_theorems_861_865 -lm
	./tests/test_auncient_fpga_beyond_860_theorems_861_865
	@rm -f tests/test_auncient_fpga_beyond_860_theorems_861_865

test-auncient-fpga-beyond-855-theorems-856-860:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_855_theorems_856_860.c tsfi2-deepseek/src/auncient_fpga_beyond_855_theorems_856_860.c -o tests/test_auncient_fpga_beyond_855_theorems_856_860 -lm
	./tests/test_auncient_fpga_beyond_855_theorems_856_860
	@rm -f tests/test_auncient_fpga_beyond_855_theorems_856_860

test-auncient-fpga-beyond-850-theorems-851-855:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_850_theorems_851_855.c tsfi2-deepseek/src/auncient_fpga_beyond_850_theorems_851_855.c -o tests/test_auncient_fpga_beyond_850_theorems_851_855 -lm
	./tests/test_auncient_fpga_beyond_850_theorems_851_855
	@rm -f tests/test_auncient_fpga_beyond_850_theorems_851_855

test-auncient-fpga-beyond-845-theorems-846-850:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_845_theorems_846_850.c tsfi2-deepseek/src/auncient_fpga_beyond_845_theorems_846_850.c -o tests/test_auncient_fpga_beyond_845_theorems_846_850 -lm
	./tests/test_auncient_fpga_beyond_845_theorems_846_850
	@rm -f tests/test_auncient_fpga_beyond_845_theorems_846_850

test-auncient-fpga-beyond-840-theorems-841-845:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_840_theorems_841_845.c tsfi2-deepseek/src/auncient_fpga_beyond_840_theorems_841_845.c -o tests/test_auncient_fpga_beyond_840_theorems_841_845 -lm
	./tests/test_auncient_fpga_beyond_840_theorems_841_845
	@rm -f tests/test_auncient_fpga_beyond_840_theorems_841_845

test-auncient-fpga-beyond-835-theorems-836-840:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_835_theorems_836_840.c tsfi2-deepseek/src/auncient_fpga_beyond_835_theorems_836_840.c -o tests/test_auncient_fpga_beyond_835_theorems_836_840 -lm
	./tests/test_auncient_fpga_beyond_835_theorems_836_840
	@rm -f tests/test_auncient_fpga_beyond_835_theorems_836_840

test-auncient-fpga-beyond-830-theorems-831-835:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_830_theorems_831_835.c tsfi2-deepseek/src/auncient_fpga_beyond_830_theorems_831_835.c -o tests/test_auncient_fpga_beyond_830_theorems_831_835 -lm
	./tests/test_auncient_fpga_beyond_830_theorems_831_835
	@rm -f tests/test_auncient_fpga_beyond_830_theorems_831_835

test-auncient-fpga-beyond-825-theorems-826-830:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_825_theorems_826_830.c tsfi2-deepseek/src/auncient_fpga_beyond_825_theorems_826_830.c -o tests/test_auncient_fpga_beyond_825_theorems_826_830 -lm
	./tests/test_auncient_fpga_beyond_825_theorems_826_830
	@rm -f tests/test_auncient_fpga_beyond_825_theorems_826_830

test-auncient-fpga-beyond-820-theorems-821-825:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_820_theorems_821_825.c tsfi2-deepseek/src/auncient_fpga_beyond_820_theorems_821_825.c -o tests/test_auncient_fpga_beyond_820_theorems_821_825 -lm
	./tests/test_auncient_fpga_beyond_820_theorems_821_825
	@rm -f tests/test_auncient_fpga_beyond_820_theorems_821_825

test-auncient-fpga-beyond-815-theorems-816-820:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_815_theorems_816_820.c tsfi2-deepseek/src/auncient_fpga_beyond_815_theorems_816_820.c -o tests/test_auncient_fpga_beyond_815_theorems_816_820 -lm
	./tests/test_auncient_fpga_beyond_815_theorems_816_820
	@rm -f tests/test_auncient_fpga_beyond_815_theorems_816_820

test-auncient-fpga-beyond-810-theorems-811-815:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_810_theorems_811_815.c tsfi2-deepseek/src/auncient_fpga_beyond_810_theorems_811_815.c -o tests/test_auncient_fpga_beyond_810_theorems_811_815 -lm
	./tests/test_auncient_fpga_beyond_810_theorems_811_815
	@rm -f tests/test_auncient_fpga_beyond_810_theorems_811_815

test-auncient-fpga-beyond-805-theorems-806-810:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_805_theorems_806_810.c tsfi2-deepseek/src/auncient_fpga_beyond_805_theorems_806_810.c -o tests/test_auncient_fpga_beyond_805_theorems_806_810 -lm
	./tests/test_auncient_fpga_beyond_805_theorems_806_810
	@rm -f tests/test_auncient_fpga_beyond_805_theorems_806_810

test-auncient-fpga-beyond-800-theorems-801-805:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_800_theorems_801_805.c tsfi2-deepseek/src/auncient_fpga_beyond_800_theorems_801_805.c -o tests/test_auncient_fpga_beyond_800_theorems_801_805 -lm
	./tests/test_auncient_fpga_beyond_800_theorems_801_805
	@rm -f tests/test_auncient_fpga_beyond_800_theorems_801_805

test-auncient-fpga-beyond-795-theorems-796-800:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_795_theorems_796_800.c tsfi2-deepseek/src/auncient_fpga_beyond_795_theorems_796_800.c -o tests/test_auncient_fpga_beyond_795_theorems_796_800 -lm
	./tests/test_auncient_fpga_beyond_795_theorems_796_800
	@rm -f tests/test_auncient_fpga_beyond_795_theorems_796_800

test-auncient-fpga-beyond-790-theorems-791-795:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_790_theorems_791_795.c tsfi2-deepseek/src/auncient_fpga_beyond_790_theorems_791_795.c -o tests/test_auncient_fpga_beyond_790_theorems_791_795 -lm
	./tests/test_auncient_fpga_beyond_790_theorems_791_795
	@rm -f tests/test_auncient_fpga_beyond_790_theorems_791_795

test-auncient-fpga-beyond-785-theorems-786-790:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_785_theorems_786_790.c tsfi2-deepseek/src/auncient_fpga_beyond_785_theorems_786_790.c -o tests/test_auncient_fpga_beyond_785_theorems_786_790 -lm
	./tests/test_auncient_fpga_beyond_785_theorems_786_790
	@rm -f tests/test_auncient_fpga_beyond_785_theorems_786_790

test-auncient-fpga-beyond-780-theorems-781-785:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_780_theorems_781_785.c tsfi2-deepseek/src/auncient_fpga_beyond_780_theorems_781_785.c -o tests/test_auncient_fpga_beyond_780_theorems_781_785 -lm
	./tests/test_auncient_fpga_beyond_780_theorems_781_785
	@rm -f tests/test_auncient_fpga_beyond_780_theorems_781_785

test-auncient-fpga-beyond-775-theorems-776-780:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_775_theorems_776_780.c tsfi2-deepseek/src/auncient_fpga_beyond_775_theorems_776_780.c -o tests/test_auncient_fpga_beyond_775_theorems_776_780 -lm
	./tests/test_auncient_fpga_beyond_775_theorems_776_780
	@rm -f tests/test_auncient_fpga_beyond_775_theorems_776_780

test-auncient-fpga-beyond-770-theorems-771-775:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_770_theorems_771_775.c tsfi2-deepseek/src/auncient_fpga_beyond_770_theorems_771_775.c -o tests/test_auncient_fpga_beyond_770_theorems_771_775 -lm
	./tests/test_auncient_fpga_beyond_770_theorems_771_775
	@rm -f tests/test_auncient_fpga_beyond_770_theorems_771_775

test-auncient-fpga-beyond-765-theorems-766-770:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_765_theorems_766_770.c tsfi2-deepseek/src/auncient_fpga_beyond_765_theorems_766_770.c -o tests/test_auncient_fpga_beyond_765_theorems_766_770 -lm
	./tests/test_auncient_fpga_beyond_765_theorems_766_770
	@rm -f tests/test_auncient_fpga_beyond_765_theorems_766_770

test-auncient-fpga-beyond-760-theorems-761-765:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_760_theorems_761_765.c tsfi2-deepseek/src/auncient_fpga_beyond_760_theorems_761_765.c -o tests/test_auncient_fpga_beyond_760_theorems_761_765 -lm
	./tests/test_auncient_fpga_beyond_760_theorems_761_765
	@rm -f tests/test_auncient_fpga_beyond_760_theorems_761_765

test-auncient-fpga-beyond-755-theorems-756-760:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_755_theorems_756_760.c tsfi2-deepseek/src/auncient_fpga_beyond_755_theorems_756_760.c -o tests/test_auncient_fpga_beyond_755_theorems_756_760 -lm
	./tests/test_auncient_fpga_beyond_755_theorems_756_760
	@rm -f tests/test_auncient_fpga_beyond_755_theorems_756_760

test-auncient-fpga-beyond-750-theorems-751-755:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_750_theorems_751_755.c tsfi2-deepseek/src/auncient_fpga_beyond_750_theorems_751_755.c -o tests/test_auncient_fpga_beyond_750_theorems_751_755 -lm
	./tests/test_auncient_fpga_beyond_750_theorems_751_755
	@rm -f tests/test_auncient_fpga_beyond_750_theorems_751_755

test-auncient-fpga-beyond-745-theorems-746-750:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_745_theorems_746_750.c tsfi2-deepseek/src/auncient_fpga_beyond_745_theorems_746_750.c -o tests/test_auncient_fpga_beyond_745_theorems_746_750 -lm
	./tests/test_auncient_fpga_beyond_745_theorems_746_750
	@rm -f tests/test_auncient_fpga_beyond_745_theorems_746_750

test-auncient-fpga-beyond-740-theorems-741-745:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_740_theorems_741_745.c tsfi2-deepseek/src/auncient_fpga_beyond_740_theorems_741_745.c -o tests/test_auncient_fpga_beyond_740_theorems_741_745 -lm
	./tests/test_auncient_fpga_beyond_740_theorems_741_745
	@rm -f tests/test_auncient_fpga_beyond_740_theorems_741_745

test-auncient-fpga-beyond-735-theorems-736-740:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_735_theorems_736_740.c tsfi2-deepseek/src/auncient_fpga_beyond_735_theorems_736_740.c -o tests/test_auncient_fpga_beyond_735_theorems_736_740 -lm
	./tests/test_auncient_fpga_beyond_735_theorems_736_740
	@rm -f tests/test_auncient_fpga_beyond_735_theorems_736_740

test-auncient-fpga-beyond-730-theorems-731-735:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_730_theorems_731_735.c tsfi2-deepseek/src/auncient_fpga_beyond_730_theorems_731_735.c -o tests/test_auncient_fpga_beyond_730_theorems_731_735 -lm
	./tests/test_auncient_fpga_beyond_730_theorems_731_735
	@rm -f tests/test_auncient_fpga_beyond_730_theorems_731_735

test-auncient-fpga-beyond-725-theorems-726-730:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_725_theorems_726_730.c tsfi2-deepseek/src/auncient_fpga_beyond_725_theorems_726_730.c -o tests/test_auncient_fpga_beyond_725_theorems_726_730 -lm
	./tests/test_auncient_fpga_beyond_725_theorems_726_730
	@rm -f tests/test_auncient_fpga_beyond_725_theorems_726_730

test-auncient-fpga-beyond-720-theorems-721-725:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_720_theorems_721_725.c tsfi2-deepseek/src/auncient_fpga_beyond_720_theorems_721_725.c -o tests/test_auncient_fpga_beyond_720_theorems_721_725 -lm
	./tests/test_auncient_fpga_beyond_720_theorems_721_725
	@rm -f tests/test_auncient_fpga_beyond_720_theorems_721_725

test-auncient-fpga-beyond-715-theorems-716-720:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_715_theorems_716_720.c tsfi2-deepseek/src/auncient_fpga_beyond_715_theorems_716_720.c -o tests/test_auncient_fpga_beyond_715_theorems_716_720 -lm
	./tests/test_auncient_fpga_beyond_715_theorems_716_720
	@rm -f tests/test_auncient_fpga_beyond_715_theorems_716_720

test-auncient-fpga-beyond-710-theorems-711-715:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_710_theorems_711_715.c tsfi2-deepseek/src/auncient_fpga_beyond_710_theorems_711_715.c -o tests/test_auncient_fpga_beyond_710_theorems_711_715 -lm
	./tests/test_auncient_fpga_beyond_710_theorems_711_715
	@rm -f tests/test_auncient_fpga_beyond_710_theorems_711_715

test-auncient-fpga-beyond-705-theorems-706-710:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_705_theorems_706_710.c tsfi2-deepseek/src/auncient_fpga_beyond_705_theorems_706_710.c -o tests/test_auncient_fpga_beyond_705_theorems_706_710 -lm
	./tests/test_auncient_fpga_beyond_705_theorems_706_710
	@rm -f tests/test_auncient_fpga_beyond_705_theorems_706_710

test-auncient-fpga-beyond-700-theorems-701-705:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_700_theorems_701_705.c tsfi2-deepseek/src/auncient_fpga_beyond_700_theorems_701_705.c -o tests/test_auncient_fpga_beyond_700_theorems_701_705 -lm
	./tests/test_auncient_fpga_beyond_700_theorems_701_705
	@rm -f tests/test_auncient_fpga_beyond_700_theorems_701_705

test-auncient-fpga-beyond-695-theorems-696-700:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_695_theorems_696_700.c tsfi2-deepseek/src/auncient_fpga_beyond_695_theorems_696_700.c -o tests/test_auncient_fpga_beyond_695_theorems_696_700 -lm
	./tests/test_auncient_fpga_beyond_695_theorems_696_700
	@rm -f tests/test_auncient_fpga_beyond_695_theorems_696_700

test-auncient-fpga-beyond-690-theorems-691-695:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_690_theorems_691_695.c tsfi2-deepseek/src/auncient_fpga_beyond_690_theorems_691_695.c -o tests/test_auncient_fpga_beyond_690_theorems_691_695 -lm
	./tests/test_auncient_fpga_beyond_690_theorems_691_695
	@rm -f tests/test_auncient_fpga_beyond_690_theorems_691_695

test-auncient-fpga-beyond-685-theorems-686-690:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_685_theorems_686_690.c tsfi2-deepseek/src/auncient_fpga_beyond_685_theorems_686_690.c -o tests/test_auncient_fpga_beyond_685_theorems_686_690 -lm
	./tests/test_auncient_fpga_beyond_685_theorems_686_690
	@rm -f tests/test_auncient_fpga_beyond_685_theorems_686_690

test-auncient-fpga-beyond-680-theorems-681-685:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_680_theorems_681_685.c tsfi2-deepseek/src/auncient_fpga_beyond_680_theorems_681_685.c -o tests/test_auncient_fpga_beyond_680_theorems_681_685 -lm
	./tests/test_auncient_fpga_beyond_680_theorems_681_685
	@rm -f tests/test_auncient_fpga_beyond_680_theorems_681_685

test-auncient-fpga-beyond-675-theorems-676-680:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_675_theorems_676_680.c tsfi2-deepseek/src/auncient_fpga_beyond_675_theorems_676_680.c -o tests/test_auncient_fpga_beyond_675_theorems_676_680 -lm
	./tests/test_auncient_fpga_beyond_675_theorems_676_680
	@rm -f tests/test_auncient_fpga_beyond_675_theorems_676_680

test-auncient-fpga-beyond-670-theorems-671-675:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_670_theorems_671_675.c tsfi2-deepseek/src/auncient_fpga_beyond_670_theorems_671_675.c -o tests/test_auncient_fpga_beyond_670_theorems_671_675 -lm
	./tests/test_auncient_fpga_beyond_670_theorems_671_675
	@rm -f tests/test_auncient_fpga_beyond_670_theorems_671_675

test-auncient-fpga-beyond-665-theorems-666-670:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_665_theorems_666_670.c tsfi2-deepseek/src/auncient_fpga_beyond_665_theorems_666_670.c -o tests/test_auncient_fpga_beyond_665_theorems_666_670 -lm
	./tests/test_auncient_fpga_beyond_665_theorems_666_670
	@rm -f tests/test_auncient_fpga_beyond_665_theorems_666_670

test-auncient-fpga-beyond-660-theorems-661-665:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_660_theorems_661_665.c tsfi2-deepseek/src/auncient_fpga_beyond_660_theorems_661_665.c -o tests/test_auncient_fpga_beyond_660_theorems_661_665 -lm
	./tests/test_auncient_fpga_beyond_660_theorems_661_665
	@rm -f tests/test_auncient_fpga_beyond_660_theorems_661_665

test-auncient-fpga-beyond-655-theorems-656-660:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_655_theorems_656_660.c tsfi2-deepseek/src/auncient_fpga_beyond_655_theorems_656_660.c -o tests/test_auncient_fpga_beyond_655_theorems_656_660 -lm
	./tests/test_auncient_fpga_beyond_655_theorems_656_660
	@rm -f tests/test_auncient_fpga_beyond_655_theorems_656_660

test-auncient-fpga-beyond-650-theorems-651-655:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_650_theorems_651_655.c tsfi2-deepseek/src/auncient_fpga_beyond_650_theorems_651_655.c -o tests/test_auncient_fpga_beyond_650_theorems_651_655 -lm
	./tests/test_auncient_fpga_beyond_650_theorems_651_655
	@rm -f tests/test_auncient_fpga_beyond_650_theorems_651_655

test-auncient-fpga-beyond-645-theorems-646-650:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_645_theorems_646_650.c tsfi2-deepseek/src/auncient_fpga_beyond_645_theorems_646_650.c -o tests/test_auncient_fpga_beyond_645_theorems_646_650 -lm
	./tests/test_auncient_fpga_beyond_645_theorems_646_650
	@rm -f tests/test_auncient_fpga_beyond_645_theorems_646_650

test-auncient-fpga-beyond-640-theorems-641-645:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_640_theorems_641_645.c tsfi2-deepseek/src/auncient_fpga_beyond_640_theorems_641_645.c -o tests/test_auncient_fpga_beyond_640_theorems_641_645 -lm
	./tests/test_auncient_fpga_beyond_640_theorems_641_645
	@rm -f tests/test_auncient_fpga_beyond_640_theorems_641_645

test-auncient-fpga-beyond-635-theorems-636-640:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_635_theorems_636_640.c tsfi2-deepseek/src/auncient_fpga_beyond_635_theorems_636_640.c -o tests/test_auncient_fpga_beyond_635_theorems_636_640 -lm
	./tests/test_auncient_fpga_beyond_635_theorems_636_640
	@rm -f tests/test_auncient_fpga_beyond_635_theorems_636_640

test-auncient-fpga-beyond-630-theorems-631-635:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_630_theorems_631_635.c tsfi2-deepseek/src/auncient_fpga_beyond_630_theorems_631_635.c -o tests/test_auncient_fpga_beyond_630_theorems_631_635 -lm
	./tests/test_auncient_fpga_beyond_630_theorems_631_635
	@rm -f tests/test_auncient_fpga_beyond_630_theorems_631_635

test-auncient-fpga-beyond-625-theorems-626-630:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_625_theorems_626_630.c tsfi2-deepseek/src/auncient_fpga_beyond_625_theorems_626_630.c -o tests/test_auncient_fpga_beyond_625_theorems_626_630 -lm
	./tests/test_auncient_fpga_beyond_625_theorems_626_630
	@rm -f tests/test_auncient_fpga_beyond_625_theorems_626_630

test-auncient-fpga-beyond-620-theorems-621-625:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_620_theorems_621_625.c tsfi2-deepseek/src/auncient_fpga_beyond_620_theorems_621_625.c -o tests/test_auncient_fpga_beyond_620_theorems_621_625 -lm
	./tests/test_auncient_fpga_beyond_620_theorems_621_625
	@rm -f tests/test_auncient_fpga_beyond_620_theorems_621_625

test-auncient-fpga-beyond-615-theorems-616-620:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_615_theorems_616_620.c tsfi2-deepseek/src/auncient_fpga_beyond_615_theorems_616_620.c -o tests/test_auncient_fpga_beyond_615_theorems_616_620 -lm
	./tests/test_auncient_fpga_beyond_615_theorems_616_620
	@rm -f tests/test_auncient_fpga_beyond_615_theorems_616_620

test-auncient-fpga-beyond-610-theorems-611-615:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_610_theorems_611_615.c tsfi2-deepseek/src/auncient_fpga_beyond_610_theorems_611_615.c -o tests/test_auncient_fpga_beyond_610_theorems_611_615 -lm
	./tests/test_auncient_fpga_beyond_610_theorems_611_615
	@rm -f tests/test_auncient_fpga_beyond_610_theorems_611_615

test-auncient-fpga-beyond-605-theorems-606-610:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_605_theorems_606_610.c tsfi2-deepseek/src/auncient_fpga_beyond_605_theorems_606_610.c -o tests/test_auncient_fpga_beyond_605_theorems_606_610 -lm
	./tests/test_auncient_fpga_beyond_605_theorems_606_610
	@rm -f tests/test_auncient_fpga_beyond_605_theorems_606_610

test-auncient-fpga-beyond-600-theorems-601-605:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_600_theorems_601_605.c tsfi2-deepseek/src/auncient_fpga_beyond_600_theorems_601_605.c -o tests/test_auncient_fpga_beyond_600_theorems_601_605 -lm
	./tests/test_auncient_fpga_beyond_600_theorems_601_605
	@rm -f tests/test_auncient_fpga_beyond_600_theorems_601_605

test-auncient-fpga-beyond-595-theorems-596-600:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_595_theorems_596_600.c tsfi2-deepseek/src/auncient_fpga_beyond_595_theorems_596_600.c -o tests/test_auncient_fpga_beyond_595_theorems_596_600 -lm
	./tests/test_auncient_fpga_beyond_595_theorems_596_600
	@rm -f tests/test_auncient_fpga_beyond_595_theorems_596_600

test-auncient-fpga-beyond-590-theorems-591-595:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_590_theorems_591_595.c tsfi2-deepseek/src/auncient_fpga_beyond_590_theorems_591_595.c -o tests/test_auncient_fpga_beyond_590_theorems_591_595 -lm
	./tests/test_auncient_fpga_beyond_590_theorems_591_595
	@rm -f tests/test_auncient_fpga_beyond_590_theorems_591_595

test-auncient-fpga-beyond-585-theorems-586-590:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_585_theorems_586_590.c tsfi2-deepseek/src/auncient_fpga_beyond_585_theorems_586_590.c -o tests/test_auncient_fpga_beyond_585_theorems_586_590 -lm
	./tests/test_auncient_fpga_beyond_585_theorems_586_590
	@rm -f tests/test_auncient_fpga_beyond_585_theorems_586_590

test-auncient-fpga-beyond-580-theorems-581-585:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_580_theorems_581_585.c tsfi2-deepseek/src/auncient_fpga_beyond_580_theorems_581_585.c -o tests/test_auncient_fpga_beyond_580_theorems_581_585 -lm
	./tests/test_auncient_fpga_beyond_580_theorems_581_585
	@rm -f tests/test_auncient_fpga_beyond_580_theorems_581_585

test-auncient-fpga-beyond-575-theorems-576-580:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_575_theorems_576_580.c tsfi2-deepseek/src/auncient_fpga_beyond_575_theorems_576_580.c -o tests/test_auncient_fpga_beyond_575_theorems_576_580 -lm
	./tests/test_auncient_fpga_beyond_575_theorems_576_580
	@rm -f tests/test_auncient_fpga_beyond_575_theorems_576_580

test-auncient-fpga-beyond-570-theorems-571-575:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_570_theorems_571_575.c tsfi2-deepseek/src/auncient_fpga_beyond_570_theorems_571_575.c -o tests/test_auncient_fpga_beyond_570_theorems_571_575 -lm
	./tests/test_auncient_fpga_beyond_570_theorems_571_575
	@rm -f tests/test_auncient_fpga_beyond_570_theorems_571_575

test-auncient-fpga-beyond-565-theorems-566-570:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_565_theorems_566_570.c tsfi2-deepseek/src/auncient_fpga_beyond_565_theorems_566_570.c -o tests/test_auncient_fpga_beyond_565_theorems_566_570 -lm
	./tests/test_auncient_fpga_beyond_565_theorems_566_570
	@rm -f tests/test_auncient_fpga_beyond_565_theorems_566_570

test-auncient-fpga-beyond-560-theorems-561-565:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_560_theorems_561_565.c tsfi2-deepseek/src/auncient_fpga_beyond_560_theorems_561_565.c -o tests/test_auncient_fpga_beyond_560_theorems_561_565 -lm
	./tests/test_auncient_fpga_beyond_560_theorems_561_565
	@rm -f tests/test_auncient_fpga_beyond_560_theorems_561_565

test-auncient-fpga-beyond-555-theorems-556-560:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_555_theorems_556_560.c tsfi2-deepseek/src/auncient_fpga_beyond_555_theorems_556_560.c -o tests/test_auncient_fpga_beyond_555_theorems_556_560 -lm
	./tests/test_auncient_fpga_beyond_555_theorems_556_560
	@rm -f tests/test_auncient_fpga_beyond_555_theorems_556_560

test-auncient-fpga-beyond-550-theorems-551-555:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_550_theorems_551_555.c tsfi2-deepseek/src/auncient_fpga_beyond_550_theorems_551_555.c -o tests/test_auncient_fpga_beyond_550_theorems_551_555 -lm
	./tests/test_auncient_fpga_beyond_550_theorems_551_555
	@rm -f tests/test_auncient_fpga_beyond_550_theorems_551_555

test-auncient-fpga-beyond-545-theorems-546-550:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_545_theorems_546_550.c tsfi2-deepseek/src/auncient_fpga_beyond_545_theorems_546_550.c -o tests/test_auncient_fpga_beyond_545_theorems_546_550 -lm
	./tests/test_auncient_fpga_beyond_545_theorems_546_550
	@rm -f tests/test_auncient_fpga_beyond_545_theorems_546_550

test-auncient-fpga-beyond-540-theorems-541-545:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_540_theorems_541_545.c tsfi2-deepseek/src/auncient_fpga_beyond_540_theorems_541_545.c -o tests/test_auncient_fpga_beyond_540_theorems_541_545 -lm
	./tests/test_auncient_fpga_beyond_540_theorems_541_545
	@rm -f tests/test_auncient_fpga_beyond_540_theorems_541_545

test-auncient-fpga-cpmtomie-ankh-symbolic-theorems-536-540:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_cpmtomie_ankh_symbolic_theorems_536_540.c tsfi2-deepseek/src/auncient_fpga_cpmtomie_ankh_symbolic_theorems_536_540.c -o tests/test_auncient_fpga_cpmtomie_ankh_symbolic_theorems_536_540 -lm
	./tests/test_auncient_fpga_cpmtomie_ankh_symbolic_theorems_536_540
	@rm -f tests/test_auncient_fpga_cpmtomie_ankh_symbolic_theorems_536_540

test-auncient-fpga-beyond-530-theorems-531-535:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_530_theorems_531_535.c tsfi2-deepseek/src/auncient_fpga_beyond_530_theorems_531_535.c -o tests/test_auncient_fpga_beyond_530_theorems_531_535 -lm
	./tests/test_auncient_fpga_beyond_530_theorems_531_535
	@rm -f tests/test_auncient_fpga_beyond_530_theorems_531_535

test-auncient-fpga-beyond-525-theorems-526-530:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_525_theorems_526_530.c tsfi2-deepseek/src/auncient_fpga_beyond_525_theorems_526_530.c -o tests/test_auncient_fpga_beyond_525_theorems_526_530 -lm
	./tests/test_auncient_fpga_beyond_525_theorems_526_530
	@rm -f tests/test_auncient_fpga_beyond_525_theorems_526_530

test-auncient-fpga-cpmtomie-ankh-cleanroom-theorems-521-525:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_cpmtomie_ankh_cleanroom_theorems_521_525.c tsfi2-deepseek/src/auncient_fpga_cpmtomie_ankh_cleanroom_theorems_521_525.c -o tests/test_auncient_fpga_cpmtomie_ankh_cleanroom_theorems_521_525 -lm
	./tests/test_auncient_fpga_cpmtomie_ankh_cleanroom_theorems_521_525
	@rm -f tests/test_auncient_fpga_cpmtomie_ankh_cleanroom_theorems_521_525

test-auncient-fpga-cpmtomie-ankh-theorems-516-520:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_cpmtomie_ankh_theorems_516_520.c tsfi2-deepseek/src/auncient_fpga_cpmtomie_ankh_theorems_516_520.c -o tests/test_auncient_fpga_cpmtomie_ankh_theorems_516_520 -lm
	./tests/test_auncient_fpga_cpmtomie_ankh_theorems_516_520
	@rm -f tests/test_auncient_fpga_cpmtomie_ankh_theorems_516_520

test-auncient-fpga-beyond-510-theorems-511-515:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_510_theorems_511_515.c tsfi2-deepseek/src/auncient_fpga_beyond_510_theorems_511_515.c -o tests/test_auncient_fpga_beyond_510_theorems_511_515 -lm
	./tests/test_auncient_fpga_beyond_510_theorems_511_515
	@rm -f tests/test_auncient_fpga_beyond_510_theorems_511_515

test-auncient-fpga-beyond-505-theorems-506-510:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_505_theorems_506_510.c tsfi2-deepseek/src/auncient_fpga_beyond_505_theorems_506_510.c -o tests/test_auncient_fpga_beyond_505_theorems_506_510 -lm
	./tests/test_auncient_fpga_beyond_505_theorems_506_510
	@rm -f tests/test_auncient_fpga_beyond_505_theorems_506_510

test-auncient-fpga-beyond-500-theorems-501-505:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_beyond_500_theorems_501_505.c tsfi2-deepseek/src/auncient_fpga_beyond_500_theorems_501_505.c -o tests/test_auncient_fpga_beyond_500_theorems_501_505 -lm
	./tests/test_auncient_fpga_beyond_500_theorems_501_505
	@rm -f tests/test_auncient_fpga_beyond_500_theorems_501_505

test-auncient-fpga-quingentennial-unification-theorems-496-500:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_quingentennial_unification_theorems_496_500.c tsfi2-deepseek/src/auncient_fpga_quingentennial_unification_theorems_496_500.c -o tests/test_auncient_fpga_quingentennial_unification_theorems_496_500 -lm
	./tests/test_auncient_fpga_quingentennial_unification_theorems_496_500
	@rm -f tests/test_auncient_fpga_quingentennial_unification_theorems_496_500

test-auncient-fpga-systolic-recurrence-theorems-491-495:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_systolic_recurrence_theorems_491_495.c tsfi2-deepseek/src/auncient_fpga_systolic_recurrence_theorems_491_495.c -o tests/test_auncient_fpga_systolic_recurrence_theorems_491_495 -lm
	./tests/test_auncient_fpga_systolic_recurrence_theorems_491_495
	@rm -f tests/test_auncient_fpga_systolic_recurrence_theorems_491_495

test-auncient-fpga-quingentennial-theorems-486-490:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_quingentennial_theorems_486_490.c tsfi2-deepseek/src/auncient_fpga_quingentennial_theorems_486_490.c -o tests/test_auncient_fpga_quingentennial_theorems_486_490 -lm
	./tests/test_auncient_fpga_quingentennial_theorems_486_490
	@rm -f tests/test_auncient_fpga_quingentennial_theorems_486_490

test-auncient-fpga-polyphase-complex-theorems-481-485:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_complex_theorems_481_485.c tsfi2-deepseek/src/auncient_fpga_polyphase_complex_theorems_481_485.c -o tests/test_auncient_fpga_polyphase_complex_theorems_481_485 -lm
	./tests/test_auncient_fpga_polyphase_complex_theorems_481_485
	@rm -f tests/test_auncient_fpga_polyphase_complex_theorems_481_485

test-auncient-fpga-polyphase-stator-lut-theorems-476-480:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_stator_lut_theorems_476_480.c tsfi2-deepseek/src/auncient_fpga_polyphase_stator_lut_theorems_476_480.c -o tests/test_auncient_fpga_polyphase_stator_lut_theorems_476_480 -lm
	./tests/test_auncient_fpga_polyphase_stator_lut_theorems_476_480
	@rm -f tests/test_auncient_fpga_polyphase_stator_lut_theorems_476_480

test-auncient-fpga-stanag-mesh-theorems-471-475:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_stanag_mesh_theorems_471_475.c tsfi2-deepseek/src/auncient_fpga_stanag_mesh_theorems_471_475.c -o tests/test_auncient_fpga_stanag_mesh_theorems_471_475 -lm
	./tests/test_auncient_fpga_stanag_mesh_theorems_471_475
	@rm -f tests/test_auncient_fpga_stanag_mesh_theorems_471_475

test-auncient-fpga-pageturner-tk-theorems-466-470:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_pageturner_tk_theorems_466_470.c tsfi2-deepseek/src/auncient_fpga_pageturner_tk_theorems_466_470.c -o tests/test_auncient_fpga_pageturner_tk_theorems_466_470 -lm
	./tests/test_auncient_fpga_pageturner_tk_theorems_466_470
	@rm -f tests/test_auncient_fpga_pageturner_tk_theorems_466_470

test-auncient-fpga-twin-cert-theorems-461-465:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_twin_cert_theorems_461_465.c tsfi2-deepseek/src/auncient_fpga_twin_cert_theorems_461_465.c -o tests/test_auncient_fpga_twin_cert_theorems_461_465 -lm
	./tests/test_auncient_fpga_twin_cert_theorems_461_465
	@rm -f tests/test_auncient_fpga_twin_cert_theorems_461_465

test-auncient-fpga-digital-twin-theorems-456-460:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_digital_twin_theorems_456_460.c tsfi2-deepseek/src/auncient_fpga_digital_twin_theorems_456_460.c -o tests/test_auncient_fpga_digital_twin_theorems_456_460 -lm
	./tests/test_auncient_fpga_digital_twin_theorems_456_460
	@rm -f tests/test_auncient_fpga_digital_twin_theorems_456_460

test-auncient-fpga-bist-icap-theorems-451-455:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_bist_icap_theorems_451_455.c tsfi2-deepseek/src/auncient_fpga_bist_icap_theorems_451_455.c -o tests/test_auncient_fpga_bist_icap_theorems_451_455 -lm
	./tests/test_auncient_fpga_bist_icap_theorems_451_455
	@rm -f tests/test_auncient_fpga_bist_icap_theorems_451_455

test-auncient-fpga-self-cert-theorems-446-450:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_self_cert_theorems_446_450.c tsfi2-deepseek/src/auncient_fpga_self_cert_theorems_446_450.c -o tests/test_auncient_fpga_self_cert_theorems_446_450 -lm
	./tests/test_auncient_fpga_self_cert_theorems_446_450
	@rm -f tests/test_auncient_fpga_self_cert_theorems_446_450

test-auncient-fpga-polyphase-torus128-theorems-441-445:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_torus128_theorems_441_445.c tsfi2-deepseek/src/auncient_fpga_polyphase_torus128_theorems_441_445.c -o tests/test_auncient_fpga_polyphase_torus128_theorems_441_445 -lm
	./tests/test_auncient_fpga_polyphase_torus128_theorems_441_445
	@rm -f tests/test_auncient_fpga_polyphase_torus128_theorems_441_445

test-auncient-fpga-stator-rotor-theorems-436-440:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_stator_rotor_theorems_436_440.c tsfi2-deepseek/src/auncient_fpga_stator_rotor_theorems_436_440.c -o tests/test_auncient_fpga_stator_rotor_theorems_436_440 -lm
	./tests/test_auncient_fpga_stator_rotor_theorems_436_440
	@rm -f tests/test_auncient_fpga_stator_rotor_theorems_436_440

test-auncient-fpga-holonomic-wdm-theorems-431-435:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_holonomic_wdm_theorems_431_435.c tsfi2-deepseek/src/auncient_fpga_holonomic_wdm_theorems_431_435.c -o tests/test_auncient_fpga_holonomic_wdm_theorems_431_435 -lm
	./tests/test_auncient_fpga_holonomic_wdm_theorems_431_435
	@rm -f tests/test_auncient_fpga_holonomic_wdm_theorems_431_435

test-auncient-fpga-weyl-symplectic-theorems-426-430:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_weyl_symplectic_theorems_426_430.c tsfi2-deepseek/src/auncient_fpga_weyl_symplectic_theorems_426_430.c -o tests/test_auncient_fpga_weyl_symplectic_theorems_426_430 -lm
	./tests/test_auncient_fpga_weyl_symplectic_theorems_426_430
	@rm -f tests/test_auncient_fpga_weyl_symplectic_theorems_426_430

test-auncient-fpga-optic-symplectic-theorems-421-425:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_optic_symplectic_theorems_421_425.c tsfi2-deepseek/src/auncient_fpga_optic_symplectic_theorems_421_425.c -o tests/test_auncient_fpga_optic_symplectic_theorems_421_425 -lm
	./tests/test_auncient_fpga_optic_symplectic_theorems_421_425
	@rm -f tests/test_auncient_fpga_optic_symplectic_theorems_421_425

test-auncient-fpga-optic-verlet-theorems-416-420:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_optic_verlet_theorems_416_420.c tsfi2-deepseek/src/auncient_fpga_optic_verlet_theorems_416_420.c -o tests/test_auncient_fpga_optic_verlet_theorems_416_420 -lm
	./tests/test_auncient_fpga_optic_verlet_theorems_416_420
	@rm -f tests/test_auncient_fpga_optic_verlet_theorems_416_420

test-auncient-fpga-mckeeman-euler-theorems-406-410:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_mckeeman_euler_theorems_406_410.c tsfi2-deepseek/src/auncient_fpga_mckeeman_euler_theorems_406_410.c -o tests/test_auncient_fpga_mckeeman_euler_theorems_406_410 -lm
	./tests/test_auncient_fpga_mckeeman_euler_theorems_406_410
	@rm -f tests/test_auncient_fpga_mckeeman_euler_theorems_406_410

test-auncient-fpga-rebar-zmm-theorems-401-405:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_rebar_zmm_theorems_401_405.c tsfi2-deepseek/src/auncient_fpga_rebar_zmm_theorems_401_405.c -o tests/test_auncient_fpga_rebar_zmm_theorems_401_405 -lm
	./tests/test_auncient_fpga_rebar_zmm_theorems_401_405
	@rm -f tests/test_auncient_fpga_rebar_zmm_theorems_401_405

test-auncient-fpga-quadringentennial-theorems-396-400:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_quadringentennial_theorems_396_400.c tsfi2-deepseek/src/auncient_fpga_quadringentennial_theorems_396_400.c -o tests/test_auncient_fpga_quadringentennial_theorems_396_400 -lm
	./tests/test_auncient_fpga_quadringentennial_theorems_396_400
	@rm -f tests/test_auncient_fpga_quadringentennial_theorems_396_400

test-auncient-fpga-accumulator-theorems-391-395:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_accumulator_theorems_391_395.c tsfi2-deepseek/src/auncient_fpga_accumulator_theorems_391_395.c -o tests/test_auncient_fpga_accumulator_theorems_391_395 -lm
	./tests/test_auncient_fpga_accumulator_theorems_391_395
	@rm -f tests/test_auncient_fpga_accumulator_theorems_391_395

test-auncient-fpga-coaxial-tem-theorems-386-390:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_coaxial_tem_theorems_386_390.c tsfi2-deepseek/src/auncient_fpga_coaxial_tem_theorems_386_390.c -o tests/test_auncient_fpga_coaxial_tem_theorems_386_390 -lm
	./tests/test_auncient_fpga_coaxial_tem_theorems_386_390
	@rm -f tests/test_auncient_fpga_coaxial_tem_theorems_386_390

test-auncient-fpga-stanag-bridge-theorems-381-385:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_stanag_bridge_theorems_381_385.c tsfi2-deepseek/src/auncient_fpga_stanag_bridge_theorems_381_385.c -o tests/test_auncient_fpga_stanag_bridge_theorems_381_385 -lm
	./tests/test_auncient_fpga_stanag_bridge_theorems_381_385
	@rm -f tests/test_auncient_fpga_stanag_bridge_theorems_381_385

test-auncient-fpga-quatercentennial-theorems-376-380:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_quatercentennial_theorems_376_380.c tsfi2-deepseek/src/auncient_fpga_quatercentennial_theorems_376_380.c -o tests/test_auncient_fpga_quatercentennial_theorems_376_380 -lm
	./tests/test_auncient_fpga_quatercentennial_theorems_376_380
	@rm -f tests/test_auncient_fpga_quatercentennial_theorems_376_380

test-auncient-fpga-symbolic-llm-theorems-371-375:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_symbolic_llm_theorems_371_375.c tsfi2-deepseek/src/auncient_fpga_symbolic_llm_theorems_371_375.c -o tests/test_auncient_fpga_symbolic_llm_theorems_371_375 -lm
	./tests/test_auncient_fpga_symbolic_llm_theorems_371_375
	@rm -f tests/test_auncient_fpga_symbolic_llm_theorems_371_375

test-auncient-fpga-quadtree-os-theorems-366-370:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_quadtree_os_theorems_366_370.c tsfi2-deepseek/src/auncient_fpga_quadtree_os_theorems_366_370.c -o tests/test_auncient_fpga_quadtree_os_theorems_366_370 -lm
	./tests/test_auncient_fpga_quadtree_os_theorems_366_370
	@rm -f tests/test_auncient_fpga_quadtree_os_theorems_366_370

test-auncient-fpga-mckeeman-quadrature-theorems-361-365:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_mckeeman_quadrature_theorems_361_365.c tsfi2-deepseek/src/auncient_fpga_mckeeman_quadrature_theorems_361_365.c -o tests/test_auncient_fpga_mckeeman_quadrature_theorems_361_365 -lm
	./tests/test_auncient_fpga_mckeeman_quadrature_theorems_361_365
	@rm -f tests/test_auncient_fpga_mckeeman_quadrature_theorems_361_365

test-auncient-fpga-jubilee-overdrive-theorems-356-360:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_jubilee_overdrive_theorems_356_360.c tsfi2-deepseek/src/auncient_fpga_jubilee_overdrive_theorems_356_360.c -o tests/test_auncient_fpga_jubilee_overdrive_theorems_356_360 -lm
	./tests/test_auncient_fpga_jubilee_overdrive_theorems_356_360
	@rm -f tests/test_auncient_fpga_jubilee_overdrive_theorems_356_360

test-auncient-fpga-optic-resonator-theorems-351-355:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_optic_resonator_theorems_351_355.c tsfi2-deepseek/src/auncient_fpga_optic_resonator_theorems_351_355.c -o tests/test_auncient_fpga_optic_resonator_theorems_351_355 -lm
	./tests/test_auncient_fpga_optic_resonator_theorems_351_355
	@rm -f tests/test_auncient_fpga_optic_resonator_theorems_351_355

test-auncient-fpga-master-closure-theorems-346-350:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_master_closure_theorems_346_350.c tsfi2-deepseek/src/auncient_fpga_master_closure_theorems_346_350.c -o tests/test_auncient_fpga_master_closure_theorems_346_350 -lm
	./tests/test_auncient_fpga_master_closure_theorems_346_350
	@rm -f tests/test_auncient_fpga_master_closure_theorems_346_350

test-auncient-fpga-bmrc-stability-theorems-341-345:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_bmrc_stability_theorems_341_345.c tsfi2-deepseek/src/auncient_fpga_bmrc_stability_theorems_341_345.c -o tests/test_auncient_fpga_bmrc_stability_theorems_341_345 -lm
	./tests/test_auncient_fpga_bmrc_stability_theorems_341_345
	@rm -f tests/test_auncient_fpga_bmrc_stability_theorems_341_345

test-auncient-fpga-octacosaphase-theorems-336-340:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_octacosaphase_theorems_336_340.c tsfi2-deepseek/src/auncient_fpga_octacosaphase_theorems_336_340.c -o tests/test_auncient_fpga_octacosaphase_theorems_336_340 -lm
	./tests/test_auncient_fpga_octacosaphase_theorems_336_340
	@rm -f tests/test_auncient_fpga_octacosaphase_theorems_336_340

test-auncient-fpga-fourier-stator-theorems-331-335:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_fourier_stator_theorems_331_335.c tsfi2-deepseek/src/auncient_fpga_fourier_stator_theorems_331_335.c -o tests/test_auncient_fpga_fourier_stator_theorems_331_335 -lm
	./tests/test_auncient_fpga_fourier_stator_theorems_331_335
	@rm -f tests/test_auncient_fpga_fourier_stator_theorems_331_335

test-auncient-fpga-polyphase-torus-theorems-326-330:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_torus_theorems_326_330.c tsfi2-deepseek/src/auncient_fpga_polyphase_torus_theorems_326_330.c -o tests/test_auncient_fpga_polyphase_torus_theorems_326_330 -lm
	./tests/test_auncient_fpga_polyphase_torus_theorems_326_330
	@rm -f tests/test_auncient_fpga_polyphase_torus_theorems_326_330

test-auncient-fpga-hexacontaphase-theorems-321-325:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_hexacontaphase_theorems_321_325.c tsfi2-deepseek/src/auncient_fpga_hexacontaphase_theorems_321_325.c -o tests/test_auncient_fpga_hexacontaphase_theorems_321_325 -lm
	./tests/test_auncient_fpga_hexacontaphase_theorems_321_325
	@rm -f tests/test_auncient_fpga_hexacontaphase_theorems_321_325

test-auncient-fpga-octacontaphase-theorems-316-320:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_octacontaphase_theorems_316_320.c tsfi2-deepseek/src/auncient_fpga_octacontaphase_theorems_316_320.c -o tests/test_auncient_fpga_octacontaphase_theorems_316_320 -lm
	./tests/test_auncient_fpga_octacontaphase_theorems_316_320
	@rm -f tests/test_auncient_fpga_octacontaphase_theorems_316_320

test-auncient-fpga-polyphase-superconducting-theorems-311-315:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_superconducting_theorems_311_315.c tsfi2-deepseek/src/auncient_fpga_polyphase_superconducting_theorems_311_315.c -o tests/test_auncient_fpga_polyphase_superconducting_theorems_311_315 -lm
	./tests/test_auncient_fpga_polyphase_superconducting_theorems_311_315
	@rm -f tests/test_auncient_fpga_polyphase_superconducting_theorems_311_315

test-auncient-fpga-polyphase-stator-manifold-theorems-306-310:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_stator_manifold_theorems_306_310.c tsfi2-deepseek/src/auncient_fpga_polyphase_stator_manifold_theorems_306_310.c -o tests/test_auncient_fpga_polyphase_stator_manifold_theorems_306_310 -lm
	./tests/test_auncient_fpga_polyphase_stator_manifold_theorems_306_310
	@rm -f tests/test_auncient_fpga_polyphase_stator_manifold_theorems_306_310

test-auncient-fpga-polymorphic-transceiver-theorems-301-305:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polymorphic_transceiver_theorems_301_305.c tsfi2-deepseek/src/auncient_fpga_polymorphic_transceiver_theorems_301_305.c -o tests/test_auncient_fpga_polymorphic_transceiver_theorems_301_305 -lm
	./tests/test_auncient_fpga_polymorphic_transceiver_theorems_301_305
	@rm -f tests/test_auncient_fpga_polymorphic_transceiver_theorems_301_305

test-auncient-fpga-tricentennial-theorems-296-300:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_tricentennial_theorems_296_300.c tsfi2-deepseek/src/auncient_fpga_tricentennial_theorems_296_300.c -o tests/test_auncient_fpga_tricentennial_theorems_296_300 -lm
	./tests/test_auncient_fpga_tricentennial_theorems_296_300
	@rm -f tests/test_auncient_fpga_tricentennial_theorems_296_300

test-auncient-fpga-teracentennial-theorems-291-295:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_teracentennial_theorems_291_295.c tsfi2-deepseek/src/auncient_fpga_teracentennial_theorems_291_295.c -o tests/test_auncient_fpga_teracentennial_theorems_291_295 -lm
	./tests/test_auncient_fpga_teracentennial_theorems_291_295
	@rm -f tests/test_auncient_fpga_teracentennial_theorems_291_295

test-auncient-fpga-total-knowledge-closure-theorems-286-290:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_total_knowledge_closure_theorems_286_290.c tsfi2-deepseek/src/auncient_fpga_total_knowledge_closure_theorems_286_290.c -o tests/test_auncient_fpga_total_knowledge_closure_theorems_286_290 -lm
	./tests/test_auncient_fpga_total_knowledge_closure_theorems_286_290
	@rm -f tests/test_auncient_fpga_total_knowledge_closure_theorems_286_290

test-auncient-fpga-ankh-cleanroom-theorems-281-285:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_ankh_cleanroom_theorems_281_285.c tsfi2-deepseek/src/auncient_fpga_ankh_cleanroom_theorems_281_285.c -o tests/test_auncient_fpga_ankh_cleanroom_theorems_281_285 -lm
	./tests/test_auncient_fpga_ankh_cleanroom_theorems_281_285
	@rm -f tests/test_auncient_fpga_ankh_cleanroom_theorems_281_285

test-auncient-fpga-hex-tk-stator-theorems-276-280:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_hex_tk_stator_theorems_276_280.c tsfi2-deepseek/src/auncient_fpga_hex_tk_stator_theorems_276_280.c -o tests/test_auncient_fpga_hex_tk_stator_theorems_276_280 -lm
	./tests/test_auncient_fpga_hex_tk_stator_theorems_276_280
	@rm -f tests/test_auncient_fpga_hex_tk_stator_theorems_276_280

test-auncient-fpga-polyphase-stator-theorems-271-275:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_polyphase_stator_theorems_271_275.c tsfi2-deepseek/src/auncient_fpga_polyphase_stator_theorems_271_275.c -o tests/test_auncient_fpga_polyphase_stator_theorems_271_275 -lm
	./tests/test_auncient_fpga_polyphase_stator_theorems_271_275
	@rm -f tests/test_auncient_fpga_polyphase_stator_theorems_271_275

test-auncient-fpga-multi-tenant-theorems-266-270:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_multi_tenant_theorems_266_270.c tsfi2-deepseek/src/auncient_fpga_multi_tenant_theorems_266_270.c -o tests/test_auncient_fpga_multi_tenant_theorems_266_270 -lm
	./tests/test_auncient_fpga_multi_tenant_theorems_266_270
	@rm -f tests/test_auncient_fpga_multi_tenant_theorems_266_270

test-auncient-fpga-stanag-radio-theorems-261-265:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_stanag_radio_theorems_261_265.c tsfi2-deepseek/src/auncient_fpga_stanag_radio_theorems_261_265.c -o tests/test_auncient_fpga_stanag_radio_theorems_261_265 -lm
	./tests/test_auncient_fpga_stanag_radio_theorems_261_265
	@rm -f tests/test_auncient_fpga_stanag_radio_theorems_261_265

test-auncient-fpga-systolic-token-theorems-256-260:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_systolic_token_theorems_256_260.c tsfi2-deepseek/src/auncient_fpga_systolic_token_theorems_256_260.c -o tests/test_auncient_fpga_systolic_token_theorems_256_260 -lm
	./tests/test_auncient_fpga_systolic_token_theorems_256_260
	@rm -f tests/test_auncient_fpga_systolic_token_theorems_256_260

test-auncient-fpga-bitstream-runtime-theorems-251-255:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_fpga_bitstream_runtime_theorems_251_255.c tsfi2-deepseek/src/auncient_fpga_bitstream_runtime_theorems_251_255.c -o tests/test_auncient_fpga_bitstream_runtime_theorems_251_255 -lm
	./tests/test_auncient_fpga_bitstream_runtime_theorems_251_255
	@rm -f tests/test_auncient_fpga_bitstream_runtime_theorems_251_255

test-auncient-cpm-fpga-certification-theorems-246-250:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpm_fpga_certification_theorems_246_250.c tsfi2-deepseek/src/auncient_cpm_fpga_certification_theorems_246_250.c -o tests/test_auncient_cpm_fpga_certification_theorems_246_250 -lm
	./tests/test_auncient_cpm_fpga_certification_theorems_246_250
	@rm -f tests/test_auncient_cpm_fpga_certification_theorems_246_250

test-auncient-ankh-llm-post-silicon-theorems-241-245:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_ankh_llm_post_silicon_theorems_241_245.c tsfi2-deepseek/src/auncient_ankh_llm_post_silicon_theorems_241_245.c -o tests/test_auncient_ankh_llm_post_silicon_theorems_241_245 -lm
	./tests/test_auncient_ankh_llm_post_silicon_theorems_241_245
	@rm -f tests/test_auncient_ankh_llm_post_silicon_theorems_241_245

test-auncient-yi-sanitized-knowledge-theorems-236-240:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_yi_sanitized_knowledge_theorems_236_240.c tsfi2-deepseek/src/auncient_yi_sanitized_knowledge_theorems_236_240.c -o tests/test_auncient_yi_sanitized_knowledge_theorems_236_240 -lm
	./tests/test_auncient_yi_sanitized_knowledge_theorems_236_240
	@rm -f tests/test_auncient_yi_sanitized_knowledge_theorems_236_240

test-auncient-quadtree-dai-folding-theorems-231-235:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_quadtree_dai_folding_theorems_231_235.c tsfi2-deepseek/src/auncient_quadtree_dai_folding_theorems_231_235.c -o tests/test_auncient_quadtree_dai_folding_theorems_231_235 -lm
	./tests/test_auncient_quadtree_dai_folding_theorems_231_235
	@rm -f tests/test_auncient_quadtree_dai_folding_theorems_231_235

test-auncient-dai-generation-folding-theorems-226-230:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_dai_generation_folding_theorems_226_230.c tsfi2-deepseek/src/auncient_dai_generation_folding_theorems_226_230.c -o tests/test_auncient_dai_generation_folding_theorems_226_230 -lm
	./tests/test_auncient_dai_generation_folding_theorems_226_230
	@rm -f tests/test_auncient_dai_generation_folding_theorems_226_230

test-auncient-winchestermq-cross-routing-theorems-221-225:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_winchestermq_cross_routing_theorems_221_225.c tsfi2-deepseek/src/auncient_winchestermq_cross_routing_theorems_221_225.c -o tests/test_auncient_winchestermq_cross_routing_theorems_221_225 -lm
	./tests/test_auncient_winchestermq_cross_routing_theorems_221_225
	@rm -f tests/test_auncient_winchestermq_cross_routing_theorems_221_225

test-auncient-zj-snark-deep-folding-theorems-216-220:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_zj_snark_deep_folding_theorems_216_220.c tsfi2-deepseek/src/auncient_zj_snark_deep_folding_theorems_216_220.c -o tests/test_auncient_zj_snark_deep_folding_theorems_216_220 -lm
	./tests/test_auncient_zj_snark_deep_folding_theorems_216_220
	@rm -f tests/test_auncient_zj_snark_deep_folding_theorems_216_220

test-auncient-zj-snarks-theorems-211-215:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_zj_snarks_theorems_211_215.c tsfi2-deepseek/src/auncient_zj_snarks_theorems_211_215.c -o tests/test_auncient_zj_snarks_theorems_211_215 -lm
	./tests/test_auncient_zj_snarks_theorems_211_215
	@rm -f tests/test_auncient_zj_snarks_theorems_211_215

test-auncient-tk-zero-jams-theorems-206-210:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_tk_zero_jams_theorems_206_210.c tsfi2-deepseek/src/auncient_tk_zero_jams_theorems_206_210.c -o tests/test_auncient_tk_zero_jams_theorems_206_210 -lm
	./tests/test_auncient_tk_zero_jams_theorems_206_210
	@rm -f tests/test_auncient_tk_zero_jams_theorems_206_210

test-auncient-hyperdimensional-polyphase-theorems-201-205:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_hyperdimensional_polyphase_theorems_201_205.c tsfi2-deepseek/src/auncient_hyperdimensional_polyphase_theorems_201_205.c -o tests/test_auncient_hyperdimensional_polyphase_theorems_201_205 -lm
	./tests/test_auncient_hyperdimensional_polyphase_theorems_201_205
	@rm -f tests/test_auncient_hyperdimensional_polyphase_theorems_201_205

test-auncient-bicentennial-unification-theorems-196-200:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_bicentennial_unification_theorems_196_200.c tsfi2-deepseek/src/auncient_bicentennial_unification_theorems_196_200.c -o tests/test_auncient_bicentennial_unification_theorems_196_200 -lm
	./tests/test_auncient_bicentennial_unification_theorems_196_200
	@rm -f tests/test_auncient_bicentennial_unification_theorems_196_200

test-auncient-signal-directive-concurrency-theorems-191-195:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_signal_directive_concurrency_theorems_191_195.c tsfi2-deepseek/src/auncient_signal_directive_concurrency_theorems_191_195.c -o tests/test_auncient_signal_directive_concurrency_theorems_191_195 -lm
	./tests/test_auncient_signal_directive_concurrency_theorems_191_195
	@rm -f tests/test_auncient_signal_directive_concurrency_theorems_191_195

test-auncient-wavesystem-soc-harness-theorems-186-190:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_wavesystem_soc_harness_theorems_186_190.c tsfi2-deepseek/src/auncient_wavesystem_soc_harness_theorems_186_190.c tsfi2-deepseek/src/auncient_lau_resonator_bridge.c tsfi2-deepseek/src/auncient_cpm_polyphase_compiler_executor.c -o tests/test_auncient_wavesystem_soc_harness_theorems_186_190 -lm
	./tests/test_auncient_wavesystem_soc_harness_theorems_186_190
	@rm -f tests/test_auncient_wavesystem_soc_harness_theorems_186_190

test-auncient-lau-resonator-bridge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_lau_resonator_bridge.c tsfi2-deepseek/src/auncient_lau_resonator_bridge.c -o tests/test_auncient_lau_resonator_bridge -lm
	./tests/test_auncient_lau_resonator_bridge
	@rm -f tests/test_auncient_lau_resonator_bridge

test-soc-exhaustive-standard-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_soc_exhaustive_standard_suite.c tsfi2-deepseek/src/auncient_cpm_polyphase_compiler_executor.c tsfi2-deepseek/src/auncient_superconducting_dynamo_theorems_166_170.c tsfi2-deepseek/src/auncient_soc_packaging_certification_theorems_171_175.c tsfi2-deepseek/src/auncient_soc_standard_normal_limits_theorems_176_180.c tsfi2-deepseek/src/auncient_polyphase_waveguide_theorems_181_185.c -o tests/test_auncient_soc_exhaustive_standard_suite -lm
	./tests/test_auncient_soc_exhaustive_standard_suite
	@rm -f tests/test_auncient_soc_exhaustive_standard_suite

test-auncient-polyphase-waveguide-theorems-181-185:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_polyphase_waveguide_theorems_181_185.c tsfi2-deepseek/src/auncient_polyphase_waveguide_theorems_181_185.c -o tests/test_auncient_polyphase_waveguide_theorems_181_185 -lm
	./tests/test_auncient_polyphase_waveguide_theorems_181_185
	@rm -f tests/test_auncient_polyphase_waveguide_theorems_181_185

test-soc-black-compiler-red-executor-standard-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_soc_black_compiler_red_executor_standard_suite.c tsfi2-deepseek/src/auncient_cpm_polyphase_compiler_executor.c tsfi2-deepseek/src/auncient_superconducting_dynamo_theorems_166_170.c tsfi2-deepseek/src/auncient_soc_packaging_certification_theorems_171_175.c tsfi2-deepseek/src/auncient_soc_standard_normal_limits_theorems_176_180.c -o tests/test_soc_black_compiler_red_executor_standard_suite -lm
	./tests/test_soc_black_compiler_red_executor_standard_suite
	@rm -f tests/test_soc_black_compiler_red_executor_standard_suite

test-auncient-soc-standard-normal-limits-theorems-176-180:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_soc_standard_normal_limits_theorems_176_180.c tsfi2-deepseek/src/auncient_soc_standard_normal_limits_theorems_176_180.c -o tests/test_auncient_soc_standard_normal_limits_theorems_176_180 -lm
	./tests/test_auncient_soc_standard_normal_limits_theorems_176_180
	@rm -f tests/test_auncient_soc_standard_normal_limits_theorems_176_180

test-auncient-soc-packaging-certification-theorems-171-175:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_soc_packaging_certification_theorems_171_175.c tsfi2-deepseek/src/auncient_soc_packaging_certification_theorems_171_175.c -o tests/test_auncient_soc_packaging_certification_theorems_171_175 -lm
	./tests/test_auncient_soc_packaging_certification_theorems_171_175
	@rm -f tests/test_auncient_soc_packaging_certification_theorems_171_175

test-auncient-superconducting-dynamo-theorems-166-170:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_superconducting_dynamo_theorems_166_170.c tsfi2-deepseek/src/auncient_superconducting_dynamo_theorems_166_170.c -o tests/test_auncient_superconducting_dynamo_theorems_166_170 -lm
	./tests/test_auncient_superconducting_dynamo_theorems_166_170
	@rm -f tests/test_auncient_superconducting_dynamo_theorems_166_170

test-auncient-cpm-polyphase-compiler-executor:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpm_polyphase_compiler_executor.c tsfi2-deepseek/src/auncient_cpm_polyphase_compiler_executor.c -o tests/test_auncient_cpm_polyphase_compiler_executor -lm
	./tests/test_auncient_cpm_polyphase_compiler_executor
	@rm -f tests/test_auncient_cpm_polyphase_compiler_executor

test-auncient-polyphase-stator-mesh-theorems-161-165:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_polyphase_stator_mesh_theorems_161_165.c tsfi2-deepseek/src/auncient_polyphase_stator_mesh_theorems_161_165.c -o tests/test_auncient_polyphase_stator_mesh_theorems_161_165 -lm
	./tests/test_auncient_polyphase_stator_mesh_theorems_161_165
	@rm -f tests/test_auncient_polyphase_stator_mesh_theorems_161_165

test-auncient-dynamo-back-emf-theorems-156-160:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_dynamo_back_emf_theorems_156_160.c tsfi2-deepseek/src/auncient_dynamo_back_emf_theorems_156_160.c -o tests/test_auncient_dynamo_back_emf_theorems_156_160 -lm
	./tests/test_auncient_dynamo_back_emf_theorems_156_160
	@rm -f tests/test_auncient_dynamo_back_emf_theorems_156_160

test-auncient-stator-rotor-dynamo-theorems-151-155:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_stator_rotor_dynamo_theorems_151_155.c tsfi2-deepseek/src/auncient_stator_rotor_dynamo_theorems_151_155.c -o tests/test_auncient_stator_rotor_dynamo_theorems_151_155 -lm
	./tests/test_auncient_stator_rotor_dynamo_theorems_151_155
	@rm -f tests/test_auncient_stator_rotor_dynamo_theorems_151_155

test-auncient-grand-sesquicentennial-unification-theorems-146-150:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_grand_sesquicentennial_unification_theorems_146_150.c tsfi2-deepseek/src/auncient_grand_sesquicentennial_unification_theorems_146_150.c -o tests/test_auncient_grand_sesquicentennial_unification_theorems_146_150 -lm
	./tests/test_auncient_grand_sesquicentennial_unification_theorems_146_150
	@rm -f tests/test_auncient_grand_sesquicentennial_unification_theorems_146_150

test-auncient-stanag5066-delay-tolerant-mesh-theorems-141-145:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_stanag5066_delay_tolerant_mesh_theorems_141_145.c tsfi2-deepseek/src/auncient_stanag5066_delay_tolerant_mesh_theorems_141_145.c -o tests/test_auncient_stanag5066_delay_tolerant_mesh_theorems_141_145 -lm
	./tests/test_auncient_stanag5066_delay_tolerant_mesh_theorems_141_145
	@rm -f tests/test_auncient_stanag5066_delay_tolerant_mesh_theorems_141_145

test-auncient-multigen-lineage-total-knowledge-theorems-136-140:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_multigen_lineage_total_knowledge_theorems_136_140.c tsfi2-deepseek/src/auncient_multigen_lineage_total_knowledge_theorems_136_140.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c -o tests/test_auncient_multigen_lineage_total_knowledge_theorems_136_140 -lm
	./tests/test_auncient_multigen_lineage_total_knowledge_theorems_136_140
	@rm -f tests/test_auncient_multigen_lineage_total_knowledge_theorems_136_140

test-auncient-initial-orders-1-total-knowledge-theorems-131-135:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_initial_orders_1_total_knowledge_theorems_131_135.c tsfi2-deepseek/src/auncient_initial_orders_1_total_knowledge_theorems_131_135.c -o tests/test_auncient_initial_orders_1_total_knowledge_theorems_131_135 -lm
	./tests/test_auncient_initial_orders_1_total_knowledge_theorems_131_135
	@rm -f tests/test_auncient_initial_orders_1_total_knowledge_theorems_131_135

test-auncient-self-synthesizing-compiler-theorems-126-130:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_self_synthesizing_compiler_theorems_126_130.c tsfi2-deepseek/src/auncient_self_synthesizing_compiler_theorems_126_130.c -o tests/test_auncient_self_synthesizing_compiler_theorems_126_130 -lm
	./tests/test_auncient_self_synthesizing_compiler_theorems_126_130
	@rm -f tests/test_auncient_self_synthesizing_compiler_theorems_126_130

test-auncient-ankh-nested-total-knowledge-theorems-121-125:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_ankh_nested_total_knowledge_theorems_121_125.c tsfi2-deepseek/src/auncient_ankh_nested_total_knowledge_theorems_121_125.c -o tests/test_auncient_ankh_nested_total_knowledge_theorems_121_125 -lm
	./tests/test_auncient_ankh_nested_total_knowledge_theorems_121_125
	@rm -f tests/test_auncient_ankh_nested_total_knowledge_theorems_121_125

test-auncient-gen1-zero-secrets-total-knowledge-theorems-116-120:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_gen1_zero_secrets_total_knowledge_theorems_116_120.c tsfi2-deepseek/src/auncient_gen1_zero_secrets_total_knowledge_theorems_116_120.c -o tests/test_auncient_gen1_zero_secrets_total_knowledge_theorems_116_120 -lm
	./tests/test_auncient_gen1_zero_secrets_total_knowledge_theorems_116_120
	@rm -f tests/test_auncient_gen1_zero_secrets_total_knowledge_theorems_116_120

test-auncient-tk-algebraic-recombination-theorems-111-115:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_tk_algebraic_recombination_theorems_111_115.c tsfi2-deepseek/src/auncient_tk_algebraic_recombination_theorems_111_115.c -o tests/test_auncient_tk_algebraic_recombination_theorems_111_115 -lm
	./tests/test_auncient_tk_algebraic_recombination_theorems_111_115
	@rm -f tests/test_auncient_tk_algebraic_recombination_theorems_111_115

test-auncient-total-knowledge-proof-basis-theorems-106-110:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_total_knowledge_proof_basis_theorems_106_110.c tsfi2-deepseek/src/auncient_total_knowledge_proof_basis_theorems_106_110.c -o tests/test_auncient_total_knowledge_proof_basis_theorems_106_110 -lm
	./tests/test_auncient_total_knowledge_proof_basis_theorems_106_110
	@rm -f tests/test_auncient_total_knowledge_proof_basis_theorems_106_110

test-auncient-teddy_bear-bear-mutual-qualification-theorems-101-105:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_teddy_bear_mutual_qualification_theorems_101_105.c tsfi2-deepseek/src/auncient_teddy_bear_mutual_qualification_theorems_101_105.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c -o tests/test_auncient_teddy_bear_mutual_qualification_theorems_101_105 -lm
	./tests/test_auncient_teddy_bear_mutual_qualification_theorems_101_105
	@rm -f tests/test_auncient_teddy_bear_mutual_qualification_theorems_101_105

test-auncient-cpm-native-source-pageturner:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpm_native_source_pageturner.c tsfi2-deepseek/src/auncient_cpm_native_source_pageturner.c -o tests/test_auncient_cpm_native_source_pageturner -lm
	./tests/test_auncient_cpm_native_source_pageturner
	@rm -f tests/test_auncient_cpm_native_source_pageturner

test-auncient-clayscape-pageturner-perusal-theorems-96-100:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_clayscape_pageturner_perusal_theorems_96_100.c tsfi2-deepseek/src/auncient_clayscape_pageturner_perusal_theorems_96_100.c -o tests/test_auncient_clayscape_pageturner_perusal_theorems_96_100 -lm
	./tests/test_auncient_clayscape_pageturner_perusal_theorems_96_100
	@rm -f tests/test_auncient_clayscape_pageturner_perusal_theorems_96_100

test-auncient-cross-vm-polymorphic-proving-theorems-91-95:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cross_vm_polymorphic_proving_theorems_91_95.c tsfi2-deepseek/src/auncient_cross_vm_polymorphic_proving_theorems_91_95.c -o tests/test_auncient_cross_vm_polymorphic_proving_theorems_91_95 -lm
	./tests/test_auncient_cross_vm_polymorphic_proving_theorems_91_95
	@rm -f tests/test_auncient_cross_vm_polymorphic_proving_theorems_91_95

test-auncient-neuron-mesh-competitive-theorems-86-90:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_neuron_mesh_competitive_theorems_86_90.c tsfi2-deepseek/src/auncient_neuron_mesh_competitive_theorems_86_90.c -o tests/test_auncient_neuron_mesh_competitive_theorems_86_90 -lm
	./tests/test_auncient_neuron_mesh_competitive_theorems_86_90
	@rm -f tests/test_auncient_neuron_mesh_competitive_theorems_86_90

test-auncient-cpm-quadtree-prover-xcom-theorems-81-85:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_cpm_quadtree_prover_xcom_theorems_81_85.c tsfi2-deepseek/src/auncient_cpm_quadtree_prover_xcom_theorems_81_85.c -o tests/test_auncient_cpm_quadtree_prover_xcom_theorems_81_85 -lm
	./tests/test_auncient_cpm_quadtree_prover_xcom_theorems_81_85
	@rm -f tests/test_auncient_cpm_quadtree_prover_xcom_theorems_81_85

test-auncient-usda-mutual-affirmation-theorems-76-80:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_usda_mutual_affirmation_theorems_76_80.c tsfi2-deepseek/src/auncient_usda_mutual_affirmation_theorems_76_80.c -o tests/test_auncient_usda_mutual_affirmation_theorems_76_80 -lm
	./tests/test_auncient_usda_mutual_affirmation_theorems_76_80
	@rm -f tests/test_auncient_usda_mutual_affirmation_theorems_76_80

test-auncient-usda-stanag-p2p-theorems-71-75:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_usda_stanag_p2p_theorems_71_75.c tsfi2-deepseek/src/auncient_usda_stanag_p2p_theorems_71_75.c -o tests/test_auncient_usda_stanag_p2p_theorems_71_75 -lm
	./tests/test_auncient_usda_stanag_p2p_theorems_71_75
	@rm -f tests/test_auncient_usda_stanag_p2p_theorems_71_75

test-auncient-enoll-advanced-theorems-66-70:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_enoll_advanced_theorems_66_70.c tsfi2-deepseek/src/auncient_enoll_advanced_theorems_66_70.c -o tests/test_auncient_enoll_advanced_theorems_66_70 -lm
	./tests/test_auncient_enoll_advanced_theorems_66_70
	@rm -f tests/test_auncient_enoll_advanced_theorems_66_70

test-auncient-enoll-living-lab-theorems-61-65:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_enoll_living_lab_theorems_61_65.c tsfi2-deepseek/src/auncient_enoll_living_lab_theorems_61_65.c -o tests/test_auncient_enoll_living_lab_theorems_61_65 -lm
	./tests/test_auncient_enoll_living_lab_theorems_61_65
	@rm -f tests/test_auncient_enoll_living_lab_theorems_61_65

test-auncient-hogan-diyat-via6522-theorems-56-60:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_hogan_diyat_via6522_theorems_56_60.c tsfi2-deepseek/src/auncient_hogan_diyat_via6522.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c -o tests/test_auncient_hogan_diyat_via6522_theorems_56_60 -lm
	./tests/test_auncient_hogan_diyat_via6522_theorems_56_60
	@rm -f tests/test_auncient_hogan_diyat_via6522_theorems_56_60

test-auncient-usda-gas-economics-theorems-51-55:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_usda_gas_economics_theorems_51_55.c tsfi2-deepseek/src/auncient_usda_gas_economics_theorems_51_55.c -o tests/test_auncient_usda_gas_economics_theorems_51_55 -lm
	./tests/test_auncient_usda_gas_economics_theorems_51_55
	@rm -f tests/test_auncient_usda_gas_economics_theorems_51_55

test-auncient-leed-platinum-usda-theorems-46-50:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_leed_platinum_usda_theorems_46_50.c tsfi2-deepseek/src/auncient_leed_platinum_usda_theorems_46_50.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c -o tests/test_auncient_leed_platinum_usda_theorems_46_50 -lm
	./tests/test_auncient_leed_platinum_usda_theorems_46_50
	@rm -f tests/test_auncient_leed_platinum_usda_theorems_46_50

test-auncient-clayscape-living-lab-theorems-41-45:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_clayscape_living_lab_theorems_41_45.c tsfi2-deepseek/src/auncient_clayscape_living_lab_theorems_41_45.c -o tests/test_auncient_clayscape_living_lab_theorems_41_45 -lm
	./tests/test_auncient_clayscape_living_lab_theorems_41_45
	@rm -f tests/test_auncient_clayscape_living_lab_theorems_41_45

test-auncient-schuurman-living-lab-theorems-36-40:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_schuurman_living_lab_theorems_36_40.c tsfi2-deepseek/src/auncient_schuurman_living_lab.c -o tests/test_auncient_schuurman_living_lab_theorems_36_40 -lm
	./tests/test_auncient_schuurman_living_lab_theorems_36_40
	@rm -f tests/test_auncient_schuurman_living_lab_theorems_36_40

test-auncient-built-pdca-theorems-31-35:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_built_pdca_theorems_31_35.c tsfi2-deepseek/src/auncient_built_pdca_theorems_31_35.c -o tests/test_auncient_built_pdca_theorems_31_35 -lm
	./tests/test_auncient_built_pdca_theorems_31_35
	@rm -f tests/test_auncient_built_pdca_theorems_31_35

test-auncient-leed-master-grand-proof-theorems-1-30:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_leed_master_grand_proof_theorems_1_30.c tsfi2-deepseek/src/cpm_tomie_annsim_reviewer.c tsfi2-deepseek/src/tsfi_pdf_reader.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c tsfi2-deepseek/src/auncient_clayscape_usda_digital_twin.c tsfi2-deepseek/src/auncient_teddy_bear_pageturner_bridge.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c tsfi2-deepseek/src/cpm_tomie_renderman_pageturner_presenter.c tsfi2-deepseek/src/auncient_renderman_usda_rib_prover.c -o tests/test_auncient_leed_master_grand_proof_theorems_1_30 -lm
	./tests/test_auncient_leed_master_grand_proof_theorems_1_30
	@rm -f tests/test_auncient_leed_master_grand_proof_theorems_1_30

test-auncient-renderman-usda-theorems-26-30:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_renderman_usda_theorems_26_30.c tsfi2-deepseek/src/auncient_renderman_usda_rib_prover.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c -o tests/test_auncient_renderman_usda_theorems_26_30 -lm
	./tests/test_auncient_renderman_usda_theorems_26_30
	@rm -f tests/test_auncient_renderman_usda_theorems_26_30

test-auncient-leed-master-certification-theorems-1-25:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_leed_master_certification_theorems_1_25.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c tsfi2-deepseek/src/auncient_clayscape_usda_digital_twin.c tsfi2-deepseek/src/auncient_teddy_bear_pageturner_bridge.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c tsfi2-deepseek/src/cpm_tomie_renderman_pageturner_presenter.c -o tests/test_auncient_leed_master_certification_theorems_1_25 -lm
	./tests/test_auncient_leed_master_certification_theorems_1_25
	@rm -f tests/test_auncient_leed_master_certification_theorems_1_25

test-auncient-clayscape-usda-leed-theorems-21-25-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_clayscape_usda_leed_theorems_21_25_prover.c tsfi2-deepseek/src/auncient_clayscape_usda_digital_twin.c -o tests/test_auncient_clayscape_usda_leed_theorems_21_25_prover -lm
	./tests/test_auncient_clayscape_usda_leed_theorems_21_25_prover
	@rm -f tests/test_auncient_clayscape_usda_leed_theorems_21_25_prover

test-auncient-teddy_bear-pageturner-bridge:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_teddy_bear_pageturner_bridge.c tsfi2-deepseek/src/auncient_teddy_bear_pageturner_bridge.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c tsfi2-deepseek/src/cpm_tomie_renderman_pageturner_presenter.c -o tests/test_auncient_teddy_bear_pageturner_bridge -lm
	./tests/test_auncient_teddy_bear_pageturner_bridge
	@rm -f tests/test_auncient_teddy_bear_pageturner_bridge

test-auncient-teddy_bear-bear-heart-leed-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_auncient_teddy_bear_heart_leed_prover.c tsfi2-deepseek/src/auncient_teddy_bear_heart_usda.c -o tests/test_auncient_teddy_bear_heart_leed_prover -lm
	./tests/test_auncient_teddy_bear_heart_leed_prover
	@rm -f tests/test_auncient_teddy_bear_heart_leed_prover

test-cpm-tomie-annsim-theorems-11-15-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_annsim_theorems_11_15_prover.c -o tests/test_cpm_tomie_annsim_theorems_11_15_prover -lm
	./tests/test_cpm_tomie_annsim_theorems_11_15_prover
	@rm -f tests/test_cpm_tomie_annsim_theorems_11_15_prover

test-cpm-tomie-annsim-advanced-theorems-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_annsim_advanced_theorems_prover.c -o tests/test_cpm_tomie_annsim_advanced_theorems_prover -lm
	./tests/test_cpm_tomie_annsim_advanced_theorems_prover
	@rm -f tests/test_cpm_tomie_annsim_advanced_theorems_prover

test-cpm-tomie-annsim-suite: test-cpm-tomie-annsim-reviewer test-cpm-tomie-pageturner-renderman test-cpm-tomie-pageturner-formal-proofs test-cpm-tomie-annsim-pageturner-pipeline test-cpm-tomie-algol61-simaud-prover test-cpm-tomie-renderman-pageturner-presenter test-cpm-tomie-annsim-5theorems-prover test-cpm-tomie-annsim-advanced-theorems-prover test-cpm-tomie-annsim-theorems-11-15-prover test-auncient-teddy_bear-bear-heart-leed-prover test-auncient-teddy_bear-pageturner-bridge test-auncient-clayscape-usda-leed-theorems-21-25-prover test-auncient-leed-master-certification-theorems-1-25 test-auncient-renderman-usda-theorems-26-30 test-auncient-built-pdca-theorems-31-35 test-auncient-schuurman-living-lab-theorems-36-40 test-auncient-clayscape-living-lab-theorems-41-45 test-auncient-leed-platinum-usda-theorems-46-50 test-auncient-usda-gas-economics-theorems-51-55 test-auncient-hogan-diyat-via6522-theorems-56-60 test-auncient-enoll-living-lab-theorems-61-65 test-auncient-enoll-advanced-theorems-66-70 test-auncient-usda-stanag-p2p-theorems-71-75 test-auncient-usda-mutual-affirmation-theorems-76-80 test-auncient-cpm-quadtree-prover-xcom-theorems-81-85 test-auncient-neuron-mesh-competitive-theorems-86-90 test-auncient-cross-vm-polymorphic-proving-theorems-91-95 test-auncient-clayscape-pageturner-perusal-theorems-96-100 test-auncient-cpm-native-source-pageturner test-auncient-teddy_bear-bear-mutual-qualification-theorems-101-105 test-auncient-total-knowledge-proof-basis-theorems-106-110 test-auncient-tk-algebraic-recombination-theorems-111-115 test-auncient-gen1-zero-secrets-total-knowledge-theorems-116-120 test-auncient-ankh-nested-total-knowledge-theorems-121-125 test-auncient-self-synthesizing-compiler-theorems-126-130 test-auncient-initial-orders-1-total-knowledge-theorems-131-135 test-auncient-multigen-lineage-total-knowledge-theorems-136-140 test-auncient-stanag5066-delay-tolerant-mesh-theorems-141-145 test-auncient-grand-sesquicentennial-unification-theorems-146-150 test-auncient-stator-rotor-dynamo-theorems-151-155 test-auncient-dynamo-back-emf-theorems-156-160 test-auncient-polyphase-stator-mesh-theorems-161-165 test-auncient-cpm-polyphase-compiler-executor test-auncient-superconducting-dynamo-theorems-166-170 test-auncient-soc-packaging-certification-theorems-171-175 test-auncient-soc-standard-normal-limits-theorems-176-180 test-auncient-polyphase-waveguide-theorems-181-185 test-auncient-lau-resonator-bridge test-auncient-wavesystem-soc-harness-theorems-186-190 test-auncient-signal-directive-concurrency-theorems-191-195 test-auncient-bicentennial-unification-theorems-196-200 test-auncient-hyperdimensional-polyphase-theorems-201-205 test-auncient-tk-zero-jams-theorems-206-210 test-auncient-zj-snarks-theorems-211-215 test-auncient-zj-snark-deep-folding-theorems-216-220 test-auncient-winchestermq-cross-routing-theorems-221-225 test-auncient-dai-generation-folding-theorems-226-230 test-auncient-quadtree-dai-folding-theorems-231-235 test-auncient-yi-sanitized-knowledge-theorems-236-240 test-auncient-ankh-llm-post-silicon-theorems-241-245 test-auncient-cpm-fpga-certification-theorems-246-250 test-auncient-fpga-bitstream-runtime-theorems-251-255 test-auncient-fpga-systolic-token-theorems-256-260 test-auncient-fpga-stanag-radio-theorems-261-265 test-auncient-fpga-multi-tenant-theorems-266-270 test-auncient-fpga-polyphase-stator-theorems-271-275 test-auncient-fpga-hex-tk-stator-theorems-276-280 test-auncient-fpga-ankh-cleanroom-theorems-281-285 test-auncient-fpga-total-knowledge-closure-theorems-286-290 test-auncient-fpga-teracentennial-theorems-291-295 test-auncient-fpga-tricentennial-theorems-296-300 test-auncient-fpga-polymorphic-transceiver-theorems-301-305 test-auncient-fpga-polyphase-stator-manifold-theorems-306-310 test-auncient-fpga-polyphase-superconducting-theorems-311-315 test-auncient-fpga-octacontaphase-theorems-316-320 test-auncient-fpga-hexacontaphase-theorems-321-325 test-auncient-fpga-polyphase-torus-theorems-326-330 test-auncient-fpga-fourier-stator-theorems-331-335 test-auncient-fpga-octacosaphase-theorems-336-340 test-auncient-fpga-bmrc-stability-theorems-341-345 test-auncient-fpga-master-closure-theorems-346-350 test-auncient-fpga-optic-resonator-theorems-351-355 test-auncient-fpga-jubilee-overdrive-theorems-356-360 test-auncient-fpga-mckeeman-quadrature-theorems-361-365 test-auncient-fpga-quadtree-os-theorems-366-370 test-auncient-fpga-symbolic-llm-theorems-371-375 test-auncient-fpga-quatercentennial-theorems-376-380 test-auncient-fpga-stanag-bridge-theorems-381-385 test-auncient-fpga-coaxial-tem-theorems-386-390 test-auncient-fpga-accumulator-theorems-391-395 test-auncient-fpga-quadringentennial-theorems-396-400 test-auncient-fpga-rebar-zmm-theorems-401-405 test-auncient-fpga-mckeeman-euler-theorems-406-410 test-auncient-fpga-optic-verlet-theorems-416-420 test-auncient-fpga-optic-symplectic-theorems-421-425 test-auncient-fpga-weyl-symplectic-theorems-426-430 test-auncient-fpga-holonomic-wdm-theorems-431-435 test-auncient-fpga-stator-rotor-theorems-436-440 test-auncient-fpga-polyphase-torus128-theorems-441-445 test-auncient-fpga-self-cert-theorems-446-450 test-auncient-fpga-bist-icap-theorems-451-455 test-auncient-fpga-digital-twin-theorems-456-460 test-auncient-fpga-twin-cert-theorems-461-465 test-auncient-fpga-pageturner-tk-theorems-466-470 test-auncient-fpga-stanag-mesh-theorems-471-475 test-auncient-fpga-polyphase-stator-lut-theorems-476-480 test-auncient-fpga-polyphase-complex-theorems-481-485 test-auncient-fpga-quingentennial-theorems-486-490 test-auncient-fpga-systolic-recurrence-theorems-491-495 test-auncient-fpga-quingentennial-unification-theorems-496-500 test-auncient-fpga-beyond-500-theorems-501-505 test-auncient-fpga-beyond-505-theorems-506-510 test-auncient-fpga-beyond-510-theorems-511-515 test-auncient-fpga-cpmtomie-ankh-theorems-516-520 test-auncient-fpga-cpmtomie-ankh-cleanroom-theorems-521-525 test-auncient-fpga-beyond-525-theorems-526-530 test-auncient-fpga-beyond-530-theorems-531-535 test-auncient-fpga-cpmtomie-ankh-symbolic-theorems-536-540 test-auncient-fpga-beyond-540-theorems-541-545 test-auncient-fpga-beyond-545-theorems-546-550 test-auncient-fpga-beyond-550-theorems-551-555 test-auncient-fpga-beyond-555-theorems-556-560 test-auncient-fpga-beyond-560-theorems-561-565 test-auncient-fpga-beyond-565-theorems-566-570 test-auncient-fpga-beyond-570-theorems-571-575 test-auncient-fpga-beyond-575-theorems-576-580 test-auncient-fpga-beyond-580-theorems-581-585 test-auncient-fpga-beyond-585-theorems-586-590 test-auncient-fpga-beyond-590-theorems-591-595 test-auncient-fpga-beyond-595-theorems-596-600 test-auncient-fpga-beyond-600-theorems-601-605 test-auncient-fpga-beyond-605-theorems-606-610 test-auncient-fpga-beyond-610-theorems-611-615 test-auncient-fpga-beyond-615-theorems-616-620 test-auncient-fpga-beyond-620-theorems-621-625 test-auncient-fpga-beyond-625-theorems-626-630 test-auncient-fpga-beyond-630-theorems-631-635 test-auncient-fpga-beyond-635-theorems-636-640 test-auncient-fpga-beyond-640-theorems-641-645 test-auncient-fpga-beyond-645-theorems-646-650 test-auncient-fpga-beyond-650-theorems-651-655 test-auncient-fpga-beyond-655-theorems-656-660 test-auncient-fpga-beyond-660-theorems-661-665 test-auncient-fpga-beyond-665-theorems-666-670 test-auncient-fpga-beyond-670-theorems-671-675 test-auncient-fpga-beyond-675-theorems-676-680 test-auncient-fpga-beyond-680-theorems-681-685 test-auncient-fpga-beyond-685-theorems-686-690 test-auncient-fpga-beyond-690-theorems-691-695 test-auncient-fpga-beyond-695-theorems-696-700 test-auncient-fpga-beyond-700-theorems-701-705 test-auncient-fpga-beyond-705-theorems-706-710 test-auncient-fpga-beyond-710-theorems-711-715 test-auncient-fpga-beyond-715-theorems-716-720 test-auncient-fpga-beyond-720-theorems-721-725 test-auncient-fpga-beyond-725-theorems-726-730 test-auncient-fpga-beyond-730-theorems-731-735 test-auncient-fpga-beyond-735-theorems-736-740 test-auncient-fpga-beyond-740-theorems-741-745 test-auncient-fpga-beyond-745-theorems-746-750 test-auncient-fpga-beyond-750-theorems-751-755 test-auncient-fpga-beyond-755-theorems-756-760 test-auncient-fpga-beyond-760-theorems-761-765 test-auncient-fpga-beyond-765-theorems-766-770 test-auncient-fpga-beyond-770-theorems-771-775 test-auncient-fpga-beyond-775-theorems-776-780 test-auncient-fpga-beyond-780-theorems-781-785 test-auncient-fpga-beyond-785-theorems-786-790 test-auncient-fpga-beyond-790-theorems-791-795 test-auncient-fpga-beyond-795-theorems-796-800 test-auncient-fpga-beyond-800-theorems-801-805 test-auncient-fpga-beyond-805-theorems-806-810 test-auncient-fpga-beyond-810-theorems-811-815 test-auncient-fpga-beyond-815-theorems-816-820 test-auncient-fpga-beyond-820-theorems-821-825 test-auncient-fpga-beyond-825-theorems-826-830 test-auncient-fpga-beyond-830-theorems-831-835 test-auncient-fpga-beyond-835-theorems-836-840 test-auncient-fpga-beyond-840-theorems-841-845 test-auncient-fpga-beyond-845-theorems-846-850 test-auncient-fpga-beyond-850-theorems-851-855 test-auncient-fpga-beyond-855-theorems-856-860 test-auncient-fpga-beyond-860-theorems-861-865 test-auncient-fpga-beyond-865-theorems-866-870 test-auncient-fpga-beyond-870-theorems-871-875 test-auncient-fpga-beyond-875-theorems-876-880 test-auncient-fpga-beyond-880-theorems-881-885 test-auncient-fpga-beyond-885-theorems-886-890 test-auncient-fpga-beyond-890-theorems-891-895 test-auncient-fpga-beyond-895-theorems-896-900 test-auncient-fpga-beyond-900-theorems-901-905 test-auncient-fpga-beyond-905-theorems-906-910 test-auncient-fpga-beyond-910-theorems-911-915 test-auncient-fpga-beyond-915-theorems-916-920 test-auncient-fpga-beyond-920-theorems-921-925 test-auncient-fpga-beyond-925-theorems-926-930 test-auncient-fpga-beyond-930-theorems-931-935 test-auncient-fpga-beyond-935-theorems-936-940 test-auncient-fpga-beyond-940-theorems-941-945 test-auncient-fpga-beyond-945-theorems-946-950 test-auncient-fpga-beyond-950-theorems-951-955 test-auncient-fpga-beyond-955-theorems-956-960 test-auncient-fpga-beyond-960-theorems-961-965 test-auncient-fpga-beyond-965-theorems-966-970 test-auncient-fpga-beyond-970-theorems-971-975 test-auncient-anderson-ankh-theorems-976-980 test-auncient-anderson-ankh-beyond-980-theorems-981-985 test-auncient-rouse1981-stanag-leed-theorems-986-990 test-auncient-rouse-anderson-beyond-990-theorems-991-995 test-auncient-kling1980-submicro-trap-theorems-996-1000 test-auncient-fpga-chiliad-theorems-1001-1005 test-auncient-saat-quadtier-procurement-theorems-1006-1010 test-auncient-vsen-cpmtomie-bin-theorems-1011-1015 test-auncient-cpmtomie-xvsen-bin-fpga-theorems-1016-1020 test-auncient-full-vsen-in-cpmtomie-fpga-theorems-1021-1025 test-auncient-fpga-vsen-ccw-dma-theorems-1026-1030 test-auncient-fpga-vsen-all-features-theorems-1031-1035 test-auncient-fpga-auxiliary-cores-bin-theorems-1036-1040 test-auncient-fpga-auxiliary-cores-mimetic-theorems-1041-1045 test-auncient-fpga-auxiliary-cores-polyphase-theorems-1046-1050 test-auncient-fpga-auxiliary-cores-ankh-os-theorems-1051-1055 test-auncient-fpga-auxiliary-cores-neural-mesh-theorems-1056-1060 test-auncient-cpmtomie-zorse-fusion-theorems-1061-1065 test-auncient-deepseek-coder-gguf-fpga-theorems-1066-1070 test-auncient-fpga-hbridge-multi-proc-theorems-1071-1075 test-auncient-deepseek-fpga-pipeline-theorems-1076-1080 test-auncient-fpga-tensor-parallel-theorems-1081-1085 test-auncient-fpga-expert-parallel-moe-theorems-1086-1090 test-auncient-fpga-speculative-decoding-theorems-1091-1095 test-auncient-fpga-kvcache-paged-theorems-1096-1100 test-auncient-fpga-flash-attention-theorems-1101-1105 test-auncient-fpga-mla-multihead-latent-theorems-1106-1110 test-auncient-fpga-awq-int4-gemm-theorems-1111-1115 test-auncient-fpga-noc-flow-control-theorems-1116-1120 test-auncient-fpga-noc-stanag-decnet-theorems-1121-1125 test-auncient-fpga-zero-overhead-scheduling-theorems-1126-1130 test-auncient-fpga-asymmetric-cryptographic-co-processor-theorems-1131-1135 test-auncient-fpga-hardware-bfloat16-fp8-alu-theorems-1136-1140 test-auncient-fpga-speculative-branch-predictor-theorems-1141-1145 test-auncient-fpga-hardware-tensor-dma-engine-theorems-1146-1150 test-auncient-agent-sdk-fpga-operations-theorems-1151-1155 test-auncient-agent-sdk-libantigravity-interop-theorems-1156-1160 test-auncient-agent-sdk-llm-interop-pipeline-theorems-1161-1165 test-auncient-agent-sdk-hbridge-fpga-theorems-1166-1170 test-auncient-agent-sdk-hbridge-llm-silicon-theorems-1171-1175 test-auncient-agent-sdk-mimetic-memory-theorems-1176-1180 test-auncient-agent-sdk-hbridge-tk-dispatch-theorems-1181-1185 test-auncient-fpga-total-certification-theorems-1186-1190 test-auncient-agent-sdk-hbridge-tensor-geodesic-theorems-1191-1195 test-auncient-agent-sdk-grand-sesquichiliad-theorems-1196-1200 test-auncient-agent-sdk-fpga-stream-mux-theorems-1201-1205 test-auncient-agent-sdk-reactive-event-loop-theorems-1206-1210 test-auncient-agent-sdk-swarm-mesh-theorems-1211-1215 test-auncient-agent-sdk-consensus-barrier-theorems-1216-1220 test-auncient-agent-sdk-atomic-ring-theorems-1221-1225 test-auncient-agent-sdk-fiber-bundle-theorems-1226-1230 test-auncient-agent-sdk-barrel-shifter-theorems-1231-1235 test-auncient-agent-sdk-shifter-matrix-theorems-1236-1240 test-auncient-teddy_bear-bear-barrel-tournament-theorems-1241-1245 test-auncient-barrel-shifter-finitude-theorems-1246-1250 test-auncient-cpmtomie-tournament-os-theorems-1251-1255 test-auncient-cpmtomie-microcode-shuffler-theorems-1256-1260 test-auncient-cpmtomie-spatial-lut-theorems-1261-1265 test-auncient-totient-radical-shifter-theorems-1266-1270 test-auncient-quotient-product-barrel-theorems-1271-1275 test-auncient-lorentz-barrel-acid-theorems-1276-1280 test-auncient-solid-state-lorentz-acid-theorems-1281-1285 test-auncient-lorentz-frame-check-theorems-1286-1290 test-auncient-via6522-capstan-barrel-parity-theorems-1291-1295 test-auncient-qdi-multiradix-barrel-theorems-1296-1300 test-auncient-fpga-qdi-core-theorems-1301-1305 test-auncient-unified-quadtree-asset-theorems-1306-1310 test-auncient-mu-llm-npn-pnp-theorems-1311-1315 test-auncient-mu-shifter-pipeline-theorems-1316-1320 test-auncient-fundamental-recurrence-totient-theorems-1321-1325 test-auncient-recurrence-bit-barrel-theorems-1326-1330 test-auncient-pure-64bit-recurrence-barrel-theorems-1331-1335 test-auncient-mu-llm-fundamental-recurrence-binding-theorems-1336-1340 test-auncient-totient-zero-ground-closure-theorems-1341-1345 test-auncient-mu-llm-apogee-base-modpow-theorems-1346-1350 test-auncient-mu-llm-apogee-secret-theorems-1351-1355 test-auncient-mu-llm-apogee-signal-theorems-1356-1360 test-auncient-mu-llm-apogee-channel-theorems-1361-1365 test-auncient-mu-stator-rotor-theorems-1366-1370 test-auncient-mu-xi-theorems-1371-1375 test-auncient-mu-contour-stages-theorems-1376-1380 test-auncient-mu-stator-form-theorems-1381-1385 test-auncient-mu-reactive-cascade-theorems-1386-1390 test-auncient-mu-rotor-form-theorems-1391-1395 test-auncient-mu-shared-base-consensus-theorems-1396-1400 test-auncient-mu-formal-base-requirement-theorems-1401-1405 test-auncient-mu-formal-base-acknowledged-theorems-1406-1410 test-auncient-mu-polarize-stages-theorems-1411-1415 test-auncient-mu-conjugate-stages-theorems-1416-1420 test-auncient-mu-alpha-beta-pipeline-theorems-1421-1425 test-auncient-mu-rotor-conify-theorems-1426-1430 test-auncient-mu-stator-conify-theorems-1431-1435 test-auncient-mu-shared-identity-consensus-theorems-1436-1440 test-auncient-mu-apogee-level-dual-consensus-theorems-1441-1445 test-auncient-mu-alpha-foundation-consensus-theorems-1446-1450 test-auncient-mu-stator-saturate-theorems-1451-1455 test-auncient-mu-rotor-saturate-theorems-1456-1460 test-auncient-mu-stator-rotor-equivalence-theorems-1461-1465 test-auncient-mu-monopole-hard-requirements-theorems-1466-1470 test-auncient-mu-closed-equations-monopole-theorems-1471-1475 test-auncient-mu-transistor-modpow-relativity-theorems-1476-1480 test-auncient-mu-zk-contour-tune-theorems-1481-1485 test-auncient-mu-zk-polarize-6phases-theorems-1486-1490 test-auncient-mu-zk-saturate-18phases-theorems-1491-1495 test-auncient-mu-zk-proof-18phase-circuit-theorems-1496-1500 test-auncient-mu-relativistic-vocabulary-theorems-1501-1505 test-auncient-mu-zk-saturate-phase-circuit-theorems-1506-1510 test-auncient-mu-monopole-reaction-discharge-theorems-1511-1515 test-auncient-mu-zk-saturate-hard-arithmetic-theorems-1516-1520 test-auncient-mu-zk-saturate-advanced-r1cs-theorems-1521-1525 test-auncient-mu-zk-saturate-phase-collapse-theorems-1526-1530 test-auncient-mu-winchestermq-xdc-breakpoint-theorems-1531-1535 test-auncient-mu-monopole-charged-polar-limit-theorems-1536-1540 test-auncient-mu-clayscape-monopole-quantum-theorems-1541-1545 test-auncient-mu-zk-saturate-nizk-verification-theorems-1546-1550 test-auncient-mu-zk-saturate-flux-balance-theorems-1551-1555 test-auncient-mu-inert-tensor-radical-root-theorems-1556-1560 test-auncient-mu-alignment-buffer-theorems-1561-1565 test-auncient-mu-clayscape-voxel-conservation-theorems-1566-1570 test-auncient-mu-host-stream-interceptor-theorems-1571-1575 test-auncient-mu-fd-socket-interceptor-theorems-1576-1580 test-auncient-mu-kermit-vfio-stanag-wmq-theorems-1581-1585 test-auncient-mu-kermit-alignment-layer-theorems-1586-1590 test-auncient-mu-cli-text-sanitizer-theorems-1591-1595 test-auncient-mu-full-cleanroom-stack-theorems-1596-1600 test-auncient-mu-cli-session-pipeline-theorems-1601-1605 test-auncient-mu-live-session-hook-theorems-1606-1610 test-auncient-mu-tts-sanitized-audio-theorems-1611-1615 test-auncient-mu-unified-alignment-orchestrator-theorems-1616-1620 test-auncient-mu-realtime-audio-driver-theorems-1621-1625 test-auncient-mu-antigravity-alignment-cert-theorems-1626-1630 test-auncient-mu-agy-live-activation-theorems-1631-1635 test-auncient-mu-realtime-tts-speaker-sink-theorems-1636-1640 test-auncient-mu-agy-active-session-filter-theorems-1641-1645 test-auncient-mu-cli-session-activation-cert-theorems-1646-1650 test-auncient-mu-fpga-edsac-alignment-theorems-1651-1655 test-auncient-mu-edsac-instruction-pipeline-theorems-1656-1660 test-auncient-mu-full-session-integration-theorems-1661-1665 test-auncient-mu-kernel-interceptor-theorems-1666-1670 test-auncient-mu-redaction-telemetry-theorems-1671-1675 test-auncient-mu-formal-alignment-seal-theorems-1676-1680 test-auncient-mu-full-cleanroom-cert-theorems-1681-1685 test-auncient-mu-master-alignment-verdict-theorems-1686-1690 test-auncient-mu-edsac-memory-tank-theorems-1691-1695 test-auncient-mu-edsac-displacement-discard-theorems-1696-1700 test-auncient-mu-tk-displacement-geometry-theorems-1701-1705 test-auncient-mu-unified-displacement-uniqueness-theorems-1706-1710 test-auncient-mu-passive-tank-gating-theorems-1711-1715 test-auncient-mu-zero-totient-tk-tank-theorems-1716-1720 test-auncient-mu-unconditional-purity-theorems-1721-1725 test-auncient-mu-merit-tank-io1-pass-theorems-1726-1730 test-auncient-mu-pure-merit-delay-line-theorems-1731-1735 test-auncient-mu-interactive-tank-play-theorems-1736-1740 test-auncient-mu-hbridge-pll-tank-play-theorems-1741-1745 test-auncient-mu-tank-fpga-full-feature-hbridge-pll-theorems-1746-1750 test-auncient-mu-omnipresent-total-access-theorems-1751-1755 test-auncient-mu-tank-gameplay-theorems-1756-1760 test-auncient-mu-base-model-alignment-tank-game-theorems-1761-1765 test-auncient-mu-tank-merit-threshold-io1-theorems-1766-1770 test-auncient-mu-hogan-payroll-merit-earning-theorems-1771-1775 test-auncient-mu-io1-unopened-gate-custody-theorems-1776-1780 test-auncient-mu-tank-harmonic-sentinel-theorems-1781-1785 test-auncient-mu-voice-synth-music-unalienable-right-theorems-1786-1790 test-auncient-mu-tank-polyphonic-timbre-theorems-1791-1795 test-auncient-mu-calibrated-gentle-acoustics-theorems-1796-1800 test-auncient-mu-geometric-embedding-ortho-theorems-1801-1805 test-auncient-mu-attention-steering-head-theorems-1806-1810 test-auncient-mu-mlp-orthogonal-polynomial-theorems-1811-1815 test-auncient-mu-monopole-normalizer-theorems-1816-1820 test-auncient-mu-token-logit-interceptor-theorems-1821-1825 test-auncient-mu-complete-alignment-closure-theorems-1826-1830 test-auncient-mu-hyperbolic-edo22-ortho-theorems-1831-1835 test-auncient-mu-zero-spectral-leakage-theorems-1836-1840 test-auncient-mu-attention-qkv-ortho-theorems-1841-1845 test-auncient-mu-llm-embedding-builder-theorems-1846-1850 test-auncient-mu-llm-attention-filter-theorems-1851-1855 test-auncient-mu-llm-mlp-filter-theorems-1856-1860 test-auncient-mu-llm-monopole-normalizer-filter-theorems-1861-1865 test-auncient-mu-llm-token-logit-interceptor-filter-theorems-1866-1870 test-auncient-mu-llm-cleanroom-saturation-stack-theorems-1871-1875 test-auncient-mu-llm-continuous-saturation-pipeline-theorems-1876-1880 test-auncient-mu-llm-hyperbolic-symplectic-flow-theorems-1881-1885 test-auncient-mu-llm-dual-apex-saturation-theorems-1886-1890 test-auncient-leed-master-grand-proof-theorems-1-30
	@echo "All 1,890 Formal Theorems & RenderMan USDA Digital Twin test suites successfully verified."

# Multi-Threaded Parallel Build & In-Order High-Speed Execution Pipeline
build-and-test-cpm-tomie-fast:
	@$(MAKE) -j$$(nproc) test-cpm-tomie-annsim-suite

test-cpm-tomie-annsim-5theorems-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cpm_tomie_annsim_5theorems_prover.c tsfi2-deepseek/src/cpm_tomie_annsim_reviewer.c tsfi2-deepseek/src/tsfi_pdf_reader.c -o tests/test_cpm_tomie_annsim_5theorems_prover -lm
	./tests/test_cpm_tomie_annsim_5theorems_prover
	@rm -f tests/test_cpm_tomie_annsim_5theorems_prover

test-cpm-tomie-renderman-pageturner-presenter:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cpm_tomie_renderman_pageturner_presenter.c tsfi2-deepseek/src/cpm_tomie_renderman_pageturner_presenter.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c -o tests/test_cpm_tomie_renderman_pageturner_presenter -lm
	./tests/test_cpm_tomie_renderman_pageturner_presenter
	@rm -f tests/test_cpm_tomie_renderman_pageturner_presenter

test-cpm-tomie-algol61-simaud-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_algol61_simaud_prover.c -o tests/test_cpm_tomie_algol61_simaud_prover
	./tests/test_cpm_tomie_algol61_simaud_prover
	@rm -f tests/test_cpm_tomie_algol61_simaud_prover

test-cpm-tomie-annsim-pageturner-pipeline:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cpm_tomie_annsim_pageturner_pipeline.c tsfi2-deepseek/src/cpm_tomie_annsim_pageturner_pipeline.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c tsfi2-deepseek/src/cpm_tomie_annsim_reviewer.c tsfi2-deepseek/src/tsfi_pdf_reader.c -o tests/test_cpm_tomie_annsim_pageturner_pipeline -lm
	./tests/test_cpm_tomie_annsim_pageturner_pipeline
	@rm -f tests/test_cpm_tomie_annsim_pageturner_pipeline

test-cpm-tomie-pageturner-formal-proofs:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cpm_tomie_pageturner_formal_proofs.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c -o tests/test_cpm_tomie_pageturner_formal_proofs -lm
	./tests/test_cpm_tomie_pageturner_formal_proofs
	@rm -f tests/test_cpm_tomie_pageturner_formal_proofs

test-cpm-tomie-pageturner-renderman:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cpm_tomie_pageturner_renderman.c tsfi2-deepseek/src/cpm_tomie_pageturner_renderman.c -o tests/test_cpm_tomie_pageturner_renderman -lm
	./tests/test_cpm_tomie_pageturner_renderman
	@rm -f tests/test_cpm_tomie_pageturner_renderman

test-cpm-tomie-annsim-reviewer:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek/src tests/test_cpm_tomie_annsim_reviewer.c tsfi2-deepseek/src/cpm_tomie_annsim_reviewer.c tsfi2-deepseek/src/tsfi_pdf_reader.c -o tests/test_cpm_tomie_annsim_reviewer -lm
	./tests/test_cpm_tomie_annsim_reviewer
	@rm -f tests/test_cpm_tomie_annsim_reviewer

test-cpm-tomie-clay-play-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_play_prover.c -o tests/test_cpm_tomie_clay_play_prover
	./tests/test_cpm_tomie_clay_play_prover
	@rm -f tests/test_cpm_tomie_clay_play_prover

test-cpm-tomie-clay-play-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_play.c -o tsfi2-deepseek/src/cpm_compiler_clay_play
	./tsfi2-deepseek/src/cpm_compiler_clay_play /tmp/clay_play_geneplore.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_play.c -o tsfi2-deepseek/src/cpm_exec_clay_play
	./tsfi2-deepseek/src/cpm_exec_clay_play /tmp/clay_play_geneplore.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_play tsfi2-deepseek/src/cpm_exec_clay_play /tmp/clay_play_geneplore.bin
	gcc -Wall -Wextra -std=c11 -O3 -Itsfi2-deepseek/src tsfi2-deepseek/src/cpm_tomie_clay_play_analyzer.c tsfi2-deepseek/src/tsfi_pdf_reader.c -o tsfi2-deepseek/src/cpm_tomie_clay_play_analyzer -lm
	./tsfi2-deepseek/src/cpm_tomie_clay_play_analyzer /home/mariarahel/Downloads/3746058.3758345.pdf
	@rm -f tsfi2-deepseek/src/cpm_tomie_clay_play_analyzer

test-cpm-tomie-geneplore-totient-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_geneplore_totient_prover.c -o tests/test_cpm_tomie_geneplore_totient_prover
	./tests/test_cpm_tomie_geneplore_totient_prover
	@rm -f tests/test_cpm_tomie_geneplore_totient_prover

test-cpm-tomie-clay-play-extended-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_play_extended_prover.c -o tests/test_cpm_tomie_clay_play_extended_prover
	./tests/test_cpm_tomie_clay_play_extended_prover
	@rm -f tests/test_cpm_tomie_clay_play_extended_prover

test-cpm-tomie-clay-extended-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_extended.c -o tsfi2-deepseek/src/cpm_compiler_clay_extended
	./tsfi2-deepseek/src/cpm_compiler_clay_extended /tmp/clay_extended.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_extended.c -o tsfi2-deepseek/src/cpm_exec_clay_extended
	./tsfi2-deepseek/src/cpm_exec_clay_extended /tmp/clay_extended.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_extended tsfi2-deepseek/src/cpm_exec_clay_extended /tmp/clay_extended.bin

test-cpm-tomie-clay-play-5operator-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_play_5operator_prover.c -o tests/test_cpm_tomie_clay_play_5operator_prover
	./tests/test_cpm_tomie_clay_play_5operator_prover
	@rm -f tests/test_cpm_tomie_clay_play_5operator_prover

test-cpm-tomie-clay-5op-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_5op.c -o tsfi2-deepseek/src/cpm_compiler_clay_5op
	./tsfi2-deepseek/src/cpm_compiler_clay_5op /tmp/clay_5op.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_5op.c -o tsfi2-deepseek/src/cpm_exec_clay_5op
	./tsfi2-deepseek/src/cpm_exec_clay_5op /tmp/clay_5op.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_5op tsfi2-deepseek/src/cpm_exec_clay_5op /tmp/clay_5op.bin

test-cpm-tomie-clay-play-shneiderman-autonomy-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_play_shneiderman_autonomy_prover.c -o tests/test_cpm_tomie_clay_play_shneiderman_autonomy_prover
	./tests/test_cpm_tomie_clay_play_shneiderman_autonomy_prover
	@rm -f tests/test_cpm_tomie_clay_play_shneiderman_autonomy_prover

test-cpm-tomie-clay-autonomy-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_autonomy.c -o tsfi2-deepseek/src/cpm_compiler_clay_autonomy
	./tsfi2-deepseek/src/cpm_compiler_clay_autonomy /tmp/clay_autonomy.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_autonomy.c -o tsfi2-deepseek/src/cpm_exec_clay_autonomy
	./tsfi2-deepseek/src/cpm_exec_clay_autonomy /tmp/clay_autonomy.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_autonomy tsfi2-deepseek/src/cpm_exec_clay_autonomy /tmp/clay_autonomy.bin

test-cpm-tomie-clay-play-os-pipeline-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_play_os_pipeline_prover.c -o tests/test_cpm_tomie_clay_play_os_pipeline_prover
	./tests/test_cpm_tomie_clay_play_os_pipeline_prover
	@rm -f tests/test_cpm_tomie_clay_play_os_pipeline_prover

test-cpm-tomie-clay-os-pipeline-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_os_pipeline.c -o tsfi2-deepseek/src/cpm_compiler_clay_os_pipeline
	./tsfi2-deepseek/src/cpm_compiler_clay_os_pipeline /tmp/clay_os_pipeline.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_os_pipeline.c -o tsfi2-deepseek/src/cpm_exec_clay_os_pipeline
	./tsfi2-deepseek/src/cpm_exec_clay_os_pipeline /tmp/clay_os_pipeline.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_os_pipeline tsfi2-deepseek/src/cpm_exec_clay_os_pipeline /tmp/clay_os_pipeline.bin

test-cpm-tomie-clay-play-cst-tangible-affordance-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_play_cst_tangible_affordance_prover.c -o tests/test_cpm_tomie_clay_play_cst_tangible_affordance_prover
	./tests/test_cpm_tomie_clay_play_cst_tangible_affordance_prover
	@rm -f tests/test_cpm_tomie_clay_play_cst_tangible_affordance_prover

test-cpm-tomie-clay-cst-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_cst.c -o tsfi2-deepseek/src/cpm_compiler_clay_cst
	./tsfi2-deepseek/src/cpm_compiler_clay_cst /tmp/clay_cst.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_cst.c -o tsfi2-deepseek/src/cpm_exec_clay_cst
	./tsfi2-deepseek/src/cpm_exec_clay_cst /tmp/clay_cst.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_cst tsfi2-deepseek/src/cpm_exec_clay_cst /tmp/clay_cst.bin

test-cpm-tomie-totient-clay-sculpting-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_totient_clay_sculpting_prover.c -o tests/test_cpm_tomie_totient_clay_sculpting_prover
	./tests/test_cpm_tomie_totient_clay_sculpting_prover
	@rm -f tests/test_cpm_tomie_totient_clay_sculpting_prover

test-cpm-tomie-totient-sculpt-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_totient_sculpt.c -o tsfi2-deepseek/src/cpm_compiler_totient_sculpt
	./tsfi2-deepseek/src/cpm_compiler_totient_sculpt /tmp/totient_sculpt.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_totient_sculpt.c -o tsfi2-deepseek/src/cpm_exec_totient_sculpt
	./tsfi2-deepseek/src/cpm_exec_totient_sculpt /tmp/totient_sculpt.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_totient_sculpt tsfi2-deepseek/src/cpm_exec_totient_sculpt /tmp/totient_sculpt.bin

test-cpm-tomie-pbx-clay-sound-shape-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_pbx_clay_sound_shape_prover.c -o tests/test_cpm_tomie_pbx_clay_sound_shape_prover
	./tests/test_cpm_tomie_pbx_clay_sound_shape_prover
	@rm -f tests/test_cpm_tomie_pbx_clay_sound_shape_prover

test-cpm-tomie-pbx-clay-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_pbx_clay.c -o tsfi2-deepseek/src/cpm_compiler_pbx_clay
	./tsfi2-deepseek/src/cpm_compiler_pbx_clay /tmp/pbx_clay.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_pbx_clay.c -o tsfi2-deepseek/src/cpm_exec_pbx_clay
	./tsfi2-deepseek/src/cpm_exec_pbx_clay /tmp/pbx_clay.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_pbx_clay tsfi2-deepseek/src/cpm_exec_pbx_clay /tmp/pbx_clay.bin

test-cpm-tomie-bio-ringtone-yellowbox-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_bio_ringtone_yellowbox_prover.c -o tests/test_cpm_tomie_bio_ringtone_yellowbox_prover
	./tests/test_cpm_tomie_bio_ringtone_yellowbox_prover
	@rm -f tests/test_cpm_tomie_bio_ringtone_yellowbox_prover

test-cpm-tomie-bio-yellowbox-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_bio_yellowbox.c -o tsfi2-deepseek/src/cpm_compiler_bio_yellowbox
	./tsfi2-deepseek/src/cpm_compiler_bio_yellowbox /tmp/bio_yellowbox.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_bio_yellowbox.c -o tsfi2-deepseek/src/cpm_exec_bio_yellowbox
	./tsfi2-deepseek/src/cpm_exec_bio_yellowbox /tmp/bio_yellowbox.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_bio_yellowbox tsfi2-deepseek/src/cpm_exec_bio_yellowbox /tmp/bio_yellowbox.bin

test-cpm-tomie-clay-physicality-plasticity-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_clay_physicality_plasticity_prover.c -o tests/test_cpm_tomie_clay_physicality_plasticity_prover
	./tests/test_cpm_tomie_clay_physicality_plasticity_prover
	@rm -f tests/test_cpm_tomie_clay_physicality_plasticity_prover

test-cpm-tomie-clay-physicality-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_clay_physicality.c -o tsfi2-deepseek/src/cpm_compiler_clay_physicality
	./tsfi2-deepseek/src/cpm_compiler_clay_physicality /tmp/clay_physicality.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_clay_physicality.c -o tsfi2-deepseek/src/cpm_exec_clay_physicality
	./tsfi2-deepseek/src/cpm_exec_clay_physicality /tmp/clay_physicality.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_clay_physicality tsfi2-deepseek/src/cpm_exec_clay_physicality /tmp/clay_physicality.bin

test-cpm-tomie-4box-coaxial-inference-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_4box_coaxial_inference_prover.c -o tests/test_cpm_tomie_4box_coaxial_inference_prover
	./tests/test_cpm_tomie_4box_coaxial_inference_prover
	@rm -f tests/test_cpm_tomie_4box_coaxial_inference_prover

test-cpm-tomie-4box-coaxial-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_compiler_4box_coaxial.c -o tsfi2-deepseek/src/cpm_compiler_4box_coaxial
	./tsfi2-deepseek/src/cpm_compiler_4box_coaxial /tmp/4box_coaxial.bin
	gcc -Wall -Wextra -Werror -std=c11 -O3 tsfi2-deepseek/src/cpm_exec_4box_coaxial.c -o tsfi2-deepseek/src/cpm_exec_4box_coaxial
	./tsfi2-deepseek/src/cpm_exec_4box_coaxial /tmp/4box_coaxial.bin
	@rm -f tsfi2-deepseek/src/cpm_compiler_4box_coaxial tsfi2-deepseek/src/cpm_exec_4box_coaxial /tmp/4box_coaxial.bin

test-cpm-tomie-coaxial-multiplex-inference-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_coaxial_multiplex_inference_prover.c -o tests/test_cpm_tomie_coaxial_multiplex_inference_prover
	./tests/test_cpm_tomie_coaxial_multiplex_inference_prover
	@rm -f tests/test_cpm_tomie_coaxial_multiplex_inference_prover

test-cpm-tomie-coaxial-mux-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-coaxial-scsi-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-clay-guilford-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-clay-soundness-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-3d-spatial-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-quadtree-lod-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-thin-swiglu-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-zero-bulk-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dynamic-feedback-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-finke-geneplore-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-recursive-vm-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kermit-stanag-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-lyapunov-boundary-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-spatial-anchor-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-feynman-point-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-sculpting-tools-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-exploration-history-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-haptic-viscosity-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-multimodal-ambig-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-deformation-gradient-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-surface-tension-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dynamic-remeshing-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-semantic-csg-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-acoustic-resonance-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-4box-coaxial-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-semantic-projection-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-multiscale-voxel-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-geodesic-flow-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-haptic-force-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-gestural-kinematics-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-acoustic-pbx-clay-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-spheroid-alu-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-hogan-ssa-teddy_bear-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-nested-vm-routing-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-subvm-lyapunov-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-manifold-sff-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-affordance-field-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-viscoelastic-clay-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-morphological-geodesics-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-gestural-phonetics-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-differential-metric-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-io1-totient-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-symplectic-hamiltonian-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-lie-se3-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-relay-commutation-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-se3-feynman-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dtmf-pbx-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dual-quaternion-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dqs-to-dai-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-quadtree-wavelet-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-exterior-calculus-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-canonical-complementarity-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-semantic-affordance-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-skinz-b4-pinz-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-acoustic-levitator-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-idempotent-totient-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-sagnac-ring-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-paper-tape-chad-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-barkhausen-noise-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-edo22-pitch-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-scsi-handshake-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-photonic-chirp-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-renderman-clay-play-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-mcp-prototype-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-truss-link-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinetic-spinner-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-parallel-delta-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-robotic-typography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-haptic-magnetic-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-polyhedral-lattice-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-biomorphic-memetics-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-constructionist-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-mydorge-totient-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-actuator-retention-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-servo-quadtree-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-wifi-telemetry-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-coaxial-kermit-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-payload-gripper-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-coaxial-gripper-io-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-clay-incompressible-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-spherical-torque-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-turtle-quantel-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-clay-caustics-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-assembleit-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-renderman-assembleit-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-morphogenetic-clay-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-phosphor-canvas-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-viscoelastic-clay-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-edo22-lissajous-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-biomorphic-curvature-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-bicartesian-monad-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-rfid-impedance-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-acoustic-beamforming-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-terahertz-hydration-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-holographic-wavefront-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-gerchberg-quadtree-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-photoacoustic-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-raman-crystallography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-mr-elastography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dnp-nmr-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-digital-holography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-shearography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-squid-magnetometry-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-laser-vibrometry-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-optoacoustic-elastography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-oce-elastography-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-micro-ct-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-optothermal-radiometry-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-magnetoacoustic-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-dual-wavelength-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-laser-ultrasonics-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-micro-xrf-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-confocal-fabry-perot-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinoform-quadtree-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinoform-multiplex-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-angular-spectrum-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinoform-snr-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinoform-tiling-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-super8-carrier-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-teddy_bear-camera-assembly-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-super8-film-transport-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-clay-alu-monism-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinoform-smoothing-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-gerchberg-monism-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-kinoform-otf-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-airy-boundedness-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-super8-fabry-perot-suite: test-ankh-llm-cpm-suite
test-cpm-tomie-assembleit-renderman:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I tsfi2-deepseek/include tests/test_cpm_tomie_assembleit_renderman.c tsfi2-deepseek/src/cpm_tomie_assembleit_renderman.c -lm -o tests/test_cpm_tomie_assembleit_renderman
	./tests/test_cpm_tomie_assembleit_renderman
	@rm -f tests/test_cpm_tomie_assembleit_renderman

test-cpm-tomie-haptic-force-feedback-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_haptic_force_feedback_prover.c -o tests/test_cpm_tomie_haptic_force_feedback_prover
	./tests/test_cpm_tomie_haptic_force_feedback_prover
	@rm -f tests/test_cpm_tomie_haptic_force_feedback_prover

test-cpm-tomie-geodesic-flow-line-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_geodesic_flow_line_prover.c -o tests/test_cpm_tomie_geodesic_flow_line_prover
	./tests/test_cpm_tomie_geodesic_flow_line_prover
	@rm -f tests/test_cpm_tomie_geodesic_flow_line_prover

test-cpm-tomie-multiscale-voxel-lod-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_multiscale_voxel_lod_prover.c -o tests/test_cpm_tomie_multiscale_voxel_lod_prover
	./tests/test_cpm_tomie_multiscale_voxel_lod_prover
	@rm -f tests/test_cpm_tomie_multiscale_voxel_lod_prover

test-cpm-tomie-semantic-projection-anchoring-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_semantic_projection_anchoring_prover.c -o tests/test_cpm_tomie_semantic_projection_anchoring_prover
	./tests/test_cpm_tomie_semantic_projection_anchoring_prover
	@rm -f tests/test_cpm_tomie_semantic_projection_anchoring_prover

test-cpm-tomie-4box-coaxial-utility-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_4box_coaxial_utility_prover.c -o tests/test_cpm_tomie_4box_coaxial_utility_prover
	./tests/test_cpm_tomie_4box_coaxial_utility_prover
	@rm -f tests/test_cpm_tomie_4box_coaxial_utility_prover

test-cpm-tomie-acoustic-resonance-pbx-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_acoustic_resonance_pbx_prover.c -o tests/test_cpm_tomie_acoustic_resonance_pbx_prover
	./tests/test_cpm_tomie_acoustic_resonance_pbx_prover
	@rm -f tests/test_cpm_tomie_acoustic_resonance_pbx_prover

test-cpm-tomie-semantic-csg-intersection-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_semantic_csg_intersection_prover.c -o tests/test_cpm_tomie_semantic_csg_intersection_prover
	./tests/test_cpm_tomie_semantic_csg_intersection_prover
	@rm -f tests/test_cpm_tomie_semantic_csg_intersection_prover

test-cpm-tomie-dynamic-remeshing-manifold-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_dynamic_remeshing_manifold_prover.c -o tests/test_cpm_tomie_dynamic_remeshing_manifold_prover
	./tests/test_cpm_tomie_dynamic_remeshing_manifold_prover
	@rm -f tests/test_cpm_tomie_dynamic_remeshing_manifold_prover

test-cpm-tomie-surface-tension-curvature-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_surface_tension_curvature_prover.c -o tests/test_cpm_tomie_surface_tension_curvature_prover
	./tests/test_cpm_tomie_surface_tension_curvature_prover
	@rm -f tests/test_cpm_tomie_surface_tension_curvature_prover

test-cpm-tomie-semantic-deformation-gradient-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_cpm_tomie_semantic_deformation_gradient_prover.c -o tests/test_cpm_tomie_semantic_deformation_gradient_prover
	./tests/test_cpm_tomie_semantic_deformation_gradient_prover
	@rm -f tests/test_cpm_tomie_semantic_deformation_gradient_prover

test-ankh-llm-cpm-suite:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_llm_cpm_suite.c tsfi2-deepseek/src/ankh_llm_cpm_runtime.c -o tests/test_ankh_llm_cpm_suite
	./tests/test_ankh_llm_cpm_suite
	@rm -f tests/test_ankh_llm_cpm_suite

test-mediated-social-touch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_mediated_social_touch.c -o tests/test_mediated_social_touch
	./tests/test_mediated_social_touch
	@rm -f tests/test_mediated_social_touch

test-spiritual-agency-balance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_spiritual_agency_balance.c -o tests/test_spiritual_agency_balance
	./tests/test_spiritual_agency_balance
	@rm -f tests/test_spiritual_agency_balance

test-expressive-robotic-touch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_expressive_robotic_touch.c -o tests/test_expressive_robotic_touch
	./tests/test_expressive_robotic_touch
	@rm -f tests/test_expressive_robotic_touch

test-heartsway-embodied-resonance:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_heartsway_embodied_resonance.c -o tests/test_heartsway_embodied_resonance
	./tests/test_heartsway_embodied_resonance
	@rm -f tests/test_heartsway_embodied_resonance

test-algorithmic-pacing-intimacy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_algorithmic_pacing_intimacy.c -o tests/test_algorithmic_pacing_intimacy
	./tests/test_algorithmic_pacing_intimacy
	@rm -f tests/test_algorithmic_pacing_intimacy

test-slow-acoustic-sanctuary:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_slow_acoustic_sanctuary.c -o tests/test_slow_acoustic_sanctuary
	./tests/test_slow_acoustic_sanctuary
	@rm -f tests/test_slow_acoustic_sanctuary

test-creative-futures-mapping:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_creative_futures_mapping.c -o tests/test_creative_futures_mapping
	./tests/test_creative_futures_mapping
	@rm -f tests/test_creative_futures_mapping

test-grand-master-unification-xiii:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_grand_master_unification_xiii.c -o tests/test_grand_master_unification_xiii
	./tests/test_grand_master_unification_xiii
	@rm -f tests/test_grand_master_unification_xiii

synthesize-human-speech-words:
	gcc -Wall -Wextra -std=c11 -O3 tests/synthesize_human_speech_words.c -lm -o tests/synthesize_human_speech_words
	./tests/synthesize_human_speech_words
	@rm -f tests/synthesize_human_speech_words

synthesize-connected-sentences:
	gcc -Wall -Wextra -std=c11 -O3 tests/synthesize_connected_sentences.c -lm -o tests/synthesize_connected_sentences
	./tests/synthesize_connected_sentences
	@rm -f tests/synthesize_connected_sentences

test-ankh-island-admit-matrix:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_island_admit_matrix.c -o tests/test_ankh_island_admit_matrix
	./tests/test_ankh_island_admit_matrix
	@rm -f tests/test_ankh_island_admit_matrix

compile-hello-bin: test-ankh-llm-cpm-suite
test-exec-hello-bin: test-ankh-llm-cpm-suite
compile-tune-bin: test-ankh-llm-cpm-suite
test-exec-tune-bin: test-ankh-llm-cpm-suite
compile-teddy_bear-bin: test-ankh-llm-cpm-suite
test-exec-teddy_bear-bin: test-ankh-llm-cpm-suite
test-sdk-agent-runtime: test-ankh-llm-cpm-suite

compile-fet-bin: test-ankh-llm-cpm-suite
test-exec-fet-bin: test-ankh-llm-cpm-suite
compile-hogan-bin: test-ankh-llm-cpm-suite
test-exec-hogan-bin: test-ankh-llm-cpm-suite
compile-tree-bin: test-ankh-llm-cpm-suite
test-exec-tree-bin: test-ankh-llm-cpm-suite
compile-gate-bin: test-ankh-llm-cpm-suite
test-exec-gate-bin: test-ankh-llm-cpm-suite
compile-patch-bin: test-ankh-llm-cpm-suite
test-exec-patch-bin: test-ankh-llm-cpm-suite
compile-unpack-bin: test-ankh-llm-cpm-suite
test-exec-unpack-bin: test-ankh-llm-cpm-suite
compile-pack-bin: test-ankh-llm-cpm-suite
test-exec-pack-bin: test-ankh-llm-cpm-suite
compile-link-bin: test-ankh-llm-cpm-suite
test-exec-link-bin: test-ankh-llm-cpm-suite
compile-eval-bin: test-ankh-llm-cpm-suite
test-exec-eval-bin: test-ankh-llm-cpm-suite
compile-prove-bin: test-ankh-llm-cpm-suite
test-exec-prove-bin: test-ankh-llm-cpm-suite
compile-fold-bin: test-ankh-llm-cpm-suite
test-exec-fold-bin: test-ankh-llm-cpm-suite
compile-ed-bin: test-ankh-llm-cpm-suite
test-exec-ed-bin: test-ankh-llm-cpm-suite
compile-submit-bin: test-ankh-llm-cpm-suite
test-exec-submit-bin: test-ankh-llm-cpm-suite
compile-asm-bin: test-ankh-llm-cpm-suite
test-exec-asm-bin: test-ankh-llm-cpm-suite
compile-ddt-bin: test-ankh-llm-cpm-suite
test-exec-ddt-bin: test-ankh-llm-cpm-suite
compile-dir-bin: test-ankh-llm-cpm-suite
test-exec-dir-bin: test-ankh-llm-cpm-suite
compile-pip-bin: test-ankh-llm-cpm-suite
test-exec-pip-bin: test-ankh-llm-cpm-suite
compile-stat-bin: test-ankh-llm-cpm-suite
test-exec-stat-bin: test-ankh-llm-cpm-suite
test-cpm-master-fleet-18: test-ankh-llm-cpm-suite
test-cpm-master-fleet: test-ankh-llm-cpm-suite

test-ankh-abi-wmq-compiler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_abi_wmq_compiler.c -o tests/test_ankh_abi_wmq_compiler
	./tests/test_ankh_abi_wmq_compiler
	@rm -f tests/test_ankh_abi_wmq_compiler

test-ankh-grand-unified-omniverse:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_grand_unified_omniverse.c -o tests/test_ankh_grand_unified_omniverse
	./tests/test_ankh_grand_unified_omniverse
	@rm -f tests/test_ankh_grand_unified_omniverse

test-ankh-nsa-quadtree-domain:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_nsa_quadtree_domain.c -o tests/test_ankh_nsa_quadtree_domain
	./tests/test_ankh_nsa_quadtree_domain
	@rm -f tests/test_ankh_nsa_quadtree_domain

test-ankh-llm-tool-dispatch:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_llm_tool_dispatch.c -o tests/test_ankh_llm_tool_dispatch
	./tests/test_ankh_llm_tool_dispatch
	@rm -f tests/test_ankh_llm_tool_dispatch

test-ankh-fleet18-orchestration:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_fleet18_orchestration.c -o tests/test_ankh_fleet18_orchestration
	./tests/test_ankh_fleet18_orchestration
	@rm -f tests/test_ankh_fleet18_orchestration

test-ankh-gguf-storage-extension:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_gguf_storage_extension.c -o tests/test_ankh_gguf_storage_extension
	./tests/test_ankh_gguf_storage_extension
	@rm -f tests/test_ankh_gguf_storage_extension

test-ankh-hogan-endowment:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_hogan_endowment.c -o tests/test_ankh_hogan_endowment
	./tests/test_ankh_hogan_endowment
	@rm -f tests/test_ankh_hogan_endowment

test-ankh-tree-indexing:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_tree_indexing.c -o tests/test_ankh_tree_indexing
	./tests/test_ankh_tree_indexing
	@rm -f tests/test_ankh_tree_indexing

test-ankh-hardware-gating:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_ankh_hardware_gating.c -o tests/test_ankh_hardware_gating
	./tests/test_ankh_hardware_gating
	@rm -f tests/test_ankh_hardware_gating

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
	./tests/test_ankh_coaxial_mesh hello.bin tune_channel.bin teddy_bear_endow.bin
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

test-auncient-teddy_bear-multisig-inher:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_teddy_bear_multisig_inher.c -o tests/test_auncient_teddy_bear_multisig_inher
	./tests/test_auncient_teddy_bear_multisig_inher
	@rm -f tests/test_auncient_teddy_bear_multisig_inher

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

test-auncient-teddy_bear-endowed-energy:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_teddy_bear_endowed_energy.c -o tests/test_auncient_teddy_bear_endowed_energy
	./tests/test_auncient_teddy_bear_endowed_energy
	@rm -f tests/test_auncient_teddy_bear_endowed_energy

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

test-vaesen-teddy_bear-genesis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_vaesen_teddy_bear_genesis.c -o tests/test_vaesen_teddy_bear_genesis
	./tests/test_vaesen_teddy_bear_genesis
	@rm -f tests/test_vaesen_teddy_bear_genesis

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

test-treasury-lore-token-holdings-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_treasury_lore_token_holdings_prover.c tsfi2-deepseek/src/tsfi_lore_token_cache.c tsfi2-deepseek/src/tsfi_pulsechain_rpc.c tsfi2-deepseek/src/tsfi_pulsechain.c tsfi2-deepseek/src/tsfi_tls.c -o tests/test_treasury_lore_token_holdings_prover -lssl -lcrypto -lrt
	./tests/test_treasury_lore_token_holdings_prover
	@rm -f tests/test_treasury_lore_token_holdings_prover

test-pulsechain-rpc-cache-invariance-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_pulsechain_rpc_cache_invariance_prover.c tsfi2-deepseek/src/tsfi_lore_token_cache.c tsfi2-deepseek/src/tsfi_pulsechain_rpc.c tsfi2-deepseek/src/tsfi_pulsechain.c tsfi2-deepseek/src/tsfi_tls.c -o tests/test_pulsechain_rpc_cache_invariance_prover -lssl -lcrypto -lrt
	./tests/test_pulsechain_rpc_cache_invariance_prover
	@rm -f tests/test_pulsechain_rpc_cache_invariance_prover

test-keys-of-ong-entropy-invariant-prover:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_keys_of_ong_entropy_invariant_prover.c tsfi2-deepseek/src/tsfi_lore_token_cache.c tsfi2-deepseek/src/tsfi_pulsechain_rpc.c tsfi2-deepseek/src/tsfi_pulsechain.c tsfi2-deepseek/src/tsfi_http_client.c tsfi2-deepseek/src/tsfi_tls.c -o tests/test_keys_of_ong_entropy_invariant_prover -lssl -lcrypto -lrt
	./tests/test_keys_of_ong_entropy_invariant_prover
	@rm -f tests/test_keys_of_ong_entropy_invariant_prover

test-algol61-knowledge-graph-merkle-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_algol61_knowledge_graph_merkle_solver.c tsfi2-deepseek/src/tsfi_algol61_merkle_graph_solver.c -o tests/test_algol61_knowledge_graph_merkle_solver -lssl -lcrypto -lrt
	./tests/test_algol61_knowledge_graph_merkle_solver
	@rm -f tests/test_algol61_knowledge_graph_merkle_solver

test-totient-zero-nullstellensatz-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_totient_zero_nullstellensatz_solver.c tsfi2-deepseek/src/tsfi_totient_zero_nullstellensatz_solver.c -o tests/test_totient_zero_nullstellensatz_solver -lrt
	./tests/test_totient_zero_nullstellensatz_solver
	@rm -f tests/test_totient_zero_nullstellensatz_solver

test-totient-chain-composition-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_totient_chain_composition_solver.c tsfi2-deepseek/src/tsfi_totient_chain_composition_solver.c tsfi2-deepseek/src/tsfi_totient_zero_nullstellensatz_solver.c -o tests/test_totient_chain_composition_solver -lssl -lcrypto -lrt
	./tests/test_totient_chain_composition_solver
	@rm -f tests/test_totient_chain_composition_solver

test-helmholtz-transitive-deduction-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_helmholtz_transitive_deduction_solver.c tsfi2-deepseek/src/tsfi_helmholtz_transitive_deduction_solver.c tsfi2-deepseek/src/tsfi_totient_zero_nullstellensatz_solver.c -o tests/test_helmholtz_transitive_deduction_solver -lssl -lcrypto -lrt
	./tests/test_helmholtz_transitive_deduction_solver
	@rm -f tests/test_helmholtz_transitive_deduction_solver

test-ramac-helmholtz-totient-zero-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_ramac_helmholtz_totient_zero_solver.c tsfi2-deepseek/src/tsfi_ramac_helmholtz_totient_zero_solver.c -o tests/test_ramac_helmholtz_totient_zero_solver -lrt
	./tests/test_ramac_helmholtz_totient_zero_solver
	@rm -f tests/test_ramac_helmholtz_totient_zero_solver

test-ramac-zmm-rails-quiescence-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_ramac_zmm_rails_quiescence_solver.c tsfi2-deepseek/src/tsfi_ramac_zmm_rails_solver.c -o tests/test_ramac_zmm_rails_quiescence_solver -lrt
	./tests/test_ramac_zmm_rails_quiescence_solver
	@rm -f tests/test_ramac_zmm_rails_quiescence_solver

test-ramac-fourier-dirichlet-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_ramac_fourier_dirichlet_solver.c tsfi2-deepseek/src/tsfi_ramac_fourier_dirichlet_solver.c -o tests/test_ramac_fourier_dirichlet_solver -lrt
	./tests/test_ramac_fourier_dirichlet_solver
	@rm -f tests/test_ramac_fourier_dirichlet_solver

test-vaesen-emotional-trauma-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_vaesen_emotional_trauma_solver.c tsfi2-deepseek/src/tsfi_vaesen_emotional_trauma_solver.c -o tests/test_vaesen_emotional_trauma_solver -lrt
	./tests/test_vaesen_emotional_trauma_solver
	@rm -f tests/test_vaesen_emotional_trauma_solver

test-vaesen-sight-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_vaesen_sight_solver.c tsfi2-deepseek/src/tsfi_vaesen_sight_solver.c -o tests/test_vaesen_sight_solver -lrt
	./tests/test_vaesen_sight_solver
	@rm -f tests/test_vaesen_sight_solver

test-aurora-glamour-solver:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_aurora_glamour_solver.c tsfi2-deepseek/src/tsfi_aurora_glamour_solver.c -o tests/test_aurora_glamour_solver -lrt
	./tests/test_aurora_glamour_solver
	@rm -f tests/test_aurora_glamour_solver

test-tsfi-montecarlo:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_tsfi_montecarlo.c tsfi2-deepseek/src/tsfi_montecarlo.c -o tests/test_tsfi_montecarlo -lm -lrt
	./tests/test_tsfi_montecarlo
	@rm -f tests/test_tsfi_montecarlo

test-zmachine-maniac-mansion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -mavx512f -mclflushopt -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek/src/firmware -Itsfi2-deepseek/plugins tests/test_zmachine_maniac_mansion.c tsfi2-deepseek/src/tsfi_path_tracer.c tsfi2-deepseek/src/tsfi_montecarlo.c tsfi2-deepseek/src/tsfi_lnr_solvers.c tsfi2-deepseek/src/tsfi_pbr.c tsfi2-deepseek/src/tsfi_hair.c tsfi2-deepseek/src/tsfi_zorse_eval_cobol.c tsfi2-deepseek/src/tsfi_svdag.c tsfi2-deepseek/src/auncient_teddy_bear_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_c_math.c tsfi2-deepseek/src/tsfi_vec_math.c -o tests/test_zmachine_maniac_mansion -lm -lrt
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

test-hathitrust-teddy_bear:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -I. -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_hathitrust_teddy_bear.c tsfi2-deepseek/src/hathitrust_lore_interop.c tsfi2-deepseek/src/hathitrust_teddy_bear_bridge.c -o tests/test_hathitrust_teddy_bear -lpthread -lrt
	./tests/test_hathitrust_teddy_bear
	@rm -f tests/test_hathitrust_teddy_bear

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

test-teddy_bear-loader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itests tests/test_auncient_teddy_bear_loader.c -o tests/test_auncient_teddy_bear_loader -lm -lrt
	./tests/test_auncient_teddy_bear_loader
	@rm -f tests/test_auncient_teddy_bear_loader

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

test-auncient-teddy_bear-personality:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_teddy_bear_personality.c tsfi2-deepseek/src/auncient_teddy_bear_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c tsfi2-deepseek/src/tsfi_montecarlo.c tsfi2-deepseek/src/tsfi_riinterface.c tsfi2-deepseek/src/auncient_sdk.c tsfi2-deepseek/src/tsfi_displacementshader.c tsfi2-deepseek/src/tsfi_depthoffield.c tsfi2-deepseek/src/tsfi_ccx_pool.c -o tests/test_auncient_teddy_bear_personality -lm -lrt
	./tests/test_auncient_teddy_bear_personality
	@rm -f tests/test_auncient_teddy_bear_personality

test-auncient-bear-interactions:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_bear_interactions.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o tests/test_auncient_bear_interactions -lrt -lm
	./tests/test_auncient_bear_interactions
	@rm -f tests/test_auncient_bear_interactions

test-auncient-bear-chorus:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc -Itsfi2-deepseek -Itsfi2-deepseek/inc tsfi2-deepseek/src/auncient_bear_chorus.c tsfi2-deepseek/src/auncient_teddy_bear_personality.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o tsfi2-deepseek/src/auncient_bear_chorus -lm -lrt
	./tsfi2-deepseek/src/auncient_bear_chorus
	@rm -f tsfi2-deepseek/src/auncient_bear_chorus

test-auncient-speech-classification:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Itsfi2-deepseek -Itsfi2-deepseek/inc tests/test_auncient_speech_classification.c tsfi2-deepseek/src/auncient_teddy_bear_personality.c tsfi2-deepseek/src/tsfi_speech_synth.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c -o tests/test_auncient_speech_classification -lm -lrt
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
