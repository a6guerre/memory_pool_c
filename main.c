#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_pool.h"

void test_2(mem_pool *pool)
{
  uint8_t *buf = mp_malloc(pool, 10);
  uint8_t arr[] = {1,2,3,4,5,6,7,8,9,10};

  memcpy(buf, arr, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf);
    ++buf;
  }
  buf -= 10;

  uint8_t *buf2 = mp_malloc(pool, 50);
  
  memcpy(buf2, arr, 10);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf2);
    ++buf2;
  }
  uint8_t arr2[] = {0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20};

  uint8_t *buf3 = mp_malloc(pool, 10);

  memcpy(buf3, arr2, 10);
  mp_mem_dump(pool);
  buf2 -= 10;
  mp_free(buf2, pool);
  mp_mem_dump(pool);
  mp_realloc(buf, pool, 60);
  mp_mem_dump(pool);
    //mp_mem_dump(pool);
  // buf2 -= 10;
  //mp_realloc(buf2, pool, 20);
 // mp_mem_dump(pool);

}

void test_1(mem_pool *pool)
{

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
  mp_mem_dump(pool);

  for(int i = 0; i < 10; ++i)
  {
    printf("%d\n", *buf3);
    ++buf3;
  }
  //trivial examples for now, will add more rigorous unit tests
  //buf -=10;
  //mp_free(buf, pool);
  mp_mem_dump(pool);
  buf2 -=10;
  mp_free(buf2, pool);
  mp_mem_dump(pool);
  mp_realloc(buf, pool, 20);
  mp_mem_dump(pool);

  //uint8_t *buf4 = mp_malloc(pool, 20);
/*  buf3 -= 10;
  mp_free(buf3, pool);*/
  //mp_mem_dump(pool);

}

static int test_create_large()
{
  mem_pool *pool = mp_create(2048);
  void *buf1, *buf2, *buf3;
  int ret = 0;

  buf1 = mp_malloc(pool, 256);
  if (!buf1)
    return -1;

  buf2 = mp_malloc(pool, 256);
  if (!buf2) {
    goto free_1;
    ret = -1;
  }

  buf3 = mp_malloc(pool, 256);
  if (!buf3) {
    goto free_2;
    ret = -1;
  }

  ret = mp_pool_consistent(pool);

  mp_free(buf3, pool);
free_2:
  mp_free(buf2, pool);
free_1:
  mp_free(buf1, pool);

  return ret;
}

int main(void)
{
  mem_pool *pool = mp_create(256);
  test_2(pool);
  //mem_header *header = (mem_header*)pool->buf;
 
  //memcpy(header, pool->buf, sizeof(mem_header));

  if (test_create_large())
    printf("test_create_large failed\n");

  return 0;
}
