.PHONY: help test-all test-dashboard test-container test-git-ci test-unit tpu-benchmarks livrps-benchmark test-mann test-comp-pass test-mvarsel test-reroute test-delegate-sig test-stagecomp test-ar test-sdfformat test-hydrascene test-renderdelegate test-stagelock test-renderindex test-usdshade

help:
	@echo "Available test targets:"
	@echo "  make test-all        - Run all E2E and unit tests in the project"
	@echo "  make test-dashboard  - Run NoNukes Dashboard E2E tests (including SPA)"
	@echo "  make test-container  - Run Teddy Bear Diagnostics container E2E tests"
	@echo "  make test-git-ci     - Run Git post-commit hook pipeline E2E tests"
	@echo "  make test-unit       - Run Python unit tests under tests/"
	@echo "  make sdk-benchmark   - Run Auncient SDK DbC contract latency benchmarks"
	@echo "  make tpu-benchmarks  - Run all TPU fast inference performance benchmarks"
	@echo "  make livrps-benchmark - Run LIVRPS USD composition latency benchmarks"
	@echo "  make curves-benchmark - Run Curves schema latency benchmarks"


test-all: test-dashboard test-container test-git-ci test-unit test-mann test-comp-pass test-mvarsel test-reroute test-delegate-sig test-stagecomp test-ar test-sdfformat test-hydrascene test-renderdelegate test-stagelock test-renderindex test-usdshade test-geomcamera test-geomcurves test-geompoints test-luxlight test-geomxform test-stageroot test-primroot test-stagepop test-attributeroot test-stagecache test-stagemask test-geomsubset test-stagearc test-stageinherits test-stagepayloads test-stagevariants test-stagetraverser test-stagemetadata test-camerafrustum test-stagereferences test-stagespecializes test-stagerelationship test-stagetimecodes test-vactrolpickup test-npnpnpvalve test-curvesbasis test-singularityring test-subdivcurves test-subdivscheme test-curvestension test-curveswidths test-curveswrap test-timesamples test-micropolygonmesh test-shadowmap test-quasirandomsampler test-displacementshader test-texgensample test-depthoffield test-quadtreeslicer test-riinterface test-cinefex-adv test-sss test-envmap test-hairshading test-proximity-occlusion test-krowz-sickness
	@echo "All tests completed successfully."

test-mann:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_mann_controller.c tsfi2-deepseek/src/tsfi_mann_controller.c -o tests/test_mann_controller -lm -lrt
	./tests/test_mann_controller
	@rm -f tests/test_mann_controller

test-comp-pass:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_comp_pass.c tsfi2-deepseek/src/tsfi_comp_pass.c -o tests/test_comp_pass -lm -lrt
	./tests/test_comp_pass
	@rm -f tests/test_comp_pass

test-mvarsel:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_mvarsel.c tsfi2-deepseek/src/tsfi_mvarsel.c -o tests/test_mvarsel -lm -lrt
	./tests/test_mvarsel
	@rm -f tests/test_mvarsel

test-reroute:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_reroute.c tsfi2-deepseek/src/tsfi_reroute.c -o tests/test_reroute -lm -lrt
	./tests/test_reroute
	@rm -f tests/test_reroute

test-delegate-sig:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_delegate_sig.c tsfi2-deepseek/src/tsfi_delegate_sig.c -o tests/test_delegate_sig -lssl -lcrypto -lm -lrt
	./tests/test_delegate_sig
	@rm -f tests/test_delegate_sig

test-stagecomp:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagecomp.c tsfi2-deepseek/src/tsfi_stagecomp.c -o tests/test_stagecomp -lm -lrt
	./tests/test_stagecomp
	@rm -f tests/test_stagecomp

test-ar:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_ar.c tsfi2-deepseek/src/tsfi_ar.c -o tests/test_ar -lm -lrt
	./tests/test_ar
	@rm -f tests/test_ar

test-sdfformat:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_sdfformat.c tsfi2-deepseek/src/tsfi_sdfformat.c -o tests/test_sdfformat -lm -lrt
	./tests/test_sdfformat
	@rm -f tests/test_sdfformat

test-hydrascene:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_hydrascene.c tsfi2-deepseek/src/tsfi_hydrascene.c -o tests/test_hydrascene -lm -lrt
	./tests/test_hydrascene
	@rm -f tests/test_hydrascene

test-renderdelegate:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_renderdelegate.c tsfi2-deepseek/src/tsfi_renderdelegate.c -o tests/test_renderdelegate -lm -lrt
	./tests/test_renderdelegate
	@rm -f tests/test_renderdelegate

test-stagelock:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagelock.c tsfi2-deepseek/src/tsfi_stagelock.c -o tests/test_stagelock -lpthread -lm -lrt
	./tests/test_stagelock
	@rm -f tests/test_stagelock

test-renderindex:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_renderindex.c tsfi2-deepseek/src/tsfi_renderindex.c tsfi2-deepseek/src/tsfi_hydrascene.c tsfi2-deepseek/src/tsfi_renderdelegate.c -o tests/test_renderindex -lm -lrt
	./tests/test_renderindex
	@rm -f tests/test_renderindex

test-usdshade:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_usdshade.c tsfi2-deepseek/src/tsfi_usdshade.c -o tests/test_usdshade -lm -lrt
	./tests/test_usdshade
	@rm -f tests/test_usdshade

test-geomcamera:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_geomcamera.c tsfi2-deepseek/src/tsfi_geomcamera.c -o tests/test_geomcamera -lm -lrt
	./tests/test_geomcamera
	@rm -f tests/test_geomcamera

test-geomcurves:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_geomcurves.c tsfi2-deepseek/src/tsfi_geomcurves.c -o tests/test_geomcurves -lm -lrt
	./tests/test_geomcurves
	@rm -f tests/test_geomcurves

test-geompoints:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_geompoints.c tsfi2-deepseek/src/tsfi_geompoints.c -o tests/test_geompoints -lm -lrt
	./tests/test_geompoints
	@rm -f tests/test_geompoints

test-luxlight:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_luxlight.c tsfi2-deepseek/src/tsfi_luxlight.c -o tests/test_luxlight -lm -lrt
	./tests/test_luxlight
	@rm -f tests/test_luxlight

test-geomxform:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_geomxform.c tsfi2-deepseek/src/tsfi_geomxform.c -o tests/test_geomxform -lm -lrt
	./tests/test_geomxform
	@rm -f tests/test_geomxform

test-stageroot:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stageroot.c tsfi2-deepseek/src/tsfi_stageroot.c -o tests/test_stageroot -lm -lrt
	./tests/test_stageroot
	@rm -f tests/test_stageroot

test-primroot:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_primroot.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_primroot -lm -lrt
	./tests/test_primroot
	@rm -f tests/test_primroot

test-stagepop:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagepop.c tsfi2-deepseek/src/tsfi_stagepop.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_stagepop -lm -lrt
	./tests/test_stagepop
	@rm -f tests/test_stagepop

test-attributeroot:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_attributeroot.c tsfi2-deepseek/src/tsfi_attributeroot.c -o tests/test_attributeroot -lm -lrt
	./tests/test_attributeroot
	@rm -f tests/test_attributeroot

test-stagecache:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagecache.c tsfi2-deepseek/src/tsfi_stagecache.c tsfi2-deepseek/src/tsfi_stageroot.c -o tests/test_stagecache -lm -lrt
	./tests/test_stagecache
	@rm -f tests/test_stagecache

test-stagemask:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagemask.c tsfi2-deepseek/src/tsfi_stagemask.c -o tests/test_stagemask -lm -lrt
	./tests/test_stagemask
	@rm -f tests/test_stagemask

test-geomsubset:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_geomsubset.c tsfi2-deepseek/src/tsfi_geomsubset.c -o tests/test_geomsubset -lm -lrt
	./tests/test_geomsubset
	@rm -f tests/test_geomsubset

test-stagearc:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagearc.c tsfi2-deepseek/src/tsfi_stagearc.c -o tests/test_stagearc -lm -lrt
	./tests/test_stagearc
	@rm -f tests/test_stagearc

test-stageinherits:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stageinherits.c tsfi2-deepseek/src/tsfi_stageinherits.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_stageinherits -lm -lrt
	./tests/test_stageinherits
	@rm -f tests/test_stageinherits

test-stagepayloads:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagepayloads.c tsfi2-deepseek/src/tsfi_stagepayloads.c -o tests/test_stagepayloads -lm -lrt
	./tests/test_stagepayloads
	@rm -f tests/test_stagepayloads

test-stagevariants:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagevariants.c tsfi2-deepseek/src/tsfi_stagevariants.c -o tests/test_stagevariants -lm -lrt
	./tests/test_stagevariants
	@rm -f tests/test_stagevariants

test-stagetraverser:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagetraverser.c tsfi2-deepseek/src/tsfi_stagetraverser.c tsfi2-deepseek/src/tsfi_primroot.c -o tests/test_stagetraverser -lm -lrt
	./tests/test_stagetraverser
	@rm -f tests/test_stagetraverser

test-stagemetadata:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagemetadata.c tsfi2-deepseek/src/tsfi_stagemetadata.c -o tests/test_stagemetadata -lm -lrt
	./tests/test_stagemetadata
	@rm -f tests/test_stagemetadata

test-camerafrustum:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_camerafrustum.c tsfi2-deepseek/src/tsfi_camerafrustum.c -o tests/test_camerafrustum -lm -lrt
	./tests/test_camerafrustum
	@rm -f tests/test_camerafrustum

test-stagereferences:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagereferences.c tsfi2-deepseek/src/tsfi_stagereferences.c -o tests/test_stagereferences -lm -lrt
	./tests/test_stagereferences
	@rm -f tests/test_stagereferences

test-stagespecializes:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagespecializes.c tsfi2-deepseek/src/tsfi_stagespecializes.c -o tests/test_stagespecializes -lm -lrt
	./tests/test_stagespecializes
	@rm -f tests/test_stagespecializes

test-stagerelationship:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagerelationship.c tsfi2-deepseek/src/tsfi_stagerelationship.c -o tests/test_stagerelationship -lm -lrt
	./tests/test_stagerelationship
	@rm -f tests/test_stagerelationship

test-stagetimecodes:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_stagetimecodes.c tsfi2-deepseek/src/tsfi_stagetimecodes.c -o tests/test_stagetimecodes -lm -lrt
	./tests/test_stagetimecodes
	@rm -f tests/test_stagetimecodes

test-vactrolpickup:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_vactrolpickup.c tsfi2-deepseek/src/tsfi_vactrolpickup.c -o tests/test_vactrolpickup -lm -lrt
	./tests/test_vactrolpickup
	@rm -f tests/test_vactrolpickup

test-npnpnpvalve:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_npnpnpvalve.c tsfi2-deepseek/src/tsfi_npnpnpvalve.c -o tests/test_npnpnpvalve -lm -lrt
	./tests/test_npnpnpvalve
	@rm -f tests/test_npnpnpvalve

test-curvesbasis:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_curvesbasis.c tsfi2-deepseek/src/tsfi_curvesbasis.c -o tests/test_curvesbasis -lm -lrt
	./tests/test_curvesbasis
	@rm -f tests/test_curvesbasis

test-singularityring:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_singularityring.c tsfi2-deepseek/src/tsfi_singularityring.c -o tests/test_singularityring -lm -lrt
	./tests/test_singularityring
	@rm -f tests/test_singularityring

test-subdivcurves:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_subdivcurves.c tsfi2-deepseek/src/tsfi_subdivcurves.c -o tests/test_subdivcurves -lm -lrt
	./tests/test_subdivcurves
	@rm -f tests/test_subdivcurves

test-subdivscheme:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_subdivscheme.c tsfi2-deepseek/src/tsfi_subdivscheme.c -o tests/test_subdivscheme -lm -lrt
	./tests/test_subdivscheme
	@rm -f tests/test_subdivscheme

test-curvestension:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_curvestension.c tsfi2-deepseek/src/tsfi_curvestension.c -o tests/test_curvestension -lm -lrt
	./tests/test_curvestension
	@rm -f tests/test_curvestension

test-curveswidths:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_curveswidths.c tsfi2-deepseek/src/tsfi_curveswidths.c -o tests/test_curveswidths -lm -lrt
	./tests/test_curveswidths
	@rm -f tests/test_curveswidths

test-curveswrap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_curveswrap.c tsfi2-deepseek/src/tsfi_curveswrap.c -o tests/test_curveswrap -lm -lrt
	./tests/test_curveswrap
	@rm -f tests/test_curveswrap

test-timesamples:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_timesamples.c tsfi2-deepseek/src/tsfi_timesamples.c -o tests/test_timesamples -lm -lrt
	./tests/test_timesamples
	@rm -f tests/test_timesamples

test-micropolygonmesh:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_micropolygonmesh.c tsfi2-deepseek/src/tsfi_micropolygonmesh.c -o tests/test_micropolygonmesh -lm -lrt
	./tests/test_micropolygonmesh
	@rm -f tests/test_micropolygonmesh

test-shadowmap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_shadowmap.c tsfi2-deepseek/src/tsfi_shadowmap.c -o tests/test_shadowmap -lm -lrt
	./tests/test_shadowmap
	@rm -f tests/test_shadowmap

test-quasirandomsampler:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_quasirandomsampler.c tsfi2-deepseek/src/tsfi_quasirandomsampler.c -o tests/test_quasirandomsampler -lm -lrt
	./tests/test_quasirandomsampler
	@rm -f tests/test_quasirandomsampler

test-displacementshader:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_displacementshader.c tsfi2-deepseek/src/tsfi_displacementshader.c -o tests/test_displacementshader -lm -lrt
	./tests/test_displacementshader
	@rm -f tests/test_displacementshader

test-texgensample:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_texgensample.c tsfi2-deepseek/src/tsfi_texgen.c -o tests/test_texgensample -lm -lrt
	./tests/test_texgensample
	@rm -f tests/test_texgensample

test-depthoffield:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_depthoffield.c tsfi2-deepseek/src/tsfi_depthoffield.c -o tests/test_depthoffield -lm -lrt
	./tests/test_depthoffield
	@rm -f tests/test_depthoffield

test-quadtreeslicer:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_quadtreeslicer.c tsfi2-deepseek/src/tsfi_quadtreeslicer.c -o tests/test_quadtreeslicer -lm -lrt
	./tests/test_quadtreeslicer
	@rm -f tests/test_quadtreeslicer

test-riinterface:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_riinterface.c tsfi2-deepseek/src/tsfi_riinterface.c -o tests/test_riinterface -lm -lrt
	./tests/test_riinterface
	@rm -f tests/test_riinterface

test-cinefex-adv:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_cinefex_adv.c tsfi2-deepseek/src/tsfi_fbm_landscape.c tsfi2-deepseek/src/tsfi_volumetric.c tsfi2-deepseek/src/tsfi_motionblur.c tsfi2-deepseek/src/tsfi_shadowmap.c -o tests/test_cinefex_adv -lm -lrt
	./tests/test_cinefex_adv
	@rm -f tests/test_cinefex_adv

test-sss:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_sss.c tsfi2-deepseek/src/tsfi_sss.c -o tests/test_sss -lm -lrt
	./tests/test_sss
	@rm -f tests/test_sss

test-envmap:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_envmap.c tsfi2-deepseek/src/tsfi_envmap.c -o tests/test_envmap -lm -lrt
	./tests/test_envmap
	@rm -f tests/test_envmap

test-hairshading:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_hairshading.c tsfi2-deepseek/src/tsfi_hairshading.c -o tests/test_hairshading -lm -lrt
	./tests/test_hairshading
	@rm -f tests/test_hairshading

test-proximity-occlusion:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_proximity_occlusion.c tsfi2-deepseek/src/tsfi_proximity_occlusion.c -o tests/test_proximity_occlusion -lm -lrt
	./tests/test_proximity_occlusion
	@rm -f tests/test_proximity_occlusion

test-krowz-sickness:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_krowz_sickness.c tsfi2-deepseek/src/tsfi_krowz_sickness.c -o tests/test_krowz_sickness -lm -lrt
	./tests/test_krowz_sickness
	@rm -f tests/test_krowz_sickness

sdk-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc src/auncient_sdk.c tests/test_auncient_sdk_benchmarks.c -o tests/test_auncient_sdk_benchmarks -lm -lrt
	./tests/test_auncient_sdk_benchmarks

livrps-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_livrps_benchmarks.c -o tests/test_auncient_livrps_benchmarks -lm -lrt
	./tests/test_auncient_livrps_benchmarks

curves-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_auncient_curves_benchmarks.c tsfi2-deepseek/src/tsfi_curveswidths.c tsfi2-deepseek/src/tsfi_curveswrap.c -o tests/test_auncient_curves_benchmarks -lm -lrt
	./tests/test_auncient_curves_benchmarks
	@rm -f tests/test_auncient_curves_benchmarks

tpu-benchmarks: sdk-build
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_fast_inference_benchmark.c src/auncient_sdk.o -o tests/test_auncient_fast_inference_benchmark -lm -lrt
	./tests/test_auncient_fast_inference_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_neural_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_neural_benchmark -lm -lrt
	./tests/test_auncient_tpu_neural_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_recurrent_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_recurrent_benchmark -lm -lrt
	./tests/test_auncient_tpu_recurrent_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_polyphonic_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_polyphonic_benchmark -lm -lrt
	./tests/test_auncient_tpu_polyphonic_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_security_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_security_benchmark -lm -lrt
	./tests/test_auncient_tpu_security_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_gating_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_gating_benchmark -lm -lrt
	./tests/test_auncient_tpu_gating_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_mesh_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_mesh_benchmark -lm -lrt
	./tests/test_auncient_tpu_mesh_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_decay_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_decay_benchmark -lm -lrt
	./tests/test_auncient_tpu_decay_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_spelling_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_spelling_benchmark -lm -lrt
	./tests/test_auncient_tpu_spelling_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_tpu_bond_benchmark.c src/auncient_sdk.o -o tests/test_auncient_tpu_bond_benchmark -lm -lrt
	./tests/test_auncient_tpu_bond_benchmark
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_swarm_consensus.c src/auncient_sdk.o -o tests/test_auncient_swarm_consensus -lm -lrt
	./tests/test_auncient_swarm_consensus
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc tests/test_auncient_swarm_metaprogramming.c src/auncient_sdk.o -o tests/test_auncient_swarm_metaprogramming -lm -lrt
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
	python3 -m unittest tests/e2e/test_nonukes_dashboard.py
	python3 -m unittest tests/e2e/test_nonukes_dashboard_spa.py
	python3 -m unittest tests/e2e/test_nonukes_adversarial.py

test-container:
	python3 -m unittest tests/e2e/test_dashboard_container.py

test-git-ci:
	python3 tests/e2e/run_e2e_tests.py

test-unit:
	python3 -m unittest discover -s tests -p "test_*.py"

.PHONY: sdk-build sdk-minify sdk-package

sdk-build:
	mkdir -p dist
	gcc -Wall -Wextra -Werror -std=c11 -O3 -fPIC -Iinc -c src/auncient_sdk.c -o src/auncient_sdk.o
	ar rcs libauncient_sdk.a src/auncient_sdk.o

sdk-minify: sdk-build
	strip --strip-unneeded src/auncient_sdk.o
	ar rcs libauncient_sdk.a src/auncient_sdk.o
	strip -S libauncient_sdk.a

sdk-package: sdk-minify
	tar -czf dist/auncient_sdk.tar.gz inc/auncient_sdk.h libauncient_sdk.a
