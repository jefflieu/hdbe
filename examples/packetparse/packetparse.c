
#include "packetparse.h"


/*
void packetparse(u64* pkthdr, _Bool sop, _Bool eop,
                 u64* mac_dst, u64* mac_src,
                 u16* ethType)
{
  u16 wordcnt = 0;
  u64 pktdata = 0;
  u16 mac_src_16;
  
  for(wordcnt = 0; wordcnt < 2; wordcnt ++) {
    pktdata = pkthdr[wordcnt];
    if(wordcnt == 0) {
      *mac_dst = OCTET(pktdata, 0, 5);
      mac_src_16 = OCTET(pktdata, 6, 7);
    } else if (wordcnt == 1) {
      *mac_src = OCTET(pktdata, 0, 3) | ((u64) mac_src_16 << 32);  
      *ethType = OCTET(pktdata, 4, 5);
    }  
  }  
  return;
} 
*/



void packetparse(u64  pktdata, _Bool sop, _Bool eop,
                 u64* mac_dst, u64* mac_src,
                 u16* eth_type, u16* eth_length)
{
  static u16 wordcnt = 0;
  static u16 tmp_16;
  u16 wordcnt_last;
  u16 type_length;

  wordcnt_last = wordcnt;
  if (eop) wordcnt = 0; else wordcnt ++;

  switch(wordcnt_last)
  {
  case 0 : *mac_dst = OCTET(pktdata, 0, 5);
            tmp_16  = OCTET(pktdata, 6, 7);
            break;
  case 1 : *mac_src = ((u64)tmp_16 << 32) | OCTET(pktdata, 0, 3);
           type_length = OCTET(pktdata, 4, 5);
           if (type_length <= 1500)
             *eth_length = type_length;
           else 
             *eth_type   = type_length;
           break;
   
  default : break;
  }
  return;  
}

