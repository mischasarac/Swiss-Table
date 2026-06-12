CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic

GTEST_LIBS    := -lgtest -lgtest_main -pthread
GBENCH_LIBS   := -lbenchmark -lbenchmark_main -pthread
ABSL_LIBS     := -labsl_raw_hash_set -labsl_hash -labsl_city -labsl_low_level_hash \
                  -labsl_raw_logging_internal -labsl_throw_delegate \
                  -labsl_bad_optional_access -labsl_bad_variant_access \
                  -labsl_hashtablez_sampler -labsl_exponential_biased \
                  -labsl_synchronization -labsl_stacktrace -labsl_symbolize \
                  -labsl_time -labsl_time_zone -labsl_int128 -labsl_base
BENCHMARK_LIBS := $(GBENCH_LIBS) $(ABSL_LIBS)

.PHONY: test test_insert compilation_test benchmark_random check_deps \
        check_gtest check_gbenchmark check_absl check_boost clean

# ---------------------------------------------------------------------
# Dependency checks
# ---------------------------------------------------------------------

check_gtest:
	@echo '#include <gtest/gtest.h>' | $(CXX) $(CXXFLAGS) -x c++ -fsyntax-only - \
		2>/dev/null && echo "gtest/gtest.h" || \
		{ echo "ERROR: gtest headers not found. Install with: sudo apt-get install libgtest-dev"; exit 1; }
	@echo 'int main(){return 0;}' | $(CXX) $(CXXFLAGS) -x c++ - $(GTEST_LIBS) -o /tmp/.gtest_check \
		2>/dev/null && rm -f /tmp/.gtest_check || \
		{ echo "ERROR: gtest libraries not found. Install with: sudo apt-get install libgtest-dev"; exit 1; }

check_gbenchmark:
	@echo '#include <benchmark/benchmark.h>' | $(CXX) $(CXXFLAGS) -x c++ -fsyntax-only - \
		2>/dev/null && echo "benchmark/benchmark.h" || \
		{ echo "ERROR: google benchmark headers not found. Install with: sudo apt-get install libbenchmark-dev"; exit 1; }
	@echo 'int main(){return 0;}' | $(CXX) $(CXXFLAGS) -x c++ - $(GBENCH_LIBS) -o /tmp/.gbench_check \
		2>/dev/null && rm -f /tmp/.gbench_check || \
		{ echo "ERROR: google benchmark libraries not found. Install with: sudo apt-get install libbenchmark-dev"; exit 1; }

check_absl:
	@echo '#include "absl/container/flat_hash_map.h"' | $(CXX) $(CXXFLAGS) -x c++ -fsyntax-only - \
		2>/dev/null && echo "absl/container/flat_hash_map.h" || \
		{ echo "ERROR: abseil headers not found. Install abseil-cpp (https://abseil.io/docs/cpp/quickstart) or via vcpkg/conan."; exit 1; }
	@echo 'int main(){return 0;}' | $(CXX) $(CXXFLAGS) -x c++ - $(ABSL_LIBS) -o /tmp/.absl_check \
		2>/dev/null && rm -f /tmp/.absl_check || \
		{ echo "ERROR: abseil libraries not found/linkable. Ensure absl libs are installed and on LD path."; exit 1; }

check_boost:
	@echo '#include <boost/unordered_map.hpp>' | $(CXX) $(CXXFLAGS) -x c++ -fsyntax-only - \
		2>/dev/null && echo "boost/unordered_map.hpp" || \
		{ echo "ERROR: boost headers not found. Install with: sudo apt-get install libboost-all-dev"; exit 1; }

check_deps: check_gtest check_gbenchmark check_absl check_boost
	@echo "All dependencies found."

# ---------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------

test: check_gtest
	$(CXX) $(CXXFLAGS) \
		./testing/swiss_table_tests.cpp \
		$(GTEST_LIBS) \
		-o tests
	./tests
	rm -f tests

test_insert: check_gtest
	$(CXX) $(CXXFLAGS) \
		./testing/insert_tests.cpp \
		$(GTEST_LIBS) \
		-o tests
	./tests
	rm -f tests

compilation_test: check_deps
	$(CXX) $(CXXFLAGS) \
		-fsyntax-only \
		./testing/compilation_tests.cpp

# ---------------------------------------------------------------------
# Benchmarks
# ---------------------------------------------------------------------

benchmark_random: check_gbenchmark check_absl check_boost
	$(CXX) $(CXXFLAGS) \
		./benchmarks/random_values.cpp \
		$(BENCHMARK_LIBS) \
		-o benchmark_random
	./benchmark_random
	rm -f benchmark_random

# ---------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------

clean:
	rm -f tests benchmark_random