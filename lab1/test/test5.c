#include <stdlib.h>

int main(void)
{
  void *a, *b;

  a = malloc(128);
  free(a);
  b = realloc(a, 0);
  b = realloc(a, 128);
  b = realloc(a, 1 << 20);

  return 0;
}
