
#ifndef __SORTER__
#define __SORTER__

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

typedef unsigned _ExtInt(12) u12;
typedef unsigned _ExtInt(13) u13;

void sorter(s32 * data, u13 size);

#ifdef __cplusplus 
}
#endif

#endif
