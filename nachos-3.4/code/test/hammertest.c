/* matmult.c 
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"
#include "../userprog/syscall.h"


#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int
main()
{
   Exec("../test/matmult");

   Write("Matult is runing. calling sort\n", 30, ConsoleOutput);
   Exec("../test/sort");


   Write("Sort is running. \n", 30, ConsoleOutput);

   
}
