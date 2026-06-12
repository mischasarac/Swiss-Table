#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <type_traits>
#include <limits>
#include <cmath>
#include <algorithm>
#include <stdexcept>

#include <unordered_map>
#include <boost/unordered_map.hpp>
#include "../src/swiss_map.h"
#include "absl/container/flat_hash_map.h"

// Modifyable parameters:
constexpr size_t num_elements = 1'000'000;
constexpr size_t num_trials   = 20;
using test_type  = size_t;
using entry_type = size_t;
constexpr size_t str_len = 100;
const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

constexpr bool test_type_is_string = std::is_same_v<test_type, std::string>;
constexpr size_t int_limit =
    test_type_is_string ? 0 : std::numeric_limits<test_type>::max();

// Prevents the optimizer from eliding lookups whose results are unused.
volatile unsigned long long g_sink = 0;

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

// ---------------------------------------------------------------------
// Stats helpers
// ---------------------------------------------------------------------

struct Stats {
    double mean;
    double stdev;
    double min;
    double max;
};

Stats compute_stats(const std::vector<double>& values) {
    double sum = 0.0;
    for (double v : values) sum += v;
    double mean = sum / values.size();

    double sq_sum = 0.0;
    for (double v : values) sq_sum += (v - mean) * (v - mean);
    double stdev = std::sqrt(sq_sum / values.size());

    auto min_it = std::min_element(values.begin(), values.end());
    auto max_it = std::max_element(values.begin(), values.end());

    return {mean, stdev, *min_it, *max_it};
}

void print_stats(const std::string& label, const Stats& s) {
    std::cout << "  " << label << ":\n"
              << "    mean  = " << s.mean  << " ms\n"
              << "    stdev = " << s.stdev << " ms\n"
              << "    min   = " << s.min   << " ms\n"
              << "    max   = " << s.max   << " ms\n";
}

// ---------------------------------------------------------------------
// Benchmark operations
// ---------------------------------------------------------------------

template <typename T>
double bench_insert(T& map, const std::vector<test_type>& values) {
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < values.size(); i++) {
        map[values[i]] = static_cast<entry_type>(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

template <typename T>
double bench_lookup(T& map, const std::vector<test_type>& values) {
    size_t found_count = 0;
    entry_type sink{};
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& v : values) {
        try {
            sink += map.at(v);
            found_count++;
        } catch (const std::out_of_range&) {
            // not found
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    g_sink += sink;
    g_sink += found_count;
    return std::chrono::duration<double, std::milli>(end - start).count();
}

template <typename T>
double bench_erase(T& map, const std::vector<test_type>& values) {
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& v : values) {
        map.erase(v);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// ---------------------------------------------------------------------
// Per-map benchmark runner
// ---------------------------------------------------------------------

template <typename T>
void run_map_benchmark(const std::string& name,
                        const std::vector<std::vector<test_type>>& trial_values) {
    std::cout << "=== " << name << " ===\n";

    std::vector<double> insert_times;
    std::vector<double> lookup_times;
    std::vector<double> erase_times;

    insert_times.reserve(num_trials);
    lookup_times.reserve(num_trials);
    erase_times.reserve(num_trials);

    for (size_t trial = 0; trial < num_trials; trial++) {
        T map;

        const auto& values = trial_values[trial];

        insert_times.push_back(bench_insert(map, values));
        lookup_times.push_back(bench_lookup(map, values));
        erase_times.push_back(bench_erase(map, values));
    }

    print_stats("Insert", compute_stats(insert_times));
    print_stats("Lookup", compute_stats(lookup_times));
    print_stats("Erase",  compute_stats(erase_times));
    std::cout << "\n";
}

// ---------------------------------------------------------------------
// Main benchmark driver
// ---------------------------------------------------------------------

void run_benchmark() {
    std::cout << "Benchmark: " << num_elements << " elements, "
              << num_trials << " trials each\n\n";

    // Pre-generate all random datasets up front so generation cost
    // doesn't pollute the timed sections, and so all maps see the
    // same data on the same trial number.
    std::vector<std::vector<test_type>> trial_values;
    trial_values.reserve(num_trials);
    for (size_t t = 0; t < num_trials; t++) {
        trial_values.push_back(generate_random(num_elements));
    }

    run_map_benchmark<std::unordered_map<test_type, entry_type>>(
        "std::unordered_map", trial_values);

    run_map_benchmark<boost::unordered_map<test_type, entry_type>>(
        "boost::unordered_map", trial_values);

    run_map_benchmark<absl::flat_hash_map<test_type, entry_type>>(
        "absl::flat_hash_map", trial_values);

    run_map_benchmark<mischa::swiss_map<test_type, entry_type>>(
        "mischa::swiss_map", trial_values);
}

int main() {
    run_benchmark();
    return 0;
}