# `std::unordered_map`

The `std::unordered_map` is a hash table-based associative container that provides fast access to elements based on their keys. It allows for efficient insertion, deletion, and retrieval of key-value pairs. The keys are unique, and the values can be accessed using the corresponding keys.

For now, the priority of implementation will be ordered as follows:
1. `CRITICAL` - Functions that are essential for the basic functionality of the unordered map, such as insertion, deletion, and retrieval of elements.
2. `IMPORTANT` - Functions that enhance the usability and performance of the unordered map, such as iterators, capacity management, and hash policy functions.
3. `OPTIONAL` - Functions that provide additional features or optimizations but are not essential for the core functionality of the unordered map, such as observers and non-member functions.


## Construction

```c++
template<
    class Key,
    class T,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>,
    class Allocator = std::allocator<std::pair<const Key, T>>
> class unordered_map;
```

**key** - The type pushed into the map -- typically a primative type or a string. Custom hash types can be set with the "Hash" template parameter.

**T** - The type of the value associated with the key. This can be any type, including user-defined types.

**Hash** - A unary function object that takes a key as an argument and returns a hash value. The default is `std::hash<Key>`, which provides a hash function for built-in types and some standard library types.

**KeyEqual** - A binary function object that takes two keys as arguments and returns `true` if they are considered equal. The default is `std::equal_to<Key>`, which uses the equality operator (`==`) to compare keys. Defining a custom equality function can be useful when you want to compare keys in a specific way, such as case-insensitive string comparison.

**Allocator** - The allocator type used to manage memory for the elements in the container. The default is `std::allocator<std::pair<const Key, T>>`, which is a standard allocator for pairs of keys and values.


## Functions

### Initialisation
 - (Constructor) 
 `CRITICAL`

 - (Destructor) 
 `CRITICAL`

 - Copy Constructor (`unordered_map(const unordered_map& other)`) 
 `IMPORTANT`

 - Move Constructor (`unordered_map(unordered_map&& other) noexcept`) 
 `IMPORTANT`

 - Copy Assignment Operator (`unordered_map& operator=(const unordered_map& other)`) 
 `IMPORTANT`

 - Move Assignment Operator (`unordered_map& operator=(unordered_map&& other) noexcept`) 
 `IMPORTANT`

 - `std::swap` (non-member function) 
 `OPTIONAL`

 - `erase_if()` (non-member function) 
 `OPTIONAL`

 - `get_allocator() -> allocator_type` 
 `OPTIONAL`

### Iterators (Not sure how to do this at this point, as I haven't got a grasp on iterators yet.)

 - `begin() -> std::unordered_map<Key, T>::iterator` - `cbegin() -> std::unordered_map<Key, T>::const_iterator`
 `OPTIONAL`

 - `end() -> std::unordered_map<Key, T>::iterator` - `cend() -> std::unordered_map<Key, T>::const_iterator`
 `OPTIONAL`

### Capacity

- `empty() -> bool`
`IMPORTANT`

- `size() -> size_type`
`IMPORTANT`

- `max_size() -> size_type`
`OPTIONAL`

### Modifiers

- `clear() -> void`
`IMPORTANT`

- `insert() -> std::pair<iterator, bool>`
`CRITICAL`

- `insert_range() -> void`
`OPTIONAL`

- `insert_or_assign() -> std::pair<iterator, bool>`
`OPTIONAL`

- `emplace() -> std::pair<iterator, bool>`
`IMPORTANT`

- `emplace_hint() -> iterator`
`OPTIONAL`

- `try_emplace() -> std::pair<iterator, bool>`
`OPTIONAL`

- `erase() -> size_type`
`IMPORTANT`

- `swap() -> void`
`IMPORTANT`

- `extract() -> node_type`
`OPTIONAL`

- `merge() -> void`
`OPTIONAL`

### Lookup
- `at() -> T&`
`IMPORTANT`
- `operator[] -> T&`
`CRITICAL`
- `count() -> size_type`
`IMPORTANT`
- `find() -> iterator`
`IMPORTANT`
- `contains() -> bool`
`OPTIONAL`
- `equal_range() -> std::pair<iterator, iterator>`
`OPTIONAL`

### Bucket Interface
- `bucket_count() -> size_type`
`OPTIONAL`
- `max_bucket_count() -> size_type`
`OPTIONAL`
- `bucket_size() -> size_type`
`OPTIONAL`
- `bucket() -> size_type`
`OPTIONAL`

### Hash Policy
- `load_factor() -> float`
`OPTIONAL`
- `max_load_factor() -> float`
`OPTIONAL`
- `rehash() -> void`
`OPTIONAL`
- `reserve() -> void`
`OPTIONAL`

### Observers
- `hash_function() -> Hash`
- `key_eq() -> KeyEqual`



