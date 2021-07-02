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

  uint8_t *buf = mp_malloc(pool, 10);
  uint8_t arr[] = {1,2,3,4,5,6,7,8,9,10};

  memcpy(buf, arr, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf);
    ++buf;
  }
  uint8_t *buf2 = mp_malloc(pool, 10);
  
  memcpy(buf2, arr, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf2);
    ++buf2;
  }
  uint8_t arr2[] = {0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20};

  uint8_t *buf3 = mp_malloc(pool, 10);

  memcpy(buf3, arr2, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf3);
    ++buf3;
  }
  //trivial examples for now, will add more rigorous unit tests
  mp_mem_dump(pool);
 /* buf -=10;
  mp_free(buf, pool);
  mp_mem_dump(pool);
  */
  buf2 -=10;
  mp_free(buf2, pool);
  mp_mem_dump(pool);
  
  buf3 -= 10;
  mp_free(buf3, pool);
  mp_mem_dump(pool);


  return 0;
}
