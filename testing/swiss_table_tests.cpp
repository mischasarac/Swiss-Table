#include <gtest/gtest.h>
#include "../src/swiss_map.h"
#include <chrono>
#include <iostream>

TEST(swiss_map, compile_test) {
    mischa::swiss_map<int, int> sm;
}

TEST(swiss_map, constructor_test) {
    mischa::swiss_map<int, int> sm(1'000'000);
    // Validate move constructor is faster than copy constructor

    const auto start_cpy = std::chrono::steady_clock::now();
    mischa::swiss_map<int, int> cpy_sm(sm);
    const auto end_cpy = std::chrono::steady_clock::now();
    const std::chrono::duration<double> time_elapsed_cpy = end_cpy - start_cpy;

    const auto start_move = std::chrono::steady_clock::now();
    mischa::swiss_map<int, int> move_sm(std::move(sm));
    const auto end_move = std::chrono::steady_clock::now();
    const std::chrono::duration<double> time_elapsed_move = end_move - start_move;

    // Simple comparison of move being at least 1000x faster
    ASSERT_GT(time_elapsed_cpy / 1000, time_elapsed_move);
}