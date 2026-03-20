//
// Starting code for the portfolio exercise. Some required routines are included in a separate
// file (ending '_extra.h'); this file should not be altered, as it will be replaced with a different
// version for assessment.
//
// Compile as normal, e.g.,
//
// > gcc -o portfolioExercise portfolioExercise.c
//
// and launch with the problem size N and number of threads p as command line arguments, e.g.,
//
// > ./portfolioExercise 12 4
//


//
// Includes.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "portfolioExercise_extra.h"        // Contains routines not essential to the assessment.

typedef struct {
    int startRow;
    int endRow;
} calcRowArgs_t;

pthread_t *thread_ids;
calcRowArgs_t *calcRowArgs;

int rowsPerThread;
int N, nThreads;
float **M, *u, *v;
float dotProduct = 0.0f;        // You should leave the result of your calculation in this variable.
//
// Main.
//

// Matrix-vector multiplication Mu = v.
void* calculateRows( void* threadArgs )
{
    calcRowArgs_t *args = (calcRowArgs_t*) threadArgs;
        
    for(int row=args->startRow; row<=args->endRow; row++ ) 
    {
        for( int col=0; col<N; col++)
            v[row] += M[row][col] * u[col];
    }
        
    return NULL;
}

void threadPoolMult()
{
    // Initialise the global arrays used by the thread pool.
    thread_ids = (pthread_t*)  malloc( nThreads*sizeof(pthread_t) );
    calcRowArgs = (calcRowArgs_t*) malloc( nThreads*sizeof(calcRowArgs_t) );

    // Calculate how many rows each thread needs to calculate
    rowsPerThread = N / nThreads;

    // Launch each thread with one row.
    for( int t=0; t<nThreads; t++ )
    {
        calcRowArgs[t].startRow = t * rowsPerThread;
        calcRowArgs[t].endRow = calcRowArgs[t].startRow + rowsPerThread - 1;

        pthread_create( &thread_ids[t], NULL, calculateRows, &calcRowArgs[t] );
    }
    // Makes main thread wait so dot product is not reached before matrix multiplication is finished
    for( int t=0; t<nThreads; t++ )
    {
        pthread_join( thread_ids[t], NULL);
    }
}


void* calcDotProduct( void* threadArgs )
{   
    calcRowArgs_t *args = (calcRowArgs_t*) threadArgs;
        
    for( int row=args->startRow; row<=args->endRow; row++ ) 
    {   
        dotProduct += v[row] * v[row];
    }
        
    return NULL;
}
        

void threadPoolDot()
{
    // Launch each thread with one row.
    for( int t=0; t<nThreads; t++ )
    {
        pthread_create( &thread_ids[t], NULL, calcDotProduct, &calcRowArgs[t] );
    }
    // Make main thread wait until all threads are finished
    for( int t=0; t<nThreads; t++ )
    {
        pthread_join( thread_ids[t], NULL);
    }
}

int main( int argc, char **argv )
{
    //
    // Initialisation and set-up.
    //

    // Get problem size and number of threads from command line arguments.
    if( parseCmdLineArgs(argc,argv,&N,&nThreads)==-1 ) return EXIT_FAILURE;

    // Initialise (i.e, allocate memory and assign values to) the matrix and the vectors.
    if( initialiseMatrixAndVector(N,&M,&u,&v)==-1 ) return EXIT_FAILURE;

    // For debugging purposes; only display small problems (e.g., N=8 and nThreads=2 or 4).
    if( N<=12 ) displayProblem( N, M, u, v );

    // Start the timing now.
    struct timespec startTime, endTime;
    clock_gettime( CLOCK_REALTIME, &startTime );

    //
    // Parallel operations, timed.
    //

    // Step 1. Matrix-vector multiplication Mu = v.

    threadPoolMult();

    // After completing Step 1, you can uncomment the following line to display M, u and v, to check your solution so far.
    if( N<=12 ) displayProblem( N, M, u, v );

    // Step 2. The dot product of the vector v with itself.
    threadPoolDot();
    

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of parallel calculation: %f\n", dotProduct );

    //
    // Check against the serial calculation.
    //

    // Output final time taken.
    clock_gettime( CLOCK_REALTIME, &endTime );
    double seconds = (double)( endTime.tv_sec + 1e-9*endTime.tv_nsec - startTime.tv_sec - 1e-9*startTime.tv_nsec );
    printf( "Time for parallel calculations: %g secs.\n", seconds );

    // Step 1. Matrix-vector multiplication Mu = v.
    
    

    // Step 2: The dot product of the vector v with itself
    float dotProduct_serial = 0.0f;
    for( int i=0; i<N; i++ ) dotProduct_serial += v[i]*v[i];

    // DO NOT REMOVE OR MODIFY THIS PRINT STATEMENT AS IT IS REQUIRED BY THE ASSESSMENT.
    printf( "Result of the serial calculation: %f\n", dotProduct_serial );

    //
    // Clear up and quit.
    //
    freeMatrixAndVector( N, M, u, v );

    return EXIT_SUCCESS;
}