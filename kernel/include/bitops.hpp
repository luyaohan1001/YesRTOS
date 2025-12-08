#pragma once
#include <stdint.h>
#include <cstddef>

template<typename T>
void set_bitpos(T& bitmask, const T& bitpos) {
  bitmask |= 1 << bitpos;
}

template<typename T>
void clr_bitpos(T& bitmask, const T& bitpos) {
  bitmask &= ~(1 << bitpos);
}

template<typename T>
size_t count_leading_zero(T& val) {
  return __builtin_clz(val);
}

template<typename T>
size_t count_trailing_zero(const T& val) {
  return __builtin_ctz(val);
}

template<typename T>
bool check_is_pow_of_2(T& val) {
  return (val > 0) && !(val & (val - 1));
}