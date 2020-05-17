

#include "ctrlflow.h"


// s32 ctrlflow(u8 op, s32 a, s32 b, s32 c)
// {
//   s32 result;
//   switch(op)
//   {
//     case 0 : result = a + b; break;
//     case 1 : result = a - b; break;
//     case 2 : result = a * b; break;
//     case 3 : result = a * b + c; break;
//     default: result = 0; break;
//   }
//   return result;
// }


s32 ctrlflow(u8 op, s32 a, s32 b, s32 c)
{
  s32 result;
  switch(op)
  {
    case 0 : result = a + b; 
             switch(c)
             {
              case 2 : result = result >> 4; break;
              case 3 : result = result >> 8; break;
              default : break;
             } 

             break;
    case 1 : result = a - b; break;
    case 2 : result = a * b; break;
    case 3 : result = a * b + c; break;
    default: result = 0; break;
  }
  return result;
}