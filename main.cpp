#include <stdio.h>

#include "raylib.h"

int main()
{
    const int width  = 1500;
    const int height = 1500;

    const float xmax = 3;
    const float ymax = 3;

    const float dx = xmax / width;
    const float dy = ymax / height;

    const int Nmax    = 255;
    const float R2max = 2*2;

    float x0 = 0;
    float y0 = 0;

    int xPix = 0;
    int yPix = 0;

    InitWindow(width, height, "The Mandelbrot Set");

    Image MandelImg = GenImageColor(width, height, WHITE);

    for (yPix = 0, y0 = -ymax/2;  yPix < height;  yPix++, y0 += dy)
    {
        for (xPix = 0, x0 = -xmax/2;  xPix < width;  xPix++, x0 += dx)
        {
            float x = 0;
            float y = 0;        

            for (int N = 0; N <= Nmax; N++)
            {
                float y2 = y*y;
                float x2 = x*x;
                float xy = x*y;
                float R2 = x2 + y2;

                if (R2 > R2max) 
                {
                    ImageDrawPixel(&MandelImg, xPix, yPix, BLACK);
                    break;
                }
                
                x = x2 - y2 + x0;
                y = 2*xy + y0;
            }
        }
    }

    Texture2D MandelTexture = LoadTextureFromImage(MandelImg);
    
    UnloadImage(MandelImg);

    while (!WindowShouldClose()) 
    {
        BeginDrawing();

        DrawTexture(MandelTexture, 0, 0, BLACK);

        EndDrawing();
    }

    UnloadTexture(MandelTexture);
    CloseWindow();

    return 0;   
}