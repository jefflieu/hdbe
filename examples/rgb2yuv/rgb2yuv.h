

#ifndef __RGB2YUV__
#define __RGB2YUV__

#ifdef __cplusplus 
extern "C" {
#endif


typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;



#define FRAC 10
#define IDX(i, j) (i*3 + j)

const s16 OFFSET[3] = {64, 512, 512};
void rgb2yuv(u16* rgb, s16 * coef, s16* yuv);


#ifdef __cplusplus 
}
#endif

#endif