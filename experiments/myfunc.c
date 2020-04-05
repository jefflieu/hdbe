

int Accumulate(int in) {
  static int reg = 0;
  int acc = reg + in;
  reg = in;
  return acc;
}


//typedef struct {int addr; int length;} OutBus;
//
//void Parse(long long in, OutBus& out) {
//  static int cnt = 0;
//  if (cnt == 0)  
//    out.addr = in >> 32;
//  if (cnt == 1)
//    out.length = in >> 32;
//  cnt++;
//}


int MultiplyAccumulate(int a, int b, int c) {
  return a*b + c;
}

int MultiplyAccumulate4(int a, int b, int c, int d) {
  return a*b + c*d;
}


int Max(int a, int b)
{
  return (a > b)?a:b;
}

int ifthenelse(int a, int b)
{
  if (a > b)
    return a - b; 
  else 
    return b - a;
}

#pragma clang attribute push (__attribute__((annotate("shift_reg"))), apply_to = record)

typedef struct {int data[16];} Shift_Reg16;

#pragma clang attribute pop

int fir(Shift_Reg16 a, Shift_Reg16 b)
{
  int tmp = 0;
  for(unsigned char i = 0 ; i < 16; i ++ )
  {
    tmp += a.data[i]*b.data[i];
  }
  return tmp;
}

int fir1(int a[16], int b[16])
{
  int tmp = 0;
  for(unsigned char i = 0 ; i < 16; i ++ )
  {
    tmp += a[i]*b[i];
  }
  return tmp;
}

int fir2(int in, int b[16])
{
  static int tmp = 0;
  static int a[16];
  a[0] = in;
  tmp = 0;
  for(unsigned char i = 0 ; i < 16; i ++ )
  {
    tmp += a[i]*b[i];
    a[i] = a[i-1];
  }
  return tmp;
}

int fibo(int a)
{
  int i = 3, a0 = 2, f = 3;  
  if (a > 3) {
    while(i < a) 
      {
        f = f + a0;
        a0 = f - a0;
        i++;
      }
    return f;
  } else
    return a;
}

int pointers(int * a, int size, int patt)
{
  int i;
  for1: for(i = 0; i < size; i++)
  {
    if (a[i] == patt) break;
  }
  return i;
}

int main(void)
{
  int array[10];
  return pointers(array, 10, 0x1234);  
}