#include "swiss_map.h"

/*
Because writing it every time is a pain here it is for you to cpy paste:
```
template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
type swiss_map<K, V, Hash>::your_function() {
        
}
```

*/

namespace mischa {

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
size_t swiss_map<K, V, Hash>::H1(size_t hash) const { return hash >> 7; }

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
ctrl_t swiss_map<K, V, Hash>::H2(size_t hash) const { return static_cast<ctrl_t>(hash & 0x7F); }

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
void swiss_map<K, V, Hash>::set_table_size(size_t n) {
        this->ctrl_.assign(n + 16, Ctrl::kEmpty);
        this->table_.resize(n);
        this->size_ = n;
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
bool swiss_map<K, V, Hash>::occupied(size_t index) const {
        return (this->ctrl_[index] & 0x80) == 0;
}


template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
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
requires Hashable<K, Hash>
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
requires Hashable<K, Hash>
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
requires Hashable<K, Hash>
void swiss_map<K, V, Hash>::expand() {
        size_t old_size = this->size_;
        size_t new_size = static_cast<size_t>(old_size * this->growth_factor);
        std::vector<std::pair<K, V>> old_table = std::move(this->table_);
        std::vector<ctrl_t> old_ctrl = std::move(this->ctrl_);
        
        this->table_.clear();
        this->ctrl_.clear();
        this->bucket_count_ = 0; // Setting to 0 since we're moving 
        
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
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator 
swiss_map<K, V, Hash>::find_next_iterator(size_t index) {
        uint16_t entry = ~this->match_free_slot(index);

        while(!entry && index < this->size_) {
                index += 16;
                entry = ~this->match_free_slot(index);
        }
        // Move to first zero
        index += __builtin_ctz(entry);

        index = std::min(this->size_, index);
        return Iterator(this, index);
}


template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator 
swiss_map<K, V, Hash>::find_next_iterator(Iterator it) {
        it++;
        return it;
}


template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator 
swiss_map<K, V, Hash>::delete_at_index(size_t index) {
        if(this->occupied(index))
                this->bucket_count_--;
                
        this->ctrl_[index] = Ctrl::kDeleted;
        if(index < 16) 
                this->ctrl_[index] = Ctrl::kDeleted;
        // Don't actually have to delete at the table since we overwrite everything.

        return this->find_next_iterator(index);

}


/*
        Public Member Functions
*/

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
swiss_map<K, V, Hash>::swiss_map() {
        this->set_table_size(16);
        this->bucket_count_ = 0;
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
swiss_map<K, V, Hash>::swiss_map(size_t n) {
        this->set_table_size(n < 16 ? 16 : n);
        this->bucket_count_ = 0;
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
swiss_map<K, V, Hash>::swiss_map(std::initializer_list<std::pair<K, V>> list) : swiss_map(list.size()) {
        for(const auto& item : list) {
                this->insert(
                        item.first,
                        item.second
                );
        }
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
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
requires Hashable<K, Hash>
V& swiss_map<K, V, Hash>::insert(const K& key, const V& value) {

        // Guardrail: Force expansion at 87.5% load factor to guarantee empty slots exist
        if (this->bucket_count_ >= this->size_ * 0.875)
                this->expand();

        // Better optimisation
        size_t hash = Hash{}(key);
        size_t h1 = H1(hash);
        ctrl_t h2 = H2(hash);

        size_t table_index = h1 % this->size_;
        size_t steps = 0;

        // All 1s
        size_t first_free_slot = static_cast<size_t>(-1);

        while(steps < this->size_) {
                uint16_t matches = this->match(table_index, h2);
                uint16_t free_slot = this->match_free_slot(table_index);

                uint16_t matches_cpy = matches;

                while(matches_cpy) {
                        size_t offset = __builtin_ctz(matches_cpy);
                        size_t raw_index = (table_index + offset) % this->size_;

                        if(this->table_[raw_index].first == key) {
                                this->table_[raw_index].second = value;
                                return this->table_[raw_index].second;
                        }

                        matches_cpy &= (matches_cpy - 1);
                }
                // Store first empty slot we see
                if(first_free_slot == static_cast<size_t>(-1) && free_slot != 0) 
                        first_free_slot = (table_index + __builtin_ctz(free_slot)) % this->size_;
                // I was silly before. If we see an empty slot just return right after doing our loop
                if(this->match_empty(table_index) != 0)
                        break;
                
                table_index = (table_index + 16) % this->size_;
                steps += 16;
        }

        // Exited without finding the slot. This means we should insert a new point

        this->ctrl_[first_free_slot] = h2;
        if(first_free_slot < 16)
                this->ctrl_[first_free_slot + this->size_] = h2;
        this->table_[first_free_slot] = std::make_pair(key, value);
        this->bucket_count_++;
        return this->table_[first_free_slot].second;
        
} 

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
V& swiss_map<K, V, Hash>::operator[](const K& key) {
        
        // Guardrail: Force expansion at 87.5% load factor to guarantee empty slots exist
        if (this->bucket_count_ >= this->size_ * 0.875)
                this->expand();

        // Better optimisation
        size_t hash = Hash{}(key);
        size_t h1 = H1(hash);
        ctrl_t h2 = H2(hash);


        size_t table_index = h1 % this->size_;
        size_t steps = 0;

        // All 1s
        size_t first_free_slot = static_cast<size_t>(-1);


        while(steps < this->size_) {
                uint16_t matches = this->match(table_index, h2);
                uint16_t free_slot = this->match_free_slot(table_index);

                while(matches) {
                        size_t offset = __builtin_ctz(matches);
                        size_t raw_index = (table_index + offset) % this->size_;

                        if(this->table_[raw_index].first == key) {
                                return this->table_[raw_index].second;
                        }

                        matches &= (matches - 1);
                }
                // Store first empty slot we see
                if(first_free_slot == static_cast<size_t>(-1) && free_slot != 0) 
                        first_free_slot = (table_index + __builtin_ctz(free_slot)) % this->size_;
                // I was silly before. If we see an empty slot just return right after doing our loop
                if(this->match_empty(table_index) != 0)
                        break;
                
                table_index = (table_index + 16) % this->size_;
                steps += 16;
        }

        // Exited without finding the slot. This means we should insert a new point

        this->ctrl_[first_free_slot] = h2;
        if(first_free_slot < 16)
                this->ctrl_[first_free_slot + this->size_] = h2;
        this->table_[first_free_slot] = std::make_pair(key, V());
        this->bucket_count_++;
        return this->table_[first_free_slot].second;
}

template <typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator
swiss_map<K, V, Hash>::erase(const K& key) {
        size_t hash = Hash{}(key);
        size_t h1 = H1(hash);
        ctrl_t h2 = H2(hash);


        size_t table_index = h1 % this->size_;
        size_t steps = 0;

        while(steps < this->size_) {
                uint16_t matches = this->match(table_index, h2);
                uint16_t empty = this->match_empty(table_index);

                while(matches) {
                        size_t offset = __builtin_ctz(matches);
                        size_t raw_index = (table_index + offset) % this->size_;

                        if(this->table_[raw_index].first == key) {
                                return this->delete_at_index(raw_index);; // Delete and return
                        }
                        
                        matches &= (matches - 1);                        
                }
                
                if(empty) // Value not found just return
                        return this->end();
                
                table_index = (table_index + 16) % this->size_;
                steps += 16;

        }

        return this->end();
}



/*
================== ITERATOR FUNCTIONS ======================
*/

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator::reference
swiss_map<K, V, Hash>::Iterator::operator*() const {
        return this->map_->table_[this->index_];
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator::pointer
swiss_map<K, V, Hash>::Iterator::operator->() const {
        return &this->map_->table_[this->index_];
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator&
swiss_map<K, V, Hash>::Iterator::operator++() {
        this->index_++;
        // ~ slot = filled with some entry
        uint16_t entry = ~this->map_->match_free_slot(this->index_);
        while(!entry && this->index_ < this->map_->size_) {
                this->index_ += 16;
                entry = ~this->map_->match_free_slot(this->index_);
        }

        this->index_ += __builtin_ctz(entry);
        this->index_ = std::min(this->index_, this->map_->size_);

        return *this;
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator
swiss_map<K, V, Hash>::Iterator::operator++(int) {
        Iterator old = *this;
        ++(*this);
        return old;
}







// Getting iterators

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator
swiss_map<K, V, Hash>::begin() {
        size_t index{};
        // Non-empty slot
        uint16_t entry = ~this->match_free_slot(index);

        while(!entry && index < this->size_) {
                index += 16;
                entry = ~this->match_free_slot(index);
        }
        // Move to first zero
        index += __builtin_ctz(entry);

        index = std::min(this->size_, index);
        return Iterator(this, index);
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator
swiss_map<K, V, Hash>::end() {
        // Return element just past table indeces
        return Iterator(this, this->size_);
}

template<typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator
swiss_map<K, V, Hash>::find(const K& key) {

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
                                return Iterator(this, raw_index);
                        }
                        // Ending probe chain, value not found
                        if(is_empty) {
                                return this->end();
                        }
                }

                table_index = (table_index + 16) % this->size_;
                steps += 16;
        }
        

        return this->end(); // Default

}




template <typename K, typename V, typename Hash>
requires Hashable<K, Hash>
typename swiss_map<K, V, Hash>::Iterator
swiss_map<K, V, Hash>::erase(Iterator it) {
        // Validate same key iterator
        if(it == Iterator(this, it.index_))
                it = this->delete_at_index(it.index_); // Erase
        
        return it;
}


}

