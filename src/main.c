#include "algorithm/sort.h"
#include "mock/array.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

const int N = 32;
/* const int N = 8 * 200000000; */

int main(void) {
  int my_rank;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // proc0生成随机数组
  int *arr_p = NULL;
  if (my_rank == 0) {
    arr_p = (int *)malloc(N * sizeof(int));
    random_int_array(arr_p, N);
  }

  // 排序
  odd_even_transposition_sort(arr_p, N, MPI_COMM_WORLD);

  // 排序结果
  if (my_rank == 0) { // 这个if一开始漏了, 不能漏啊!
    printf("The result: ");
    for (int i = 0; i < N; i++)
      printf("%d ", arr_p[i]);
    printf("\n");
  }

  // free堆数组
  free(arr_p);

  MPI_Finalize();
  return 0;
}
