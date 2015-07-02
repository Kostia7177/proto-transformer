#pragma once

#include <type_traits>

template<int arg> using Int2Type = std::integral_constant<int, arg>;

// a pair of types with guarantee-different sizes.
// used within any sfinae-based detectors.
typedef char One;
struct Two { One two[2]; };
