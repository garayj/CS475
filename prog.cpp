#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

// setting the number of threads:
#ifndef NUMT
#define NUMT 1
#endif

float Height(int, int);

int main(int argc, char *argv[])
{
#ifndef _OPENMP
    fprintf(stderr, "No OpenMP support!\n");
    return 1;
#endif
    omp_set_num_threads(NUMT); // set the number of threads to use in the for-loop:`

    double time0 = omp_get_wtime();
    int placeholder = 0
#pragma omp parallel for default(none) shared(fullTileArea) reduction(+ \
                                                                      : volume)

        for (int i = 0; i < placeholder; i++)
    {
        // Do something
    }
    double time1 = omp_get_wtime();
}
