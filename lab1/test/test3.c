#include <stdlib.h>
#include <time.h>

#define N 3
#define MAX_SIZE (1<<16)

int main(void)
{
  void *a[N];
  int i;

  // we want reproducible random numbers for the evaluation
  //srand((unsigned int)time(NULL));
  srand(0);

  for (i=0; i<N; i++) {
    size_t s = rand() % MAX_SIZE;
    a[i] = rand() % 2 ? malloc(s) : calloc(1, s);
  }

  for (i=N; i>0; i--) free(a[i-1]);

  return 0;
}
