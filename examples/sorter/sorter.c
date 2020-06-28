
#include "sorter.h"

#define BUBBLE_SORT

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

/*
void sorter(s32* data, u13 size)
{
  u13 i = 0; 
  _Bool done = 1;
  do {
    s32 di = data[i];
    s32 dj = data[i+(u13)1];
    if (di > dj) {
      data[i] = dj;
      data[i+(u13)1] = di;
      done = 0;
    }
    i++;
    if (i == size && done) break;
    if (i == size && !done) done = 1;
  } while (1);
}*/

#elif defined(QUICK_SORT)


#endif
