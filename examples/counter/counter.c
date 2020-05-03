

int counter(_Bool ld, int load, int inc)
{
  static int reg;
  if (ld) reg = load; else reg = reg + inc;
  return reg;
}