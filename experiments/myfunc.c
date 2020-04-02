


int MultiplyAccumulate(int a, int b, int c) {
  return a*b + c;
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