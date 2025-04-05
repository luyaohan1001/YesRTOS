#include "bitops.hpp"

void set_bitpos(size_t& bitmask, const size_t& bitpos) {
  bitmask |= 1 << bitpos;
}

void clr_bitpos(size_t& bitmask, const size_t& bitpos) {
  bitmask &= ~(1 << bitpos);
}

size_t count_leading_zero(size_t& val) {
  return __builtin_clz(val);
}

size_t find_bitpos_rightmost_zero(const size_t& val) {
  return __builtin_ctz(~val);
}

bool check_is_pow_of_2(size_t& val) {
  return (val > 0) && !(val & (val - 1));
}