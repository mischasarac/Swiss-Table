#include <benchmark/benchmark.h>

#include <random>
#include <string>
#include <vector>
#include <type_traits>
#include <limits>
#include <stdexcept>

#include <unordered_map>
#include <boost/unordered_map.hpp>
#include "../src/swiss_map.h"
#include "absl/container/flat_hash_map.h"

// Modifyable parameters:
constexpr size_t num_elements = 100'000;
using test_type  = size_t;
using entry_type = size_t;
constexpr size_t str_len = 100;
const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

constexpr bool test_type_is_string = std::is_same_v<test_type, std::string>;

// ---------------------------------------------------------------------
// Random value generation
// ---------------------------------------------------------------------

std::vector<test_type> generate_random_strings(size_t n) {
    std::vector<test_type> result;
    result.reserve(n);

    if constexpr (test_type_is_string) {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<std::size_t> distribution(0, characters.size() - 1);

        for (size_t i = 0; i < n; i++) {
            std::string random_string;
            random_string.reserve(str_len);
            for (size_t j = 0; j < str_len; j++) {
                random_string += characters[distribution(generator)];
            }
            result.emplace_back(std::move(random_string));
        }
    }

    return result;
}

std::vector<test_type> generate_random(size_t n) {
    if constexpr (test_type_is_string) {
        return generate_random_strings(n);
    } else {
        std::vector<test_type> result;
        result.reserve(n);

        std::random_device rd;
        std::mt19937_64 generator(rd());
        std::uniform_int_distribution<unsigned long long> distribution(
            0, std::numeric_limits<unsigned long long>::max());

        for (size_t i = 0; i < n; i++) {
            result.emplace_back(static_cast<test_type>(distribution(generator)));
        }

        return result;
    }
}

// Cached so every benchmark (and every map type) operates on the same keys.
const std::vector<test_type>& random_keys() {
    static const std::vector<test_type> keys = generate_random(num_elements);
    return keys;
}

// ---------------------------------------------------------------------
// Benchmark fixtures
// ---------------------------------------------------------------------

template <typename T>
static void BM_Insert(benchmark::State& state) {
    const auto& keys = random_keys();

    for (auto _ : state) {
        T map;
        for (size_t i = 0; i < keys.size(); i++) {
            map[keys[i]] = static_cast<entry_type>(i);
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * keys.size());
}

template <typename T>
static void BM_Lookup(benchmark::State& state) {
    const auto& keys = random_keys();

    T map;
    for (size_t i = 0; i < keys.size(); i++) {
        map[keys[i]] = static_cast<entry_type>(i);
    }

    for (auto _ : state) {
        for (size_t i = 0; i < keys.size(); i++) {
            try {
                benchmark::DoNotOptimize(map.at(keys[i]));
            } catch (const std::out_of_range&) {
                // not found
            }
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * keys.size());
}

template <typename T>
static void BM_Erase(benchmark::State& state) {
    const auto& keys = random_keys();

    for (auto _ : state) {
        T map;
        for (size_t i = 0; i < keys.size(); i++) {
            map[keys[i]] = static_cast<entry_type>(i);
        }

        for (size_t i = 0; i < keys.size(); i++) {
            map.erase(keys[i]);
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * keys.size());
}

// ---------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------

BENCHMARK_TEMPLATE(BM_Insert, std::unordered_map<test_type, entry_type>)
    ->Name("Insert/std::unordered_map");
BENCHMARK_TEMPLATE(BM_Insert, boost::unordered_map<test_type, entry_type>)
    ->Name("Insert/boost::unordered_map");
BENCHMARK_TEMPLATE(BM_Insert, absl::flat_hash_map<test_type, entry_type>)
    ->Name("Insert/absl::flat_hash_map");
BENCHMARK_TEMPLATE(BM_Insert, mischa::swiss_map<test_type, entry_type>)
    ->Name("Insert/mischa::swiss_map");

BENCHMARK_TEMPLATE(BM_Lookup, std::unordered_map<test_type, entry_type>)
    ->Name("Lookup/std::unordered_map");
BENCHMARK_TEMPLATE(BM_Lookup, boost::unordered_map<test_type, entry_type>)
    ->Name("Lookup/boost::unordered_map");
BENCHMARK_TEMPLATE(BM_Lookup, absl::flat_hash_map<test_type, entry_type>)
    ->Name("Lookup/absl::flat_hash_map");
BENCHMARK_TEMPLATE(BM_Lookup, mischa::swiss_map<test_type, entry_type>)
    ->Name("Lookup/mischa::swiss_map");

BENCHMARK_TEMPLATE(BM_Erase, std::unordered_map<test_type, entry_type>)
    ->Name("Erase/std::unordered_map");
BENCHMARK_TEMPLATE(BM_Erase, boost::unordered_map<test_type, entry_type>)
    ->Name("Erase/boost::unordered_map");
BENCHMARK_TEMPLATE(BM_Erase, absl::flat_hash_map<test_type, entry_type>)
    ->Name("Erase/absl::flat_hash_map");
BENCHMARK_TEMPLATE(BM_Erase, mischa::swiss_map<test_type, entry_type>)
    ->Name("Erase/mischa::swiss_map");

BENCHMARK_MAIN();