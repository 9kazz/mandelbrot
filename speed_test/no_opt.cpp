#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <x86intrin.h>
#include <cpuid.h> 

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define MAX_ITER    10

enum EndStatus 
{
    Success_end = 1,
    Failure_end = 0
};

int MandelCalculate (int width, int height);
inline uint64_t start_time(void);

int main()
{
    const int width  = 800;
    const int height = 800;

    MandelCalculate(width, height);

    return 0;   
}

int MandelCalculate(int width, int height)
{        
    volatile const float xMaxRef = 3;
    volatile const float yMaxRef = 3;

    volatile float xMax  = xMaxRef;
    volatile float yMax  = yMaxRef;

    volatile float x0ref = -xMax / 2;
    volatile float y0ref = -yMax / 2;

    volatile int N     = 0;
    volatile const int    Nmax  = 255;
    volatile const float  R2max = 2*2;

    volatile const float dx = xMax / width;
    volatile const float dy = yMax / height;

    volatile float x0 = x0ref;
    volatile float y0 = y0ref;

    volatile int xPix = 0;
    volatile int yPix = 0;


    uint64_t time_start = start_time();

    for (int iter_cnt = 0; iter_cnt < MAX_ITER ; iter_cnt++)
    {
        for (yPix = 0;  yPix < height;  yPix++, y0 += dy)
        {
            volatile float y = 0;       
             
            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix++, x0 += dx)
            {
                volatile float x = 0;

                for (N = 0; N <= Nmax; N++)
                {
                    volatile float y2 = y*y;
                    volatile float x2 = x*x;
                    volatile float xy = x*y;
                    volatile float R2 = x2 + y2;
                
                    x = x2 - y2 + x0;
                    y = 2*xy + y0;
                }
            }
        }
    }

    uint64_t time_end = _rdtsc();

    printf("============== NO OPT ==============\n");
    printf("Count of iterations:               %d\n",     MAX_ITER);
    printf("Billions Ticks for all iterations: %ld\n",   (time_end - time_start) / 1000000);
    printf("Billions Ticks for one iteration:  %.2lf\n", (time_end - time_start) / MAX_ITER / 1000000.0);
    printf("==========================================\n");
    
    return Success_end;
}

inline uint64_t start_time(void)
{
  int useless_param1 = 0, useless_param2 = 0, useless_param3 = 0, useless_param4 = 0;
  __cpuid( 0, useless_param1, useless_param2, useless_param3, useless_param4);

  return _rdtsc();
}