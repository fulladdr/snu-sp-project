#include <stdlib.h>


int main(void)
{
  void *a;

  a = malloc(10);
  a = realloc(a, 100);
  a = realloc(a, 1000);
  a = realloc(a, 10000);
  a = realloc(a, 100000);
  a = realloc(a, 10);
  free(a);


  return 0;
}
