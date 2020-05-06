
#include "rgb2yuv.h"

void rgb2yuv(u16* rgb, s16 * coef, s16* yuv)
{
  u8 i, j;
  
  for(i = 0; i < 3; i++)
    {
      s32 s = 0;
      for(j = 0; j < 3; j++)
      {
        s+=rgb[j]*((s32)coef[IDX(i,j)]);
      }
      yuv[i] = (s >> FRAC);
    }
  
  return;
}

