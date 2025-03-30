#pragma once

#include "bitops.hpp"
#include <stdint.h>

extern uint32_t _ld_start_heap;
extern uint32_t _ld_end_heap;

class mempool {

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

  public:
    mempool();

    size_group_t match_size_grp(const uint32_t& req_size);
    alloc_t malloc(const uint32_t& req_size);
    void free(uint32_t* p_mem);
    ~mempool() = delete;

  public:
    uint32_t min_alloc_size, max_alloc_size;
    uint32_t total_capacity_byte;

  private:
    constexpr static uint32_t ALLOC_SIZE_GROUPS = 5;
    constexpr static uint32_t MIN_ALLOC_SIZE_SHAMT = 5;
    constexpr static uint32_t MAX_ALLOC_SIZE_SHAMT = 9;
    constexpr static uint32_t BITMAP_FULL_MASK = 0xFFFFFFFF;
    uint32_t alloc_bitmaps[ALLOC_SIZE_GROUPS] = {0};
    uint32_t heap_start, heap_end;
    uint32_t size_grp_start_addr[ALLOC_SIZE_GROUPS];
};