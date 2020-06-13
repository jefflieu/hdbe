

#include "vector_add.h"

#define SIZE 2048

void vector_add(s32 *a, s32 *b, s32 *sum)
{
  u16 i, j;
  #if SIZE > 16
  #pragma nounroll
  #pragma clang loop vectorize(disable)
  #endif 
  for(i = 0; i < SIZE; i++)
  {
    sum[i] = a[i] + b[i];
  } 

}