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
const size_t swiss_map<K, V, Hash>::H1(size_t hash) const { return hash >> 7; }

template<typename K, typename V, typename Hash>
const ctrl_t swiss_map<K, V, Hash>::H2(size_t hash) const { return hash & 0x7F; }

template<typename K, typename V, typename Hash>
void swiss_map<K, V, Hash>::set_table_size(size_t n) {
        this->ctrl_.assign(16 + n, Ctrl::kEmpty);
        this->table_.resize(n);
        this->size_ = n;
}



/*
        Public Member Functions
*/

template<typename K, typename V, typename Hash>
swiss_map<K, V, Hash>::swiss_map() {
        this->set_table_size(0);
        this->bucketCount_ = 0;
}

template<typename K, typename V, typename Hash>
swiss_map<K, V, Hash>::swiss_map(size_t n) {
        this->set_table_size(n);
        this->bucketCount_ = 0;
}


}

