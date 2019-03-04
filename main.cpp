#include <iostream>
#include <random>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


struct Color
{
    union 
    {
        uint32_t rgba;
        struct 
        {
            
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
            
        };
    };
};

struct Canvas
{
    Color *Buffer;
    int Channels;
    int Width;
    int Height;
};
void clear_image(Canvas *canvas, Color color)
{
    for(auto i = 0; i < canvas->Height * canvas->Width; ++i)
    {
        canvas->Buffer[i] = color;
    }
}

void write_pixel(Canvas *canvas, int x, int y, Color color)
{
    canvas->Buffer[y * canvas->Width + x] = color;
}

Color get_image_average(Canvas *canvas)
{
    size_t samples=0;
    uint64_t red =0 , green =0 ,blue = 0,alpha = 0;
    for(auto i = 0; i < canvas->Height * canvas->Width; ++i)
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
Color get_pixel_value(Canvas *canvas, int x, int y)
{
    return canvas->Buffer[y * canvas->Width + x];
}


int main(int argc, char** argv)
{
    const char testFile[] = "testimage.bmp";
    Canvas readCanvas;
    Canvas writeCanvas;

    readCanvas.Buffer = (Color *)stbi_load(testFile, &readCanvas.Width, &readCanvas.Height, &readCanvas.Channels, 4);
    
    
    writeCanvas.Channels = readCanvas.Channels;
    writeCanvas.Height = readCanvas.Height;
    writeCanvas.Width = readCanvas.Width;
    writeCanvas.Buffer = (Color *)malloc(sizeof(Color) * writeCanvas.Width * writeCanvas.Height);
    printf("W: %d H: %d\n",readCanvas.Width, readCanvas.Height);
    //get average canvas color;
    Color averageColor = get_image_average(&readCanvas);
    clear_image(&writeCanvas,averageColor);
    printf("Average Color: %08x\n",averageColor.rgba);
    stbi_write_bmp("testimage_out.bmp",writeCanvas.Width, writeCanvas.Height,4,(uint8_t*)writeCanvas.Buffer);
    return 0;

}