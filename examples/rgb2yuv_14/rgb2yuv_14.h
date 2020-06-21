

#ifndef __RGB2YUV__
#define __RGB2YUV__

#ifdef __cplusplus 
extern "C" {
#endif

#if __clang_major__ < 11
#error "This example requires clang 11 and above"
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

typedef unsigned _ExtInt(14) u14;
typedef   signed _ExtInt(14) s14;
typedef unsigned _ExtInt(28) u28;
typedef   signed _ExtInt(28) s28;



#define FRAC 10
#define IDX(i, j) (i*3 + j)

const s14 OFFSET[3] = {64, 512, 512};
void rgb2yuv_14(u14* rgb, s14 * coef, s14* yuv);


#ifdef __cplusplus 
}
#endif

#endif
