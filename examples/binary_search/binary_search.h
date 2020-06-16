#ifndef __BINARY_SEARCH__
#define __BINARY_SEARCH__

#ifdef __cplusplus
extern "C" {
typedef bool _Bool;
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned short u32;
typedef signed int s32;

_Bool binary_search(u32* mem, u32 data, u16 size, u16* match_idx);

#ifdef __cplusplus
}
#endif

#endif

