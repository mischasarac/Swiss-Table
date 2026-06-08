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
        this->ctrl_.assign(16 + n, Ctrl::kEmpty);
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




/*
        Public Member Functions
*/

template<typename K, typename V, typename Hash>
swiss_map<K, V, Hash>::swiss_map() {
        this->set_table_size(1);
        this->bucketCount_ = 0;
}

template<typename K, typename V, typename Hash>
swiss_map<K, V, Hash>::swiss_map(size_t n) {
        this->set_table_size(n);
        this->bucketCount_ = 0;
}

template<typename K, typename V, typename Hash>
V& swiss_map<K, V, Hash>::at(const K& key) {
        size_t hash = Hash{}(key);
        size_t h1 = this->H1(hash);
        h2_t h2 = this->H2(hash);
        
        size_t table_index = h1 % this->size_;
        
        // Check initial condition
        uint16_t matches = this->match(table_index, h2);
        uint16_t empty = this->match_empty(table_index);
        

        size_t offset = 0;
        uint16_t comp = 1 << 15;

        while(comp) {
                bool is_match = comp & matches != 0;
                bool is_empty = comp & empty != 0;

                // If match then compare the table key against the goal key
                if(is_match && key == this->table_[table_index + offset].first) {
                        return this->table_[table_index + offset].second;
                }

                // If it is empty that means that we've seen an empty slot and still haven't found our key which means that we haven't found our goal.
                if(is_empty)
                        throw std::out_of_range("key not found"); // Throw 404 error
                
                // Move over to next index
                offset++;
                comp >>= 1;
        }
        

        // Now run our loop to find the actual index. Throw an error if empty found after a match.
        for(size_t curr_index = (table_index + 16) % this->size_; 
                curr_index < table_index && curr_index >= table_index + 16;
                curr_index = (curr_index + 16) % this->size_) 
        {
                offset = 0;
                comp = 1 << 15;
                matches = this->match(curr_index, h2);
                empty = this->match_empty(curr_index);

                while(comp) {
                        bool is_match = comp & matches != 0;
                        bool is_empty = comp & empty != 0;

                        // If match then compare the table key against the goal key
                        if(is_match && key == this->table_[table_index + offset].first) {
                                return this->table_[table_index + offset].second;
                        }

                        // If it is empty that means that we've seen an empty slot and still haven't found our key which means that we haven't found our goal.
                        if(is_empty)
                                throw std::out_of_range("key not found"); // Throw 404 error
                        
                        // Move over to next index
                        offset++;
                        comp >>= 1;
                }


        }

        throw std::out_of_range("key not found"); // Throw 404 error


}


}

