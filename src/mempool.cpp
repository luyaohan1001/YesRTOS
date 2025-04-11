#include "mempool.hpp"

#if defined (HOST_PLATFORM)
#include <iostream>
#endif

// External instantiation of static variable declared in class.
size_t mempool::heap_start;
size_t mempool::heap_end;
size_t mempool::alloc_bitmaps[ALLOC_SIZE_GROUPS];

size_t mempool::min_alloc_size;
size_t mempool::max_alloc_size;
size_t mempool::size_grp_start_addr[ALLOC_SIZE_GROUPS];
size_t mempool::total_capacity_byte;
bool mempool::init_complete = false;

/**
 * @brief Initialize static variables of memory pool class.
 * @param None.
 * @note It should be called only once in the system initialization.
 */
void mempool::init() {
  mempool::heap_start = _ld_start_heap;
  mempool::heap_end = _ld_end_heap;

  mempool::size_grp_start_addr[0] = mempool::heap_start;
  for (size_t i = 1; i <= (MAX_ALLOC_SIZE_SHAMT - MIN_ALLOC_SIZE_SHAMT); ++i) {
    mempool::size_grp_start_addr[i] = mempool::size_grp_start_addr[i - 1] + static_cast<size_t>(32 * (1 << (i - 1)));
  }

  for (size_t& bm : alloc_bitmaps) {
    bm = 0;
  }

  mempool::min_alloc_size = 1 << MIN_ALLOC_SIZE_SHAMT;
  mempool::max_alloc_size = 1 << MAX_ALLOC_SIZE_SHAMT;
  mempool::total_capacity_byte = reinterpret_cast<size_t>(_ld_end_heap) - reinterpret_cast<size_t>(_ld_start_heap);
  init_complete = true;
}

/**
 * @brief Return the size group to allocate based on the memory size requested.
 * @param req_size Size user requested to allocate
 * @return Size group enum which the request size belongs to.
 */
mempool::size_group_t mempool::match_size_grp(const size_t& req_size) {
  size_t alloc_size = req_size;
  if (alloc_size < min_alloc_size) alloc_size = min_alloc_size;
  if (alloc_size > max_alloc_size) return SIZE_GRP_NOT_FOUND;

  size_t clz = count_leading_zero(alloc_size);
  size_t size_grp = (32 - clz) - MIN_ALLOC_SIZE_SHAMT - check_is_pow_of_2(alloc_size);
  return static_cast<size_group_t>(size_grp);
}

/**
 * @brief Memory allocation API which user calls to allocate heap memory dynamically.
 * @param req_size Size user requested to allocate.
 * @return Status and memory address if allocation is successful.
 */
mempool::alloc_t mempool::malloc(const size_t& req_size) {
  if (!init_complete) mempool::init();
  size_group_t sz_grp = match_size_grp(req_size);
  alloc_t ret = {ALLOC_FAIL, nullptr};
  if (sz_grp == SIZE_GRP_NOT_FOUND) return ret;
  if (alloc_bitmaps[sz_grp] == BITMAP_FULL_MASK) return ret;

  size_t bitpos = find_bitpos_rightmost_zero(alloc_bitmaps[sz_grp]);
  alloc_bitmaps[sz_grp] |= 1 << bitpos;
  ret.addr = reinterpret_cast<size_t*>(size_grp_start_addr[sz_grp] + bitpos * (1 << (sz_grp + MIN_ALLOC_SIZE_SHAMT)));
  ret.status = ALLOC_SUCCESS;
  return ret;
}

/**
 * @brief Free the memory on heap.
 * @param p_mem Pointer to the memory to free.
 * @return None.
 */
void mempool::free(size_t* p_mem) {
  if (!init_complete) mempool::init();
  size_t target_mem = static_cast<size_t>(reinterpret_cast<uintptr_t>(p_mem));
  for (size_t sz_grp = 0; sz_grp <= (MAX_ALLOC_SIZE_SHAMT - MIN_ALLOC_SIZE_SHAMT); ++sz_grp) {
    int32_t addr_offset = static_cast<int32_t>(target_mem - size_grp_start_addr[sz_grp]);
    if (addr_offset < 0) continue;
    size_t bitpos = addr_offset / (1 << sz_grp);
    clr_bitpos(alloc_bitmaps[sz_grp], bitpos);
    return;
  }
}

// Friend functions
size_t get_alloc_bitmap(mempool::size_group_t sz_grp) {
  return mempool::alloc_bitmaps[sz_grp];
}

size_t* get_size_grp_start_addr() {
  return &mempool::size_grp_start_addr[0];
}

size_t get_alloc_size_groups() {
  return mempool::ALLOC_SIZE_GROUPS;
}