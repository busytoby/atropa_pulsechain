.PHONY: help test-all test-dashboard test-container test-git-ci test-unit

help:
	@echo "Available test targets:"
	@echo "  make test-all        - Run all E2E and unit tests in the project"
	@echo "  make test-dashboard  - Run NoNukes Dashboard E2E tests (including SPA)"
	@echo "  make test-container  - Run Teddy Bear Diagnostics container E2E tests"
	@echo "  make test-git-ci     - Run Git post-commit hook pipeline E2E tests"
	@echo "  make test-unit       - Run Python unit tests under tests/"
	@echo "  make sdk-benchmark   - Run Auncient SDK DbC contract latency benchmarks"

test-all: test-dashboard test-container test-git-ci test-unit
	@echo "All tests completed successfully."

sdk-benchmark:
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc src/auncient_sdk.c tests/test_auncient_sdk_benchmarks.c -o tests/test_auncient_sdk_benchmarks -lm -lrt
	./tests/test_auncient_sdk_benchmarks

siggraph-projector:
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_projector.c -o tests/test_auncient_siggraph_projector -lm -lrt
	./tests/test_auncient_siggraph_projector
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_renderer.c -o tests/test_auncient_siggraph_renderer -lm -lrt
	./tests/test_auncient_siggraph_renderer
	gcc -Wall -Wextra -Werror -std=c11 -O3 tests/test_auncient_siggraph_discharge.c -o tests/test_auncient_siggraph_discharge -lm -lrt
	./tests/test_auncient_siggraph_discharge





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
	gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -c src/auncient_sdk.c -o src/auncient_sdk.o
	ar rcs libauncient_sdk.a src/auncient_sdk.o

sdk-minify: sdk-build
	strip --strip-unneeded src/auncient_sdk.o
	ar rcs libauncient_sdk.a src/auncient_sdk.o
	strip -S libauncient_sdk.a

sdk-package: sdk-minify
	tar -czf dist/auncient_sdk.tar.gz inc/auncient_sdk.h libauncient_sdk.a
