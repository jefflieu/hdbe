

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
#define LENGTH 8

typedef struct {int data[LENGTH];} SHIFT_REG;



int fir(SHIFT_REG a, SHIFT_REG b)
{
  int tmp = 0;
  for(unsigned i = 0 ; i < LENGTH; i ++ )
  {
    tmp += a.data[i]*b.data[i];
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

int main(void)
{
  int array[10];
  return pointers(array, 10, 0x1234);  
}