#include <iostream>
#include <random>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Color
{
union {
    uint32_t rgba;
    struct
    {

        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
};
};
float ColorError(Color c1, Color c2)
{
float red1 = (float)c1.r / 255.f;
float red2 = (float)c2.r / 255.f;
float green1 = (float)c1.g / 255.f;
float green2 = (float)c2.g / 255.f;
float blue1 = (float)c1.b / 255.f;
float blue2 = (float)c2.b / 255.f;

float dr = (red1-red2) * (red1 - red2);
float dg = (green1 - green2) * (green1 - green2);
float db = (blue1 - blue2) * (blue1 - blue2);
return (dr + dg + db) / 3.f;
}

struct Canvas
{
Color *Buffer;
int Channels;
int Width;
int Height;
};
void clear_image(Canvas *canvas, Color color)
{
for (auto i = 0; i < canvas->Height * canvas->Width; ++i)
{
    canvas->Buffer[i] = color;
}
}

void write_pixel(Canvas *canvas, int x, int y, Color color)
{
if (color.a == 0xFF)
{
    canvas->Buffer[y * canvas->Width + x] = color;
}
else if (color.a == 0)
{
    return;
}
else
{
    Color existing = canvas->Buffer[y * canvas->Width + x];

    float a = (float)color.a / 255.f;
    float na = 1.0f - a;
    color.a = 0xff;
    color.r = (uint8_t)(((float)color.r * a) + ((float)existing.r * na));
    color.g = (uint8_t)(((float)color.g * a) + ((float)existing.g * na));
    color.b = (uint8_t)(((float)color.b * a) + ((float)existing.b * na));
    canvas->Buffer[y * canvas->Width + x] = color;
}
}

Color get_image_average(Canvas *canvas)
{
size_t samples = 0;
uint64_t red = 0, green = 0, blue = 0, alpha = 0;
for (auto i = 0; i < canvas->Height * canvas->Width; ++i)
{
    Color c = canvas->Buffer[i];
    red += c.r;
    green += c.g;
    blue += c.b;
    alpha += c.a;
    samples++;
}
red /= samples;
green /= samples;
blue /= samples;
alpha /= samples;
Color retVal;
retVal.r = (uint8_t)red;
retVal.g = (uint8_t)green;
retVal.b = (uint8_t)blue;
retVal.a = (uint8_t)alpha;
return retVal;
}
Color read_pixel(Canvas *canvas, int x, int y)
{
return canvas->Buffer[y * canvas->Width + x];
}

void DrawHorizontalLine(Canvas *canvas, int x0, int y0, int x1, Color c)
{
if (x1 < x0)
{
    int tx = x0;
    x0 = x1;
    x1 = tx;
}
for (int x = x0; x <= x1; ++x)
{
    write_pixel(canvas, x, y0, c);
}
}
Color AvgColorForLine(Canvas *canvas, int x0, int y0, int x1, int y1)
{
int red = 0, green = 0, blue = 0, alpha = 0, samples = 0;
bool steep = abs(y1 - y0) > abs(x1 - x0);
if (steep)
{
    int tmp;
    tmp = x0;
    x0 = y0;
    y0 = tmp;

    tmp = x1;
    x1 = y1;
    y1 = tmp;
}

if (x0 > x1)
{
    int tx = x0, ty = y0;
    x0 = x1;
    y0 = y1;
    x1 = tx;
    y1 = ty;
}

int64_t dx, dy;
dx = x1 - x0;
dy = abs(y1 - y0);

int64_t err = dx / 2;
int64_t ystep;
if (y0 < y1)
{
    ystep = 1;
}
else
{
    ystep = -1;
}
int x = x0;
int y = y0;
for (; x <= x1; x++)
{
    if (steep)
    {

        Color c = read_pixel(canvas, y, x);
        samples++;
        red += c.r;
        green += c.g;
        blue += c.b;
        alpha += c.a;
    }
    else
    {
        Color c = read_pixel(canvas, x, y);
        samples++;
        red += c.r;
        green += c.g;
        blue += c.b;
        alpha += c.a;
    }
    err -= dy;
    if (err < 0)
    {
        y += ystep;
        err += dx;
    }
}
red /= samples;
blue /= samples;
green /= samples;
alpha /= samples;
Color retVal;
retVal.r = red;
retVal.g = green;
retVal.b = blue;
retVal.a = alpha;
return retVal;
}

float ErrorForLine(Canvas *canvas, int x0, int y0, int x1, int y1, Color c)
{
int samples = 0;
float totalError = 0;
bool steep = abs(y1 - y0) > abs(x1 - x0);
if (steep)
{
    int tmp;
    tmp = x0;
    x0 = y0;
    y0 = tmp;

    tmp = x1;
    x1 = y1;
    y1 = tmp;
}

if (x0 > x1)
{
    int tx = x0, ty = y0;
    x0 = x1;
    y0 = y1;
    x1 = tx;
    y1 = ty;
}

int64_t dx, dy;
dx = x1 - x0;
dy = abs(y1 - y0);

int64_t err = dx / 2;
int64_t ystep;
if (y0 < y1)
{
    ystep = 1;
}
else
{
    ystep = -1;
}
int x = x0;
int y = y0;
for (; x <= x1; x++)
{
    if (steep)
    {

        Color ec = read_pixel(canvas, y, x);
        samples++;
        totalError += ColorError(c, ec);
    }
    else
    {
        Color ec = read_pixel(canvas, x, y);
        samples++;
        totalError += ColorError(c, ec);
    }
    err -= dy;
    if (err < 0)
    {
        y += ystep;
        err += dx;
    }
}
return totalError / (float)samples;
}

void DrawLine(Canvas *canvas, int x0, int y0, int x1, int y1, Color c)
{
bool steep = abs(y1 - y0) > abs(x1 - x0);

if (y0 == y1)
{
    DrawHorizontalLine(canvas, x0, y0, x1, c);
    return;
}
if (steep)
{
    int tmp;
    tmp = x0;
    x0 = y0;
    y0 = tmp;

    tmp = x1;
    x1 = y1;
    y1 = tmp;
}

if (x0 > x1)
{
    int tx = x0, ty = y0;
    x0 = x1;
    y0 = y1;
    x1 = tx;
    y1 = ty;
}

int64_t dx, dy;
dx = x1 - x0;
dy = abs(y1 - y0);

int64_t err = dx / 2;
int64_t ystep;
if (y0 < y1)
{
    ystep = 1;
}
else
{
    ystep = -1;
}
int x = x0;
int y = y0;
for (; x <= x1; x++)
{
    if (steep)
    {

        write_pixel(canvas, y, x, c);
    }
    else
    {
        write_pixel(canvas, x, y, c);
    }
    err -= dy;
    if (err < 0)
    {
        y += ystep;
        err += dx;
    }
}
return;
}

int Length(int x0, int y0, int x1, int y1)
{
int dx = x1 - x0;
int dy = y1 - y0;
return (int)sqrt(dx * dx + dy * dy);
}
bool PointsValid(Canvas *canvas, int x0, int y0)
{
return x0 >= 0 && x0 < canvas->Width && y0 >= 0 && y0 < canvas->Height;
}

int main(int argc, char **argv)
{
srand(time(NULL));
const char testFile[] = "testimage.bmp";
Canvas readCanvas;
Canvas writeCanvas;

readCanvas.Buffer = (Color *)stbi_load(testFile, &readCanvas.Width, &readCanvas.Height, &readCanvas.Channels, 4);

writeCanvas.Channels = readCanvas.Channels;
writeCanvas.Height = readCanvas.Height;
writeCanvas.Width = readCanvas.Width;
writeCanvas.Buffer = (Color *)malloc(sizeof(Color) * writeCanvas.Width * writeCanvas.Height);

//get average canvas color;
Color averageColor = get_image_average(&readCanvas);

SDL_Init(SDL_INIT_VIDEO);
SDL_Window *window = SDL_CreateWindow("LineArt",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        writeCanvas.Width,
                                        writeCanvas.Height,
                                        0);
SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, writeCanvas.Width, writeCanvas.Height);
clear_image(&writeCanvas, averageColor);
size_t lines = 0;
while(true)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            exit(0);
        }
    }
    int x0 = rand() % writeCanvas.Width;
    int y0 = rand() % writeCanvas.Height;

    float startAlpha = 2.f * M_PI * ((float)(rand() % 1000) / 1000.f);
    for (float alpha = startAlpha; alpha < (startAlpha + M_PI * 2.0f); alpha += (2.0 * M_PI) / 200.f)
    {
        for (float lineLength = writeCanvas.Height / 2.f; lineLength > 0; lineLength -= writeCanvas.Height / 15.f)
        {

            int x1 = x0 + lineLength * cos(alpha);
            int y1 = y0 + lineLength * sin(alpha);
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    exit(0);
                }
            }
            if (PointsValid(&writeCanvas, x1, y1))
            {
                Color c = AvgColorForLine(&readCanvas, x0, y0, x1, y1);
                if (ErrorForLine(&readCanvas, x0, y0, x1, y1, c) < 0.005f)
                {
                    //c.a = 192;
                    DrawLine(&writeCanvas, x0, y0, x1, y1, c);
                    lines += 1;
                    goto out;
                }
            }
        }
    }

out:

    SDL_UpdateTexture(texture, 0, writeCanvas.Buffer, 4 * writeCanvas.Width);
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
}
SDL_Delay(10000);
stbi_write_bmp("testimage_out.bmp", writeCanvas.Width, writeCanvas.Height, 4, (uint8_t *)writeCanvas.Buffer);
return 0;
}
#if 0
template <typename T>
class DynamicArray
{
    DynamicArray(MemoryBank *bank);
    DynamicArray(MemoryBank *bank, size_t Reserved);

    DynamicArray(const DynamicArray& other);
    DynamicArray(DynamicArray&& other);

    ~DynamicArray();


    void PushBack(const T& newValue);
    void PopBack();

    T& Back();
    const T& Back() const;

    T& Front();
    const T& Front() const;

    void RemoveAt(size_t idx);
    T& At(size_t idx);
    const T& At(size_t idx) const;

    void Clear();

    const T& operator [](size_t idx) const;
    T& operator [](size_t idx) ;
    
    
    MemoryBank *StorageBank;
    size_t Length;
    size_t Reserved;
    T* Data;
}


class CircularBuffer
{
    DynamicArray(MemoryBank* bank);


    
}
#endif
