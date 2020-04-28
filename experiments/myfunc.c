

int Accumulate(int in) {
  static int reg = 0;
  reg = reg + in;  
  return reg;
}

int Accumulate2(int in, _Bool rst) {
  static int reg = 0;   
  if (!rst) {
    reg = reg + in;    
  } else {
    reg = 0;    
  }
  return reg;
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


int MultiplyAccumulate(const int a, int b, int c) {
  return a*b + c;
}

int MultiplyAccumulate2(unsigned a, unsigned  b, unsigned c) {
  return a*b + c;
}

int MultiplyAccumulate4(int a, int b, int c, int d) {
  return a*b + c*d;
}

int MultiplyAccumulate8(int a, int b, int c, int d, int e, int f, int g, int h) {
  return a*b*c + c*d*e + e*f*g + h;
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

int ifthenelse2(int a, int b, int c, int d)
{ 
  int e;
  if (a > b)
    e = (a*b) + (c*d);
  else 
    e = (b*c) + (a*d);
  
  return e;
}

int multiple_select(int a, int b)
{  
  int c;
  switch(a) 
  {
    case 0:  c = b + 1; break;
    case 1:  c = b - 1; break;
    case 2:  c = b * 2; break;
    default: c = b * 3; break;
  }
  
  return c;
}

int multiple_select2(int a, int b)
{  
  int c;
  switch(a) 
  {
    case 0:  return b + 1; 
    case 1:  return b - 1; 
    case 2:  return b * 2; 
    default: return b * 3; 
  }
}
#define LENGTH 8

typedef struct {int data[LENGTH];} SHIFT_REG;
typedef struct {int data[2]; unsigned char start:1; unsigned char end : 1;} BUS_T;


int fir(SHIFT_REG a, SHIFT_REG b)
{
  int tmp = 0;
  for(unsigned i = 0 ; i < LENGTH; i ++ )
  {
    tmp += a.data[i]*b.data[i];
  }
  return tmp;
}

int vector_sum(int a[LENGTH])
{
  int tmp = 0;
  for(unsigned i = 0 ; i < LENGTH; i ++ )
  {
    tmp += a[i];
  }
  return tmp;
}

int vector_sum2(int a[LENGTH])
{
  int tmp = 0;
  for(unsigned i = 0 ; i < LENGTH; i+=2 )
  {
    tmp += (a[i] + a[i+1]);
  }
  return tmp;
}

int fir1(int a[LENGTH], const int b[LENGTH])
{
  int tmp = 0;
  for(unsigned i = 0 ; i < LENGTH; i ++ )
  {
    tmp += a[i]*b[i];
  }
  return tmp;
}

int fir2(int in, SHIFT_REG b) //int b[LENGTH])
{
  static int tmp = 0;
  //static int a[LENGTH];
  static SHIFT_REG a;
  a.data[0] = in;
  tmp = 0;
  for(unsigned i = 0 ; i < LENGTH; i ++ )
  {
    tmp += a.data[i]*b.data[i];
    a.data[i] = a.data[i-1];
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

typedef int Array8_t [8];

int assortedArg(int * pointer, int scalar, SHIFT_REG structure, Array8_t array, BUS_T complex_struct)
{
  return complex_struct.start + complex_struct.end;
}

void returnVoid(int * a, int *b)
{
  b[0] = a[0];
}

int __main(void)
{
  int array[10];
  return pointers(array, 10, 0x1234);  
}