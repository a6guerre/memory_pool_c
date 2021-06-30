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

  mem_header *header = malloc(sizeof(mem_header));
  if(header == NULL)
  {
    return NULL;
  }
  
  header->size = size - sizeof(mem_header);
  header->is_free = 1;
  header->next = NULL;
  header->prev = NULL;

  pool->total_size = size;
  pool->free_size = size - sizeof(mem_header);

  memcpy(pool->buf, header, sizeof(header));

  return pool;
}

void *mp_malloc(mem_pool * const pool, uint32_t size)
{
  mem_header *header = (mem_header *)pool->buf;
  uint8_t *ptr = (uint8_t *)pool->buf;

  mem_header *old_header = header;
  while(!header->is_free)
  {
    printf("header next is at %x\n", header->next);
    printf("header is at %x\n", header);
    //this fixes the issue
    uint8_t *iter_pointer = (uint8_t*)header + header->size + sizeof(mem_header);
    printf("header is at %x\n", iter_pointer); 
    header = (mem_header *)iter_pointer;

    //it should really be: header = header->next, header->next ends up pointing to somewhere way off 
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
    mem_header *new_header = (mem_header *)malloc(sizeof(mem_header));
    new_header->is_free = 1;
    new_header->size = remaining_size - sizeof(mem_header);
    new_header->prev = header;
    
    ptr = (uint8_t *)header;
    printf("ptr before: %x\n", ptr);
    ptr = (uint8_t *)header + sizeof(mem_header) + header->size;
    printf("ptr after: %x\n", ptr);

    memcpy(ptr, new_header, sizeof(mem_header));
    header->next = (mem_header *)ptr;
    mem_header *verify_header = header->next;
    printf("%x\n", header->next); // this matches up with ptr, which is good.
  
    pool->free_size -= sizeof(mem_header);
  } 

  uint8_t *buf = (uint8_t*)header + sizeof(mem_header);

  return (void *)buf;
}
