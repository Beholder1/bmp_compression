#pragma once
#include <iostream>
#include <vector>
#include "assistFunctions.h"

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface);
void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);

/*PALETA_DEDYKOWANA_________________________________________________________________*/

void dedicatedPalette(SDL_Surface* bmp, palette_5bit* palette)
{
    SDL_Color color;
    loop5BitPalette(palette);

    std::vector<pixels> pixelsV; // vector przechowujacy wszystkie piksele

    for(int y = 0; y < bmp->h; y++)
        for(int x = 0; x < bmp->w; x++)
        {
            color = getPixelSurface(x, y, bmp);
            checkColor(color, pixelsV);
        }

    if(pixelsV.size() < 32) //jesli w vectorze jest mniej nic 32 piksele dodaj wszystkie z narzuconej
        for(int i = 0; i < 32; i++)
        {
            pixels tmp;
            tmp.color = impPalette[i];
            pixelsV.push_back(tmp);
        }

    std::cout << "Before Median Cut\n";

    median_Cut(pixelsV, palette); // zmniejszenie palety do 32 kolorow za pomoca algorytmu Median Cut
}

/*PALETA_NARZUCONA_________________________________________________________________*/

void imposedPalette(SDL_Surface* bmp, palette_5bit* palette)
{
    SDL_Color color;
    imposedToPalette(impPalette, palette);

    for(int y = 0; y < bmp->h; y++)
        for(int x = 0; x < bmp->w; x++)
        {
            color = getPixelSurface(x, y, bmp);
            colorCount(palette, color);
        }
}


/*SKALA_SZAROSCI_________________________________________________________________*/

void greyScale(SDL_Surface* bmp, palette_5bit* palette)
{
    SDL_Color color;

    for(int y = 0; y < bmp->h; y++)
        for(int x = 0; x < bmp->w; x++)
        {
            color = getPixelSurface(x, y, bmp);
            palette[RGBtoBWindex(color)].count++;
        }
}



