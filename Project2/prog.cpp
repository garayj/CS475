#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

#define XMIN -1.
#define XMAX 1.
#define YMIN -1.
#define YMAX 1.
#define N 2

// setting the number of threads:
#ifndef VOL
#define VOL 0
#endif

// setting the number of threads:
#ifndef NUMNODES
#define NUMNODES 10
#endif

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

    // the area of a single full-sized tile:

    float fullTileArea = (((XMAX - XMIN) / (float)(NUMNODES - 1)) *
                          ((YMAX - YMIN) / (float)(NUMNODES - 1)));

    // sum up the weighted heights into the variable "volume"
    // using an OpenMP for loop and a reduction:
    float volume = 0.0f;
    float maxPerformance = 0.; // must be declared outside the NUMTRIES loop
    double time0 = omp_get_wtime();
#pragma omp parallel for default(none) shared(fullTileArea) reduction(+ \
                                                                      : volume)

    for (int i = 0; i < NUMNODES * NUMNODES; i++)
    {
        int iu = i % NUMNODES;
        int iv = i / NUMNODES;
        float z = Height(iu, iv);
        if (iv == 0 || iu == NUMNODES)
        {
            z /= 2.;
        }
        if (iu == 0 || iu == NUMNODES - 1)
        {
            z /= 2.;
        }
        volume += z * fullTileArea;
    }
    double time1 = omp_get_wtime();
    volume *= 2;
    double megaHeightsComputedPerSecond = (double)NUMNODES * (double)NUMNODES / (time1 - time0) / 1000000.;

    if (VOL)
    {
        printf("%.2f\t", volume);
    }
    else
    {
        printf("%8.2lf\t", megaHeightsComputedPerSecond);
    }
}

float Height(int iu, int iv) // iu,iv = 0 .. NUMNODES-1
{
    float x = -1. + 2. * (float)iu / (float)(NUMNODES - 1); // -1. to +1.
    float y = -1. + 2. * (float)iv / (float)(NUMNODES - 1); // -1. to +1.

    float xn = pow(fabs(x), (double)N);
    float yn = pow(fabs(y), (double)N);
    float r = 1. - xn - yn;
    if (r < 0.)
        return 0.;
    float height = pow(1. - xn - yn, 1. / (float)N);
    return height;
}
