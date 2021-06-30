#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H
/** @file */
#include <stdint.h>

typedef struct memory_pool
{
  uint32_t total_size;
  uint32_t free_size;
  void *buf;  
}mem_pool;

typedef struct memory_header
{
  uint32_t size;
  uint8_t is_free;
  struct memory_header *next;
  struct memory_header *prev;
}mem_header;


mem_pool *mp_create(uint32_t size);
void *mp_malloc(mem_pool * const pool, uint32_t size);

#endif
