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

#define SIMD_GROUP   8
#define MAX_ITER     5

enum EndStatus 
{
    Success_end = 1,
    Failure_end = 0
};

int MandelCalculate (int width, int height, FILE* data_base);
inline uint64_t start_time(void);

int width  = 800;
int height = 800;

float xMaxRef = 3;
float yMaxRef = 3;

float xMax  = xMaxRef;
float yMax  = yMaxRef;

float x0_ref = -xMax / 2;
float y0_ref = -yMax / 2;

int   Nmax  = 255;
float R2max = 2*2;

int screen_size = MIN(width, height);
float dx = xMax / width;
float dy = yMax / height;
    
float x0_ = x0_ref;
float y0_ = y0_ref;

int xPix = 0;
int yPix = 0;

float Y[SIMD_GROUP] = {0};   
float X[SIMD_GROUP] = {0};

int    N[SIMD_GROUP] = {0};     
int Nres[SIMD_GROUP] = {0, 0, 0, 0, 0, 0, 0, 0};

float Y2[SIMD_GROUP] = {0};  
float X2[SIMD_GROUP] = {0};  
float XY[SIMD_GROUP] = {0};  
float R2[SIMD_GROUP] = {0};  

int main()
{
    int max_test_cnt = 5;

    #ifdef O3
    FILE* data_base  = fopen("data/unroll8_O3.csv", "w");
    #elif O2
    FILE* data_base  = fopen("data/unroll8_O2.csv", "w");
    #else
    FILE* data_base  = fopen("data/unroll8_O0.csv", "w");

    #endif

    for (int test_cnt = 0; test_cnt < max_test_cnt; test_cnt++)
        MandelCalculate(width, height, data_base);

    return 0;   
}

int MandelCalculate(int width, int height, FILE* data_base)
{        
    xMaxRef = 3;
    yMaxRef = 3;

    xMax  = xMaxRef;
    yMax  = yMaxRef;

    x0_ref = -xMax / 2;
    y0_ref = -yMax / 2;

    Nmax  = 255;
    R2max = 2*2;

    screen_size = MIN(width, height);
    dx = xMax / width;
    dy = yMax / height;
        
    x0_ = x0_ref;
    y0_ = y0_ref;

    xPix = 0;
    yPix = 0;

    uint64_t time_start = start_time();

    for (int iter_cnt = 0; iter_cnt < MAX_ITER; iter_cnt++)
    {
        for (yPix = 0;  yPix < height;  yPix++, y0_ += dy)
        {
            for (xPix = 0, x0_ = x0_ref;  xPix < width;  xPix += SIMD_GROUP, x0_ += SIMD_GROUP * dx)
            {                   
                Y[SIMD_GROUP] = {0};   
                X[SIMD_GROUP] = {0};

                N[SIMD_GROUP] = {0};     
                
                for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { Nres[data_cnt] = 0; }

                Y2[SIMD_GROUP] = {0};  
                X2[SIMD_GROUP] = {0};  
                XY[SIMD_GROUP] = {0};  
                R2[SIMD_GROUP] = {0};  
                
                for (int DotEscapeMask = 0; DotEscapeMask != 0b11111111; )
                {
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { Y2[data_cnt] =  Y[data_cnt] *  Y[data_cnt]; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { X2[data_cnt] =  X[data_cnt] *  X[data_cnt]; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { XY[data_cnt] =  X[data_cnt] *  Y[data_cnt]; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { R2[data_cnt] = X2[data_cnt] + Y2[data_cnt]; }

                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { if (R2[data_cnt] > R2max && Nres[data_cnt] == 0)  Nres[data_cnt] = N[data_cnt]; }
                
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { X[data_cnt] = X2[data_cnt] - Y2[data_cnt] + x0_ + dx * data_cnt; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { Y[data_cnt] = 2 * XY[data_cnt] + y0_ + dy * data_cnt; }

                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { N[data_cnt]++ ; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { if (N[data_cnt] >= Nmax && Nres[data_cnt] == 0)  Nres[data_cnt] = Nmax; }
                    for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { if (Nres[data_cnt] != 0)  DotEscapeMask |= (1 << data_cnt); }
                }
            }
        }
    }
    
    uint64_t time_end = _rdtsc();

    // printf("============== UNROLL8 ==============\n");
    // printf("Count of iterations:               %d\n",     MAX_ITER);
    // printf("Billions Ticks for all iterations: %ld\n",   (time_end - time_start) / 1000000);
    // printf("Billions Ticks for one iteration:  %.2lf\n", (time_end - time_start) / MAX_ITER / 1000000.0);
    // printf("==========================================\n");

    fprintf(data_base, "%d\t",     MAX_ITER);
    fprintf(data_base, "%ld\t",   (time_end - time_start) / 1000000);
    fprintf(data_base, "%.2lf\n", (time_end - time_start) / MAX_ITER / 1000000.0);
    
    volatile float dummy = 0.0f;
    for (int i = 0; i < SIMD_GROUP; i++) {
        dummy += Y[i] + X[i] + Nres[i];
    }
    fprintf(stderr, "%f\n", dummy); // или в data_base, но лучше отдельно

    return Success_end;
}

inline uint64_t start_time(void)
{
  int useless_param1 = 0, useless_param2 = 0, useless_param3 = 0, useless_param4 = 0;
  __cpuid( 0, useless_param1, useless_param2, useless_param3, useless_param4);

  return _rdtsc();
}
