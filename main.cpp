#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <emmintrin.h>

#include "raylib.h"

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
    const int width  = 1600;
    const int height = 1600;
    
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

    const int   Nmax  = 255;
    const float R2max = 2*2;

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

        for (yPix = 0;  yPix < height;  yPix++, y0 += dy)
        {
            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix += SIMD_GROUP, x0 += SIMD_GROUP * dx)
            {                   
                float X[SIMD_GROUP] = {0};
                float Y[SIMD_GROUP] = {0};   

                int    N[SIMD_GROUP] = {0};     
                int Nres[SIMD_GROUP] = {-1, -1, -1, -1, -1, -1, -1, -1};

                float Y2[SIMD_GROUP] = {0};  
                float X2[SIMD_GROUP] = {0};  
                float XY[SIMD_GROUP] = {0};  
                float R2[SIMD_GROUP] = {0};  

                for (int DotEscapeMask = 0; DotEscapeMask != 0b11111111; )
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

                for (int data_cnt = 0; data_cnt < SIMD_GROUP; data_cnt++)  { Pixels_buf[yPix * width + xPix + data_cnt] = {Nres[data_cnt], Nres[data_cnt], Nres[data_cnt], 255}; }
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
    const int frame_str_size = 15;
    char frame_str[frame_str_size] = {0};

    BeginDrawing();

    ClearBackground (WHITE);
    DrawTexture     (*MandelTexture, 0, 0, WHITE);

    snprintf(frame_str, frame_str_size, "Frame rate: %d", frame_rate);
    
    int text_width  = 270;
    int text_height = 30;

    DrawRectangle (width - text_width,  0,  text_width,  2 * text_height,  BLUE);
    DrawText      (frame_str,  width - text_width + text_height,  text_height / 3,  text_height,  BLACK);
    
    EndDrawing();

    return Success_end;
}