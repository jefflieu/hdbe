
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


// #define swap(a, b) do {u32 t; t = a; a = b; b = t;} while (0)

// u32 loop(u32** a, u8 r, u8 c)
// {
//   u8 i, j;
//   #pragma nounroll
//   for(i = 0; i < r; i++)
//     for(j = 0; j < c; j++)
//     {
//       swap(a[i][j], a[j][i]);
//     }
//   return 1;
// }
