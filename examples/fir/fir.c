

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;

#define LENGTH 8
s16 fir(s16 din, s16 coef[LENGTH])
{
  u8 i;
  s16 s = 0;
  static s16 shift[LENGTH];
  for(int i = LENGTH-1; i > 0; i--)
  {
     shift[i] = shift[i-1];
     s+=shift[i]*coef[i];
  }
  shift[0] = din;
  s += din*coef[0];
  return s;
}
