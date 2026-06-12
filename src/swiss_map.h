#ifndef SWISS_MAP_H
#define SWISS_MAP_H

#include <vector>
#include <algorithm>
#include <functional>
#include <immintrin.h>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>

namespace mischa {

// Predefine naming conventions

using ctrl_t = int8_t;
using h2_t = int8_t;

template <
    typename K,
    typename V,
    typename Hash = std::hash<K>
>
class swiss_map {
private: // Member variables
    size_t size_{};
    size_t bucketCount_{};
    float growth_factor{2.0};

    std::vector<ctrl_t> ctrl_{};
    std::vector<std::pair<K, V>> table_{};

    /*
    Swisse tables use 7-bits of the hash hash as a bitset for comparison within buckets.
    */

private: // Struct definitions

    enum Ctrl : ctrl_t {
        kEmpty = -128,      // 0b10000000 -- An empty table
        kDeleted = -2,      // 0b11111110 -- Missing entry
        kSentinel = -1,     // 0b11111111 -- Stop scanning table
        // kFull =          // 0b0xxxxxxx
    };


private: // Internal helper functions

    // Cache-related helpers
    size_t H1(size_t hash) const;    // Hash in slot of table
    ctrl_t H2(size_t hash) const; // Hash within the control (bitmask)

    void expand();

    uint16_t match(size_t index, h2_t hash) const;
    uint16_t match_empty(size_t index) const;
    uint16_t match_free_slot(size_t index) const;

    void set_table_size(size_t n);

    void delete_at_index(size_t index);
    

public:

    // Default constructors and deconstructors
    swiss_map();
    swiss_map(size_t n);
    swiss_map(const swiss_map&) = default;
    swiss_map(swiss_map&&) = default;
    swiss_map(std::initializer_list<std::pair<K, V>> list);

    ~swiss_map() = default;

    // Standard function
    V& at(const K& key);

    V& insert(const K& key, const V& value);

    V& operator[](const K& key);

    void erase(const K& key);

};

}

#include "swiss_map.tpp"


#endif