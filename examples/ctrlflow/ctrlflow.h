

#ifndef __CONTROL_FLOW__
#define __CONTROL_FLOW__

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

s32 ctrlflow(u8 op, s32 a, s32 b, s32 c);

#ifdef __cplusplus  
}
#endif



#endif