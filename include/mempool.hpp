#pragma once

#include <stdint.h>

#include "bitops.hpp"

extern uint32_t _ld_start_heap;
extern uint32_t _ld_end_heap;

class mempool {
  public:
  typedef enum {
    ALLOC_SUCCESS,
    ALLOC_FAIL
  } alloc_status_t;

  typedef struct {
    alloc_status_t status;
    uint32_t* addr;
  } alloc_t;

  typedef enum {
    SIZE_GRP_32_BYTE = 0,
    SIZE_GRP_64_BYTE = 1,
    SIZE_GRP_128_BYTE = 2,
    SIZE_GRP_256_BYTE = 3,
    SIZE_GRP_512_BYTE = 4,
    SIZE_GRP_NOT_FOUND,
  } size_group_t;

  static size_group_t match_size_grp(const uint32_t& req_size);
  static alloc_t malloc(const uint32_t& req_size);
  static void free(uint32_t* p_mem);
  static void init();

  private:
  // hide constructor and destructor to avoid external instantiation of this class.
  mempool();
  ~mempool() = delete;

  // friend function
  public:
  static uint32_t get_alloc_bitmap(size_group_t sz_grp) {
    return alloc_bitmaps[sz_grp];
  }

  public:
  static uint32_t min_alloc_size, max_alloc_size;
  static uint32_t total_capacity_byte;

  constexpr static uint32_t MAX_CNT_PER_SIZE_GRP = 32;  // MAX 32 instance of each allocation size.

  private:
  // "static" allocates memory at compile-time; "constexpr" forces initialization at compile-time, which compiler does constant folding.
  // below has declaration and and constexpr allows internal instantiation (memory allocated here).
  constexpr static uint32_t ALLOC_SIZE_GROUPS = 5;     // corresponding to 32 / 64 / 128 / 256 / 512 byte defined in size_group_t
  constexpr static uint32_t MIN_ALLOC_SIZE_SHAMT = 5;  // 1 << 5 = 32
  constexpr static uint32_t MAX_ALLOC_SIZE_SHAMT = 9;  // 1 << 9 = 512
  constexpr static uint32_t BITMAP_FULL_MASK = 0xFFFFFFFF;

  // below are only declared, required external instantiation later.
  static uint32_t alloc_bitmaps[ALLOC_SIZE_GROUPS];
  static uint32_t heap_start, heap_end;
  static uint32_t size_grp_start_addr[ALLOC_SIZE_GROUPS];
};