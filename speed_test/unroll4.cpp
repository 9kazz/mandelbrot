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

#define SIMD_GROUP  4
#define MAX_ITER    1000

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

    volatile const int   Nmax  = 255;
    volatile const float R2max = 2*2;

    volatile int screen_size = MIN(width, height);
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
            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix += SIMD_GROUP, x0 += SIMD_GROUP * dx)
            {                   
                volatile float Y[SIMD_GROUP] = {0};   
                volatile float X[SIMD_GROUP] = {0};

                volatile int    N[SIMD_GROUP] = {0};     
                volatile int Nres[SIMD_GROUP] = {-1, -1, -1, -1};

                volatile float Y2[SIMD_GROUP] = {0};  
                volatile float X2[SIMD_GROUP] = {0};  
                volatile float XY[SIMD_GROUP] = {0};  
                volatile float R2[SIMD_GROUP] = {0};  
                
                for (int DotEscapeMask = 0; DotEscapeMask != 0b1111; )
                {
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { Y2[data_cnt] =  Y[data_cnt] *  Y[data_cnt]; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { X2[data_cnt] =  X[data_cnt] *  X[data_cnt]; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { XY[data_cnt] =  X[data_cnt] *  Y[data_cnt]; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { R2[data_cnt] = X2[data_cnt] + Y2[data_cnt]; }

                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { if (R2[data_cnt] > R2max && Nres[data_cnt] == -1)  Nres[data_cnt] = N[data_cnt]; }
                
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { X[data_cnt] = X2[data_cnt] - Y2[data_cnt] + x0 + dx * data_cnt; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { Y[data_cnt] = 2 * XY[data_cnt] + y0 + dy * data_cnt; }

                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { N[data_cnt]++ ; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { if (N[data_cnt] >= Nmax && Nres[data_cnt] == -1)  Nres[data_cnt] = Nmax; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { if (Nres[data_cnt] != -1)  DotEscapeMask |= (1 << data_cnt); }
                }
            }
        }
    }
    
    uint64_t time_end = _rdtsc();

    printf("============== UNROLL4 ==============\n");
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