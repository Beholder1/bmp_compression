#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <algorithm>

#define title "BMP <-> WSS conversion"

SDL_Color impPalette[32]; //paleta narzucona
SDL_Window* window = NULL;
SDL_Surface* screen = NULL;

using namespace std;

/*Struktury_do_przechowywania_danych*/

struct pixels
{
    SDL_Color color;
    int count = 0;
};

struct palette_5bit
{
    Uint8 bit5; //piksel zapisany na 5 bitach w sposob RRGBB
    SDL_Color bit5_to_24; //wartosc 24 bitowa piksela otrzymana przez zapetlenie bitow z 5 bitowego odpowiednika
    SDL_Color originalC; //kolor ktory jest najblizszy z narzuconej/dedykowanej palety
    int count = 0; //liczba wystapien zblizonej wartosci piksela w pliku bmp
};

struct bufor
{
    int free = 32;
    Uint32 variable = 0;
    int dif = 0;
};

/*Struktury_do_sortowania_vectora*/

struct compareComponentsR
{
    bool operator() (pixels p1, pixels p2)
    {
        if(p1.color.r < p2.color.r) return true;
        else return false;
    }
} MyComparatorR;

struct compareComponentsG
{
    bool operator() (pixels p1, pixels p2)
    {
        if(p1.color.g < p2.color.g) return true;
        else return false;
    }
} MyComparatorG;

struct compareComponentsB
{
    bool operator() (pixels p1, pixels p2)
    {
        if(p1.color.b < p2.color.b) return true;
        else return false;
    }
} MyComparatorB;

/*===============Funkcje_modyfikujace/wyswietlajace_BMP============================================*/

SDL_Color getPixelSurface(int x, int y, SDL_Surface *surface)
{
    SDL_Color color ;
    Uint16 biHeight = surface->h;
    Uint16 biWidth = surface->w;
    Uint32 col = 0 ;
    if ((x>=0) && (x<biWidth) && (y>=0) && (y<biHeight))
    {
        //określamy pozycję
        char* pPosition=(char*)surface->pixels ;

        //przesunięcie względem y
        pPosition+=(surface->pitch*y) ;

        //przesunięcie względem x
        pPosition+=(surface->format->BytesPerPixel*x);

        //kopiujemy dane piksela
        memcpy(&col, pPosition, surface->format->BytesPerPixel);

        //konwertujemy kolor
        SDL_GetRGB(col, surface->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}

void setPixelSurface(int x, int y, Uint8 r, Uint8 g, Uint8 b, SDL_Surface* surface)
{
    if ((x >= 0) && (x < surface->w) && (y >= 0) && (y < surface->h))
    {
        Uint32 pixel = SDL_MapRGB(surface->format, r, g, b);
        int bpp = surface->format->BytesPerPixel;
        Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
        switch (bpp)
        {
        case 1: //8-bit
            *p = pixel;
            break;

        case 2: //16-bit
            *(Uint16*)p = pixel;
            break;

        case 3: //24-bit
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4: //32-bit
            *(Uint32*)p = pixel;
            break;
        }
    }
}

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B, int width, int height, SDL_Surface* screen)
{
    if ((x>=0) && (x<width) && (y>=0) && (y<height))
    {
        /* Zamieniamy poszczególne składowe koloru na format koloru piksela */
        Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

        /* Pobieramy informację ile bajtów zajmuje jeden piksel */
        int bpp = screen->format->BytesPerPixel;

        /* Obliczamy adres piksela */
        Uint8 *p1 = (Uint8 *)screen->pixels + (y) * screen->pitch + (x) * bpp;
        Uint8 *p2 = (Uint8 *)screen->pixels + (y+1) * screen->pitch + (x) * bpp;
        Uint8 *p3 = (Uint8 *)screen->pixels + (y) * screen->pitch + (x+1) * bpp;
        Uint8 *p4 = (Uint8 *)screen->pixels + (y+1) * screen->pitch + (x+1) * bpp;

        /* Ustawiamy wartość piksela, w zależnoœci od formatu powierzchni*/
        switch(bpp)
        {
        case 1: //8-bit
            *p1 = pixel;
            *p2 = pixel;
            *p3 = pixel;
            *p4 = pixel;
            break;

        case 2: //16-bit
            *(Uint16 *)p1 = pixel;
            *(Uint16 *)p2 = pixel;
            *(Uint16 *)p3 = pixel;
            *(Uint16 *)p4 = pixel;
            break;

        case 3: //24-bit
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p1[0] = (pixel >> 16) & 0xff;
                p1[1] = (pixel >> 8) & 0xff;
                p1[2] = pixel & 0xff;
                p2[0] = (pixel >> 16) & 0xff;
                p2[1] = (pixel >> 8) & 0xff;
                p2[2] = pixel & 0xff;
                p3[0] = (pixel >> 16) & 0xff;
                p3[1] = (pixel >> 8) & 0xff;
                p3[2] = pixel & 0xff;
                p4[0] = (pixel >> 16) & 0xff;
                p4[1] = (pixel >> 8) & 0xff;
                p4[2] = pixel & 0xff;
            }
            else
            {
                p1[0] = pixel & 0xff;
                p1[1] = (pixel >> 8) & 0xff;
                p1[2] = (pixel >> 16) & 0xff;
                p2[0] = pixel & 0xff;
                p2[1] = (pixel >> 8) & 0xff;
                p2[2] = (pixel >> 16) & 0xff;
                p3[0] = pixel & 0xff;
                p3[1] = (pixel >> 8) & 0xff;
                p3[2] = (pixel >> 16) & 0xff;
                p4[0] = pixel & 0xff;
                p4[1] = (pixel >> 8) & 0xff;
                p4[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4: //32-bit
            *(Uint32 *)p1 = pixel;
            *(Uint32 *)p2 = pixel;
            *(Uint32 *)p3 = pixel;
            *(Uint32 *)p4 = pixel;
            break;

        }
    }
}

void setPixelSurface(int x, int y, Uint8 R, Uint8 G, Uint8 B, int width, int height, SDL_Surface* screen)
{
    if ((x>=0) && (x<width*2) && (y>=0) && (y<height*2))
    {
        /* Zamieniamy poszczególne składowe koloru na format koloru piksela */
        Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

        /* Pobieramy informację ile bajtów zajmuje jeden piksel */
        int bpp = screen->format->BytesPerPixel;

        /* Obliczamy adres piksela */
        Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;

        /* Ustawiamy wartość piksela, w zależności od formatu powierzchni*/
        switch(bpp)
        {
        case 1: //8-bit
            *p = pixel;
            break;

        case 2: //16-bit
            *(Uint16 *)p = pixel;
            break;

        case 3: //24-bit
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4: //32-bit
            *(Uint32 *)p = pixel;
            break;
        }
    }
}

SDL_Color getPixel(int x, int y, int width, int height, SDL_Surface* screen)
{
    SDL_Color color ;
    Uint32 col = 0 ;
    if ((x>=0) && (x<width) && (y>=0) && (y<height))
    {
        //określamy pozycję
        char* pPosition=(char*)screen->pixels ;

        //przesunięcie względem y
        pPosition+=(screen->pitch*y*2) ;

        //przesunięcie względem x
        pPosition+=(screen->format->BytesPerPixel*x*2);

        //kopiujemy dane piksela
        memcpy(&col, pPosition, screen->format->BytesPerPixel);

        //konwertujemy kolor
        SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
    }
    return ( color ) ;
}

/*===============Dodanie wszystkich pikseli do vectora oraz zliczenie ich wystepowan===============*/

int addColor(SDL_Color color, std::vector<pixels>& pixelsV)
{
    pixels tmp;
    tmp.color = color;
    tmp.count = 1;
    pixelsV.push_back(tmp);
    return (pixelsV.size()-1);
}

bool compareColors(SDL_Color color1, SDL_Color color2)
{
    bool result = true;
    if (color1.r != color2.r) result = false;
    if (color1.g != color2.g) result = false;
    if (color1.b != color2.b) result = false;
    return result;
}

int checkColor(SDL_Color color, std::vector<pixels>& pixelsV)
{
    int index = -1;

    if (pixelsV.size() > 0)
        for (int i = 0; i < pixelsV.size(); i++)
        {
            if (compareColors(color, pixelsV[i].color))
            {
                index = i;
                pixelsV[i].count++;
            }
        }

    if (index < 0)
    {
        index = addColor(color, pixelsV);
    }
    return index;
}

/*===============Algorytm Median Cut z funkcjami pomocniczymi======================================*/

int longestRange(std::vector<pixels> bucket)
{
    int minR = 255, maxR = 0;
    int minG = 255, maxG = 0;
    int minB = 255, maxB = 0;

    for(int i = 0; i < bucket.size(); i++)
    {
        if(minR > bucket[i].color.r)
            minR = bucket[i].color.r;
        if(maxR < bucket[i].color.r)
            maxR = bucket[i].color.r;

        if(minG > bucket[i].color.g)
            minG = bucket[i].color.g;
        if(maxG < bucket[i].color.g)
            maxG = bucket[i].color.g;

        if(minB > bucket[i].color.b)
            minB = bucket[i].color.b;
        if(maxB < bucket[i].color.b)
            maxB = bucket[i].color.b;
    }

    int distR = maxR - minR;
    int distG = maxG - minG;
    int distB = maxB - minB;

    int longDist = std::max(distR, std::max(distG, distB));

    if(longDist == distR)
        return 0;
    else if(longDist == distG)
        return 1;
    else if(longDist == distB)
        return 2;

    return -1;
}

SDL_Color colorWithTheHighestProbability(std::vector<pixels> bucket, int& count)
{
    int maxProbability = 0;
    int index;

    for(int i = 0; i < bucket.size(); i++)
        if(bucket[i].count >= maxProbability)
        {
            maxProbability = bucket[i].count;
            index = i;
        }

    count = bucket[index].count;
    return bucket[index].color;
}

void median_Cut(std::vector<pixels> palette, palette_5bit* dedicated)
{
    std::vector<std::vector<pixels>> bucket;
    std::vector<pixels> tmp;
    pixels tmp2;

    for(int i = 0; i < palette.size(); i++)
    {
        tmp2.color = palette[i].color;
        tmp2.count = palette[i].count;
        tmp.push_back(tmp2);
    }

    bucket.push_back(tmp);

    tmp.clear();
    int targetAmount = 1;

    while(bucket.size() < 32)
    {
        for(int i = 0; i < targetAmount; i++)
        {
            if(longestRange(bucket[0]) == 0) sort(bucket[i].begin(), bucket[i].end(), MyComparatorR);
            if(longestRange(bucket[0]) == 1) sort(bucket[i].begin(), bucket[i].end(), MyComparatorG);
            if(longestRange(bucket[0]) == 2) sort(bucket[i].begin(), bucket[i].end(), MyComparatorB);

            for(int j = 0; j < bucket[0].size()/2; j++)
                tmp.push_back(bucket[0][j]);
            bucket.push_back(tmp);
            tmp.clear();

            for(int j = bucket[0].size()/2; j < bucket[0].size(); j++)
                tmp.push_back(bucket[0][j]);
            bucket.push_back(tmp);
            tmp.clear();

            bucket.erase(bucket.begin());
        }
        targetAmount*=2;
    }

    for(int i = 0; i < 32; i++)
    {
        dedicated[i].originalC = colorWithTheHighestProbability(bucket[i], dedicated[i].count);
    }
}

/*===============Poszukiwanie najblizszego sasiada=================================================*/

int closestNeighbour(palette_5bit* palette, SDL_Color color)
{
    int distanceMin = ((255*255) * 3) + 1;
    int distance;
    int index;
    int diffR, diffG, diffB;

    for(int  i = 0; i < 32; i++)
    {
        diffR = color.r - palette[i].originalC.r;
        diffG = color.g - palette[i].originalC.g;
        diffB = color.b - palette[i].originalC.b;
        distance = diffR*diffR + diffG*diffG + diffB*diffB;

        if(distance < distanceMin)
        {
            distanceMin = distance;
            index = i;
        }
    }
    return index;
}

void imposedToPalette(SDL_Color* imposed, palette_5bit* palette)
{
    int distanceMin;
    int distance;
    int index;
    int diffR, diffG, diffB;
    bool available[32] {false};

    for(int i = 0; i < 32; i++)
    {
        distanceMin = ((255*255) * 3) + 1;
        index = -1;
        for(int j = 1; j < 32; j++)
        {
            diffR = palette[i].bit5_to_24.r - imposed[j].r;
            diffG = palette[i].bit5_to_24.g - imposed[j].g;
            diffB = palette[i].bit5_to_24.b - imposed[j].b;
            distance = diffR*diffR + diffG*diffG + diffB*diffB;

            if(distanceMin > distance && available[j] == false)
            {
                distanceMin = distance;
                if(index != -1)
                    available[index] = false;
                index = j;
                available[j] = true;
            }
        }
        palette[i].originalC = imposed[index];
    }
}

/*===============Sumowanie licznikow palety========================================================*/

void colorCount(palette_5bit* palette, SDL_Color color)
{
    int distanceMin = ((255*255) * 3) + 1;
    int distance;
    int index;
    int diffR, diffG, diffB;

    for(int  i = 0; i < 32; i++)
    {
        diffR = color.r - palette[i].originalC.r;
        diffG = color.g - palette[i].originalC.g;
        diffB = color.b - palette[i].originalC.b;
        distance = diffR*diffR + diffG*diffG + diffB*diffB;

        if(distance < distanceMin)
        {
            distanceMin = distance;
            index = i;
        }
    }
    palette[index].count++;
}

double countSum(palette_5bit* palette)
{
    double count = 0;

    for(int i = 0; i < 32; i++)
        count += palette[i].count;

    return count;
}

/*===============Zapetlenie wartosci 5-bitowych====================================================*/

void loop5BitPalette(palette_5bit* palete)
{
    for(int i = 0; i < 32; i++)
    {
        palete[i].bit5 = i; //tworzymy zakres 00000-11111

        Uint8 tmp = i;
        tmp >>= 3;
        palete[i].bit5_to_24.r = tmp;

        tmp = i;
        tmp <<= 5;
        tmp >>= 7;
        palete[i].bit5_to_24.g = tmp;
        palete[i].bit5_to_24.g <<= 1;
        palete[i].bit5_to_24.g |= tmp;

        tmp = i;
        tmp <<= 6;
        tmp >>= 6;
        palete[i].bit5_to_24.b = tmp;

        for(int j = 0; j < 3; j++)
        {
            Uint8 tmp = i;
            tmp >>= 3;
            palete[i].bit5_to_24.r <<= 2;
            palete[i].bit5_to_24.r |= tmp;

            tmp = i;
            tmp <<= 5;
            tmp >>= 7;
            palete[i].bit5_to_24.g <<= 1;
            palete[i].bit5_to_24.g |= tmp;
            palete[i].bit5_to_24.g <<= 1;
            palete[i].bit5_to_24.g |= tmp;

            tmp = i;
            tmp <<= 6;
            tmp >>= 6;
            palete[i].bit5_to_24.b <<= 2;
            palete[i].bit5_to_24.b |= tmp;
        }
    }
}

/*===============RGB -> BW=========================================================================*/

Uint8 RGBtoBW(SDL_Color color)
{
    Uint8 y = (color.r*0.299) + (color.g*0.587) + (color.b*0.114);
    y >>= 3;
    y <<= 3;

    return y;
}

Uint8 RGBtoBWorg(SDL_Color color)
{
    return ((color.r*0.299) + (color.g*0.587) + (color.b*0.114));
}

Uint8 RGBtoBWindex(SDL_Color color)
{
    Uint8 y = (color.r*0.299) + (color.g*0.587) + (color.b*0.114);
    y >>= 3;

    return y;
}


/*================Narzucona paleta - kolory==============*/

void imposedColors()
{

    impPalette[0].r = 0;
    impPalette[0].g = 0;
    impPalette[0].b = 0;

    impPalette[1].r = 128;
    impPalette[1].g = 0;
    impPalette[1].b = 0;

    impPalette[2].r = 0;
    impPalette[2].g = 128;
    impPalette[2].b = 0;

    impPalette[3].r = 0;
    impPalette[3].g = 0;
    impPalette[3].b = 128;

    impPalette[4].r = 255;
    impPalette[4].g = 0;
    impPalette[4].b = 0;

    impPalette[5].r = 0;
    impPalette[5].g = 255;
    impPalette[5].b = 0;

    impPalette[6].r = 0;
    impPalette[6].g = 0;
    impPalette[6].b = 255;

    impPalette[7].r = 128;
    impPalette[7].g = 128;
    impPalette[7].b = 0;

    impPalette[8].r = 128;
    impPalette[8].g = 0;
    impPalette[8].b = 128;

    impPalette[9].r = 0;
    impPalette[9].g = 128;
    impPalette[9].b = 128;

    impPalette[10].r = 255;
    impPalette[10].g = 255;
    impPalette[10].b = 0;

    impPalette[11].r = 255;
    impPalette[11].g = 0;
    impPalette[11].b = 255;

    impPalette[12].r = 0;
    impPalette[12].g = 255;
    impPalette[12].b = 255;

    impPalette[13].r = 128;
    impPalette[13].g = 128;
    impPalette[13].b = 128;

    impPalette[14].r = 255;
    impPalette[14].g = 128;
    impPalette[14].b = 128;

    impPalette[15].r = 128;
    impPalette[15].g = 255;
    impPalette[15].b = 128;

    impPalette[16].r = 128;
    impPalette[16].g = 128;
    impPalette[16].b = 255;

    impPalette[17].r = 255;
    impPalette[17].g = 255;
    impPalette[17].b = 128;

    impPalette[18].r = 255;
    impPalette[18].g = 128;
    impPalette[18].b = 255;

    impPalette[19].r = 128;
    impPalette[19].g = 255;
    impPalette[19].b = 255;

    impPalette[20].r = 255;
    impPalette[20].g = 255;
    impPalette[20].b = 255;

    impPalette[21].r = 0;
    impPalette[21].g = 128;
    impPalette[21].b = 255;

    impPalette[22].r = 0;
    impPalette[22].g = 255;
    impPalette[22].b = 128;

    impPalette[23].r = 128;
    impPalette[23].g = 0;
    impPalette[23].b = 255;

    impPalette[24].r = 128;
    impPalette[24].g = 255;
    impPalette[24].b = 0;

    impPalette[25].r = 255;
    impPalette[25].g = 0;
    impPalette[25].b = 128;

    impPalette[26].r = 255;
    impPalette[26].g = 128;
    impPalette[26].b = 0;

    impPalette[27].r = 255;
    impPalette[27].g = 64;
    impPalette[27].b = 64;

    impPalette[28].r = 64;
    impPalette[28].g = 255;
    impPalette[28].b = 128;

    impPalette[29].r = 0;
    impPalette[29].g = 64;
    impPalette[29].b = 128;

    impPalette[30].r = 128;
    impPalette[30].g = 64;
    impPalette[30].b = 128;

    impPalette[31].r = 0;
    impPalette[31].g = 128;
    impPalette[31].b = 64;
}

/*================Dithering==============================*/

void ditheringRGB(SDL_Surface* bmp, palette_5bit* palette)
{
        int width = bmp->w;
        int height = bmp->h;
        float bledyr[width+2][height+2];
        float bledyg[width+2][height+2];
        float bledyb[width+2][height+2];

        for(int i = 0; i < height; i++)
            for(int j = 0; j < width; j++){
                bledyr[i][j] = 0;
                bledyg[i][j] = 0;
                bledyb[i][j] = 0;
            }


        int negR,negG,negB;
        int index;
        int przesuniecie=1;
        int bladr=0;
        int bladg=0;
        int bladb=0;



        SDL_Color kolor;

        for(int y = 0; y < bmp->h; y++)
        for(int x = 0; x < bmp->w; x++){
            kolor = getPixelSurface(x, y, bmp);
            int R=kolor.r;
            int G=kolor.g;
            int B=kolor.b;

            R+=bledyr[x+przesuniecie][y];
            G+=bledyg[x+przesuniecie][y];
            B+=bledyb[x+przesuniecie][y];

            if(R>255) R=255;
            if(R<0) R=0;
            if(G>255) G=255;
            if(G<0) G=0;
            if(B>255) B=255;
            if(B<0) B=0;

            /*index = closestNeighbour(palette, color);
            setPixelSurface(x, y, palette[index].originalC.r, palette[index].originalC.g, palette[index].originalC.b, bmp);
            quantR = color.r - palette[index].originalC.r;
            quantG = color.g - palette[index].originalC.g;
            quantB = color.b - palette[index].originalC.b;
            index = closestNeighbour(palette, kolor);

            bladr = R - palette[index].originalC.r;
            bladr = G - palette[index].originalC.g;
            bladr = B - palette[index].originalC.b;

            setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);*/


            if(R>127&&G<=127&&B<=127)
            {
                kolor.r = 255;
                kolor.g = 0;
                kolor.b = 0;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R - 255;
                bladg = G;
                bladb = B;
            }
            else if(R<=127&&G>127&&B<=127)
            {
                kolor.r = 0;
                kolor.g = 255;
                kolor.b = 0;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R;
                bladg = G-255;
                bladb = B;
            }
            else if(R<=127&&G<=127&&B>127)
            {
                kolor.r = 0;
                kolor.g = 0;
                kolor.b = 255;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R;
                bladg = G;
                bladb = B-255;
            }
            else if(R>127&&G>127&&B<=127)
            {
                kolor.r = 255;
                kolor.g = 255;
                kolor.b = 0;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R - 255;
                bladg = G-255;
                bladb = B;
            }
            else if(R>127&&G<=127&&B>127)
            {
                kolor.r = 255;
                kolor.g = 0;
                kolor.b = 255;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R - 255;
                bladg = G;
                bladb = B-255;
            }
            else if(R<=127&&G>127&&B>127)
            {
                kolor.r = 0;
                kolor.g = 255;
                kolor.b = 255;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R;
                bladg = G-255;
                bladb = B-255;
            }
            else if(R>127&&G>127&&B>127)
            {
                kolor.r = 255;
                kolor.g = 255;
                kolor.b = 255;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R - 255;
                bladg = G - 255;
                bladb = B - 255;
            }
            else if(R<=127&&G<=127&&B<=127)
            {
                kolor.r = 0;
                kolor.g = 0;
                kolor.b = 0;
                index = closestNeighbour(palette, kolor);
                setPixelSurface(x,y,palette[index].originalC.r,palette[index].originalC.g,palette[index].originalC.b, bmp->w, bmp->h, bmp);
                bladr = R;
                bladg = G;
                bladb = B;
            }
            bledyr[x+przesuniecie+1][y  ]+=(bladr*7.0/16.0);
            bledyr[x+przesuniecie+1][y+1]+=(bladr*1.0/16.0);
            bledyr[x+przesuniecie  ][y+1]+=(bladr*5.0/16.0);
            bledyr[x+przesuniecie-1][y+1]+=(bladr*3.0/16.0);

            bledyg[x+przesuniecie+1][y  ]+=(bladg*7.0/16.0);
            bledyg[x+przesuniecie+1][y+1]+=(bladg*1.0/16.0);
            bledyg[x+przesuniecie  ][y+1]+=(bladg*5.0/16.0);
            bledyg[x+przesuniecie-1][y+1]+=(bladg*3.0/16.0);

            bledyb[x+przesuniecie+1][y  ]+=(bladb*7.0/16.0);
            bledyb[x+przesuniecie+1][y+1]+=(bladb*1.0/16.0);
            bledyb[x+przesuniecie  ][y+1]+=(bladb*5.0/16.0);
            bledyb[x+przesuniecie-1][y+1]+=(bladb*3.0/16.0);

            /*color = getPixelSurface(x, y, bmp);
            index = closestNeighbour(palette, color);
            setPixelSurface(x, y, palette[index].originalC.r, palette[index].originalC.g, palette[index].originalC.b, bmp);
            quantR = color.r - palette[index].originalC.r;
            quantG = color.g - palette[index].originalC.r;
            quantB = color.b - palette[index].originalC.r;

            color = getPixelSurface(x+1, y, bmp);
            setPixelSurface(x+1, y, color.r + (7.0f / 16.0f)*quantR, color.g + (7.0f / 16.0f)*quantG, color.b + (7.0f / 16.0f)*quantB, bmp);
            color = getPixelSurface(x-1, y+1, bmp);
            setPixelSurface(x-1, y+1, color.r + (3.0f / 16.0f)*quantR, color.g + (3.0f / 16.0f)*quantG, color.b + (3.0f / 16.0f)*quantB, bmp);
            color = getPixelSurface(x, y+1, bmp);
            setPixelSurface(x, y+1, color.r + (5.0f / 16.0f)*quantR, color.g + (5.0f / 16.0f)*quantG, color.b + (5.0f / 16.0f)*quantB, bmp);
            color = getPixelSurface(x+1, y+1, bmp);
            setPixelSurface(x+1, y+1, color.r + (1.0f / 16.0f)*quantR, color.g + (1.0f / 16.0f)*quantG, color.b + (1.0f / 16.0f)*quantB, bmp);*/
        }
}


void ditheringBW(SDL_Surface* bmp)
{
    SDL_Color color;
    int quantBW;
    int BW;

    for(int y = 0; y < bmp->h; y++)
        for(int x = 0; x < bmp->w; x++){
            color = getPixelSurface(x, y, bmp);
            BW = RGBtoBW(color);
            setPixelSurface(x, y, BW, BW, BW, bmp);
            quantBW = RGBtoBWorg(color) - RGBtoBW(color);

            color = getPixelSurface(x+1, y, bmp);
            BW = RGBtoBW(color);
            setPixelSurface(x+1, y, BW + (7.0f / 16.0f)*quantBW, BW + (7.0f / 16.0f)*quantBW, BW + (7.0f / 16.0f)*quantBW, bmp);
            color = getPixelSurface(x-1, y+1, bmp);
            BW = RGBtoBW(color);
            setPixelSurface(x-1, y+1, BW + (3.0f / 16.0f)*quantBW, BW + (3.0f / 16.0f)*quantBW, BW + (3.0f / 16.0f)*quantBW, bmp);
            color = getPixelSurface(x, y+1, bmp);
            BW = RGBtoBW(color);
            setPixelSurface(x, y+1, BW + (5.0f / 16.0f)*quantBW, BW + (5.0f / 16.0f)*quantBW, BW + (5.0f / 16.0f)*quantBW, bmp);
            color = getPixelSurface(x+1, y+1, bmp);
            BW = RGBtoBW(color);
            setPixelSurface(x+1, y+1, BW + (1.0f / 16.0f)*quantBW, BW + (1.0f / 16.0f)*quantBW, BW + (1.0f / 16.0f)*quantBW, bmp);
        }
}





