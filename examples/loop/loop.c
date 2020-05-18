
#include "loop.h"

u32 loop(u8 n)
{
  u32 an_1 = 1, an_2 = 1, an = 1;  
  if (n == 0 || n == 1) return an;
  #pragma nounroll
  do{
      an =  an_1 + an_2;
      n--;
      an_2 = an_1;
      an_1 = an;
    } while (n >=2 );
  return an;
}
