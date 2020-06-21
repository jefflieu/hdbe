
#include "binary_search.h"


/*
_Bool binary_search(u32* mem, u32 data, u16 size, u16* match_idx)
{
  u16 idx = size/2;
  u16 upper = size-1;
  u16 lower = 0;
  _Bool done = 0;
  do{
    done = (upper == lower && lower == idx);
    if (mem[idx] < data){
      lower = idx + 1;
      idx   = (upper + idx + 1)/2; //Rounding up
    } else if(mem[idx] > data) {
      upper = idx - 1;
      idx = (idx + lower)/2; //Rounding down
    } else {
      *match_idx = idx;
      return 1;
    } 
  } while (!done);
  return 0;
}
*/


//Use additional flag to manipulate the schduling of the load instruction to reduce the looptime
//Notice that the iteration checks on the data loaded in previous iteration
_Bool binary_search(u32* mem, u32 data, u16 size, u16* match_idx)
{
  u16 upper = size-1;
  u16 lower = 0;
  u16 idx = size/2;
  u32 item;
  _Bool done = 0;
  _Bool preloop = 1;
  do{
    done = (upper == lower && lower == idx);
    if (!preloop) {
      if (item < data){
        lower = idx + 1;
        idx = (idx + 1 + upper)>>1;
      } else if(item > data) {
        upper = idx - 1;
        idx = (idx + lower)>>1;
      } else {
        *match_idx = idx;
        return 1;
      }
    }
    preloop = 0;
    item = mem[idx];
  } while (!done);
  return 0;
}

