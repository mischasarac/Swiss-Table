#include "swiss_map.h"

namespace mischa {

template<typename K, typename V, typename Hash>
const size_t swiss_map<K, V, Hash>::H1(size_t hash) const { return hash >> 7; }

template<typename K, typename V, typename Hash>
const ctrl_t swiss_map<K, V, Hash>::H2(size_t hash) const { return hash & 0x7F; }



/*
        Public Member Functions
*/


}

