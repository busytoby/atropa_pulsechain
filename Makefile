.PHONY: help test-all test-dashboard test-container test-git-ci test-unit tpu-benchmarks livrps-benchmark test-mann test-comp-pass

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


test-all: test-dashboard test-container test-git-ci test-unit test-mann test-comp-pass
	@echo "All tests completed successfully."

test-mann:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_mann_controller.c tsfi2-deepseek/src/tsfi_mann_controller.c -o tests/test_mann_controller -lm -lrt
	./tests/test_mann_controller
	@rm -f tests/test_mann_controller

test-comp-pass:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Itsfi2-deepseek/inc tests/test_comp_pass.c tsfi2-deepseek/src/tsfi_comp_pass.c -o tests/test_comp_pass -lm -lrt
	./tests/test_comp_pass
	@rm -f tests/test_comp_pass

sdk-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc src/auncient_sdk.c tests/test_auncient_sdk_benchmarks.c -o tests/test_auncient_sdk_benchmarks -lm -lrt
	./tests/test_auncient_sdk_benchmarks

livrps-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_livrps_benchmarks.c -o tests/test_auncient_livrps_benchmarks -lm -lrt
	./tests/test_auncient_livrps_benchmarks


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
