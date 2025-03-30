#include "bitops.hpp"

uint32_t set_bitpos(uint32_t& bitmask, const uint32_t& bitpos) {
  bitmask |= 1 << bitpos;
}

uint32_t clr_bitpos(uint32_t& bitmask, const uint32_t& bitpos) {
  bitmask &= ~(1 << bitpos);
}

uint32_t count_leading_zero(uint32_t& val) {
  return __builtin_clzll(alloc_size);
}

uint32_t find_bitpos_rightmost_zero(const uint32_t& val) {
  return __builtin_ctzll(~val);
}

bool check_is_pow_of_2(uint32_t& val) {
  return (val > 0) && !(val & (val - 1));
}