#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>
#include <omp.h>

#define SSE_WIDTH 4

#ifndef NUMT
#define NUMT 8
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH 10000000
#endif

#ifndef NUM_TRIALS
#define NUM_TRIALS 100
#endif
#define NUM_ELEMENTS_PER_CORE ARRAY_LENGTH / NUMT

float SimdMulSum(float *a, float *b, int len)
{
	float sum[4] = {0., 0., 0., 0.};
	int limit = (len / SSE_WIDTH) * SSE_WIDTH;
	register float *pa = a;
	register float *pb = b;

	__m128 ss = _mm_loadu_ps(&sum[0]);
	for (int i = 0; i < limit; i += SSE_WIDTH)
	{
		ss = _mm_add_ps(ss, _mm_mul_ps(_mm_loadu_ps(pa), _mm_loadu_ps(pb)));
		pa += SSE_WIDTH;
		pb += SSE_WIDTH;
	}
	_mm_storeu_ps(&sum[0], ss);

	for (int i = limit; i < len; i++)
	{
		sum[0] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}

float nonSseMulSumPerformance(float *a, float *b)
{
	float c;

	double time0 = omp_get_wtime();
	for (int i = 0; i < ARRAY_LENGTH; i++)
	{
		c += a[i] * b[i];
	}
	double time1 = omp_get_wtime();
	return (double)ARRAY_LENGTH / (time1 - time0) / 1000000.;
}

float *createArray()
{
	float *newArr = new float[ARRAY_LENGTH];
	for (int i = 0; i < ARRAY_LENGTH; ++i)
	{
		// Found on stack overflow: https://stackoverflow.com/questions/686353/random-float-number-generation
		newArr[i] = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	}
	return newArr;
}

double sseMulSumPerformance(float *a, float *b)
{
	double time0 = omp_get_wtime();
	SimdMulSum(a, b, ARRAY_LENGTH);
	double time1 = omp_get_wtime();
	return (float)ARRAY_LENGTH / (time1 - time0) / 1000000.;
}

double multiThreadSseMulSumPerformance(float *a, float *b)
{

	double time0 = omp_get_wtime();
#pragma omp parallel
	{
		int first = omp_get_thread_num() * NUM_ELEMENTS_PER_CORE;
		SimdMulSum(&a[first], &b[first], NUM_ELEMENTS_PER_CORE);
	}
	double time1 = omp_get_wtime();
	return (double)ARRAY_LENGTH / (time1 - time0) / 1000000.;
}

int main()
{
#ifndef _OPENMP
	fprintf(stderr, "OpenMP is not available\n");
	return 1;
#endif
	srand(time(NULL));
	float *a = createArray();
	float *b = createArray();
	double bestSseRun = 0.;
	double bestNonSseRun = 0.;
	double bestMultiThreadSseRun = 0.;
	omp_set_num_threads(NUMT);

	for (int i = 0; i < NUM_TRIALS; i++)
	{
		// Run sse and non sse runs.
		double sseMulSum = sseMulSumPerformance(a, b);
		double nonSseMulSum = nonSseMulSumPerformance(a, b);
		double multiThreadSseMulSum = multiThreadSseMulSumPerformance(a, b);

		// Check if the run was the best perfomance.
		if (sseMulSum > bestSseRun)
		{
			bestSseRun = sseMulSum;
		}

		if (nonSseMulSum > bestNonSseRun)
		{
			bestNonSseRun = nonSseMulSum;
		}
		if (multiThreadSseMulSum > bestMultiThreadSseRun)
		{
			bestMultiThreadSseRun = multiThreadSseMulSum;
		}
	}
	delete a;
	delete b;
	printf("Array size:\t%d\tBest SSE run:\t%.2lf\tBest non-SSE Run:\t%.2lf\tBest multithreaded run:\t%.2lf\n", ARRAY_LENGTH, bestSseRun, bestNonSseRun, bestMultiThreadSseRun);
	return 0;
}
