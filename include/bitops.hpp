#pragma once
#include <stdint.h>

void set_bitpos(uint32_t& bitmask, const uint32_t& bitpos);
void clr_bitpos(uint32_t& bitmask, const uint32_t& bitpos);
uint32_t count_leading_zero(uint32_t& val);
uint32_t find_bitpos_rightmost_zero(const uint32_t& val);
bool check_is_pow_of_2(uint32_t& val);