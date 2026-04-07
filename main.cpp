#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <immintrin.h>

#include "raylib.h"

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
    const int width  = 800;
    const int height = 800;
    
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

    const float xZoom  = xMaxRef / 20;
    const float yZoom  = yMaxRef / 20;
    const float xShift = xMaxRef / 100;
    const float yShift = yMaxRef / 100;

    while (!WindowShouldClose()) 
    {
        if(IsKeyDown(KEY_P))  { xMax -= xZoom;  yMax -= yZoom;  x0ref += xZoom / 2;  y0ref += yZoom / 2; }
        if(IsKeyDown(KEY_M))  { xMax += xZoom;  yMax += yZoom;  x0ref -= xZoom / 2;  y0ref -= yZoom / 2; }

        const float dx = xMax / width;
        const float dy = yMax / height;
            
        if(IsKeyDown(KEY_LEFT))   x0ref -= xShift;
        if(IsKeyDown(KEY_RIGHT))  x0ref += xShift;
        if(IsKeyDown(KEY_UP))     y0ref -= yShift;
        if(IsKeyDown(KEY_DOWN))   y0ref += yShift;
        
        float x0 = x0ref;
        float y0 = y0ref;

        int xPix = 0;
        int yPix = 0;

        double time_start = GetTime();

        __m128 DotShift = _mm_set_ps (0.0, 1.0, 2.0, 3.0);
        __m128 DotInc   = _mm_set1_ps(1.0);
        __m128 ScaleX   = _mm_set1_ps(dx);
        __m128 R2max    = _mm_set1_ps(4.0);
        __m128 Nmax     = _mm_set1_ps(255);
        __m128 Zero     = _mm_set1_ps(0.0);

        for (yPix = 0;  yPix < height;  yPix++, y0 += dy)
        {
            __m128 Y0 = _mm_set_ps( y0, y0,  y0,  y0);

            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix += SIMD_GROUP, x0 += SIMD_GROUP * dx)
            {                   
                __m128 X0   = _mm_set1_ps(x0);

                __m128 X    = _mm_set1_ps(0.0);
                __m128 Y    = _mm_set1_ps(0.0);
  
                __m128 N    = _mm_set1_ps(0.0);
                __m128 Nres = _mm_set1_ps(0.0);

                for (int IsDotLeft = 0; ! IsDotLeft; )
                {
                    __m128 X2 = _mm_mul_ps(X,  X);
                    __m128 Y2 = _mm_mul_ps(Y,  Y);
                    __m128 XY = _mm_mul_ps(X,  Y);
                    __m128 R2 = _mm_add_ps(X2, Y2);

                    __m128 CmpMask1    = _mm_cmp_ps   (R2, R2max, _CMP_GT_OS);
                    __m128 CmpMask2    = _mm_cmpeq_ps (Nres, Zero);
                           CmpMask1    = _mm_and_ps   (CmpMask1, CmpMask2);

                    Nres = _mm_or_ps( Nres, _mm_and_ps(N, CmpMask1) );             

                    X = _mm_add_ps( _mm_sub_ps(X2, Y2), _mm_add_ps(X0, _mm_mul_ps(DotShift, ScaleX)) );
                    Y = _mm_add_ps( _mm_add_ps(XY, XY), Y0 );

                    N = _mm_add_ps(N, DotInc);

                    CmpMask1  = _mm_cmp_ps   (N, Nmax, _CMP_GE_OS);
                    CmpMask2  = _mm_cmpeq_ps (Nres, Zero);
                    CmpMask1  = _mm_and_ps   (CmpMask1, CmpMask2);                    
                    Nres      = _mm_or_ps( Nres, _mm_and_ps(N, CmpMask1) );
 
                    CmpMask2  = _mm_cmpneq_ps(Nres, Zero);
                    IsDotLeft = _mm_test_all_ones(_mm_castps_si128(CmpMask2));
                }

                for (int data_cnt = 0;  data_cnt < SIMD_GROUP;  data_cnt++) 
                {
                    int Nval = (int) Nres[data_cnt];
                    Pixels_buf[yPix * width + xPix + data_cnt] = {Nval, Nval, Nval, 255};
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