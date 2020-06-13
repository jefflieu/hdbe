
#ifndef __VECTOR_ADD__
#define __VECTOR_ADD__


#ifdef __cplusplus
extern "C" {
typedef bool _Bool;
#endif   

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef signed long long   s64;
typedef signed int         s32;
typedef signed short       s16;
typedef signed char        s8;

#define SIZE 2048

void vector_add(s32 *a, s32 *b, s32 *sum);

#ifdef __cplusplus
}
#endif



#endif