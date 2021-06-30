#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"


int main(void)
{
  mem_pool *pool = mp_create(256);
  mem_header *header = (mem_header*)pool->buf;
  //memcpy(header, pool->buf, sizeof(mem_header));
  printf("%d\n", header->size);
  uint8_t *buf = mp_malloc(pool, 10);
  uint8_t arr[] = {1,2,3,4,5,6,7,8,9,10};

  memcpy(buf, arr, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf);
    ++buf;
  }

  printf("%d\n", pool->free_size);

  uint8_t *buf2 = mp_malloc(pool, 10);
  
  memcpy(buf2, arr, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf2);
    ++buf2;
  }

  return 0;
}
