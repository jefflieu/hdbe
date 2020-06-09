
#ifndef __PACKET_PARSE__
#define __PACKET_PARSE__

#ifdef __cplusplus
extern "C" {
#define _Bool bool
#endif

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#define MASK(n) (0xffffffffffffffff >> (n*8))
#define OCTET(data, a,b)  ((data >> (7-b)*8) & MASK(b-a+1))
#define WORD(a, b, c, d, e, f, g, h) ((((u64)a & 0xff) << 56) | \
                                      (((u64)b & 0xff) << 48) | \
                                      (((u64)c & 0xff) << 40) | \
                                      (((u64)d & 0xff) << 32) | \
                                      (((u64)e & 0xff) << 24) | \
                                      (((u64)f & 0xff) << 16) | \
                                      (((u64)g & 0xff) << 8) | \
                                      (((u64)h & 0xff) << 0))
#define RAND_WORD() WORD(rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand())

void packetparse(u64  pktdata, _Bool sop, _Bool eop,
                 u64* mac_dst, u64* mac_src,
                 u16* eth_type, u16* eth_length);

#ifdef __cplusplus  
}
#endif



#endif
