#include "swiss_map.h"

/*
Because writing it every time is a pain here it is for you to cpy paste:
```
template<typename K, typename V, typename Hash>
type swiss_map<K, V, Hash>::your_function() {
        
}
```

*/

namespace mischa {

template<typename K, typename V, typename Hash>
size_t swiss_map<K, V, Hash>::H1(size_t hash) const { return hash >> 7; }

template<typename K, typename V, typename Hash>
ctrl_t swiss_map<K, V, Hash>::H2(size_t hash) const { return static_cast<ctrl_t>(hash & 0x7F); }

template<typename K, typename V, typename Hash>
void swiss_map<K, V, Hash>::set_table_size(size_t n) {
        this->ctrl_.assign(n + 16, Ctrl::kEmpty);
        this->table_.resize(n);
        this->size_ = n;
}


template<typename K, typename V, typename Hash>
uint16_t swiss_map<K, V, Hash>::match(size_t index, h2_t hash) const
{
        __m128i target = _mm_set1_epi8(hash);
        // Get segment in array by offsetting memory by index and then casting to __m128i*
        __m128i ctrl_segment = _mm_loadu_si128(
                reinterpret_cast<const __m128i*>(this->ctrl_.data() + index)
        );

        __m128i matches = _mm_cmpeq_epi8(ctrl_segment, target);

        return static_cast<uint16_t>(_mm_movemask_epi8(matches));
}


template<typename K, typename V, typename Hash>
uint16_t swiss_map<K, V, Hash>::match_empty(size_t index) const {
        __m128i target = _mm_set1_epi8(Ctrl::kEmpty);
        // Get segment in array by offsetting memory by index and then casting to __m128i*
        __m128i ctrl_segment = _mm_loadu_si128(
                reinterpret_cast<const __m128i*>(this->ctrl_.data() + index)
        );

        __m128i matches = _mm_cmpeq_epi8(ctrl_segment, target);

        return static_cast<uint16_t>(_mm_movemask_epi8(matches));
}

template<typename K, typename V, typename Hash>
uint16_t swiss_map<K, V, Hash>::match_free_slot(size_t index) const
{
        // Check for all 3 empty slot configurations
        __m128i target_empty = _mm_set1_epi8(Ctrl::kEmpty);
        __m128i target_deleted = _mm_set1_epi8(Ctrl::kDeleted);

        // Get segment in array by offsetting memory by index and then casting to __m128i*
        __m128i ctrl_segment = _mm_loadu_si128(
                reinterpret_cast<const __m128i*>(this->ctrl_.data() + index)
        );

        __m128i matches_empty = _mm_cmpeq_epi8(ctrl_segment, target_empty);
        __m128i matches_deleted = _mm_cmpeq_epi8(ctrl_segment, target_deleted);

        uint16_t empty_bits = static_cast<uint16_t>(_mm_movemask_epi8(matches_empty));
        uint16_t deleted_bits = static_cast<uint16_t>(_mm_movemask_epi8(matches_deleted));

        return empty_bits | deleted_bits;
}



template<typename K, typename V, typename Hash>
void swiss_map<K, V, Hash>::expand() {
        size_t old_size = this->size_;
        size_t new_size = static_cast<size_t>(old_size * this->growth_factor);
        std::vector<std::pair<K, V>> old_table = std::move(this->table_);
        std::vector<ctrl_t> old_ctrl = std::move(this->ctrl_);
        
        this->table_.clear();
        this->ctrl_.clear();
        this->bucketCount_ = 0; // Setting to 0 since we're moving 
        
        this->set_table_size(new_size);
        

        for(size_t i = 0; i < old_size; i++) {
                // Check if not a null segment
                if(((old_ctrl[i]) & 0x80) == 0) {
                        this->insert(
                                std::move(old_table[i].first), 
                                std::move(old_table[i].second)
                        );
                }
        }
}


template<typename K, typename V, typename Hash>
size_t swiss_map<K, V, Hash>::find_free_slot(const K& key) {
        size_t hash = Hash{}(key);
        size_t h1 = this->H1(hash);
        h2_t h2 = this->H2(hash);
        
        size_t table_index = h1 % this->size_;

        size_t steps = 0;


        // A much better iteration method.
        while(steps < this->size_) {
                uint16_t segment = this->match_free_slot(table_index);
                size_t offset = 0;
                

                while(segment) {
                        if((segment & 1) != 0)
                                return (table_index + offset) % this->size_;
                        offset++;
                        segment >>= 1;
                }

                table_index = (table_index + 16) % this->size_;
                steps += 16;
        }

        
        
        // If we've gone through the whole map and not found an empty slot expand and calculate the same thing.
        this->expand();
        return this->find_free_slot(key);

        
}

/*
        Public Member Functions
*/

template<typename K, typename V, typename Hash>
swiss_map<K, V, Hash>::swiss_map() {
        this->set_table_size(16);
        this->bucketCount_ = 0;
}

template<typename K, typename V, typename Hash>
swiss_map<K, V, Hash>::swiss_map(size_t n) {
        this->set_table_size(n < 16 ? 16 : n);
        this->bucketCount_ = 0;
}

template<typename K, typename V, typename Hash>
V& swiss_map<K, V, Hash>::at(const K& key) {
        size_t hash = Hash{}(key);
        size_t h1 = this->H1(hash);
        h2_t h2 = this->H2(hash);
        
        size_t table_index = h1 % this->size_;

        size_t steps = 0;

        while(steps < this->size_) {
                uint16_t matches = this->match(table_index, h2);
                uint16_t empty = this->match_empty(table_index);

                for(size_t i = 0; i < 16; ++i) {
                        bool is_match = (matches & (1 << i)) != 0;
                        bool is_empty = (empty & (1 << i)) != 0;
                        size_t raw_index = (table_index + i) % this->size_;
                        
                        if(is_match && key == this->table_[raw_index].first) {
                                return this->table_[raw_index].second;
                        }
                        // Ending probe chain, value not found
                        if(is_empty) {
                                throw std::out_of_range("key not found");
                        }
                }

                table_index = (table_index + 16) % this->size_;
                steps += 16;
        }
        

        throw std::out_of_range("key not found"); // Throw 404 error


}


template<typename K, typename V, typename Hash>
V& swiss_map<K, V, Hash>::insert(const K& key, const V& value) {

        // Guardrail: Force expansion at 87.5% load factor to guarantee empty slots exist
        if (this->bucketCount_ >= this->size_ * 0.875)
                this->expand();
        

        try {
                auto& result = this->at(key);
                result = value;
                return result;
        } catch(const std::out_of_range& e) { // Need to insert the value
                size_t hash = Hash{}(key);
                size_t index = this->find_free_slot(key);
                this->ctrl_[index] = H2(hash);
                // Mirroring for ability to find beyond table size range
                if(index < 16) 
                        this->ctrl_[this->size_ + index] = H2(hash);
                

                this->table_[index] = std::make_pair(key, value);
                this->bucketCount_++;
                return this->table_[index].second;
        }
        
} 

template<typename K, typename V, typename Hash>
V& swiss_map<K, V, Hash>::operator[](const K& key) {
        if (this->bucketCount_ >= this->size_ * 0.875)
                this->expand();

        try {
                auto& result = this->at(key);
                return result;
        } catch(const std::out_of_range& e) { // Need to insert the value
                size_t hash = Hash{}(key);
                // std::cout << "Hash(" << key << ") = " << hash << "\n";
                size_t index = this->find_free_slot(key);
                this->ctrl_[index] = H2(hash);

                if(index < 16) 
                        this->ctrl_[this->size_ + index] = H2(hash);

                this->table_[index] = std::make_pair(key, V());
                this->bucketCount_++;
                return this->table_[index].second;
        }
}


}

