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

TEST(swiss_map, simple_at_function) {
    mischa::swiss_map<int, int> sm;
    ASSERT_THROW(sm.at(0), std::out_of_range);
}

TEST(swiss_map, simple_insert_function) {
    mischa::swiss_map<int, int> sm;
    
    sm.insert(1, 2);
    sm.insert(3, 1);

    ASSERT_EQ(sm.at(1), 2);
    ASSERT_EQ(sm.at(3), 1);
    
    ASSERT_THROW(sm.at(0), std::out_of_range);
    
    sm.insert(1, 3);
    ASSERT_EQ(sm.at(1), 3);
}

TEST(swiss_map, operator_square_brackets) {
    mischa::swiss_map<int, int> sm;

    int size = 1'000'000;
    std::vector<int> values(size);
    for(size_t i = 0; i < size; i++) {
        values[i] = rand();
    }
    
    
    const auto start_swiss = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++) {
        sm[values[i]] = (values[i]);
    }
    
    
    for(int i = 0; i < size; i++) 
        ASSERT_EQ(sm[values[i]], (values[i]));
    const auto end_swiss = std::chrono::steady_clock::now();
    const std::chrono::duration<double> time_elapsed_swiss = end_swiss - start_swiss;


    std::unordered_map<int, int> um;
    const auto start_std = std::chrono::steady_clock::now();

    for(int i = 0; i < size; i++)
        um[values[i]] = (values[i]);

    for(int i = 0; i < size; i++) 
        ASSERT_EQ(um[values[i]], (values[i]));

    const auto end_std = std::chrono::steady_clock::now();
    const std::chrono::duration<double> time_elapsed_std = end_std - start_std;

    // Make sure my map is better.
    ASSERT_GT(time_elapsed_std, time_elapsed_swiss);
    
}

TEST(swiss_map, test_erase) {
    mischa::swiss_map<int, int> sm;

    for(int i = 0; i < 200; i++)
        sm.insert(i, i);
    
    ASSERT_EQ(sm[10], 10);

    for(int i = 0; i < 100; i++)
        sm.erase(i);
    
    ASSERT_THROW(sm.at(10), std::out_of_range);
    ASSERT_EQ(sm.at(100), 100);
    
}