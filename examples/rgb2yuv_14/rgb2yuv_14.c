
#include "rgb2yuv_14.h"

void rgb2yuv_14(u14* rgb, s14 * coef, s14* yuv)
{
  u8 i, j;
  
  for(i = 0; i < 3; i++)
    {
      s28 s = 0;
      for(j = 0; j < 3; j++)
      {
        s+=(s28)rgb[j]*(s28)coef[IDX(i,j)];
      }
      yuv[i] = (s14)(s >> FRAC) + OFFSET[i];
    }
  
  return;
}

