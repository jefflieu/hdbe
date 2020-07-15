
#include "sorter.h"

#define QUICK_SORT

#if defined(BUBBLE_SORT)
// sort 
void sorter(s32* data, u13 size)
{
  u13 i, j; 
  for(i = 0; i < size; i++)
  {
    s32 di = data[i];
    for(j = i; j < size; j++)
    {
      s32 dj = data[j];
      if (di > dj) {
        data[i] = dj;
        data[j] = di;
        di = dj;
      }
    }
  }
}

#elif defined(QUICK_SORT)

// sort 
void sorter(s32* data, u13 size)
{
  u13 i, j, p; 
  u13 left, right;
  static u13 left_stack[32];
  static u13 right_stack[32];
  s32 d_pivot, dj, di;
  u8  ptr = 1;
  left_stack[ptr] = 0;
  right_stack[ptr] = size;
  while(ptr>0)
  {
    //Find pivot
    left = left_stack[ptr]; 
    right = right_stack[ptr]; 
    DPRINT("Do range left %d right %d (ptr%d)\n", (int)left, (int)right, ptr);
    ptr--;
    if (left >= right) continue;
    p = right-(u13)1;
    d_pivot = data[p];
    j = left;
    dj = data[j];
    #pragma nounroll
    #pragma clang loop vectorize(disable)
    for(i = left; i <= right; i++)
    {
      //DPRINT("left %3d, right %3d, di %3d d_pivot %3d  (%d)\n", (int)left, (int)right, di, d_pivot, (di < d_pivot && i > j));
      if (i > left) {
        if (di < d_pivot) {
          if (j < (i-(u13)1)) {data[i-(u13)1] = dj; data[j] = di;} 
          j++;
          dj = data[j];
        }
      }
      di = data[i];
      for(int i = 0; i < (int)size; i++)
      {
        DPRINT("%d ", data[i]);
      }
      DPRINT("-(%d)\n", i);
    }
    data[p] = data[j];
    data[j] = d_pivot;
    ptr++; left_stack[ptr] = left; right_stack[ptr] = j;
    ptr++; left_stack[ptr] = j+(u13)1; right_stack[ptr] = right;
    DPRINT("Ptr %d, left %d, pivot %d, right %d\n", ptr, (int)left, j, (int)right);
    for(int i = 0; i < (int)size; i++)
    {
      DPRINT("%d ", data[i]);
    }
    DPRINT("\n");
      
  }

}

#endif

#ifdef MAIN_DEBUG 
#define SIZE 16  
#define RANGE 50
#include <stdio.h>
int main(void)
{
  s32 data[SIZE] = {1, 2, 4, 9, 3, 4, 6, 10, 12, 15, 20, 4, 5, 9, 8, 7};
  printf("Data In:\n");
  for(int i = 0; i < SIZE; i++)
  {
    //data[i] = (rand() % (2*RANGE)) - RANGE;
    //data[i] = SIZE-i;
    printf("%d ", data[i]);
  }
  printf("\n");
  
  sorter(data, (u13)SIZE);

  printf("Data out:\n");
  for(int i = 0; i < SIZE; i++)
  {
    printf("%d ", data[i]);
  }
  printf("\n");
}
#endif 

