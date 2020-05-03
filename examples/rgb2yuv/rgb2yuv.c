

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;


#define FRAC 10
#define IDX(i, j) (i*3 + j)

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
      yuv[i] = s >> FRAC;
    }
  
  return;
}

