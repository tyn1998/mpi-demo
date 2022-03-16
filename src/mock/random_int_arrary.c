#include "mock/array.h"

#include <stdlib.h>
#include <time.h>

void random_int_array(int *array, int length) {
  srand(time(NULL));

  for (int i = 0; i < length; i++) {
    /* array[i] = rand(); */
    array[i] = rand() % 1000 - 500;
  }
}
