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

    volatile int screen_size = MIN(width, height);
    volatile const float dx  = xMax / screen_size;
    volatile const float dy  = yMax / screen_size;

    volatile float x0 = x0ref;
    volatile float y0 = y0ref;

    volatile int xPix = 0;
    volatile int yPix = 0;
    
    volatile __m128 DotShift = _mm_set_ps (0.0, 1.0, 2.0, 3.0);
    volatile __m128 DotInc   = _mm_set1_ps(1.0);
    volatile __m128 ScaleX   = _mm_set1_ps(dx);
    volatile __m128 R2max    = _mm_set1_ps(4.0);
    volatile __m128 Nmax     = _mm_set1_ps(255);
    volatile __m128 Zero     = _mm_set1_ps(0.0);
    
    uint64_t time_start = start_time();

    for (int iter_cnt = 0; iter_cnt < MAX_ITER ; iter_cnt++)
    {
        for (yPix = 0;  yPix < height;  yPix++, y0 += dy)
        {
            volatile __m128 Y0 = _mm_set1_ps(y0);

            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix += SIMD_GROUP, x0 += SIMD_GROUP * dx)
            {                   
                volatile __m128 X0   = _mm_set1_ps(x0);

                volatile __m128 X    = _mm_set1_ps(0.0);
                volatile __m128 Y    = _mm_set1_ps(0.0);
  
                volatile __m128 N    = _mm_set1_ps(0.0);
                volatile __m128 Nres = _mm_set1_ps(0.0);

                for (int IsDotLeft = 0; ! IsDotLeft; )
                {
                    volatile __m128 X2 = _mm_mul_ps(X,  X);
                    volatile __m128 Y2 = _mm_mul_ps(Y,  Y);
                    volatile __m128 XY = _mm_mul_ps(X,  Y);
                    volatile __m128 R2 = _mm_add_ps(X2, Y2);

                    volatile __m128 CmpMask1 = _mm_cmp_ps (R2, R2max, _CMP_GT_OS);
                    volatile __m128 CmpMask2 = _mm_cmp_ps (Nres, Zero, _CMP_EQ_OS);
                                    CmpMask1 = _mm_and_ps (CmpMask1, CmpMask2);

                    Nres = _mm_or_ps( Nres, _mm_and_ps(N, CmpMask1) );             

                    X = _mm_add_ps( _mm_sub_ps(X2, Y2), _mm_add_ps(X0, _mm_mul_ps(DotShift, ScaleX)) );
                    Y = _mm_add_ps( _mm_add_ps(XY, XY), Y0 );

                    N = _mm_add_ps(N, DotInc);

                    CmpMask1  = _mm_cmp_ps (N, Nmax, _CMP_GE_OS);
                    CmpMask2  = _mm_cmp_ps (Nres, Zero, _CMP_EQ_OS);
                    CmpMask1  = _mm_and_ps (CmpMask1, CmpMask2);                    
                    Nres      = _mm_or_ps  (Nres, _mm_and_ps(N, CmpMask1));
 
                    CmpMask2  = _mm_cmp_ps (Nres, Zero, _CMP_EQ_OS);
                    IsDotLeft = _mm_testz_ps(CmpMask2, CmpMask2);
                }
            }
        }
    }

    uint64_t time_end = _rdtsc();

    printf("============== INTRIN_XMM ==============\n");
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