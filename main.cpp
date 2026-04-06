#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"

#define XMAXREF  (float) 3
#define YMAXREF  (float) 3

int main()
{
    const int width  = 1500;
    const int height = 1500;
    
    float xMax  = XMAXREF;
    float yMax  = YMAXREF;

    float x0ref = -xMax / 2;
    float y0ref = -yMax / 2;

    const float xZoom  = XMAXREF/20;
    const float yZoom  = YMAXREF/20;
    const float xShift = XMAXREF/100;
    const float yShift = YMAXREF/100;
    
    int N = 0;
    const int   Nmax  = 255;
    const float R2max = 2*2;

    InitWindow(width, height, "The Mandelbrot Set");

    Image MandelImg = GenImageColor(width, height, WHITE);

    Texture2D MandelTexture = LoadTextureFromImage(MandelImg);

    Color* Pixels_buf = (Color*) calloc(width * height, sizeof(Color));

    while (!WindowShouldClose()) 
    {
        if(IsKeyDown(KEY_P))  { xMax -= xZoom;  yMax -= yZoom;  x0ref += xZoom / 2;  y0ref += yZoom / 2; }
        if(IsKeyDown(KEY_M))  { xMax += xZoom;  yMax += yZoom;  x0ref -= xZoom / 2;  y0ref -= yZoom / 2; }

        const float dx = xMax / width;
        const float dy = yMax / height;
            
        if(IsKeyDown(KEY_LEFT))   x0ref += xShift;
        if(IsKeyDown(KEY_RIGHT))  x0ref -= xShift;
        if(IsKeyDown(KEY_UP))     y0ref += yShift;
        if(IsKeyDown(KEY_DOWN))   y0ref -= yShift;
        
        float x0 = x0ref;
        float y0 = y0ref;

        int xPix = 0;
        int yPix = 0;

        for (yPix = 0;  yPix < height;  yPix++, y0 += dy)
        {
            for (xPix = 0, x0 = x0ref;  xPix < width;  xPix++, x0 += dx)
            {
                float x = 0;
                float y = 0;        

                for (N = 0; N <= Nmax; N++)
                {
                    float y2 = y*y;
                    float x2 = x*x;
                    float xy = x*y;
                    float R2 = x2 + y2;

                    if (R2 > R2max) 
                    {
                        Pixels_buf[yPix * width + xPix] = BLACK;
                        break;
                    }
                
                    x = x2 - y2 + x0;
                    y = 2*xy + y0;
                }

                if (N > Nmax)
                    Pixels_buf[yPix * width + xPix] = WHITE;

            }
        }

        UpdateTexture(MandelTexture, Pixels_buf);
        
        BeginDrawing();
            ClearBackground(WHITE);
            DrawTexture(MandelTexture, 0, 0, WHITE);
        EndDrawing();
    }

    UnloadImage(MandelImg);
    UnloadTexture(MandelTexture);
    free(Pixels_buf);

    CloseWindow();

    return 0;   
}