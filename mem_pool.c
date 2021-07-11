#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"

static void *resize_block(mem_header *header, int size_change);
static void *move_block(mem_pool * const pool, mem_header *header, int size_change);

mem_pool *mp_create(uint32_t size)
{
  mem_pool *pool = malloc(sizeof(mem_pool));
  if(pool == NULL)
  {
    return NULL;
  }

  pool->buf = malloc(size);
  if(pool->buf == NULL)
  {
    return NULL;
  }

  mem_header *header = (mem_header *)pool->buf;
  
  header->size = size - sizeof(mem_header);
  header->is_free = 1;
  header->next = NULL;
  header->prev = NULL;

  pool->total_size = size;
  pool->free_size = size - sizeof(mem_header);

  return pool;
}

void mp_mem_dump(mem_pool * const pool)
{
  printf("Total Pool Size: %d\n", pool->total_size);
  printf("Space Available: %d\n", pool->free_size);
  
  mem_header *hdr = (mem_header *)pool->buf;
  while(hdr != NULL)
  {
    printf("Header at: %p\nPrev Header at: %p\nNext Header at: %p\n", hdr, hdr->prev, hdr->next);
    printf("Block is free: %d\n", hdr->is_free);
    printf("Size of payload: %d\n", hdr->size);
    uint8_t *buf = (uint8_t*)hdr + sizeof(mem_header);

    for(int i = 0; i < hdr->size; ++i, ++buf)
    {
      printf("%02x ", *buf);
    }
    printf("\n");
    hdr = hdr->next;
  } 
}

void *mp_malloc(mem_pool * const pool, uint32_t size)
{
  mem_header *header = (mem_header *)pool->buf;
  uint8_t remaining_size;

  if(pool == NULL)
  { 
    return NULL;
  }
  if(size > pool->free_size)
  {
    return NULL;
  }

  // The header must be free, and of adequate size.
  while(!header->is_free || header->size < size)
  {
    header = header->next;
    if(header == NULL)
    {
      return NULL;
    }
  }

  remaining_size = header->size - size;
  header->is_free = 0;
  header->size = size;
  
  pool->free_size -= size;

  if(remaining_size > sizeof(mem_header))
  {  
    uint8_t* ptr = (uint8_t *)header + sizeof(mem_header) + header->size;
    mem_header *new_header = (mem_header *) ptr;
    new_header->is_free = 1;
    new_header->size = remaining_size - sizeof(mem_header);
    new_header->next = header->next; // in the edge case where the next block is actually there.
    new_header->prev = header;

    header->next = new_header;
    pool->free_size -= sizeof(mem_header);
  }  
  return (void *)header + sizeof(mem_header);
}

void mp_free(void *buf, mem_pool * const pool){
  
  uint8_t *ptr = (uint8_t *)buf - sizeof(mem_header);
  mem_header *header = (mem_header *)ptr;
  uint8_t next_free = (header->next != NULL)? header->next->is_free : 0;
  uint8_t prev_free = (header->prev != NULL)? header->prev->is_free : 0;

  mem_header *prev_header = header->prev;
  mem_header *next_header = header->next;

  if(prev_free)
  {
    prev_header->size += sizeof(mem_header) + header->size;
    pool->free_size += sizeof(mem_header) + header->size;
    prev_header->next = next_header;

    if(next_header != NULL)
      next_header->prev = prev_header;

    if(next_free)
    {
      pool->free_size += sizeof(mem_header);
      prev_header->next = next_header->next;
      prev_header->size += next_header->size + sizeof(mem_header);

      if(next_header->next != NULL)
        next_header->next->prev = prev_header;
    }
    return;
  }
  else if(next_free)
  {
    header->is_free = 1;
    header->size += next_header->size + sizeof(mem_header);
    pool->free_size += sizeof(mem_header);
    header->next = next_header->next;
    if(next_header->next != NULL)
      next_header->next->prev = header;
    return;
  }
  // update pool size
  pool->free_size += header->size;
  header->is_free = 1;
}

void *mp_realloc(void *buf, mem_pool * const pool, uint32_t new_size)
{
  uint8_t *ptr = (uint8_t *)buf - sizeof(mem_header);
  mem_header *header = (mem_header *)ptr;
  mem_header *next_hdr = header->next;
  uint8_t *result = (uint8_t *)buf;
  int size_change = new_size - header->size;
  int remaining_size = next_hdr->size - size_change;
  
  if(next_hdr->is_free == 1 && remaining_size > sizeof(mem_header))
  {
    if(size_change != 0)
    {
       result = (uint8_t*)resize_block(header, size_change); 
    }
    else
    {
      return (void *)result;
    }
  }
  else
  {
    result = (uint8_t*)move_block(pool, header, size_change);
  }
  return (void *)result;
}

static void *resize_block(mem_header *header, int size_change)
{
  uint8_t *result = (uint8_t *) ((uint8_t *)header + sizeof(mem_header));
  mem_header *source = header->next;
  mem_header *dest = (mem_header *)((uint8_t *)result + header->size + size_change);
  int remaining_size = source->size - size_change;

  if(remaining_size > sizeof(mem_header))
  {
    memmove(dest, source, sizeof(mem_header));
    if(size_change > 0)
    {
      dest->size += size_change;
    }
    else
    {
      dest->size -= size_change;
    }
    header->next = dest;
    header->size += size_change;
  
    if(dest->next != NULL)
    {
      dest->next->prev = dest;

    }
  }
  
  return (void *)result;
}

static void *move_block(mem_pool * const pool, mem_header *header, int size_change)
{
  uint8_t *result = (uint8_t *)mp_malloc(pool, header->size + size_change);
  void *buf = (void *)((uint8_t *)header + sizeof(mem_header)); 

  if(result == NULL)
  {
    return buf;
  }
  
  mp_free(buf, pool);
  return result;
}
