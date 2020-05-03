
#define LENGTH 4

typedef unsigned char u8;

int shiftreg(int din)
{
  static int reg[LENGTH];
  u8 i;
  for(i = LENGTH-1; i > 0; i--)
   reg[i] = reg[i-1];
  reg[0] = din;  
return reg[LENGTH-1];
}
