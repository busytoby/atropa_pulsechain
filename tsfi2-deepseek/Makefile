CC       = gcc
CXX      = g++
CFLAGS   = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -Iinc -I../inc -Isrc -Iplugins -Iplugins/vulkan -Iplugins/window_inc -Iplugins/window_src -Iassets/sd_cpp/include -Isrc/firmware \
           -O3 -g -march=native -flax-vector-conversions \
           -fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIC \
           -DTSFI_PROVENANCE_KEY=\"SIG_2026_USLM_AFFIRMED\"
CXXFLAGS = -Wall -Wextra -Wno-error -std=c++17 -O3 -g -march=native -fPIC -Iinc -Iplugins -Iplugins/vulkan -Iassets/sd_cpp/include
LDFLAGS  = -L/usr/local/lib -ljpeg -rdynamic -lwayland-client -lvulkan -Wl,-z,noexecstack -lm -lrt -lpthread -ldl -lstdc++ -lasound -lssl -lcrypto -lpulse-simple -lpulse

OBJ_DIR  = obj
BIN_DIR  = bin
SRC_DIR  = src

# --- Core Simulation Objects ---
CORE_OBJS = $(OBJ_DIR)/tsfi_wire_firmware.o $(OBJ_DIR)/tsfi_logic.o $(OBJ_DIR)/tsfi_hotloader.o \
            $(OBJ_DIR)/tsfi_ai_core.o $(OBJ_DIR)/tsfi_displacementshader.o \
            $(OBJ_DIR)/tsfi_fourier.o $(OBJ_DIR)/tsfi_genetic.o $(OBJ_DIR)/tsfi_hilbert_avx512.o \
            $(OBJ_DIR)/tsfi_vision.o $(OBJ_DIR)/lau_audit.o $(OBJ_DIR)/tsfi_crash_handler.o \
            $(OBJ_DIR)/tsfi_spirv_kernels.o $(OBJ_DIR)/tsfi_fused_operator.o $(OBJ_DIR)/tsfi_vision_metrics.o \
            $(OBJ_DIR)/lau_registry.o $(OBJ_DIR)/tsfi_ottype_geom.o $(OBJ_DIR)/tsfi_hilbert.o \
            $(OBJ_DIR)/tsfi_font_shaping_parser.o $(OBJ_DIR)/tsfi_k0rn_compiler.o \
            $(OBJ_DIR)/tsfi_broadcaster_core.o $(OBJ_DIR)/tsfi_vision_vk.o $(OBJ_DIR)/tsfi_ottype_shape.o \
            $(OBJ_DIR)/tsfi_video_thunks.o $(OBJ_DIR)/tsfi_io.o $(OBJ_DIR)/tsfi_rtmp.o \
            $(OBJ_DIR)/tsfi_vision_thunks.o $(OBJ_DIR)/tsfi_broadcaster_bandwidth_throttle.o \
            $(OBJ_DIR)/tsfi_wire_pty.o $(OBJ_DIR)/tsfi_zhao.o $(OBJ_DIR)/lau_vram.o \
            $(OBJ_DIR)/tsfi_resonance.o $(OBJ_DIR)/tsfi_dl_thunks.o $(OBJ_DIR)/tsfi_k0rn_codegen.o \
            $(OBJ_DIR)/tsfi_reaction.o $(OBJ_DIR)/tsfi_font_rasterizer.o $(OBJ_DIR)/tsfi_phenotype_loader.o \
            $(OBJ_DIR)/tsfi_broadcaster_ui.o $(OBJ_DIR)/tsfi_font_ai.o $(OBJ_DIR)/tsfi_ottype_layout.o \
            $(OBJ_DIR)/tsfi_helmk0rn_factory.o $(OBJ_DIR)/tsfi_vulkan_video_encode.o $(OBJ_DIR)/tsfi_vision_io.o \
            $(OBJ_DIR)/tsfi_vec_math.o $(OBJ_DIR)/tsfi_variations.o $(OBJ_DIR)/tsfi_font_engine.o \
            $(OBJ_DIR)/tsfi_vision_analysis.o $(OBJ_DIR)/tsfi_math.o $(OBJ_DIR)/tsfi_config.o \
            $(OBJ_DIR)/tsfi_dys_math.o $(OBJ_DIR)/tsfi_cff.o $(OBJ_DIR)/tsfi_wave512_asm.o \
            $(OBJ_DIR)/tsfi_trace.o $(OBJ_DIR)/tsfi_ottype.o $(OBJ_DIR)/tsfi_dysnomia.o \
            $(OBJ_DIR)/tsfi_zmm_vm.o $(OBJ_DIR)/tsfi_font_registry.o \
            $(OBJ_DIR)/tsfi_ipomoea_lifecycle.o \
            $(OBJ_DIR)/tsfi_mozilla_wmq_bridge.o \
            $(OBJ_DIR)/tsfi_shader_dispatch.o $(OBJ_DIR)/lau_memory.o $(OBJ_DIR)/tsfi_elf_reflect.o \
            $(OBJ_DIR)/tsfi_opt_zmm_unity.o $(OBJ_DIR)/tsfi_opt_zmm.o $(OBJ_DIR)/tsfi_glob.o \
            $(OBJ_DIR)/tsfi_operator.o $(OBJ_DIR)/tsfi_yang.o $(OBJ_DIR)/tsfi_quan.o \
            $(OBJ_DIR)/tsfi_c_math.o $(OBJ_DIR)/firmware/LauWireFirmware_rtl.o \
            $(OBJ_DIR)/tsfi_vision_classify.o $(OBJ_DIR)/tsfi_cli.o $(OBJ_DIR)/tsfi_svdag.o \
            $(OBJ_DIR)/tsfi_hogan_core.o $(OBJ_DIR)/tsfi_hogan_lfs.o $(OBJ_DIR)/tsfi_hogan_journal.o $(OBJ_DIR)/tsfi_hogan_compliance.o $(OBJ_DIR)/tsfi_hogan_compliance_limits.o $(OBJ_DIR)/tsfi_hogan_integrations.o $(OBJ_DIR)/tsfi_mainframe_computerworld.o \
            $(OBJ_DIR)/lau_thunk.o $(OBJ_DIR)/lau_yul_thunk_common.o $(OBJ_DIR)/lau_yul_thunk_compiler.o $(OBJ_DIR)/lau_yul_thunk_interpreter.o $(OBJ_DIR)/lau_yul_thunk_memo.o $(OBJ_DIR)/lau_yul_thunk_nested.o $(OBJ_DIR)/tsfi_wiring.o $(OBJ_DIR)/tsfi_vulkan_cells.o \
            $(OBJ_DIR)/tsfi_merkle.o $(OBJ_DIR)/tsfi_steiner_evi.o $(OBJ_DIR)/tsfi_nebula.o $(OBJ_DIR)/tsfi_divider.o $(OBJ_DIR)/tsfi_serge_usg.o $(OBJ_DIR)/tsfi_dtl_core.o $(OBJ_DIR)/tsfi_delay_line.o $(OBJ_DIR)/tsfi_rtl_latch.o $(OBJ_DIR)/tsfi_shaft_encoder.o $(OBJ_DIR)/tsfi_core_plane.o $(OBJ_DIR)/tsfi_tunnel_latch.o $(OBJ_DIR)/tsfi_thin_film.o $(OBJ_DIR)/tsfi_integrated_circuit.o $(OBJ_DIR)/tsfi_core_rope.o $(OBJ_DIR)/tsfi_ttl_gate.o $(OBJ_DIR)/tsfi_parametron.o $(OBJ_DIR)/tsfi_slt.o $(OBJ_DIR)/tsfi_cla.o $(OBJ_DIR)/tsfi_plated_wire.o $(OBJ_DIR)/tsfi_mema.o $(OBJ_DIR)/tsfi_sar_adc.o $(OBJ_DIR)/tsfi_schmitt.o $(OBJ_DIR)/tsfi_diode_matrix.o $(OBJ_DIR)/tsfi_byte_issue21.o $(OBJ_DIR)/tsfi_byte_issue22.o $(OBJ_DIR)/tsfi_byte_issue23.o $(OBJ_DIR)/tsfi_byte_issue24.o $(OBJ_DIR)/tsfi_oregon_trail.o $(OBJ_DIR)/tsfi_elektuur_issue25.o $(OBJ_DIR)/tsfi_prophecy.o $(OBJ_DIR)/tsfi_valve.o \
            $(OBJ_DIR)/tsfi_pool.o $(OBJ_DIR)/tsfi_time.o $(OBJ_DIR)/tsfi_staging.o $(OBJ_DIR)/tsfi_nsa_tempest.o \
            $(OBJ_DIR)/tsfi_design_pipeline.o \
            $(OBJ_DIR)/tsfi_temporal_warper.o \
            $(OBJ_DIR)/tsfi_safetensors_cache.o \
            $(OBJ_DIR)/tsfi_zmm_traverse_driver.o \
            $(OBJ_DIR)/tsfi_gguf_loader.o \
            $(OBJ_DIR)/tsfi_totient_qing_tomography.o \
            $(OBJ_DIR)/tsfi_zhu_model_compression.o \
            $(OBJ_DIR)/tsfi_loraprune_deepseek.o \
            $(OBJ_DIR)/tsfi_faster_lighter_llm.o \
            $(OBJ_DIR)/tsfi_ai_core.o \
            $(OBJ_DIR)/tsfi_xplos_shell_cbt_vtam.o \
            $(OBJ_DIR)/tsfi_helmholtz_ring.o \
            $(OBJ_DIR)/tsfi_chamberland_duality.o \
            $(OBJ_DIR)/tsfi_alessandrini_condenser.o \
            $(OBJ_DIR)/tsfi_stekloff_duality.o \
            $(OBJ_DIR)/tsfi_zorse_tomography.o \
            $(OBJ_DIR)/tsfi_geometric_algebra.o \
            $(OBJ_DIR)/tsfi_subword_trie_cache.o \
            $(OBJ_DIR)/tsfi_quantized_rag.o \
            $(OBJ_DIR)/tsfi_code_review.o \
            $(OBJ_DIR)/tsfi_telpa_program_analysis.o \
            $(OBJ_DIR)/tsfi_flash_deepseek.o \
            $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o \
            $(OBJ_DIR)/tsfi_deepseek_cordis_advanced.o \
            $(OBJ_DIR)/tsfi_deepseek_cordis_tools.o \
            $(OBJ_DIR)/tsfi_deepseek_cordis_chain.o \
            $(OBJ_DIR)/tsfi_deepseek_edsac_acp_firewall.o \
            $(OBJ_DIR)/tsfi_deepseek_stanag_kermit.o \
            $(OBJ_DIR)/tsfi_deepseek_agent_vm_operator.o \
            $(OBJ_DIR)/tsfi_deepseek_cordis_composability.o \
            $(OBJ_DIR)/tsfi_deepseek_extensions.o \
            $(OBJ_DIR)/tsfi_deepseek_gguf_extended.o \
            $(OBJ_DIR)/tsfi_zmm_winchester_handshake.o \
            $(OBJ_DIR)/tsfi_depthoffield.o \
            $(OBJ_DIR)/tsfi_riinterface.o \
            $(OBJ_DIR)/auncient_sdk.o \
            $(OBJ_DIR)/tsfi_ccx_pool.o \
            $(OBJ_DIR)/tsfi_ccx_pool.o \
            $(OBJ_DIR)/tsfi_sd_thunk.o \
            $(OBJ_DIR)/tsfi_taste_cache.o \
            $(OBJ_DIR)/tsfi_shading_manifold.o \
            $(OBJ_DIR)/tsfi_vae_firmware.o $(OBJ_DIR)/tsfi_liang_barsky.o \
            $(OBJ_DIR)/tsfi_nand_trap_firmware.o $(OBJ_DIR)/tsfi_vessel_anchor.o \
            $(OBJ_DIR)/tsfi_vae_upsampler.o \
            $(OBJ_DIR)/tsfi_fur_shader.o \
            $(OBJ_DIR)/tsfi_masterpiece_render.o \
            $(OBJ_DIR)/tsfi_moon_chuuker.o \
            $(OBJ_DIR)/tsfi_dream_streamer.o \
            $(OBJ_DIR)/tsfi_path_tracer.o \
            $(OBJ_DIR)/tsfi_montecarlo.o \
            $(OBJ_DIR)/tsfi_lnr_solvers.o \
            $(OBJ_DIR)/auncient_teddy_bear_personality.o \
            $(OBJ_DIR)/tsfi_pbr.o \
            $(OBJ_DIR)/tsfi_hair.o \
            $(OBJ_DIR)/tsfi_merkle_fast.o $(OBJ_DIR)/tsfi_scramble_fast.o $(OBJ_DIR)/tsfi_jpeg_encoder.o \
            $(OBJ_DIR)/tsfi_yang_baseline.o $(OBJ_DIR)/tsfi_yang_fast.o $(OBJ_DIR)/tsfi_comm.o \
            $(OBJ_DIR)/tsfi_controlnet_shm.o $(OBJ_DIR)/tsfi_puppetry.o \
            $(OBJ_DIR)/tsfi_pulsechain.o $(OBJ_DIR)/tsfi_pulsechain_rpc.o $(OBJ_DIR)/tsfi_tls.o \
            $(OBJ_DIR)/tsfi_journal.o $(OBJ_DIR)/tsfi_drm_lease_thunk.o $(OBJ_DIR)/tsfi_evolution_bridge.o $(OBJ_DIR)/tsfi_ui_kernels.o \
            $(OBJ_DIR)/tsfi_dl_thunks.o $(OBJ_DIR)/lau_vram.o \
            $(OBJ_DIR)/tsfi_studio_core.o $(OBJ_DIR)/tsfi_wavefront_core.o $(OBJ_DIR)/tsfi_obj_loader.o \
            $(OBJ_DIR)/tsfi_lc_tank.o \
            $(OBJ_DIR)/tsfi_ammeter.o \
            $(OBJ_DIR)/tsfi_zener.o \
            $(OBJ_DIR)/drm-lease-v1-protocol.o \
            $(OBJ_DIR)/tsfi_computel_blue_box.o \
            $(OBJ_DIR)/tsfi_computel_yellow_box.o \
            $(OBJ_DIR)/tsfi_computel_bb_index.o \
            $(OBJ_DIR)/tsfi_computel_bb_signal.o \
            $(OBJ_DIR)/tsfi_computel_bb_quad.o \
            $(OBJ_DIR)/tsfi_ouroboros_pll.o \
            $(OBJ_DIR)/tsfi_sort.o \
            $(OBJ_DIR)/tsfi_qing_bst.o \
            $(OBJ_DIR)/tsfi_qing_graph.o \
            $(OBJ_DIR)/tsfi_trie.o \
            $(OBJ_DIR)/tsfi_trie_dispatcher.o \
            $(OBJ_DIR)/tsfi_tst.o \
            $(OBJ_DIR)/tsfi_dat.o \
            $(OBJ_DIR)/tsfi_graph_select.o \
            $(OBJ_DIR)/tsfi_zmm_rpc.o \
            $(OBJ_DIR)/tsfi_zmm_rpc_manifold.o \
            $(OBJ_DIR)/tsfi_zmm_rpc_wave.o \
            $(OBJ_DIR)/tsfi_zmm_rpc_pulse.o \
            $(OBJ_DIR)/tsfi_mcp_system_list.o \
            $(OBJ_DIR)/tsfi_block_monitor.o \
            $(OBJ_DIR)/tsfi_block_monitor_price.o \
            $(OBJ_DIR)/tsfi_block_monitor_token.o \
            $(OBJ_DIR)/tsfi_doxastic_classifier.o \
            $(OBJ_DIR)/tsfi_doxastic_projector.o \
            $(OBJ_DIR)/tsfi_algebraic_query.o \
            $(OBJ_DIR)/tsfi_bayesian.o \
            $(OBJ_DIR)/tsfi_unified_classifier.o \
            $(OBJ_DIR)/tsfi_akb.o \
            $(OBJ_DIR)/tsfi_box_orchestrator.o \
            $(OBJ_DIR)/tsfi_synth_akb.o \
            $(OBJ_DIR)/tsfi_synth_direct.o \
            $(OBJ_DIR)/tsfi_synth_perf.o \
            $(OBJ_DIR)/tsfi_winchester_bridge.o \
            $(OBJ_DIR)/tsfi_dual_stack.o \
            $(OBJ_DIR)/tsfi_doxastic_pipeline.o \
            $(OBJ_DIR)/tsfi_nlp.o \
            $(OBJ_DIR)/tsfi_phoneme_synth.o \
            $(OBJ_DIR)/tsfi_zorse_eval.o \
            $(OBJ_DIR)/tsfi_markov_lm.o \
            $(OBJ_DIR)/tsfi_shm_prune.o \
            $(OBJ_DIR)/tsfi_lexical_trie.o \
            $(OBJ_DIR)/tsfi_anvil_vm.o \
            $(OBJ_DIR)/tsfi_ot_accumulator.o \
            $(OBJ_DIR)/tsfi_coaxial_observer.o \
            $(OBJ_DIR)/tsfi_coaxial_agent.o \
            $(OBJ_DIR)/tsfi_priority_queue.o \
            $(OBJ_DIR)/tsfi_strategy_lang.o \
            $(OBJ_DIR)/tsfi_spec_queues.o \
            $(OBJ_DIR)/tsfi_coaxial_strategy.o \
            $(OBJ_DIR)/tsfi_coaxial_telemetry.o \
            $(OBJ_DIR)/tsfi_ramac_layout.o \
            $(OBJ_DIR)/tsfi_ramac_acc.o \
            $(OBJ_DIR)/tsfi_ramac_atlas.o \
            $(OBJ_DIR)/tsfi_ramac_cp.o \
            $(OBJ_DIR)/tsfi_ramac_db.o \
            $(OBJ_DIR)/tsfi_ramac_emul.o \
            $(OBJ_DIR)/tsfi_ramac_emul_legacy.o \
            $(OBJ_DIR)/tsfi_ramac_lang.o \
            $(OBJ_DIR)/tsfi_mainframe_v370.o \
            $(OBJ_DIR)/tsfi_mainframe_fips.o \
            $(OBJ_DIR)/tsfi_mainframe_decnet.o \
            $(OBJ_DIR)/tsfi_mainframe_sna.o \
            $(OBJ_DIR)/tsfi_mainframe_security.o \
            $(OBJ_DIR)/tsfi_mainframe_misc.o \
             $(OBJ_DIR)/tsfi_s370_dat_extensions.o \
             $(OBJ_DIR)/tsfi_ray_tracer.o \
             $(OBJ_DIR)/tsfi_micro_focus.o \
              $(OBJ_DIR)/tsfi_majormud.o \
               $(OBJ_DIR)/tsfi_cade_imf.o \
                 $(OBJ_DIR)/tsfi_cade_imf_nato.o \
                 $(OBJ_DIR)/tsfi_cade_vulkan.o \
               $(OBJ_DIR)/tsfi_eer_emergency.o \
              $(OBJ_DIR)/tsfi_nsa_tempest.o \
              $(OBJ_DIR)/tsfi_reuter_tx.o \
              $(OBJ_DIR)/tsfi_gray_tx.o \
              $(OBJ_DIR)/tsfi_autodin_sage.o \
              $(OBJ_DIR)/tsfi_encodings.o \
              $(OBJ_DIR)/tsfi_encodings_opt.o \
              $(OBJ_DIR)/tsfi_parc_bitblt.o \
              $(OBJ_DIR)/tsfi_parc_smalltalk.o \
              $(OBJ_DIR)/tsfi_parc_window.o \
              $(OBJ_DIR)/tsfi_parc_ethernet.o \
               $(OBJ_DIR)/tsfi_parc_superpaint.o \
               $(OBJ_DIR)/tsfi_parc_interpress.o \
               $(OBJ_DIR)/tsfi_parc_bravo.o \
                $(OBJ_DIR)/tsfi_parc_cursor.o \
                $(OBJ_DIR)/tsfi_parc_brush.o \
                $(OBJ_DIR)/tsfi_parc_runlength.o \
                 $(OBJ_DIR)/tsfi_parc_csmacd.o \
                 $(OBJ_DIR)/tsfi_parc_keyset.o \
                 $(OBJ_DIR)/tsfi_parc_gc.o \
                 $(OBJ_DIR)/tsfi_parc_bfs.o \
                 $(OBJ_DIR)/tsfi_parc_stcomp.o \
                  $(OBJ_DIR)/tsfi_parc_video.o \
                  $(OBJ_DIR)/tsfi_parc_shaders.o \
                  $(OBJ_DIR)/tsfi_parc_videodeck.o \
                  $(OBJ_DIR)/tsfi_parc_clipboard.o \
                  $(OBJ_DIR)/tsfi_parc_mvc.o \
                  $(OBJ_DIR)/tsfi_parc_laurel.o \
                  $(OBJ_DIR)/tsfi_parc_disktool.o \
                  $(OBJ_DIR)/tsfi_parc_notetaker.o \
                  $(OBJ_DIR)/tsfi_parc_alfont.o \
                  $(OBJ_DIR)/tsfi_parc_routing.o \
                  $(OBJ_DIR)/tsfi_parc_keyboard.o \
                  $(OBJ_DIR)/tsfi_parc_knoll.o \
                  $(OBJ_DIR)/tsfi_parc_tknoll.o \
                  $(OBJ_DIR)/tsfi_parc_rle.o \
                  $(OBJ_DIR)/tsfi_parc_figma.o \
                  $(OBJ_DIR)/tsfi_parc_figma_advanced.o \
                  $(OBJ_DIR)/tsfi_parc_figma_super.o \
                  $(OBJ_DIR)/tsfi_parc_figma_proto.o \
                  $(OBJ_DIR)/tsfi_parc_figma_vector.o \
                  $(OBJ_DIR)/tsfi_parc_runcible_lang.o \
                  $(OBJ_DIR)/tsfi_parc_tape_label_yul.o $(OBJ_DIR)/tsfi_parc_tape_catalog.o \
            $(OBJ_DIR)/tsfi_parc_schrack.o $(OBJ_DIR)/tsfi_parc_naur_ab14.o \
            $(OBJ_DIR)/tsfi_parc_runcible_cics.o $(OBJ_DIR)/tsfi_parc_ingerman_translator.o \
            $(OBJ_DIR)/tsfi_parc_ingerman_alg141.o \
            $(OBJ_DIR)/tsfi_parc_ingerman_thunk.o \
            $(OBJ_DIR)/tsfi_parc_ingerman_display.o \
            $(OBJ_DIR)/tsfi_parc_ingerman_syntax.o \
            $(OBJ_DIR)/tsfi_parc_contract_coefficients.o \
            $(OBJ_DIR)/tsfi_parc_pulsechain_lp_matrix.o \
                  $(OBJ_DIR)/tsfi_autodin_tape_proof.o \
                  $(OBJ_DIR)/tsfi_isomorphism_engine.o \
                  $(OBJ_DIR)/tsfi_iso_transpiler.o \
                  $(OBJ_DIR)/tsfi_parc_tape_trunk.o \
                  $(OBJ_DIR)/tsfi_yul_deployer.o \
                  $(OBJ_DIR)/tsfi_autodin_zmm_tx.o \
                  $(OBJ_DIR)/tsfi_autodin_anvil_oracle.o \
                  $(OBJ_DIR)/tsfi_parc_trunk_queue.o \
                  $(OBJ_DIR)/tsfi_iso_security_engine.o \
                  $(OBJ_DIR)/tsfi_helmholtz_dispatcher.o \
                  $(OBJ_DIR)/tsfi_logan_synth.o \
                  $(OBJ_DIR)/tsfi_lynch_exec.o \
                  $(OBJ_DIR)/tsfi_helmholtz_lynch_bridge.o \
                  $(OBJ_DIR)/tsfi_zevm_vm_selector.o \
                  $(OBJ_DIR)/tsfi_lynch_channel.o \
                  $(OBJ_DIR)/tsfi_lynch_sync.o \
                  $(OBJ_DIR)/tsfi_knuth_storage.o \
                  $(OBJ_DIR)/tsfi_knuth_fet_gas.o \
                  $(OBJ_DIR)/tsfi_electricity_meter.o \
                  $(OBJ_DIR)/tsfi_knuth_yul_transpiler.o \
                  $(OBJ_DIR)/tsfi_knuth_fft.o \
                  $(OBJ_DIR)/tsfi_knuth_quadtree.o \
                  $(OBJ_DIR)/tsfi_knuth_rb_dragon.o \
                  $(OBJ_DIR)/tsfi_lowpower_fet.o \
                  $(OBJ_DIR)/tsfi_lowpower_mode.o \
                  $(OBJ_DIR)/tsfi_dvfs_governor.o \
                  $(OBJ_DIR)/tsfi_drum_latency.o \
                  $(OBJ_DIR)/tsfi_minimized_drum_buffer.o \
                  $(OBJ_DIR)/tsfi_card_spooler.o \
                  $(OBJ_DIR)/tsfi_nanosecond_drum.o \
                  $(OBJ_DIR)/tsfi_nadler_optimizer.o \
                  $(OBJ_DIR)/tsfi_nadler_advanced.o \
                  $(OBJ_DIR)/tsfi_transcendental_math.o \
                  $(OBJ_DIR)/tsfi_cobol_algol_power.o \
                  $(OBJ_DIR)/tsfi_fortran_dml_power.o \
                  $(OBJ_DIR)/tsfi_continuous_zero_power.o \
                  $(OBJ_DIR)/tsfi_defcon_power_alarm.o \
                  $(OBJ_DIR)/tsfi_nadler_hull_automaton.o \
                  $(OBJ_DIR)/tsfi_nadler_agentic_automaton.o \
                  $(OBJ_DIR)/tsfi_nadler_skeletonizer.o \
                  $(OBJ_DIR)/tsfi_nadler_syntactic_parser.o \
                  $(OBJ_DIR)/tsfi_nadler_contour_code.o \
                  $(OBJ_DIR)/tsfi_transcendental_cf.o \
                  $(OBJ_DIR)/tsfi_nather_photometry.o \
                  $(OBJ_DIR)/tsfi_algol60_block_scope.o \
                  $(OBJ_DIR)/tsfi_algol61_logic_power.o \
                  $(OBJ_DIR)/tsfi_auncient_ether.o \
                  $(OBJ_DIR)/tsfi_rb_llm_neurology.o \
                  $(OBJ_DIR)/tsfi_algol68_mode_transpiler.o \
                  $(OBJ_DIR)/tsfi_clendenin_synth.o \
                  $(OBJ_DIR)/tsfi_clendenin_transport.o \
                  $(OBJ_DIR)/tsfi_clendenin_matrix_exp.o \
                  $(OBJ_DIR)/tsfi_clendenin_quadrature_synth.o \
                  $(OBJ_DIR)/tsfi_conway_coroutine.o \
                  $(OBJ_DIR)/tsfi_autodin_conway_tx.o \
                  $(OBJ_DIR)/tsfi_conway_pipe.o \
                  $(OBJ_DIR)/tsfi_autodin_tx_pipe.o \
                  $(OBJ_DIR)/tsfi_conway_ledger_process.o \
                  $(OBJ_DIR)/tsfi_conway_interledger_signal.o \
                  $(OBJ_DIR)/tsfi_conway_dynamic_stack.o \
                  $(OBJ_DIR)/tsfi_hershkowitz_fit.o \
                  $(OBJ_DIR)/tsfi_hershkowitz_consensus.o \
                  $(OBJ_DIR)/tsfi_smalgol61_engine.o \
                  $(OBJ_DIR)/tsfi_ezvm_smalgol61_stack.o \
                  $(OBJ_DIR)/tsfi_speroni_params.o \
                  $(OBJ_DIR)/tsfi_speroni_ir_engine.o \
                  $(OBJ_DIR)/tsfi_speroni_conway_cobol.o \
                  $(OBJ_DIR)/tsfi_speroni_lynch_stream.o \
                  $(OBJ_DIR)/tsfi_decnet_exec_stack.o \
                  $(OBJ_DIR)/tsfi_tree_net_vm.o \
                  $(OBJ_DIR)/tsfi_speroni_param_list.o \
                  $(OBJ_DIR)/tsfi_speroni_define_format.o \
                  $(OBJ_DIR)/tsfi_speroni_sort_merge.o \
                  $(OBJ_DIR)/tsfi_dempster_scattering.o \
                  $(OBJ_DIR)/tsfi_bachelor_parse_dict.o \
                  $(OBJ_DIR)/tsfi_smalgol61_datatype_std.o \
                  $(OBJ_DIR)/tsfi_algol61_evm_types.o \
                  $(OBJ_DIR)/tsfi_algol_cobol_vulkan.o \
                  $(OBJ_DIR)/tsfi_universal_abi_firstclass.o \
                  $(OBJ_DIR)/tsfi_manderfield_engine.o \
                  $(OBJ_DIR)/tsfi_manderfield_compiler.o \
                  $(OBJ_DIR)/tsfi_manderfield_npl_transpiler.o \
                  $(OBJ_DIR)/tsfi_thacher_solver.o \
                  $(OBJ_DIR)/tsfi_stearman_engine.o \
                  $(OBJ_DIR)/tsfi_lgp30_algol_engine.o \
                  $(OBJ_DIR)/tsfi_sherman_engine.o \
                  $(OBJ_DIR)/tsfi_scott_engine.o \
                  $(OBJ_DIR)/tsfi_schmidt_engine.o \
                  $(OBJ_DIR)/tsfi_parc_ui_def.o \
                  $(OBJ_DIR)/tsfi_chapple_engine.o \
                  $(OBJ_DIR)/tsfi_naur_engine.o \
                  $(OBJ_DIR)/tsfi_hogan_tax_permutation.o \
                  $(OBJ_DIR)/tsfi_autodin_cumulative_permutation.o \
                  $(OBJ_DIR)/tsfi_autodin_cumulative_telemetry.o \
                  $(OBJ_DIR)/tsfi_autodin_cumulative_gateway.o \
                  $(OBJ_DIR)/tsfi_autodin_cumulative_quadtree.o \
                   $(OBJ_DIR)/tsfi_autodin_cumulative_winchester.o \
                   $(OBJ_DIR)/tsfi_autodin_cumulative_contract_resolver.o \
                   $(OBJ_DIR)/tsfi_autodin_cumulative_contract_comprehensive.o \
                   $(OBJ_DIR)/tsfi_parc_dahm_lock.o \
                    $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o \
                    $(OBJ_DIR)/tsfi_mann_controller.o \
                    $(OBJ_DIR)/tsfi_comp_pass.o \
                    $(OBJ_DIR)/tsfi_mvarsel.o \
                    $(OBJ_DIR)/tsfi_reroute.o \
                    $(OBJ_DIR)/tsfi_delegate_sig.o \
                    $(OBJ_DIR)/tsfi_stagecomp.o \
                    $(OBJ_DIR)/tsfi_ar.o \
                    $(OBJ_DIR)/tsfi_sdfformat.o \
                    $(OBJ_DIR)/tsfi_hydrascene.o \
                    $(OBJ_DIR)/tsfi_renderdelegate.o \
                    $(OBJ_DIR)/tsfi_stagelock.o \
                    $(OBJ_DIR)/tsfi_renderindex.o \
                    $(OBJ_DIR)/tsfi_mariner_fabric_dispatch.o \
                    $(OBJ_DIR)/tsfi_chancery_docket.o




# --- Plugin Objects ---
VULKAN_PLUGIN_OBJS = $(OBJ_DIR)/vulkan_init.o $(OBJ_DIR)/vulkan_main.o $(OBJ_DIR)/vulkan_render.o \
                     $(OBJ_DIR)/vulkan_input.o $(OBJ_DIR)/backend_display.o $(OBJ_DIR)/input_vty.o \
                     $(OBJ_DIR)/tsfi_input.o $(OBJ_DIR)/window_rendering.o \
                     $(OBJ_DIR)/vulkan_logic.o $(OBJ_DIR)/vulkan_xdg.o $(OBJ_DIR)/vulkan_dmabuf.o \
                     $(OBJ_DIR)/xdg-shell-protocol.o $(OBJ_DIR)/linux-dmabuf-v1-protocol.o \
                     $(OBJ_DIR)/wlr-layer-shell-unstable-v1-protocol.o \
                     $(OBJ_DIR)/tsfi_vulkan_guide_engine.o $(OBJ_DIR)/tsfi_vulkan_guide_app.o

.PHONY: all clean test

all: tsfi2 bin/tsfi_video_renderer bin/tsfi_paint_app bin/tsfi_mcp_server bin/tsfi_mcp_worker bin/tsfi_sd_worker bin/test_sd_thunk_execution bin/tsfi_dna_crossover tests/test_ahoy_issue1_resonance bin/test_wayland_terminal_shell bin/test_active_window_ocr bin/tsfi_font_editor bin/test_spider_registry_reap bin/test_spider_120f_reap tests/test_wayland_lunar_lander tests/test_wayland_space_war tests/test_wayland_gauntlet tests/test_wayland_yars_revenge tests/test_yars_revenge_features tests/test_zmm_cpu6502 tests/test_zmm_tape_verification tests/test_zmm_mvs_certification tests/test_zmm_mystery_house tests/test_shadow_registers tests/test_epyx_opcodes tests/test_aub_complete tests/test_wayland_sprite_editor tests/test_lode_runner tests/test_karateka tests/test_choplifter tests/test_cannonball bin/tsfi_sprite_generator tests/test_compuserve tests/test_speech_c_controller bin/benchmark_neuro_backprop bin/benchmark_neuro_activation bin/test_wave512_asm bin/test_walsh bin/test_walsh_audio bin/test_steiner_evi bin/test_nebula bin/test_divider bin/test_serge_usg bin/test_dtl_core bin/test_delay_line bin/test_rtl_latch bin/test_shaft_encoder bin/test_core_plane bin/test_tunnel_latch bin/test_thin_film bin/test_integrated_circuit bin/test_core_rope bin/test_ttl_gate bin/test_parametron bin/test_slt bin/test_cla bin/test_plated_wire bin/test_mema bin/test_sar_adc bin/test_schmitt bin/test_diode_matrix bin/test_byte_issue21 bin/test_byte_issue22 bin/test_byte_issue23 bin/test_byte_issue24 bin/test_oregon_trail bin/test_elektuur_issue25 bin/test_prophecy_synthesis bin/test_yang_synthesis tests/test_rooted_browser_zmm tests/test_rooted_browser_wmq tests/test_rooted_browser_wmq_collision tests/test_base0_protocol tests/test_dilemma_synthesizer tests/test_bessel_zener_trapping bin/test_zmm_teddy_bear_render bin/test_sort bin/test_graph_select tests/test_folklore_select tests/benchmark_neuro_folklore tests/test_qing_bst tests/test_qing_graph tests/test_qing_nesting tests/test_speech_trie tests/test_trie_routers tests/test_trie_to_bst tests/benchmark_neurology bin/test_formant_filter_c_rpc bin/test_ring_modulator_c_rpc bin/test_equa_amplifier_c_rpc bin/test_mostronome_c_rpc bin/test_gyrator_drum_c_rpc bin/test_germanium_stage_c_rpc bin/test_steam_whistle_c_rpc bin/rhythm_generator_c_rpc bin/compander_c_rpc bin/hysteresis_limiter_c_rpc bin/piezo_limiter_c_rpc bin/zener_dither_c_rpc bin/subharmonic_divider_c_rpc bin/wien_bridge_notch_c_rpc bin/bionika_reflex_c_rpc bin/tape_saturation_c_rpc bin/thermal_half_life_c_rpc bin/axonal_delay_c_rpc bin/vactrol_limiter_c_rpc bin/germanium_coax_string_c_rpc bin/zmm_disk_mount_c_rpc bin/test_ramac_layout bin/test_ramac_extended bin/test_ramac_extended_part2 bin/generate_mainframe_ramac_demo bin/test_mainframe_decnet bin/test_hogan_integration tests/test_nsa_tempest tests/test_mainframe_computerworld tests/test_wmq_tso_redundancy tests/test_coax_vtam_redundancy tests/test_xpl_skeleton_integration bin/test_stanag_mcp_redbox bin/test_listening_ears_security_suites bin/test_stanag4607_encoder bin/test_zmm_stanag_rollback bin/test_listening_ears_advanced_df bin/test_via6522_tdoa_fdoa bin/test_via_zmm_helmholtz_integration bin/test_stanag_mcp_spontaneous





# --- Primary Targets ---
tsfi2: $(OBJ_DIR)/main.o $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

libtsfi2.so: $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

bin/test_genesis_standalone: tests/test_genesis_standard_cell.c $(CORE_OBJS) $(OBJ_DIR)/Genesis_rtl.o $(OBJ_DIR)/Dai_rtl.o $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -Isrc/firmware -o $@ $^ $(LDFLAGS)

bin/test_zmm_dysnomia: tests/test_zmm_dysnomia.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_zmm_huc: tests/test_zmm_huc.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_zmm_huc_audio: tests/test_zmm_huc_audio.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_vce: tests/test_xpl_vce.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_skeleton: tests/test_xpl_skeleton.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_analyzer: tests/test_xpl_analyzer.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xplsm_vce: tests/test_xplsm_vce.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_autodin_tape_proof: tests/test_autodin_tape_proof.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_tws_upgrades: tests/test_xpl_tws_upgrades.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_tws_upgrades_exhaustive: tests/test_xpl_tws_upgrades_exhaustive.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_tws_optimizations: tests/test_xpl_tws_optimizations.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_tws_codegen: tests/test_xpl_tws_codegen.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_accumulator: tests/test_xpl_accumulator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_optimizations_exhaustive: tests/test_xpl_optimizations_exhaustive.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_tws_final: tests/test_xpl_tws_final.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_erc20: tests/test_xpl_erc20.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_machine_utils: tests/test_xpl_machine_utils.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_advanced_utils: tests/test_xpl_advanced_utils.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_wilf: tests/test_xpl_wilf.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_wilf_advanced: tests/test_xpl_wilf_advanced.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_bemer: tests/test_xpl_bemer.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_southworth: tests/test_xpl_southworth.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_runge_kutta: tests/test_xpl_runge_kutta.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_stdlib: tests/test_xpl_stdlib.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_math: tests/test_xpl_math.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_benchmarks: tests/test_xpl_benchmarks.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_coaxial_rk4: tests/test_xpl_coaxial_rk4.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_zmm_huc_hybrid_demo: tests/test_zmm_huc_hybrid_demo.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_jpeg_dct: tests/test_xpl_jpeg_dct.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/test_xpl_drawing_surface: tests/test_xpl_drawing_surface.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_system_benchmark: tests/test_xpl_system_benchmark.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_mariner_hardened: tests/test_xpl_mariner_hardened.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_network_stack: tests/test_xpl_network_stack.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_network_lib: tests/test_xpl_network_lib.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_peer_register: tests/test_xpl_peer_register.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_xpl_encrypt: tests/test_xpl_encrypt.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_huc_ocean_loader: tests/test_huc_ocean_loader.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

mariner-audit:
	bash /home/mariarahel/src/tsfi2/scripts/mariner_audit.sh


bin/benchmark_neuro_backprop: tests/benchmark_neuro_backprop.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/benchmark_neuro_activation: tests/benchmark_neuro_activation.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_hogan_integration: tests/test_hogan_integration.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_sovereign_saat_hogan_bridge: tests/test_sovereign_saat_hogan_bridge.c src/tsfi_frejlich_domain_governance.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_authentic_teddy_bear_hogan_ssa: tests/test_authentic_teddy_bear_hogan_ssa.c src/tsfi_chancery_docket.c src/auncient_cloth_material_bridge.c src/cloth_simulator.c src/auncient_timeline_autodin.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_zorse_vdso_hogan_units: tests/test_zorse_vdso_hogan_units.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_motzkin_recurrence_handshake: tests/test_motzkin_recurrence_handshake.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_biological_receptor: tests/test_mariner_biological_receptor.c src/tsfi_mariner_biological_receptor.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_bio_cloth_bridge: tests/test_mariner_bio_cloth_bridge.c src/tsfi_mariner_biological_receptor.c src/auncient_cloth_material_bridge.c src/cloth_simulator.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_svdag_bio_ingestion: tests/test_mariner_svdag_bio_ingestion.c src/tsfi_mariner_biological_receptor.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_phase2_unified_proof: tests/test_mariner_phase2_unified_proof.c src/tsfi_mariner_biological_receptor.c src/auncient_cloth_material_bridge.c src/cloth_simulator.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_bio_persistence: tests/test_mariner_bio_persistence.c src/tsfi_mariner_biological_receptor.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_bio_zmm_binding: tests/test_mariner_bio_zmm_binding.c src/tsfi_mariner_biological_receptor.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_fabric_dispatch: tests/test_mariner_fabric_dispatch.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mariner_rpc_dispatch: tests/test_mariner_rpc_dispatch.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_weighted_bayesian_filtration: tests/test_weighted_bayesian_filtration.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_motzkin_natural_filtration: tests/test_motzkin_natural_filtration.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_llm_code_diffusion_prover: tests/test_llm_code_diffusion_prover.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_prover_strategy_pivot: tests/test_prover_strategy_pivot.c src/tsfi_prover_strategy_pivot.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_tang_lut_nn_prover: tests/test_tang_lut_nn_prover.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_http_client: tests/test_http_client.c src/tsfi_http_client.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_web_extensions: tests/test_web_extensions.c src/tsfi_web_extensions.c src/tsfi_http_client.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_web_security_storage: tests/test_web_security_storage.c src/tsfi_web_security_storage.c src/tsfi_http_client.c src/tsfi_layout.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_advanced_web_stack: tests/test_advanced_web_stack.c src/tsfi_advanced_web_stack.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_compound_prover_strategy: tests/test_compound_prover_strategy.c src/tsfi_compound_prover_strategy.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_web_media_transport: tests/test_web_media_transport.c src/tsfi_web_media_transport.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lz

bin/test_webgl_audio_js_doh: tests/test_webgl_audio_js_doh.c src/tsfi_webgl_audio_js_doh.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/test_webrtc_sw_idb_flexgrid: tests/test_webrtc_sw_idb_flexgrid.c src/tsfi_webrtc_sw_idb_flexgrid.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_wasm_crypto_font_events: tests/test_wasm_crypto_font_events.c src/tsfi_wasm_crypto_font_events.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lssl -lcrypto

bin/test_svg_xpath_tex_worker: tests/test_svg_xpath_tex_worker.c src/tsfi_svg_xpath_tex_worker.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/test_tls13_media_rdf_dnd: tests/test_tls13_media_rdf_dnd.c src/tsfi_tls13_media_rdf_dnd.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_compositor_cascade_search_url: tests/test_compositor_cascade_search_url.c src/tsfi_compositor_cascade_search_url.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_bionika_synth: tests/test_bionika_synth.c src/tsfi_bionika_synth.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

bin/test_mp4_renderer: tests/test_mp4_renderer.c src/tsfi_mp4_renderer.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm























bin/test_wave512_asm: tests/test_wave512_asm.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_thunk_mq: tests/test_thunk_mq.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_walsh: tests/test_walsh.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_walsh_audio: tests/test_walsh_audio.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_steiner_evi: tests/test_steiner_evi.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_nebula: tests/test_nebula.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_divider: tests/test_divider.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_serge_usg: tests/test_serge_usg.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_dtl_core: tests/test_dtl_core.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_delay_line: tests/test_delay_line.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_rtl_latch: tests/test_rtl_latch.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_shaft_encoder: tests/test_shaft_encoder.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_core_plane: tests/test_core_plane.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_tunnel_latch: tests/test_tunnel_latch.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_thin_film: tests/test_thin_film.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_integrated_circuit: tests/test_integrated_circuit.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_core_rope: tests/test_core_rope.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_ttl_gate: tests/test_ttl_gate.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_parametron: tests/test_parametron.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_slt: tests/test_slt.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_cla: tests/test_cla.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_plated_wire: tests/test_plated_wire.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mema: tests/test_mema.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_sar_adc: tests/test_sar_adc.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_schmitt: tests/test_schmitt.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_diode_matrix: tests/test_diode_matrix.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_byte_issue21: tests/test_byte_issue21.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_byte_issue22: tests/test_byte_issue22.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_byte_issue23: tests/test_byte_issue23.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_byte_issue24: tests/test_byte_issue24.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_oregon_trail: tests/test_oregon_trail.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_elektuur_issue25: tests/test_elektuur_issue25.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_prophecy_synthesis: tests/test_prophecy_synthesis.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_yang_synthesis: tests/test_yang_synthesis.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_formant_filter_c_rpc: tests/test_formant_filter_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_ring_modulator_c_rpc: tests/test_ring_modulator_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_equa_amplifier_c_rpc: tests/test_equa_amplifier_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_mostronome_c_rpc: tests/test_mostronome_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_gyrator_drum_c_rpc: tests/test_gyrator_drum_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_germanium_stage_c_rpc: tests/test_germanium_stage_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_steam_whistle_c_rpc: tests/test_steam_whistle_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_rhythm_generator_c_rpc: tests/test_rhythm_generator_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_compander_c_rpc: tests/test_compander_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_hysteresis_limiter_c_rpc: tests/test_hysteresis_limiter_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_piezo_limiter_c_rpc: tests/test_piezo_limiter_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_zener_dither_c_rpc: tests/test_zener_dither_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_subharmonic_divider_c_rpc: tests/test_subharmonic_divider_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_wien_bridge_notch_c_rpc: tests/test_wien_bridge_notch_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_bionika_reflex_c_rpc: tests/test_bionika_reflex_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_tape_saturation_c_rpc: tests/test_tape_saturation_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_thermal_half_life_c_rpc: tests/test_thermal_half_life_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_axonal_delay_c_rpc: tests/test_axonal_delay_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_vactrol_limiter_c_rpc: tests/test_vactrol_limiter_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_germanium_coax_string_c_rpc: tests/test_germanium_coax_string_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_zmm_disk_mount_c_rpc: tests/test_zmm_disk_mount_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_zmm_ramac_rails: tests/test_zmm_ramac_rails.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_ramac_layout: tests/test_ramac_layout.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_ramac_extended: tests/test_ramac_extended.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_ramac_extended_part2: tests/test_ramac_extended_part2.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/generate_mainframe_ramac_demo: tests/generate_mainframe_ramac_demo.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_mainframe_decnet: tests/test_mainframe_decnet.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_compliance_auditor: src/tsfi_compliance_auditor.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_gas_audit: tests/test_gas_audit.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

bin/test_mcs_sync_client: tests/test_mcs_sync_client.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)



bin/test_valve_synthesis: tests/test_valve_synthesis.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_seventy_three_circuits: tests/test_seventy_three_circuits.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
bin/test_zener_simulation: tests/test_zener_simulation.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_sd_thunk_execution: tests/test_sd_thunk_execution.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_dna_crossover: src/tsfi_dna_crossover.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_dna_decoder: tests/test_dna_decoder.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_video_renderer: src/tsfi_video_renderer.c $(OBJ_DIR)/tsfi_montecarlo.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -mavx512f -o $@ $^ $(LDFLAGS)

bin/test_dualist_neuron_weights: tests/test_dualist_neuron_weights.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_dualist_pid_controller: tests/test_dualist_pid_controller.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_yang_audion_bjt_quality: tests/test_yang_audion_bjt_quality.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_yi_harmonizer: tests/test_yi_harmonizer.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_spark_helix: tests/test_spark_helix.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_hot_wire: tests/test_hot_wire.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_synth_ammeter: tests/test_synth_ammeter.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_oscillating_audion: tests/test_oscillating_audion.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_type_t_tubular: tests/test_type_t_tubular.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/bench_valve_synthesis: tests/bench_valve_synthesis.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_autodyne_demodulation: tests/test_autodyne_demodulation.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_harmonizer_melody: tests/test_harmonizer_melody.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_coil_parasitics: tests/test_coil_parasitics.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/prove_maxwell_fdtd: tests/prove_maxwell_fdtd.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_deforest_ground: tests/test_deforest_ground.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_wireless_dance: tests/test_wireless_dance.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_listening_ears: tests/test_listening_ears.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_active_window_ocr: tests/test_active_window_ocr.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_aerial_llm_bridge: tests/test_aerial_llm_bridge.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_vulkan_teddy_bear: tests/test_vulkan_teddy_bear.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_vulkan_terminal: tests/test_vulkan_terminal.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_vulkan_vision: tests/test_vulkan_vision.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_vm_dft_bridge: tests/test_vm_dft_bridge.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)







bin/test_spider_registry_reap: tests/test_spider_registry_reap.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_spider_120f_reap: tests/test_spider_120f_reap.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_ahoy_issue1_resonance: tests/test_ahoy_issue1_resonance.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_fighter: tests/test_zmm_fighter.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_quaternion_flight: tests/test_quaternion_flight.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_graphics: tests/test_zmm_graphics.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_parallel_cpu: tests/test_zmm_parallel_cpu.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_yars_revenge_features: tests/test_yars_revenge_features.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_cpu6502: tests/test_zmm_cpu6502.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_tape_verification: tests/test_zmm_tape_verification.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_mvs_certification: tests/test_zmm_mvs_certification.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_mcp_transactions: tests/test_zmm_mcp_transactions.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_rooted_browser_zmm: tests/test_rooted_browser_zmm.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_rooted_browser_wmq: tests/test_rooted_browser_wmq.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_rooted_browser_wmq_collision: tests/test_rooted_browser_wmq_collision.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_epibar_hypobar: tests/test_zmm_epibar_hypobar.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_yue_contracts: tests/test_zmm_yue_contracts.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_btc_rails: tests/test_zmm_btc_rails.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -Ofast -fomit-frame-pointer -o $@ $^ $(LDFLAGS)

tests/test_serialization_dat: tests/test_serialization_dat.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_rails_dat_c_rpc: tests/test_rails_dat_c_rpc.c
	$(CC) $(CFLAGS) -o $@ $^

tests/test_coaxial_telemetry_async: tests/test_coaxial_telemetry_async.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_qing_bst: tests/test_qing_bst.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_qing_graph: tests/test_qing_graph.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_qing_nesting: tests/test_qing_nesting.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_speech_trie: tests/test_speech_trie.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_trie_routers: tests/test_trie_routers.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_trie_to_bst: tests/test_trie_to_bst.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_lau_proof: tests/test_zmm_lau_proof.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_base0_protocol: tests/test_base0_protocol.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_auncient_integrated_cli: tests/test_auncient_integrated_cli.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_auncient_cobol_strategy: tests/test_auncient_cobol_strategy.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_auncient_lexicon_unit: tests/test_auncient_lexicon_unit.c ../src/auncient_sdk.o
	$(CC) $(CFLAGS) -o $@ $^ -lm






tests/test_dilemma_synthesizer: tests/test_dilemma_synthesizer.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_bessel_zener_trapping: tests/test_bessel_zener_trapping.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_oscillating_audion: tests/test_oscillating_audion.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_rooted_browser_wmq_prophecy: tests/test_rooted_browser_wmq_prophecy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zmm_mystery_house: tests/test_zmm_mystery_house.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_shadow_registers: tests/test_shadow_registers.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/native_rdna4_rebar: tests/native_rdna4_rebar.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/native_rdna4_wmma: tests/native_rdna4_wmma.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_epyx_opcodes: tests/test_epyx_opcodes.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_aub_complete: tests/test_aub_complete.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_auncient_btc_tree_tests: tests/test_auncient_btc_tree_tests.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_lode_runner: tests/test_lode_runner.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_karateka: tests/test_karateka.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_choplifter: tests/test_choplifter.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_cannonball: tests/test_cannonball.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_compuserve: tests/test_compuserve.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_speech_c_controller: tests/test_speech_c_controller.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_sprite_editor: tests/test_wayland_sprite_editor.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_hardball: tests/test_wayland_hardball.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_smurfs: tests/test_wayland_smurfs.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_hex_rpg: tests/test_wayland_hex_rpg.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_lunar_lander: tests/test_wayland_lunar_lander.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_space_war: tests/test_wayland_space_war.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_yars_revenge: tests/test_wayland_yars_revenge.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wayland_gauntlet: tests/test_wayland_gauntlet.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_wayland_terminal_shell: tests/test_wayland_terminal_shell.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_font_editor: src/tsfi_font_editor.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_paint_app: src/tsfi_paint_app.c $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_teddy_bear_editor: src/tsfi_teddy_bear_editor.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_teddy_bear_skeleton: src/tsfi_teddy_bear_skeleton.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


bin/tsfi_teddy_bear_animator: src/tsfi_teddy_bear_animator.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_mcp_server: src/tsfi_mcp_server.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_zmm_rpc.o $(OBJ_DIR)/vulkan_init.o $(OBJ_DIR)/tsfi_block_monitor.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_mcp_worker: src/tsfi_mcp_worker.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_zmm_rpc.o $(OBJ_DIR)/vulkan_init.o $(OBJ_DIR)/tsfi_block_monitor.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_stanag_mcp_redbox: tests/test_stanag_mcp_redbox.c $(OBJ_DIR)/tsfi_ccx_pool.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lm

bin/test_listening_ears_security_suites: tests/test_listening_ears_security_suites.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

bin/test_stanag4607_encoder: tests/test_stanag4607_encoder.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

bin/test_listening_ears_advanced_df: tests/test_listening_ears_advanced_df.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/test_via6522_tdoa_fdoa: tests/test_via6522_tdoa_fdoa.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/test_via_zmm_helmholtz_integration: tests/test_via_zmm_helmholtz_integration.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/test_stanag_mcp_spontaneous: tests/test_stanag_mcp_spontaneous.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

bin/test_zmm_stanag_rollback: tests/test_zmm_stanag_rollback.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o $(OBJ_DIR)/tsfi_zmm_rpc.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_sd_worker: src/tsfi_native_sd.cpp $(filter-out $(OBJ_DIR)/main.o, $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ src/tsfi_native_sd.cpp $(filter-out src/tsfi_native_sd.cpp, $^) -Lassets/sd_cpp/build -Lassets/sd_cpp/build/ggml/src -Lassets/sd_cpp/build/ggml/src/ggml-vulkan -lstable-diffusion -lggml -lggml-cpu -lggml-base -lggml-vulkan $(LDFLAGS) -lgomp

# --- Tool Realization ---
bin/tsfi_v2c: tools/tsfi_v2c.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

bin/tsfi_rdl2c: tools/tsfi_rdl2c.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

bin/tsfi_sprite_generator: tools/tsfi_sprite_generator.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<

bin/tsfi_ipomoea_video_gen: tools/tsfi_ipomoea_video_gen.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/demo_nadler_super8_lowpower: tests/demo_nadler_super8_lowpower.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# --- Firmware Realization ---
src/firmware/Genesis_rtl.c src/firmware/Genesis_rtl.h: src/firmware/Genesis.v bin/tsfi_v2c
	./bin/tsfi_v2c $<

src/firmware/Dai_rtl.c src/firmware/Dai_rtl.h: src/firmware/00_Dai.rdl bin/tsfi_rdl2c
	./bin/tsfi_rdl2c $<

src/firmware/LauWireFirmware_rtl.c src/firmware/LauWireFirmware_rtl.h: src/firmware/LauWireFirmware.v bin/tsfi_v2c
	./bin/tsfi_v2c $<

# --- Object Rules ---
$(OBJ_DIR)/tsfi_wire_firmware.o: src/tsfi_wire_firmware.c src/firmware/LauWireFirmware_rtl.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: thunks/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: src/firmware/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/firmware/%.o: src/firmware/%.c | $(OBJ_DIR)/firmware
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/vulkan/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: plugins/window_src/%.c | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

tests/node_interop.o: tests/node_interop.cpp tests/node_interop.h
	$(CXX) $(CXXFLAGS) -I/usr/include/node -c -o $@ $<

tests/rooted_frame_presenter: tests/rooted_frame_presenter.c tests/rooted_wayland_events.c tests/libmozilla_interop.c tests/node_interop.o $(OBJ_DIR)/tsfi_media_decode.o src/tsfi_layout.c src/tsfi_mariner_fabric_dispatch.c src/tsfi_chancery_docket.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -I/usr/include/node -I/home/mariarahel/src/pkgs/chromium-142.0.7444.175/third_party/ffmpeg -o $@ $^ $(LDFLAGS) -lnode /usr/lib/x86_64-linux-gnu/libavformat.so.62 -lavcodec -lavutil



tests/rooted_vulkan_presenter: tests/rooted_vulkan_presenter.c tests/rooted_wayland_events.c tests/libmozilla_interop.c tests/node_interop.o $(OBJ_DIR)/tsfi_media_decode.o src/tsfi_layout.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -I/usr/include/node -I/home/mariarahel/src/pkgs/chromium-142.0.7444.175/third_party/ffmpeg -o $@ $^ $(LDFLAGS) -lnode /usr/lib/x86_64-linux-gnu/libavformat.so.62 -lavcodec -lavutil

tests/test_fetch_launcher: tests/test_fetch_launcher.cpp tests/node_interop.o src/tsfi_dom.c src/tsfi_layout.c src/tsfi_paint.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CXX) $(CXXFLAGS) -I/usr/include/node -o $@ $^ $(LDFLAGS) -lnode

tests/capture_states: tests/capture_states.c src/tsfi_dom.c src/tsfi_layout.c src/tsfi_paint.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_phase6: tests/test_phase6.cpp tests/node_interop.o src/tsfi_dom.c src/tsfi_layout.c src/tsfi_paint.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CXX) $(CXXFLAGS) -I/usr/include/node -o $@ $^ $(LDFLAGS) -lnode

$(OBJ_DIR)/tsfi_media_decode.o: src/tsfi_media_decode.c inc/tsfi_media_decode.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I/home/mariarahel/src/pkgs/chromium-142.0.7444.175/third_party/ffmpeg -c -o $@ $<

tests/test_media_stream: tests/test_media_stream.cpp $(OBJ_DIR)/tsfi_media_decode.o src/tsfi_dom.c src/tsfi_layout.c src/tsfi_paint.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CXX) $(CXXFLAGS) -I/home/mariarahel/src/pkgs/chromium-142.0.7444.175/third_party/ffmpeg -o $@ $^ $(LDFLAGS) /usr/lib/x86_64-linux-gnu/libavformat.so.62 -lavcodec -lavutil

tests/test_layout_paint: tests/test_layout_paint.c src/tsfi_dom.c src/tsfi_layout.c src/tsfi_paint.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/validate_youtube_layout: tests/validate_youtube_layout.c $(OBJ_DIR)/tsfi_media_decode.o src/tsfi_dom.c src/tsfi_layout.c src/tsfi_paint.c $(CORE_OBJS) $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -I/home/mariarahel/src/pkgs/chromium-142.0.7444.175/third_party/ffmpeg -o $@ $^ $(LDFLAGS) /usr/lib/x86_64-linux-gnu/libavformat.so.62 -lavcodec -lavutil


bin/test_zmm_teddy_bear_render: tests/test_zmm_teddy_bear_render.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_verlet_poppy: tests/test_verlet_poppy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


bin/test_sort: tests/test_sort.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_graph_select: tests/test_graph_select.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_folklore_select: tests/test_folklore_select.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/benchmark_neuro_folklore: tests/benchmark_neuro_folklore.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/benchmark_neurology: tests/benchmark_neurology.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_xplos_active_bus: tests/test_xplos_active_bus.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_ast_optimizer: tests/test_ast_optimizer.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_rau_coprocessor: tests/test_rau_coprocessor.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_rau_mcp_mount: tests/test_rau_mcp_mount.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_rau_thunk: tests/test_rau_thunk.c thunks/tsfi_rau_thunk.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xplos_alu_rau_integration: tests/test_xplos_alu_rau_integration.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xplos_dynamic_coax: tests/test_xplos_dynamic_coax.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_tape_spanning: tests/test_tape_spanning.c $(OBJ_DIR)/tsfi_parc_tape_label_yul.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_naur_parser: tests/test_xpl_naur_parser.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_ispf_parser: tests/test_xpl_ispf_parser.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_vtoc: tests/test_xpl_vtoc.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_smoke_mirrors: tests/test_xpl_smoke_mirrors.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_pds_lister: tests/test_xpl_pds_lister.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_tsso_help: tests/test_xpl_tsso_help.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_copypds: tests/test_xpl_copypds.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_compress: tests/test_xpl_compress.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_renel: tests/test_xpl_renel.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_sesslog: tests/test_xpl_sesslog.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_tbdispl: tests/test_xpl_tbdispl.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_clist: tests/test_xpl_clist.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_vtoc_util: tests/test_xpl_vtoc_util.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_file300: tests/test_xpl_file300.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_lams: tests/test_xpl_lams.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_lams_disk: tests/test_xpl_lams_disk.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_daf: tests/test_xpl_daf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_daf_quadtree: tests/test_xpl_daf_quadtree.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_folklore_cpu: tests/test_xpl_folklore_cpu.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_folklore_cpu_e2e: tests/test_xpl_folklore_cpu_e2e.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_fvmp: tests/test_xpl_fvmp.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_tiger_sunset: tests/test_xpl_tiger_sunset.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_graphics_db: tests/test_xpl_graphics_db.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_xpl_winchester_mq: tests/test_xpl_winchester_mq.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tools/tsfi_run_fvmp: tools/tsfi_run_fvmp.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

benchmarks/profiler_suite/bench_xplos_bus: benchmarks/profiler_suite/bench_xplos_bus.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

benchmarks/profiler_suite/bench_rau_access: benchmarks/profiler_suite/bench_rau_access.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

benchmarks/profiler_suite/bench_folklore_cpu: benchmarks/profiler_suite/bench_folklore_cpu.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

virtual_tape_suite: tools/tsfi_copymods_terse tools/tsfi_tapemap_terse tools/tsfi_pdsload_terse tools/tsfi_offload_terse tools/tsfi_cbtupd_terse tools/tsfi_docfile_terse tools/tsfi_blkdisk_terse tools/tsfi_clist_help tools/tsfi_abi_monitor

tools/tsfi_copymods_terse: tools/tsfi_copymods_terse.c src/tsfi_terse_fieldata.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_tapemap_terse: tools/tsfi_tapemap_terse.c src/tsfi_terse_fieldata.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_pdsload_terse: tools/tsfi_pdsload_terse.c src/tsfi_terse_fieldata.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_offload_terse: tools/tsfi_offload_terse.c src/tsfi_terse_fieldata.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_cbtupd_terse: tools/tsfi_cbtupd_terse.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_docfile_terse: tools/tsfi_docfile_terse.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_blkdisk_terse: tools/tsfi_blkdisk_terse.c src/tsfi_terse_fieldata.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_clist_help: tools/tsfi_clist_help.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

tools/tsfi_abi_monitor: tools/tsfi_abi_monitor.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) tsfi2 src/firmware/*_rtl.*

.PHONY: generate_artifacts
generate_artifacts: bin/test_vulkan_teddy_bear
	bin/test_vulkan_teddy_bear --render-once
	convert /home/mariarahel/.gemini/antigravity-cli/brain/821a69ae-01ae-45c0-9773-e2e6e367181c/scratch/photorealistic_teddy_bear.ppm /home/mariarahel/.gemini/antigravity-cli/brain/821a69ae-01ae-45c0-9773-e2e6e367181c/photorealistic_bear_final.png

tests/test_ac_compositor_interop: tests/test_ac_compositor_interop.c tests/libmozilla_interop.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_carrier_monitor: tests/test_computel_carrier_monitor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_trunk_selector: tests/test_computel_trunk_selector.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_dtmf_decoder: tests/test_computel_dtmf_decoder.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_dtmf_loopback: tests/test_computel_dtmf_loopback.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_dtmf_sequencer: tests/test_computel_dtmf_sequencer.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_yul_firewall: tests/test_computel_yul_firewall.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_dtmf_progress_tones: tests/test_computel_dtmf_progress_tones.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_rotary_decoder: tests/test_computel_rotary_decoder.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_secure_kermit: tests/test_computel_secure_kermit.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_udp_mcp: tests/test_computel_udp_mcp.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_advanced_kermit: tests/test_computel_advanced_kermit.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_controller_thunk: tests/test_computel_controller_thunk.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_dmt_udp_benchmark: tests/test_computel_dmt_udp_benchmark.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_fsk_modulator: tests/test_computel_fsk_modulator.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_secure_tty: tests/test_computel_secure_tty.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_blue_box: tests/test_computel_blue_box.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_red_box: tests/test_computel_red_box.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_computel_yellow_box: tests/test_computel_yellow_box.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_pdp11_mounts: ../tests/test_pdp11_mounts.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_pdp11_encrypted_tunnel: ../tests/test_pdp11_encrypted_tunnel.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_mcp_tcp_proxy: ../tests/test_mcp_tcp_proxy.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_mcp_multiplexed_encrypted_proxy: ../tests/test_mcp_multiplexed_encrypted_proxy.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_level_up_scheduler: ../tests/test_level_up_scheduler.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_rails_sync_daemon: src/tsfi_rails_sync_daemon.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_logos_agent_kernel: tests/test_logos_agent_kernel.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_datalog_unification: tests/test_datalog_unification.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_algebraic_unification: tests/test_algebraic_unification.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_reuter_tx: tests/test_reuter_tx.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_gray_tx: tests/test_gray_tx.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_autodin_sage: tests/test_autodin_sage.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_encodings: tests/test_encodings.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_doxastic_thermodynamics: tests/test_doxastic_thermodynamics.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_knowledge_graph_unification: tests/test_knowledge_graph_unification.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_probabilistic_unification: tests/test_probabilistic_unification.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_turing_semantics: tests/test_turing_semantics.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_horn_abduction: tests/test_horn_abduction.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_turing_probabilistic: tests/test_turing_probabilistic.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_disjunction_backtracking: tests/test_disjunction_backtracking.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_disjunction_metaphor: tests/test_disjunction_metaphor.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_datalog_retraction: tests/test_datalog_retraction.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_abductive_retraction: tests/test_abductive_retraction.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/bench_doxastic_abduction: tests/bench_doxastic_abduction.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_question_crystallization: tests/test_question_crystallization.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_doxastic_classifier: tests/test_doxastic_classifier.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_strain_classifier: tests/test_strain_classifier.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_doxastic_rules: tests/test_doxastic_rules.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_doxastic_loop: tests/test_doxastic_loop.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_peer_abduction: tests/test_peer_abduction.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_doxastic_projector: tests/test_doxastic_projector.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_algebraic_measures: tests/test_algebraic_measures.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_algebraic_query: tests/test_algebraic_query.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_markov_selection: tests/test_markov_selection.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_complex_reasoning: tests/test_complex_reasoning.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_bayesian_matrix: tests/test_bayesian_matrix.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_unified_classifier: tests/test_unified_classifier.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/bench_unified_classifier: tests/bench_unified_classifier.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_akb: tests/test_akb.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_box_orchestrator: tests/test_box_orchestrator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_synth_akb: tests/test_synth_akb.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_synth_direct: tests/test_synth_direct.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_synth_perf: tests/test_synth_perf.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/bench_synth_perf: tests/bench_synth_perf.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_winchester_bridge: tests/test_winchester_bridge.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_dual_stack: tests/test_dual_stack.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_doxastic_pipeline: tests/test_doxastic_pipeline.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_nlp: tests/test_nlp.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_phoneme_synth: tests/test_phoneme_synth.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_zorse_eval: tests/test_zorse_eval.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_markov_lm: tests/test_markov_lm.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_shm_prune: tests/test_shm_prune.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_lexical_trie: tests/test_lexical_trie.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_anvil_vm: tests/test_anvil_vm.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_mpc_arpanet: tests/test_mpc_arpanet.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_ot_accumulator: tests/test_ot_accumulator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_coaxial_observer: tests/test_coaxial_observer.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_coaxial_agent: tests/test_coaxial_agent.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_priority_queue: tests/test_priority_queue.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_nsa_tempest: tests/test_nsa_tempest.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_mainframe_computerworld: tests/test_mainframe_computerworld.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_wmq_tso_redundancy: tests/test_wmq_tso_redundancy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_coax_vtam_redundancy: tests/test_coax_vtam_redundancy.c src/tsfi_xplos_shell_cbt_vtam.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_tso.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

tests/test_xpl_skeleton_integration: tests/test_xpl_skeleton_integration.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_vtam.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_tso.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

tests/test_xpl_central_components: tests/test_xpl_central_components.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_vtam.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_tso.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz



tests/test_xplos_explorer_vsam: tests/test_xplos_explorer_vsam.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_vtam.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_tso.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

tests/test_skeleton_hasp_book: tests/test_skeleton_hasp_book.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_tso.c ../src/auncient_cloth_material_bridge.c ../src/cloth_simulator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

tests/bench_jcl_spool: tests/bench_jcl_spool.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_tso.c ../src/auncient_cloth_material_bridge.c ../src/cloth_simulator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

tests/bench_cics_queues: tests/bench_cics_queues.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_tso.c ../src/auncient_cloth_material_bridge.c ../src/cloth_simulator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

bin/cics_cli: src/cics_cli.c src/tsfi_xplos_shell_cbt_cics.c src/tsfi_xplos_shell_cbt_jcl.c src/tsfi_xplos_shell_cbt_jes.c src/tsfi_xplos_shell_book.c src/tsfi_xplos_kernel.c src/tsfi_xplos_shell_legacy.c src/tsfi_xplos_shell_tape.c src/tsfi_xplos_shell_explorer.c src/tsfi_xplos_shell_xdc.c src/tsfi_cbt_inmemory.c src/tsfi_xpl_mallgren.c src/tsfi_xplos_shell_cbt_tso.c ../src/auncient_cloth_material_bridge.c ../src/cloth_simulator.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	@mkdir -p bin
	$(CC) $(filter-out -Werror,$(CFLAGS)) -o $@ $^ $(LDFLAGS) -lz

tests/test_jes_compiler_generator: tests/test_jes_compiler_generator.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lm

tests/test_strategy_lang: tests/test_strategy_lang.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_logic_vs_strategy: tests/test_logic_vs_strategy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_spec_queues: tests/test_spec_queues.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_strategy_contract: tests/test_strategy_contract.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_rails_strategy: tests/test_rails_strategy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_domain_strategies: tests/test_domain_strategies.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/bench_logical_inferences: tests/bench_logical_inferences.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_coaxial_strategy: tests/test_coaxial_strategy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_direct_ring_strategy: tests/test_direct_ring_strategy.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_helmholtz_command_rails: tests/test_helmholtz_command_rails.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_coaxial_telemetry: tests/test_coaxial_telemetry.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_coaxial_kermit_rails: tests/test_coaxial_kermit_rails.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_mcp_stanag_kermit:
	node tests/test_mcp_stanag_kermit.js

tests/test_cooperative_sync_hosts: tests/test_cooperative_sync_hosts.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_cooperative_sync_hosts_net: tests/test_cooperative_sync_hosts_net.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_cooperative_sync_clients_net: tests/test_cooperative_sync_clients_net.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_cooperative_sync_telemetry: tests/test_cooperative_sync_telemetry.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_unified_sync_sandbox: tests/test_unified_sync_sandbox.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_shm_ipc_cooperative: tests/test_shm_ipc_cooperative.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_shm_kermit_cooperative: tests/test_shm_kermit_cooperative.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_shm_ring_cooperative: tests/test_shm_ring_cooperative.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

benchmarks/profiler_suite/bench_strategy_vm: benchmarks/profiler_suite/bench_strategy_vm.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_ray_tracer: tests/test_ray_tracer.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/demo_quantel_synthesizer: tests/demo_quantel_synthesizer.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/demo_tiger_super8: tests/demo_tiger_super8.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/tsfi_vulkan_guide: tests/tsfi_vulkan_guide_main.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_vulkan_guide_suite: tests/test_vulkan_guide_suite.c $(CORE_OBJS) $(OBJ_DIR)/tsfi_paint.o $(OBJ_DIR)/tsfi_dom.o $(VULKAN_PLUGIN_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_micro_focus: tests/test_micro_focus.c tests/test_micro_focus_part2.c tests/test_micro_focus_part3.c tests/test_micro_focus_part4.c tests/test_micro_focus_part5.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

bin/test_micro_focus_part2: bin/test_micro_focus
	ln -sf test_micro_focus bin/test_micro_focus_part2


test: bin/test_vulkan_teddy_bear tests/test_rooted_browser_wmq_collision tests/test_ac_compositor_interop tests/test_computel_carrier_monitor tests/test_computel_trunk_selector tests/test_computel_dtmf_decoder tests/test_computel_dtmf_loopback tests/test_computel_dtmf_sequencer tests/test_computel_yul_firewall tests/test_computel_dtmf_progress_tones tests/test_computel_rotary_decoder tests/test_computel_secure_kermit tests/test_computel_udp_mcp tests/test_computel_advanced_kermit tests/test_computel_controller_thunk tests/test_computel_dmt_udp_benchmark tests/test_computel_fsk_modulator tests/test_computel_secure_tty tests/test_computel_blue_box tests/test_computel_red_box tests/test_computel_yellow_box tests/test_pdp11_mounts tests/test_pdp11_encrypted_tunnel tests/test_mcp_tcp_proxy tests/test_mcp_multiplexed_encrypted_proxy tests/test_level_up_scheduler
	bin/test_vulkan_teddy_bear --test
	./tests/test_rooted_browser_wmq_collision
	./tests/test_ac_compositor_interop
	./tests/test_computel_carrier_monitor
	./tests/test_computel_trunk_selector
	./tests/test_computel_dtmf_decoder
	./tests/test_computel_dtmf_loopback
	./tests/test_computel_dtmf_sequencer
	./tests/test_computel_yul_firewall
	./tests/test_computel_dtmf_progress_tones
	./tests/test_computel_rotary_decoder
	./tests/test_computel_secure_kermit
	./tests/test_computel_udp_mcp
	./tests/test_computel_advanced_kermit
	./tests/test_computel_controller_thunk
	./tests/test_computel_dmt_udp_benchmark
	./tests/test_computel_fsk_modulator
	./tests/test_computel_secure_tty
	./tests/test_computel_blue_box
	./tests/test_computel_red_box
	./tests/test_computel_yellow_box
	./tests/test_pdp11_mounts
	./tests/test_pdp11_encrypted_tunnel
	./tests/test_mcp_tcp_proxy
	./tests/test_mcp_multiplexed_encrypted_proxy
	./tests/test_level_up_scheduler

bin/test_schrack_suite: tests/test_schrack_suite.c $(OBJ_DIR)/tsfi_parc_schrack.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_naur_ab14_suite: tests/test_naur_ab14_suite.c $(OBJ_DIR)/tsfi_parc_naur_ab14.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_runcible_cics_suite: tests/test_runcible_cics_suite.c $(OBJ_DIR)/tsfi_parc_runcible_cics.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_ingerman_translator_suite: tests/test_ingerman_translator_suite.c $(OBJ_DIR)/tsfi_parc_ingerman_translator.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_ingerman_alg141_suite: tests/test_ingerman_alg141_suite.c $(OBJ_DIR)/tsfi_parc_ingerman_alg141.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_ingerman_thunk_suite: tests/test_ingerman_thunk_suite.c $(OBJ_DIR)/tsfi_parc_ingerman_thunk.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_ingerman_display_suite: tests/test_ingerman_display_suite.c $(OBJ_DIR)/tsfi_parc_ingerman_display.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_ingerman_syntax_suite: tests/test_ingerman_syntax_suite.c $(OBJ_DIR)/tsfi_parc_ingerman_syntax.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_contract_coefficients_suite: tests/test_contract_coefficients_suite.c $(OBJ_DIR)/tsfi_parc_contract_coefficients.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_mariarahel_lau_view: tests/test_mariarahel_lau_view.c $(OBJ_DIR)/tsfi_parc_contract_coefficients.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_pulsechain_lp_matrix_suite: tests/test_pulsechain_lp_matrix_suite.c $(OBJ_DIR)/tsfi_parc_pulsechain_lp_matrix.o $(OBJ_DIR)/tsfi_parc_ingerman_alg141.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

tests/bench_zero_overhead: tests/bench_zero_overhead.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	@mkdir -p tests
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

tests/bench_deepseek_mla: tests/bench_deepseek_mla.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	@mkdir -p tests
	$(CC) $(filter-out -fstack-protector-strong -D_FORTIFY_SOURCE=3, $(CFLAGS)) -ffast-math -funroll-loops -fomit-frame-pointer $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_zorse_gguf_flllm_suite: tests/test_zorse_gguf_flllm_suite.c $(OBJ_DIR)/tsfi_faster_lighter_llm.o $(OBJ_DIR)/tsfi_gguf_loader.o $(OBJ_DIR)/tsfi_ai_core.o $(OBJ_DIR)/tsfi_xplos_shell_cbt_vtam.o $(OBJ_DIR)/tsfi_helmholtz_ring.o $(OBJ_DIR)/tsfi_chamberland_duality.o $(OBJ_DIR)/tsfi_alessandrini_condenser.o $(OBJ_DIR)/tsfi_stekloff_duality.o $(OBJ_DIR)/tsfi_zorse_tomography.o $(OBJ_DIR)/tsfi_geometric_algebra.o $(OBJ_DIR)/tsfi_subword_trie_cache.o $(OBJ_DIR)/tsfi_quantized_rag.o $(OBJ_DIR)/tsfi_code_review.o $(OBJ_DIR)/tsfi_telpa_program_analysis.o $(OBJ_DIR)/tsfi_flash_deepseek.o $(OBJ_DIR)/tsfi_totient_qing_tomography.o $(OBJ_DIR)/tsfi_zhu_model_compression.o $(OBJ_DIR)/tsfi_loraprune_deepseek.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -std=c11 -D_POSIX_C_SOURCE=200809L -Iinc -I../inc -Isrc -O3 -g -march=native -flax-vector-conversions -fPIC $^ -o $@ -lm

bin/zorse_prompt: tools/zorse_prompt.c $(OBJ_DIR)/tsfi_faster_lighter_llm.o $(OBJ_DIR)/tsfi_gguf_loader.o $(OBJ_DIR)/tsfi_ai_core.o $(OBJ_DIR)/tsfi_xplos_shell_cbt_vtam.o $(OBJ_DIR)/tsfi_helmholtz_ring.o $(OBJ_DIR)/tsfi_chamberland_duality.o $(OBJ_DIR)/tsfi_alessandrini_condenser.o $(OBJ_DIR)/tsfi_stekloff_duality.o $(OBJ_DIR)/tsfi_zorse_tomography.o $(OBJ_DIR)/tsfi_geometric_algebra.o $(OBJ_DIR)/tsfi_subword_trie_cache.o $(OBJ_DIR)/tsfi_quantized_rag.o $(OBJ_DIR)/tsfi_code_review.o $(OBJ_DIR)/tsfi_telpa_program_analysis.o $(OBJ_DIR)/tsfi_flash_deepseek.o $(OBJ_DIR)/tsfi_totient_qing_tomography.o $(OBJ_DIR)/tsfi_zhu_model_compression.o $(OBJ_DIR)/tsfi_loraprune_deepseek.o $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o $(OBJ_DIR)/tsfi_deepseek_cordis_advanced.o $(OBJ_DIR)/tsfi_deepseek_cordis_tools.o $(OBJ_DIR)/tsfi_deepseek_cordis_chain.o $(OBJ_DIR)/tsfi_deepseek_edsac_acp_firewall.o $(OBJ_DIR)/tsfi_deepseek_stanag_kermit.o
	$(CC) -Wall -Wextra -Wno-unused-result -Wno-unused-parameter -std=c11 -D_POSIX_C_SOURCE=200809L -Iinc -I../inc -Isrc -O3 -g -march=native -flax-vector-conversions -fPIC $^ -o $@ -lm

bin/zorse_trace_inspector: tools/zorse_trace_inspector.c
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O3 -g $^ -o $@

bin/test_deepseek_cordis: tests/test_deepseek_cordis.c $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_cordis_advanced: tests/test_deepseek_cordis_advanced.c $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o $(OBJ_DIR)/tsfi_deepseek_cordis_advanced.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_cordis_tools: tests/test_deepseek_cordis_tools.c $(OBJ_DIR)/tsfi_deepseek_cordis_tools.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_cordis_chain: tests/test_deepseek_cordis_chain.c $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o $(OBJ_DIR)/tsfi_deepseek_cordis_chain.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_edsac_acp_firewall: tests/test_deepseek_edsac_acp_firewall.c $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o $(OBJ_DIR)/tsfi_deepseek_edsac_acp_firewall.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_stanag_kermit: tests/test_deepseek_stanag_kermit.c $(OBJ_DIR)/tsfi_deepseek_stanag_kermit.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_agent_vm_operator: tests/test_deepseek_agent_vm_operator.c $(OBJ_DIR)/tsfi_deepseek_agent_vm_operator.o $(OBJ_DIR)/tsfi_deepseek_edsac_acp_firewall.o $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_cordis_composability: tests/test_deepseek_cordis_composability.c $(OBJ_DIR)/tsfi_deepseek_cordis_composability.o $(OBJ_DIR)/tsfi_deepseek_edsac_acp_firewall.o $(OBJ_DIR)/tsfi_deepseek_cordis_harness.o $(OBJ_DIR)/tsfi_deepseek_stanag_kermit.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_extensions: tests/test_deepseek_extensions.c $(OBJ_DIR)/tsfi_deepseek_extensions.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_deepseek_gguf_extended: tests/test_deepseek_gguf_extended.c $(OBJ_DIR)/tsfi_deepseek_gguf_extended.o
	@mkdir -p bin
	$(CC) -Wall -Wextra -std=c11 -O2 -Iinc -Isrc $^ -o $@ -lm

bin/test_melting_point_correspondence: tests/test_melting_point_correspondence.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_all_strategies_equivalence: tests/test_all_strategies_equivalence.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

bin/test_cobol_compliance: tests/test_cobol_compliance.c $(CORE_OBJS) $(OBJ_DIR)/vulkan_init.o
	@mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@ $(LDFLAGS)

.PHONY: test_flllm check_flllm profile test_cordis
test_cordis: bin/test_deepseek_cordis bin/test_deepseek_cordis_advanced bin/test_deepseek_cordis_tools bin/test_deepseek_cordis_chain bin/test_deepseek_edsac_acp_firewall bin/test_deepseek_stanag_kermit bin/test_deepseek_agent_vm_operator bin/test_deepseek_cordis_composability bin/test_deepseek_extensions bin/test_deepseek_gguf_extended bin/test_melting_point_correspondence bin/test_all_strategies_equivalence bin/test_cobol_compliance
	./bin/test_deepseek_cordis
	./bin/test_deepseek_cordis_advanced
	./bin/test_deepseek_cordis_tools
	./bin/test_deepseek_cordis_chain
	./bin/test_deepseek_edsac_acp_firewall
	./bin/test_deepseek_stanag_kermit
	./bin/test_deepseek_agent_vm_operator
	./bin/test_deepseek_cordis_composability
	./bin/test_deepseek_extensions
	./bin/test_deepseek_gguf_extended
	./bin/test_melting_point_correspondence
	./bin/test_all_strategies_equivalence
	./bin/test_cobol_compliance

test_flllm: bin/test_zorse_gguf_flllm_suite bin/zorse_prompt bin/zorse_trace_inspector test_cordis
	./bin/test_zorse_gguf_flllm_suite

check_flllm: test_flllm

profile:
	bash benchmarks/profiler_suite/run_benchmarks.sh
