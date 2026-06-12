#include <gtest/gtest.h>
#include "../src/swiss_map.h"
#include <chrono>
#include <iostream>
#include <random>

#include <unordered_map>

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
    
    
    const auto start_cpy = std::chrono::steady_clock::now();
    for(int i = 0; i < size; i++) {
        sm[values[i]] = (values[i]);
    }
    
    
    for(int i = 0; i < size; i++) 
        ASSERT_EQ(sm[values[i]], (values[i]));
    const auto end_cpy = std::chrono::steady_clock::now();
    const std::chrono::duration<double> time_elapsed_cpy = end_cpy - start_cpy;


    std::unordered_map<int, int> um;
    const auto start_move = std::chrono::steady_clock::now();

    for(int i = 0; i < size; i++)
        um[values[i]] = (values[i]);

    for(int i = 0; i < size; i++) 
        ASSERT_EQ(um[values[i]], (values[i]));

    const auto end_move = std::chrono::steady_clock::now();
    const std::chrono::duration<double> time_elapsed_move = end_move - start_move;

    std::cout << "My map : " << time_elapsed_cpy << "\nunordered_map : " << time_elapsed_move << "\n";
    
}