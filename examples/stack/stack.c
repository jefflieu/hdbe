#include "stack.h"

#define SIZE 512 
/*
_Bool stack(u64 in, u64* out, _Bool push, _Bool pop)
{
static u64 stack[SIZE];
static u16 ptr = 0;
_Bool result = 1;
if (push && ptr < SIZE) {
  stack[ptr++] = in;
} else  if (pop && ptr > 0) {
  *out = stack[--ptr];
} else 
  result = 0;

return result;
}
*/

_Bool stack(u64 in, u64* out, _Bool push, _Bool pop)
{
static u64 stack[SIZE];
static u16 push_ptr = 0;
static u16 pop_ptr = 0;
_Bool result = 1;
if (push && push_ptr < SIZE) {
  pop_ptr = push_ptr;
  stack[push_ptr++] = in;
} else  if (pop && push_ptr > 0) {
  push_ptr = pop_ptr;
  *out = stack[pop_ptr--];
} else 
  result = 0;

return result;
}