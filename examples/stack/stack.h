
#ifndef __STACK_H__
#define __STACK_H__

#ifdef __cplusplus
extern "C" {
#define _Bool bool
#endif

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

_Bool stack(u64 item_in, u64* item_out,  _Bool push, _Bool pop);

#ifdef __cplusplus  
}
#endif



#endif
