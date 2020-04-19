#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>

// setting the number of threads:
#ifndef NUMT
#define NUMT 4
#endif

// Constants
const float GRAIN_GROWS_PER_MONTH = 9.0;
const float ONE_DEER_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH = 7.0; // average
const float AMP_PRECIP_PER_MONTH = 6.0; // plus or minus
const float RANDOM_PRECIP = 2.0;        // plus or minus noise

const float AVG_TEMP = 60.0;    // average
const float AMP_TEMP = 20.0;    // plus or minus
const float RANDOM_TEMP = 10.0; // plus or minus noise

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

omp_lock_t Lock;
int NumInThreadTeam;
int NumAtBarrier;
int NumGone;

float Randf(unsigned int *, float, float);
int Randf(unsigned int *, int, int);
void Grain();
void Watcher();
void GrainDeer();
void MyAgent();
void InitBarrier(int);
void WaitBarrier();

int main(int argc, char *argv[])
{
#ifndef _OPENMP
    fprintf(stderr, "No OpenMP support!\n");
    return 1;
#endif

    // All State Variables
    int NowYear;  // 2020 - 2025
    int NowMonth; // 0 - 11

    float NowPrecip; // inches of rain per month
    float NowTemp;   // temperature this month
    float NowHeight; // grain height in inches
    int NowNumDeer;  // number of deer in the current population

    // starting date and time:
    NowMonth = 0;
    NowYear = 2020;

    // starting state
    NowNumDeer = 1;
    NowHeight = 1.;

    // compute a temporary next-value for this quantity
    // based on the current state of the simulation:

    omp_set_num_threads(NUMT); // same as # of sections
    InitBarrier(NUMT);

    unsigned int seed = 0; // a thread-private variable
    float x = Ranf(&seed, -1.f, 1.f);

// Start Parrots
#pragma omp parallel sections
    {
#pragma omp section
        {
            GrainDeer();
        }

#pragma omp section
        {
            Grain();
        }

#pragma omp section
        {
            Watcher();
        }

#pragma omp section
        {
            MyAgent(); // your own
        }
    } // implied barrier -- all functions must return in order
      // to allow any of them to get past here
}

void GrainDeer(int NowYear)
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:
        WaitBarrier();

        // DoneAssigning barrier:
        WaitBarrier();

        // DonePrinting barrier:
        WaitBarrier();
    }
}

void Grain(int NowYear)
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:
        WaitBarrier();

        // DoneAssigning barrier:
        WaitBarrier();

        // DonePrinting barrier:
        WaitBarrier();
    }
}

void Watcher(int NowYear)
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:
        WaitBarrier();

        // DoneAssigning barrier:
        WaitBarrier();

        // DonePrinting barrier:
        WaitBarrier();
    }
}

void MyAgent(int NowYear)
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:
        WaitBarrier();

        // DoneAssigning barrier:
        WaitBarrier();

        // DonePrinting barrier:
        WaitBarrier();
    }
}

void InitBarrier(int n)
{
    NumInThreadTeam = n;
    NumAtBarrier = 0;
    omp_init_lock(&Lock);
}

// have the calling thread wait here until all the other threads catch up:

void WaitBarrier()
{
    omp_set_lock(&Lock);
    {
        NumAtBarrier++;
        if (NumAtBarrier == NumInThreadTeam)
        {
            NumGone = 0;
            NumAtBarrier = 0;
            // let all other threads get back to what they were doing
            // before this one unlocks, knowing that they might immediately
            // call WaitBarrier( ) again:
            while (NumGone != NumInThreadTeam - 1)
                ;
            omp_unset_lock(&Lock);
            return;
        }
    }
    omp_unset_lock(&Lock);

    while (NumAtBarrier != 0)
        ; // this waits for the nth thread to arrive

#pragma omp atomic
    NumGone++; // this flags how many threads have returned
}

float Ranf(unsigned int *seedp, float low, float high)
{
    float r = (float)rand_r(seedp); // 0 - RAND_MAX

    return (low + r * (high - low) / (float)RAND_MAX);
}

int Ranf(unsigned int *seedp, int ilow, int ihigh)
{
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;

    return (int)(Ranf(seedp, low, high));
}
