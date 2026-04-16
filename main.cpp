#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>

#include "raylib.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define SIMD_GROUP  4

enum EndStatus 
{
    Success_end = 1,
    Failure_end = 0
};

int MandelDraw      (int width, Texture2D* MandelTexture, int frame_rate);
int MandelCalculate (int width, int height, Color* Pixels_buf, Texture2D* MandelTexture);

int main()
{
    const int width  = 1500;
    const int height = 1500;
    
    InitWindow(width, height, "The Mandelbrot Set");

    Image     MandelImg     = GenImageColor(width, height, WHITE);
    Texture2D MandelTexture = LoadTextureFromImage(MandelImg);
    Color*    Pixels_buf    = (Color*) calloc(width * height, sizeof(Color));   // WIDTH * HEIGHT + SIMD_GROUP

    MandelCalculate(width, height, Pixels_buf, &MandelTexture);

    UnloadImage(MandelImg);
    UnloadTexture(MandelTexture);
    free(Pixels_buf);

    CloseWindow();

    return 0;   
}

int MandelCalculate(int width, int height, Color* Pixels_buf, Texture2D* MandelTexture) 
{        
    const double xMaxRef = 3;
    const double yMaxRef = 3;

    double xMax  = xMaxRef;
    double yMax  = yMaxRef;

    double x0ref = -xMax / 2;
    double y0ref = -yMax / 2;

    double xZoom  = 0;
    double yZoom  = 0;
    double xShift = 0;
    double yShift = 0;

    while (!WindowShouldClose()) 
    {
        if(IsKeyDown(KEY_P))  { xMax -= xZoom;  yMax -= yZoom;  x0ref += xZoom / 2;  y0ref += yZoom / 2; }
        if(IsKeyDown(KEY_M))  { xMax += xZoom;  yMax += yZoom;  x0ref -= xZoom / 2;  y0ref -= yZoom / 2; }

        xZoom  = xMax / 30;
        yZoom  = yMax / 30;
        xShift = xMax / 50;
        yShift = yMax / 50;

        int screen_size = MIN(width, height);
        const double dx  = xMax / screen_size;
        const double dy  = yMax / screen_size;
            
        if(IsKeyDown(KEY_LEFT))   x0ref -= xShift;
        if(IsKeyDown(KEY_RIGHT))  x0ref += xShift;
        if(IsKeyDown(KEY_UP))     y0ref -= yShift;
        if(IsKeyDown(KEY_DOWN))   y0ref += yShift;
        
        double x0 = x0ref;
        double y0 = y0ref;

        int xPix = 0;
        int yPix = 0;

        double time_start = GetTime();

        __m256d DotShift = _mm256_set_pd (3.0, 2.0, 1.0, 0.0);
        __m256d DotInc   = _mm256_set1_pd(1.0);
        __m256d ScaleX   = _mm256_set1_pd(dx);
        __m256d R2max    = _mm256_set1_pd(4.0);
        __m256d Nmax     = _mm256_set1_pd(255);
        __m256d Zero     = _mm256_set1_pd(0.0);

        #pragma omp parallel for
        for (yPix = 0;  yPix < height;  yPix++)
        {
            y0 = y0ref + yPix * dy;
            __m256d Y0 = _mm256_set1_pd(y0);
            
            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix += SIMD_GROUP, x0 += SIMD_GROUP * dx)
            {                   
                __m256d X0     = _mm256_set1_pd(x0);
                __m256d X0shft = _mm256_add_pd( X0, _mm256_mul_pd(DotShift, ScaleX) );

                __m256d X    = _mm256_set1_pd(0.0);
                __m256d Y    = _mm256_set1_pd(0.0);
  
                __m256d N    = _mm256_set1_pd(0.0);
                __m256d Nres = _mm256_set1_pd(0.0);

                __m256d R2res = _mm256_set1_pd(0.0);

                for (int IsDotLeft = 0; ! IsDotLeft; )
                {
                    __m256d X2 = _mm256_mul_pd(X,  X);
                    __m256d Y2 = _mm256_mul_pd(Y,  Y);
                    __m256d XY = _mm256_mul_pd(X,  Y);
                    __m256d R2 = _mm256_add_pd(X2, Y2);

                    __m256d CmpMask1 = _mm256_cmp_pd (R2, R2max, _CMP_GT_OS);
                    __m256d CmpMask2 = _mm256_cmp_pd (Nres, Zero, _CMP_EQ_OS);
                           CmpMask1 = _mm256_and_pd (CmpMask1, CmpMask2);

                    Nres  = _mm256_or_pd( Nres,  _mm256_and_pd(N, CmpMask1) );             
                    R2res = _mm256_or_pd( R2res, _mm256_and_pd(R2, CmpMask1) );  

                    X = _mm256_add_pd( _mm256_sub_pd(X2, Y2), X0shft );
                    Y = _mm256_add_pd( _mm256_add_pd(XY, XY), Y0 );

                    N = _mm256_add_pd(N, DotInc);

                    CmpMask1  = _mm256_cmp_pd (N, Nmax, _CMP_GE_OS);
                    CmpMask2  = _mm256_cmp_pd (Nres, Zero, _CMP_EQ_OS);
                    CmpMask1  = _mm256_and_pd (CmpMask1, CmpMask2);                    
                    Nres      = _mm256_or_pd  (Nres, _mm256_and_pd(N, CmpMask1));
                    R2res     = _mm256_or_pd( R2res, _mm256_and_pd(R2, CmpMask1) );  
 
                    CmpMask2  = _mm256_cmp_pd (Nres, Zero, _CMP_EQ_OS);
                    IsDotLeft = _mm256_testz_pd(CmpMask2, CmpMask2);
                }

                for (int data_cnt = 0;  data_cnt < SIMD_GROUP;  data_cnt++) 
                {
                    int Rval = (int) R2res[data_cnt] * 20;
                    Pixels_buf[yPix * width + xPix + data_cnt] = {0, 0, Rval, 255};

                    // int Nval = (int) Nres[data_cnt];
                    // Pixels_buf[yPix * width + xPix + data_cnt] = {Nval, Nval, Nval, 255};
                }
            }
        }

        double time_end   = GetTime();
        int    frame_rate = round( 1 / (time_end - time_start) );

        UpdateTexture (*MandelTexture, Pixels_buf);
        MandelDraw    (width, MandelTexture, frame_rate);
    }

    return Success_end;
}


int MandelDraw(int width, Texture2D* MandelTexture, int frame_rate)
{
    const int frame_str_size = 17;
    char frame_str[frame_str_size] = {0};

    BeginDrawing();

    ClearBackground (WHITE);
    DrawTexture     (*MandelTexture, 0, 0, WHITE);

    snprintf(frame_str, frame_str_size, "Frame rate: %d", frame_rate);
    
    int text_width  = 300;
    int text_height = 30;

    DrawRectangle (width - text_width,  0,  text_width,  2 * text_height,  BLUE);
    DrawText      (frame_str,  width - text_width + text_height,  text_height / 3,  text_height,  BLACK);
    
    EndDrawing();

    return Success_end;
}