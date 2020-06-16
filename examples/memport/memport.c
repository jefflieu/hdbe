
#include "memport.h"

// u16 memport(u32 mem[1024], u32 data, u16 size)
// {
//   u16 idx;
//   for(idx = 0; idx < size; idx++)
//   {
//    if(mem[idx] == data) return idx;
//   }
//   return -1;
// }

u16 __attribute__((noinline)) memport(u32* mem, u32 data, u16 size)
{
  u16 idx;
  u16 preload;
  for(idx = 0; idx <= size; idx++)
  {
   if (idx > 0 && preload == data) return idx - 1;
   preload = mem[idx];
  }
  return -1;
}

#define SIZE 1024
u16 context(u32 data)
{
u32 mem[SIZE];
return memport(mem, data, SIZE);
}
