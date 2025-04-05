#pragma once
#include <stdint.h>

#include <cstddef>

void set_bitpos(size_t& bitmask, const size_t& bitpos);
void clr_bitpos(size_t& bitmask, const size_t& bitpos);
size_t count_leading_zero(size_t& val);
size_t find_bitpos_rightmost_zero(const size_t& val);
bool check_is_pow_of_2(size_t& val);