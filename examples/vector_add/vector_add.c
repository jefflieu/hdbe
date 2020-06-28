

#include "vector_add.h"


void vector_add(s32 *a, s32 *b, s32 *sum)
{
  u16 i, j;
  s32 a0, b0;
  #if SIZE > 16
  #pragma nounroll
  #pragma clang loop vectorize(disable)
  #endif
  for(i = 0; i < SIZE; i++)
  {
    if(i > 0) sum[i-1] = a0+b0;
    a0 = a[i]; 
    b0 = b[i];
  } 
  sum[SIZE-1] = a0+b0;

}