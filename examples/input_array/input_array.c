

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;

#define LENGTH 4
u16 input_array(u16 din[LENGTH])
{
  u8 i;
  u16 s = 1;
  for(int i = 0; i < LENGTH; i++)
  {
    s+=din[i];
  }
  return s;
}