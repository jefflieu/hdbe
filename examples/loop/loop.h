

#ifndef __LOOP__
#define __LOOP__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef signed long long   s64;
typedef signed int         s32;
typedef signed short       s16;
typedef signed char        s8;

u32 loop(u8 n);
//u32 loop(u32** a, u8 r, u8 c);

#ifdef __cplusplus  
}
#endif



#endif
