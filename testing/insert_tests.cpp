#include <gtest/gtest.h>
#include "../src/swiss_map.h"
#include <chrono>
#include <iostream>

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