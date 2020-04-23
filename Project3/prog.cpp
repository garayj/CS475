#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>

// setting the number of threads:
#ifndef NUMT
#define NUMT 4
#endif

#ifndef MONTHSONLY
#define MONTHSONLY 0
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

float Ranf(unsigned int *, float, float);
int Ranf(unsigned int *, int, int);
// Add more to the typing
void Grain();
void Watcher();
void GrainDeer();
void MyAgent();
void InitBarrier(int);
void WaitBarrier();
float SQR(float);
// All State Variables
int NowYear;  // 2020 - 2025
int NowMonth; // 0 - 11

float NowPrecip; // inches of rain per month
float NowTemp;   // temperature this month

float NowHeight; // grain height in inches
int NowNumDeer;  // number of deer in the current population
int Something;
unsigned int seed = 0;

int main(int argc, char *argv[])
{
#ifndef _OPENMP
    fprintf(stderr, "No OpenMP support!\n");
    return 1;
#endif

    // starting date and time:
    NowMonth = 0;
    NowYear = 2020;

    // starting state
    NowNumDeer = 1;
    NowHeight = 1.;

    // compute a temporary next-value for this quantity
    // based on the current state of the simulation:

    float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);

    NowTemp = (AVG_TEMP - AMP_TEMP) * cos(ang) + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

    // Something = 1;
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
    if (NowPrecip < 0.)
    {
        NowPrecip = 0.;
    }

    if (MONTHSONLY)
    {
        printf("Month\tTemp (C)\tPrecip(cm)\tHeight(cm)\tDeer\n");
        printf("%d\t%.2lf\t%.2lf\t%.2lf\t%d\n", NowMonth, (5. / 9.) * (NowTemp - 32), NowPrecip * 2.54, NowHeight * 2.54, NowNumDeer);
    }
    else
    {
        printf("Year\tMonth\tTemp\tPrecip\tHeight\tDeer\n");
        printf("%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%d\n", NowYear, NowMonth, (5. / 9.) * (NowTemp - 32), NowPrecip * 2.54, NowHeight * 2.54, NowNumDeer);
    }

    omp_set_num_threads(NUMT); // same as # of sections
    InitBarrier(NUMT);
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

void GrainDeer()
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:

        // Create a flag for the increment or decrement of the number of deer.
        bool enoughFoodForMoreDeer = NowHeight > (float)NowNumDeer;
        WaitBarrier();

        // After the DoneComputing barrier, assign new NowNumDeer.
        if (enoughFoodForMoreDeer)
        {
            NowNumDeer++;
        }
        else if (NowNumDeer > 0)
        {
            NowNumDeer--;
        }

        // DoneAssigning barrier:
        WaitBarrier();

        // Do nothing
        WaitBarrier();
    }
}

void Grain()
{
    while (NowYear < 2026)
    {
        // Calculate factors
        float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
        float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));

        // DoneComputing barrier:
        WaitBarrier();

        // Assign new NowHeight.
        NowHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        NowHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
        if (NowHeight < 0.0)
        {
            NowHeight = 0.0;
        }

        // DoneAssigning barrier:
        WaitBarrier();

        // Do nothing
        WaitBarrier();
    }
}

void Watcher()
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:
        WaitBarrier();

        // DoneAssigning barrier:
        WaitBarrier();

        // NowYear += 1;
        // NowMonth += 1;

        if (MONTHSONLY)
        {
            NowMonth++;
            if (NowMonth % 12 == 0)
            {
                NowYear++;
            }
        }
        else
        {
            NowMonth = (NowMonth + 1) % 12;
            if (NowMonth == 0)
            {
                NowYear++;
            }
        }
        float threadAng = (30. * (float)NowMonth + 15.) * (M_PI / 180.);

        float temp = AVG_TEMP - AMP_TEMP * cos(threadAng);
        NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

        float threadPrecip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(threadAng);
        NowPrecip = threadPrecip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
        if (NowPrecip < 0.)
            NowPrecip = 0.;
        if (MONTHSONLY)
        {
            printf("%d\t%.2lf\t%.2lf\t%.2lf\t%d\n", NowMonth, (5. / 9.) * (NowTemp - 32), NowPrecip * 2.54, NowHeight * 2.54, NowNumDeer);
        }
        else
        {
            printf("%d\t%d\t%.2lf\t%.2lf\t%.2lf\t%d\n", NowYear, NowMonth, (5. / 9.) * (NowTemp - 32), NowPrecip * 2.54, NowHeight * 2.54, NowNumDeer);
        }
        WaitBarrier();
    }
}

void MyAgent()
{
    while (NowYear < 2026)
    {
        // DoneComputing barrier:
        Something++;
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

float SQR(float x)
{
    return x * x;
}
