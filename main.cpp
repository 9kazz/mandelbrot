#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>

#include "raylib.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define SIMD_GROUP  8

enum EndStatus 
{
    Success_end = 1,
    Failure_end = 0
};

int MandelDraw      (int width, Texture2D* MandelTexture, int frame_rate);
int MandelCalculate (int width, int height, Color* Pixels_buf, Texture2D* MandelTexture);

int main()
{
    const int width  = 1200;
    const int height = 1200;
    
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
    const float xMaxRef = 3;
    const float yMaxRef = 3;

    float xMax  = xMaxRef;
    float yMax  = yMaxRef;

    float x0ref = -xMax / 2;
    float y0ref = -yMax / 2;

    const float xZoom  = xMaxRef / 40;
    const float yZoom  = yMaxRef / 40;
    const float xShift = xMaxRef / 100;
    const float yShift = yMaxRef / 100;

    while (!WindowShouldClose()) 
    {
        if(IsKeyDown(KEY_P))  { xMax -= xZoom;  yMax -= yZoom;  x0ref += xZoom / 2;  y0ref += yZoom / 2; }
        if(IsKeyDown(KEY_M))  { xMax += xZoom;  yMax += yZoom;  x0ref -= xZoom / 2;  y0ref -= yZoom / 2; }

        int screen_size = MIN(width, height);
        const float dx  = xMax / screen_size;
        const float dy  = yMax / screen_size;
            
        if(IsKeyDown(KEY_LEFT))   x0ref -= xShift;
        if(IsKeyDown(KEY_RIGHT))  x0ref += xShift;
        if(IsKeyDown(KEY_UP))     y0ref -= yShift;
        if(IsKeyDown(KEY_DOWN))   y0ref += yShift;
        
        float x0 = x0ref;
        float y0 = y0ref;

        int xPix = 0;
        int yPix = 0;

        double time_start = GetTime();

        __m256 DotShift = _mm256_set_ps (7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m256 DotInc   = _mm256_set1_ps(1.0);
        __m256 ScaleX   = _mm256_set1_ps(dx);
        __m256 R2max    = _mm256_set1_ps(4.0);
        __m256 Nmax     = _mm256_set1_ps(255);
        __m256 Zero     = _mm256_set1_ps(0.0);

        #pragma omp parallel for
        for (yPix = 0;  yPix < height;  yPix++)
        {
            y0 = y0ref + yPix * dy;
            __m256 Y0 = _mm256_set1_ps(y0);
            
            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix += SIMD_GROUP, x0 += SIMD_GROUP * dx)
            {                   
                __m256 X0     = _mm256_set1_ps(x0);
                __m256 X0shft = _mm256_add_ps( X0, _mm256_mul_ps(DotShift, ScaleX) );

                __m256 X    = _mm256_set1_ps(0.0);
                __m256 Y    = _mm256_set1_ps(0.0);
  
                __m256 N    = _mm256_set1_ps(0.0);
                __m256 Nres = _mm256_set1_ps(0.0);

                __m256 R2res = _mm256_set1_ps(0.0);

                for (int IsDotLeft = 0; ! IsDotLeft; )
                {
                    __m256 X2 = _mm256_mul_ps(X,  X);
                    __m256 Y2 = _mm256_mul_ps(Y,  Y);
                    __m256 XY = _mm256_mul_ps(X,  Y);
                    __m256 R2 = _mm256_add_ps(X2, Y2);

                    __m256 CmpMask1 = _mm256_cmp_ps (R2, R2max, _CMP_GT_OS);
                    __m256 CmpMask2 = _mm256_cmp_ps (Nres, Zero, _CMP_EQ_OS);
                           CmpMask1 = _mm256_and_ps (CmpMask1, CmpMask2);

                    Nres  = _mm256_or_ps( Nres,  _mm256_and_ps(N, CmpMask1) );             
                    R2res = _mm256_or_ps( R2res, _mm256_and_ps(R2, CmpMask1) );  

                    X = _mm256_add_ps( _mm256_sub_ps(X2, Y2), X0shft );
                    Y = _mm256_add_ps( _mm256_add_ps(XY, XY), Y0 );

                    N = _mm256_add_ps(N, DotInc);

                    CmpMask1  = _mm256_cmp_ps (N, Nmax, _CMP_GE_OS);
                    CmpMask2  = _mm256_cmp_ps (Nres, Zero, _CMP_EQ_OS);
                    CmpMask1  = _mm256_and_ps (CmpMask1, CmpMask2);                    
                    Nres      = _mm256_or_ps  (Nres, _mm256_and_ps(N, CmpMask1));
                    R2res     = _mm256_or_ps( R2res, _mm256_and_ps(R2, CmpMask1) );  
 
                    CmpMask2  = _mm256_cmp_ps (Nres, Zero, _CMP_EQ_OS);
                    IsDotLeft = _mm256_testz_ps(CmpMask2, CmpMask2);
                }

                for (int data_cnt = 0;  data_cnt < SIMD_GROUP;  data_cnt++) 
                {
                    int Rval = (int) R2res[data_cnt] * 15;
                    Pixels_buf[yPix * width + xPix + data_cnt] = {0, 0, Rval, 255};

                    // int Nval = (int) Nres[data_cnt] * 4;
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