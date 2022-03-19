/* File:     mpi_trap1.c
 * Purpose:  Use MPI to implement a parallel version of the trapezoidal
 *           rule.  In this version the endpoints of the interval and
 *           the number of trapezoids are hardwired.
 *
 * Input:    None.
 * Output:   Estimate of the integral from a to b of f(x)
 *           using the trapezoidal rule and n trapezoids.
 *
 * Compile:  mpicc -g -Wall -o mpi_trap1 mpi_trap1.c
 * Run:      mpiexec -n <number of processes> ./mpi_trap1
 *
 * Algorithm:
 *    1.  Each process calculates "its" interval of
 *        integration.
 *    2.  Each process estimates the integral of f(x)
 *        over its interval using the trapezoidal rule.
 *    3a. Each process != 0 sends its integral to 0.
 *    3b. Process 0 sums the calculations received from
 *        the individual processes and prints the result.
 *
 * Note:  f(x), a, b, and n are all hardwired.
 *
 * IPP:   Section 3.2.2 (pp. 96 and ff.)
 *
 * Plotar função online: https://www.desmos.com/calculator?lang=pt-BR
 *     f(x)=x^2-4x +8
 *     a = 0
 *     b = 3
 *     0 <= y <= f(x){(a-x)(b-x)<0}
 */
#include <stdio.h>
#include <stdlib.h>

/* We'll be using MPI routines, definitions, etc. */
#include <mpi.h>

/* Usando os includes para PTHREAD */
#include <pthread.h>
#include <semaphore.h>

// parallel specific variables
int n;
double a, b, h;

int thread_count;
double GLOBAL_MUTEX_SUM = 0;
pthread_mutex_t mutex;

//função do trapezoide para o calculo da integral
void* mutex_Trap(void* rank);

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n)  (BLOCK_LOW((id)+1,p,n)-1)


/* Function we're integrating */
double f(double x);

int main(void) {
   n = 10000000;
   a = 1.0;
   b = 5.0;

   h = (b-a)/n;  /* h is the same for all processes */

  /* chamada do mutex trapezoidal com 2 threads */

  thread_count = 2; // numeros de THREDS
  pthread_t* thread_handles;
  GLOBAL_MUTEX_SUM = 0; // iniciando a variavel mutex global

  // Criando thread handles e inicando o mutex
  pthread_mutex_init(&mutex, NULL);
  thread_handles = malloc( thread_count * sizeof(pthread_t));

  // criando as pthreads na funcao mutex trap
  long thread;
  for( thread=0; thread < thread_count; thread++)
    pthread_create( &thread_handles[thread], NULL, mutex_Trap,
      (void*) thread );

  // unindo todas as thread handles
  for( thread=0; thread < thread_count; thread++)
    pthread_join( thread_handles[thread], NULL);

  // free thread handles and mutex
  free(thread_handles);
  pthread_mutex_destroy(&mutex);

  // imprimindo
  printf("----- Numero de threads: %d -----\n", thread_count);
  printf("With n = %ld trapezoids, our estimate\n", n);
  printf("of the integral from %f to %f = %.15f\n",
    a, b, GLOBAL_MUTEX_SUM);

   return 0;
} /*  main  */


void* mutex_Trap(void* rank) // função TRAP adaptada para pthreds
{
  long thread_rank = (long)rank;
  double local_int = 0.0;
  long long i;
  int especial_case = (int)thread_rank;

  // allocate a chunk of work to the thread
  long long local_a = BLOCK_LOW(thread_rank, thread_count, n);
  long long local_b = BLOCK_HIGH(thread_rank, thread_count, n);


  if( especial_case == 1)
  {
    local_int += (f(a)+f(b))/2.0;
  }

  // calculo pela forma do trapezoid
  for( i= local_a; i <= local_b; i++)
  {
    local_int += f(a+(i*h));
  }
  local_int = local_int * h;

  // update das variaveis globais
  pthread_mutex_lock(&mutex);
  GLOBAL_MUTEX_SUM += local_int;
  pthread_mutex_unlock(&mutex);

  return NULL;

}

/*------------------------------------------------------------------
 * Function:    f
 * Purpose:     Compute value of function to be integrated
 * Input args:  x
 */
double f(double x) {
   return x*x - 4*x + 8;
   //return x*x;
} /* f */

