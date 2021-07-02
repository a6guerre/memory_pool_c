#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"

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
    printf("Header at: %x\nPrev Header at: %x\nNext Header at: %x\n", hdr, hdr->prev, hdr->next);
    printf("Block is free: %d\n", hdr->is_free);
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

  while(!header->is_free)
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
    new_header->next = NULL;
    new_header->prev = header;

    header->next = new_header;
    pool->free_size -= sizeof(mem_header);
  } 

  // my mistake was here, I had (uint8_t*)header + header->size;
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
    pool->free_size += sizeof(mem_header); 
    prev_header->next = next_header;

    if(next_header != NULL)
    {
      next_header->prev = prev_header;
    }
    if(next_free)
    {
      prev_header->size += sizeof(mem_header) + next_header->size;
      pool->free_size += sizeof(mem_header); 
      prev_header->next = next_header->next;
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
    {
      next_header->next->prev = header;
    }
    return;
  }
  // update pool size
  pool->free_size += header->size;
  header->is_free = 1;
}
