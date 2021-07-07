#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"

static uint32_t compute_write_size(mem_header * header);

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
  if(pool == NULL)
  { 
    return NULL;
  }
  if(size > pool->free_size)
  {
    return NULL;
  }

  mem_header *header = (mem_header *)pool->buf;

  // The header must be free, and of adequate size.
  while(!header->is_free || header->size < size)
  {
    header = header->next;
    if(header == NULL)
    {
      return NULL;
    }
  }

  header->is_free = 0;
  header->size = size;
  
  pool->free_size -= size;

  uint8_t remaining_size = pool->free_size - sizeof(mem_header);

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
  printf("header size %d\n", header->size);
  return (void *)header + sizeof(mem_header);
}

void mp_free(void *buf, mem_pool * const pool){
  
  uint8_t *ptr = (uint8_t *)buf - sizeof(mem_header);
  mem_header *header = (mem_header *)ptr;
  uint8_t next_free = (header->next != NULL)? header->next->is_free : 0;
  uint8_t prev_free = (header->prev != NULL)? header->prev->is_free : 0;
  printf("%d\n", prev_free);

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

void mp_realloc(void *buf, mem_pool * const pool, uint32_t new_size)
{
  uint8_t *ptr = (uint8_t *)buf - sizeof(mem_header);
  mem_header *header = (mem_header *)ptr;

  int size_change = (int) ((uint32_t)new_size - (uint32_t)header->size);
  printf("size change %d\n", header->size);

  if(size_change > pool->free_size)
    return;

  header->size = new_size;
  header = header->next;
  ptr = (uint8_t *)header + size_change;

  //compute write size
  uint32_t write_size = compute_write_size(header);

  if(size_change > 0)
    write_size -= size_change;

  printf("write size: %d\n", write_size);
  memmove(ptr, header, write_size);
  
  header = (mem_header *) ptr;

  int i = 0;
  while(header->next != NULL)
  {
    ptr = (uint8_t *)header->next + size_change;
    header->next = (mem_header *)ptr;
    if(i > 0)
    {
      header->prev = (mem_header *)((uint8_t *)header->prev + size_change);
    }
    else
    {
      header->prev->next = header;
    }
    header = header->next;
    ++i;
  }

  pool->free_size -= size_change;
  
  if(header->is_free)
  {
    header->size -= size_change;
    header->prev = (mem_header *)((uint8_t *)header->prev + size_change);
  }
  mp_mem_dump(pool);
}

static uint32_t compute_write_size(mem_header * header)
{
  uint32_t write_size = 0;
  if(header == NULL)
    return write_size;

  write_size += header->size + sizeof(mem_header);

  while(header->next != NULL)
  {
    header = header->next;
    write_size += header->size + sizeof(mem_header);
    printf("we're here\n");
  }

  return write_size;
}
