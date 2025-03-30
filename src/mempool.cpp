#include "mempool.hpp"

mempool::mempool(): heap_start(_ld_start_heap), heap_end(_ld_end_heap) {
  total_capacity_byte = reinterpret_cast<uint32_t>(_ld_end_heap) - reinterpret_cast<uint32_t>(_ld_start_heap);

  min_alloc_size = 1 << MIN_ALLOC_SIZE_SHAMT;
  max_alloc_size = 1 << MAX_ALLOC_SIZE_SHAMT;

  size_grp_start_addr[0] = heap_start;
  for (uint32_t i = MIN_ALLOC_SIZE_SHAMT + 1; i <= MAX_ALLOC_SIZE_SHAMT; ++i) {
      size_grp_start_addr[i] =                                  \
        size_grp_start_addr[i - 1]                              \
        + static_cast<uint32_t>(32 * (1 << (i - 1)));           \
  }
}

mempool::size_group_t mempool::match_size_grp(const uint32_t& req_size) {
  uint32_t alloc_size = req_size;
  if (alloc_size < min_alloc_size) alloc_size = min_alloc_size;
  if (alloc_size > max_alloc_size) return SIZE_GRP_NOT_FOUND;

  uint32_t clz = __builtin_clzll(alloc_size);
  uint32_t size_grp = 32 - clz - check_is_pow_of_2(alloc_size);
  return static_cast<size_group_t>(size_grp);
}

mempool::alloc_t mempool::malloc(const uint32_t& req_size) {
  size_group_t sz_grp = match_size_grp(req_size);
  alloc_t ret = {ALLOC_FAIL, reinterpret_cast<uint32_t *>(-1)};
  if (sz_grp == SIZE_GRP_NOT_FOUND) return ret;
  if (alloc_bitmaps[sz_grp] == BITMAP_FULL_MASK) return ret;

  uint32_t bitpos = find_bitpos_rightmost_zero(alloc_bitmaps[sz_grp]);
  alloc_bitmaps[sz_grp] |= 1 << bitpos;
  ret.addr = reinterpret_cast<uint32_t *>(size_grp_start_addr[sz_grp] + bitpos * (1 << (sz_grp + MIN_ALLOC_SIZE_SHAMT)));
  ret.status = ALLOC_SUCCESS;
  return ret;
}

void mempool::free(uint32_t* p_mem) {
  uint32_t target_mem = reinterpret_cast<uint32_t>(p_mem);
  for (uint32_t sz_grp = MIN_ALLOC_SIZE_SHAMT; sz_grp <= MAX_ALLOC_SIZE_SHAMT; ++sz_grp) {
    int32_t addr_offset = static_cast<int32_t>(target_mem - alloc_bitmaps[sz_grp]);
    if (addr_offset < 0) continue;
    uint32_t bitpos = addr_offset / (1 << sz_grp);
    clr_bitpos(alloc_bitmaps[sz_grp], bitpos);
  }
}

