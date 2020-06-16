#ifndef __MEMPORT__
#define __MEMPORT__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned short u32;
typedef signed int s32;

u16 memport(u32* mem, u32 data, u16 size);

#ifdef __cplusplus
}
#endif

#endif

