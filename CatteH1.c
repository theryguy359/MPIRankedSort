/*===================================================
            Spring 22 CSUN Department of Computer Science
            COMP 620-2 Computer System Architecture  
			
 Name		: CatteH1.c  
 Author     : Ryan Catterson
 Date       : March 2, 22
 
 Description: Multithreaded C program that reads n > 0 integers from any
			  input file, stores them in an array.
			  Next prints array k elements per line before and after
			  rank sort. Works when n is NOT evenly divisible by p.
			  Compiles and executes at the command prompt.
			  
			  To compile: mpicc CatteH1.c
			  To execute: mpiexec -n p a.out n k < anyinputfile
			  Example: mpiexec -n 5 a.out 11 4 < data.txt
 ===================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <mpi.h>
#include <string.h>  
#define mpc MPI_COMM_WORLD



void printArray(int *arr, int n, int k) {
  int i;
  for (i = 0; i < n; i++){
    printf("%5d ", arr[i]);
    if ((i+1) % k == 0) printf("\n");
  }
  printf("\n"); 
}


void readArray(int *arr, int n) {
  int i;
  for (i = 0; i < n; i++)
      scanf("%d", &arr[i]);

}

int main(int argc, char *argv[]){
	int n, k, procs, err, id, share, i, j;
	MPI_Status status; 
	err = MPI_Init(&argc, &argv); 
	if (err != MPI_SUCCESS){
		printf("\nError initializing MPI.\n");
		MPI_Abort(mpc, err);
	}
	MPI_Comm_size(mpc, &procs);
	MPI_Comm_rank(mpc, &id);

	
	if (id == 0 && argc < 3) {  
		printf("\n\tMultithreaded C program that reads n > 0 integers from" 
		"\n\tany input file, stores them in an array."
		"\n\tNext prints array k elements per line before and after"
		"\n\trank sort."
		"\n\tWorks when n is NOT evenly divisible by p."
		"\n\tCompiles and executes at the command prompt\n."
			  
		"\n\t\tTo compile: mpicc CatteH1.c"
		"\n\t\tTo execute: mpiexec -n p a.out n k < anyinputfile"
		"\n\t\tExample: mpiexec -n 5 a.out 10 4 < data.txt");

	}
	if(argc < 3) {
		MPI_Finalize();
		return 0;
	}
		
	n = atoi(argv[1]);
	k = atoi(argv[2]);
	share = n / procs;
	
	if(procs > n && id == 0) {
		printf("\nToo many processes entered.\nMake sure n >= p!\n\n");
	}
	if(procs > n) {
		MPI_Finalize();
		return 0;		
	}
	
	
	if (id == 0) {
		//initialize and read in the array from the file
		int *arr;
		arr = malloc (n * sizeof (int));		
		readArray(arr, n);
		printf("\nINPUT DATA before Rank Sort\n");
		printArray(arr, n, k);
		int * ranks = malloc (n * sizeof (int));
		int * b = malloc (n * sizeof (int));
		for(i = 0; i < n; i++) {
			b[i] = arr[i];
		}
		for(i = 1; i < procs; i++) {
			//sending the whole array to each process
			MPI_Send(arr, n, MPI_INT, i, 0, mpc);
		}
		for(i = 0; i < share; i++) {
			//calculating the ranks for process 0's portion
			int rank = 0;
			for(j = 0; j < n; j++) {
				if ((b[i] > b[j] && i != j) || (b[i] == b[j] && i > j)) {
					rank++;
				}
			
			ranks[i] = rank;
				
			}
		}
		for(i = 1; i < procs; i++) {
			//retrieving the ranks from the other processes and storing their information in ranks[]
			int inc = 0;
			int newShare = share;
			int start = i * share;
			int end = i * share + share;
			if(i == procs - 1) {
				//checks to see if n is divisible by p
				//if not, it will change the ending for the loop so that no items are missed
				newShare = n - (share * i);
				end = n;
			}
			int * tmp = malloc(newShare * sizeof(int));
			MPI_Recv(tmp, newShare, MPI_INT, i, i, mpc, &status);
			for(j = start; j < end; j++) {
				ranks[j] = tmp[inc];
				inc++;
			}
			free(tmp);
		}
		for(i = 0; i < n; i++){
			b[ranks[i]] = arr[i];
		}
		printf("\nINPUT DATA after Rank Sort\n");
		printArray(b, n, k);
		free(b);
		free(ranks);
		time_t t;  
		time(&t);	
		printf("\n\tRyan Catterson: %s", ctime(&t));	
		free(arr);
	} 
	else {
		
		int *tmp = malloc(n * sizeof(int));
		MPI_Recv(tmp, n, MPI_INT, 0, 0, mpc, &status);
		int inc = 0;
		int end = id * share + share;
		int start = share * id;
		if(id == procs - 1) {
			if(n - (share * id) != share) {
				//changes the end and share variables if this is the last process and if p does not divide evenly into n 
				share = n - (share * id);
				end = n;
			}
		}
		int * ranks = malloc(share * sizeof(int));
		for(i = start; i < end; i++) {
		
			int rank = 0;
		
			for(j = 0; j < n; j++) {
				if ((tmp[i] > tmp[j] && i != j) || (tmp[i] == tmp[j] && i > j)) {
					rank++;
				}
			}
			ranks[inc] = rank;
			inc++;
		}
		MPI_Send(ranks, share, MPI_INT, 0, id, mpc);
		free(tmp);
		free(ranks);
	}
	MPI_Finalize();
	return EXIT_SUCCESS;  
}
