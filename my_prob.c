#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int a = 5;
  int b = 3;
  int result;
  
  printf(1, "a = %d, b = %d\n", a, b);
  
  result = add(a, b);
  
  printf(1, "add(%d, %d) = %d\n", a, b, result);
  
  exit();
}