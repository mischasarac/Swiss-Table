// compile_time_tests.cpp

#include "../src/swiss_map.h"

#include <functional>
#include <string>

namespace {

struct CustomType {};

struct CustomHash {
    std::size_t operator()(const CustomType&) const {
        return 0;
    }
};

struct BadHash {
    void operator()(const CustomType&) const {}
};

struct NotHashable {};

template<typename K, typename V, typename Hash = std::hash<K>>
concept SwissMapInstantiable =
    requires {
        typename mischa::swiss_map<K, V, Hash>;
    };

} // namespace

// =====================================================
// Built-in types should work with std::hash
// =====================================================

static_assert(SwissMapInstantiable<int, int>);
static_assert(SwissMapInstantiable<long, int>);
static_assert(SwissMapInstantiable<unsigned, int>);
static_assert(SwissMapInstantiable<char, int>);
static_assert(SwissMapInstantiable<bool, int>);

// =====================================================
// Standard library types with std::hash specializations
// =====================================================

static_assert(SwissMapInstantiable<std::string, int>);

// =====================================================
// User-defined types with custom hashes
// =====================================================

static_assert(
    SwissMapInstantiable<
        CustomType,
        int,
        CustomHash
    >
);

// =====================================================
// Invalid cases
// =====================================================

// No std::hash specialization.
static_assert(
    !SwissMapInstantiable<
        NotHashable,
        int
    >
);

// User-defined type but no hash supplied.
static_assert(
    !SwissMapInstantiable<
        CustomType,
        int
    >
);

// Hash exists but does not return something
// convertible to std::size_t.
static_assert(
    !SwissMapInstantiable<
        CustomType,
        int,
        BadHash
    >
);