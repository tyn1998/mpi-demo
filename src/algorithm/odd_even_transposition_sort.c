#include "algorithm/sort.h"
#include "mpi.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int cmpfunc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int compute_partner(int phase, int my_rank, int comm_sz) {
  int partner;
  if (phase % 2 == 0) {
    if (my_rank % 2 != 0)
      partner = my_rank - 1;
    else
      partner = my_rank + 1;
  } else {
    if (my_rank % 2 != 0)
      partner = my_rank + 1;
    else
      partner = my_rank - 1;
  }
  if (partner == -1 ||
      partner == comm_sz) // specificlly for two end elements in odd phase
    partner = MPI_PROC_NULL;
  return partner;
}

// merge two ASC ordered array into one half-lengthed ASC ordered array
void merge_half(int half, int *local_arr_p, int *partner_arr_p, int *temp_arr_p,
                int local_n) {
  int l_i, p_i, t_i;

  if (half == 0) { // merge low
    l_i = p_i = t_i = 0;
    while (t_i < local_n) {
      if (local_arr_p[l_i] <= partner_arr_p[p_i]) {
        temp_arr_p[t_i] = local_arr_p[l_i];
        l_i++;
      } else {
        temp_arr_p[t_i] = partner_arr_p[p_i];
        p_i++;
      }
      t_i++;
    }
  } else { // merge high
    l_i = p_i = t_i = local_n - 1;
    while (t_i >= 0) {
      if (local_arr_p[l_i] > partner_arr_p[p_i]) {
        temp_arr_p[t_i] = local_arr_p[l_i];
        l_i--;
      } else {
        temp_arr_p[t_i] = partner_arr_p[p_i];
        p_i--;
      }
      t_i--;
    }
  }

  for (l_i = 0; l_i < local_n; l_i++)
    local_arr_p[l_i] = temp_arr_p[l_i];
}

void odd_even_transposition_sort(int *arr_p, int n, MPI_Comm comm) {
  int comm_sz;
  int my_rank;

  MPI_Comm_size(comm, &comm_sz);
  MPI_Comm_rank(comm, &my_rank);

  int local_n = n / comm_sz; // assume that comm_sz evenly divides n
  int *local_arr_p = NULL, *partner_arr_p = NULL, *temp_arr_p = NULL;
  local_arr_p = (int *)malloc(local_n * sizeof(int));
  partner_arr_p = (int *)malloc(local_n * sizeof(int));
  temp_arr_p = (int *)malloc(local_n * sizeof(int));

  MPI_Scatter(arr_p, local_n, MPI_INT, local_arr_p, local_n, MPI_INT, 0, comm);

  // print local_arr
  printf("my_rank is %d: ", my_rank);
  for (int i = 0; i < local_n; i++)
    printf("%d ", local_arr_p[i]);
  printf("\n");

  double local_start, local_finish, local_elapsed, elapsed;
  MPI_Barrier(comm);
  local_start = MPI_Wtime();

  /**********************
   *  begin of core code  *
   **********************/

  // first, sort local keys
  qsort(local_arr_p, local_n, sizeof(int), cmpfunc);
  // phases iterations
  int phase, partner;
  for (phase = 0; phase < comm_sz; phase++) {
    partner = compute_partner(phase, my_rank, comm_sz);

    if (partner != MPI_PROC_NULL) {
      MPI_Sendrecv(local_arr_p, local_n, MPI_INT, partner, 0, partner_arr_p,
                   local_n, MPI_INT, partner, 0, comm,
                   MPI_STATUS_IGNORE); // both "partner" in args!
      if (my_rank < partner)
        merge_half(0, local_arr_p, partner_arr_p, temp_arr_p, local_n);
      else
        merge_half(1, local_arr_p, partner_arr_p, temp_arr_p, local_n);
    }
  }

  /**********************
   *  end of core code  *
   **********************/

  local_finish = MPI_Wtime();
  local_elapsed = local_finish - local_start;
  MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

  printf("my_rank is %d: %f s\n", my_rank, local_elapsed);
  if (my_rank == 0) {
    printf("Elapsed time of core code = %f seconds\n", elapsed);
  }

  MPI_Gather(local_arr_p, local_n, MPI_INT, arr_p, local_n, MPI_INT, 0, comm);

  free(local_arr_p);
  free(partner_arr_p);
  free(temp_arr_p);
}
