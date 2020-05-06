
#include "input_array.h"

u16 input_array(u16 din[LENGTH])
{
  u8 i;
  u16 s = 1;
  for(int i = 0; i < LENGTH; i++)
  {
    s+=din[i];
  }
  return s;
}