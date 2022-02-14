#include <stdlib.h>


int main(void)
{
  void *a;

  a = malloc(10);
  free(a);

  a = realloc(a, 10000);

  a = realloc(a, 0);

  a = realloc(NULL, 10000);
  free(a);


  return 0;
}
