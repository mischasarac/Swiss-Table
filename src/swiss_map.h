#ifndef SWISS_MAP_H
#define SWISS_MAP_H

#include <vector>
#include <algorithm>
#include <functional>
#include <immintrin.h>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>
#include <type_traits>
#include <string>
#include <concepts>

namespace mischa {

// Predefine naming conventions

using ctrl_t = int8_t;
using h2_t = int8_t;

template<typename K, typename Hash>
concept Hashable =
    requires(Hash h, K k) {
        { h(k) } -> std::convertible_to<std::size_t>;
    };

template <
    typename K,
    typename V,
    typename Hash = std::hash<K>
>
requires Hashable<K, Hash>
class swiss_map {
    private: // Member variables
    size_t size_{};
    size_t bucket_count_{};
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


public:
/*
==================== Iterators ====================
*/

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;

        using value_type = std::pair<K, V>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        // Functions
        reference operator*() const;
        pointer operator->() const;

        Iterator& operator++();
        Iterator operator++(int);

        bool operator==(const Iterator&) const = default;
        bool operator!=(const Iterator&) const = default;

        // Constructor
        Iterator(swiss_map* map, std::size_t index)
        : map_(map), index_(index) {}

    private:
        swiss_map* map_;
        size_t index_;
        friend class swiss_map;

    };

    Iterator begin();
    Iterator end();

    Iterator find(const K& key);



private: // Internal helper functions

    // Cache-related helpers
    size_t H1(size_t hash) const;    // Hash in slot of table
    ctrl_t H2(size_t hash) const; // Hash within the control (bitmask)

    void expand();

    bool occupied(size_t index) const;

    uint16_t match(size_t index, h2_t hash) const;
    uint16_t match_empty(size_t index) const;
    uint16_t match_free_slot(size_t index) const;

    void set_table_size(size_t n);

    Iterator delete_at_index(size_t index);

    Iterator find_next_iterator(size_t index);
    Iterator find_next_iterator(Iterator it);
    

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

    Iterator erase(const K& key);
    Iterator erase(Iterator it);

};

}

#include "swiss_map.tpp"


#endif
