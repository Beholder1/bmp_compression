#pragma once
#include <iostream>
#include "paletteFunctions.h"
#include "huffman.h"
#include <fstream>

void BMP_to_WSS_conversion(SDL_Surface* bmp);
void WSS_to_BMP_conversion(std::string path);

bool menu()
{
    char choice;
    std::string path;

    std::cout << "\nWybierz operacje: \n";
    std::cout << "0 - Wyjscie z programu\n";
    std::cout << "1 - konwersja z pliku BMP na WSS\n";
    std::cout << "2 - konwersja z pliku WSS na BMP\n\n";
    std::cin >> choice;
    system("cls");
    switch(choice)
    {
    case '0':
        return false;
    case '1':{
        imposedColors();
        std::cout << "\nPodaj sciezke do pliku BMP: ";
        getline(std::cin, path);
        std::cin >> path;
        int rozmiar=path.length();
        if (rozmiar < 5)
        {
            path = path + ".bmp";
        }
        else if ((path[rozmiar - 4] == '.') && (path[rozmiar - 3] == 'b') && (path[rozmiar - 2] == 'm') && (path[rozmiar - 1] == 'p'))
        {
        }
        else
        {
            path = path + ".bmp";
        }
        system("cls");
        SDL_Surface* bmp = SDL_LoadBMP(path.c_str());
        if (!bmp){
            printf("Unable to load bitmap: %s\n", SDL_GetError());
        }
        else{
            BMP_to_WSS_conversion(bmp);
        }
        return true;
    }
    case '2':{
        std::cout << "\nPodaj sciezke do pliku WSS: ";
        getline(std::cin, path);
        std::cin >> path;
        int rozmiar=path.length();
        if (rozmiar < 5)
        {
            path = path + ".wss";
        }
        else if ((path[rozmiar - 4] == '.') && (path[rozmiar - 3] == 'w') && (path[rozmiar - 2] == 's') && (path[rozmiar - 1] == 's'))
        {
        }
        else
        {
            path = path + ".wss";
        }
        system("cls");
        std::fstream p;
        p.open(path);
        if(p.good()){
            p.close();
            WSS_to_BMP_conversion(path);
        }
        else{
            printf("Unable to load wss\n");
        }
        return true;
    }
    }
}

void BMP_to_WSS_conversion(SDL_Surface* bmp)
{
    char signature[] = "WSS";
    Uint16 biWidth = bmp->w; //szerokosc pobrana z pliku bmp
    Uint16 biHeight = bmp->h; //wysokosc pobrana z pliku bmp
    int colorMode;
    bool dithering = true;

    SDL_Color color; //zmienna tymczasowa przechowujaca kolor piksela
    int index;

    palette_5bit palette[32]; // struktura przechowujaca palete ktora zostanie wpisana do pliku
    loop5BitPalette(palette);

    do
    {
        std::cout << "\nPodaj typ konwersji: \n";
        std::cout << "0 - paleta dedykowana\n";
        std::cout << "1 - paleta narzucona\n";
        std::cout << "2 - skala szarosci\n";
        std::cout << "3 - powrot\n\n";
        std::cin >> colorMode;
        system("cls");
        switch(colorMode)
        {
        case 0:
            dedicatedPalette(bmp, palette);
            break;
        case 1:
            imposedPalette(bmp, palette);
            break;
        case 2:
            greyScale(bmp, palette);
            break;
        case 3:
            return;
    }
    }while(colorMode>2||colorMode<0);
    system("cls");

    char choice = ' ';

    do
    {
        std::cout << "\nDithering obrazu: ";
        std::cin >> choice;
        if(choice  == 'n')
            dithering = false;
        if(choice == 't')
            dithering = true;
    }while(choice != 't' && choice != 'n');

    system("cls");

    std::cout << "\nPodaj nazwe zapisywanego pliku: ";
    std::string fileName;
    std::cin>>fileName;
    int rozmiar=fileName.length();
    if (rozmiar < 5)
	{
		fileName = fileName + ".wss";
	}
	else if ((fileName[rozmiar - 4] == '.') && (fileName[rozmiar - 3] == 'w') && (fileName[rozmiar - 2] == 's') && (fileName[rozmiar - 1] == 's'))
	{
	}
	else
	{
		fileName = fileName + ".wss";
	}

    Uint32* huffDict = Dictionary(palette, countSum(palette));
    int* codeLength = dictCodeLength(huffDict); //tablica przechowujaca dlugosci kodow slownika huffmana
    std::ofstream output(fileName, std::ios::binary);

    output.write((char*)&signature, sizeof(char)*3);
    output.write((char*)&biWidth, sizeof(Uint16));
    output.write((char*)&biHeight, sizeof(Uint16));
    output.write((char*)&colorMode, sizeof(Uint8));

    if(colorMode < 2){
        if(dithering == true)
            ditheringRGB(bmp, palette);
        for(int i = 0; i < 32; i++){
            output.write((char*)&palette[i].originalC.r, sizeof(Uint8));
            output.write((char*)&palette[i].originalC.g, sizeof(Uint8));
            output.write((char*)&palette[i].originalC.b, sizeof(Uint8));
        }
    }
    else
        if(dithering == true)
            ditheringBW(bmp);


    for(int i = 0; i < 32; i++)
        output.write((char*)&huffDict[i], sizeof(Uint32));


    bufor link_variable;

    if(colorMode < 2)
        for (int y = 0; y < bmp->h; y++)
            for (int x = 0; x < bmp->w; x++){
                color = getPixelSurface(x, y, bmp);
                index = closestNeighbour(palette, color);
                huffmanCompresion(link_variable, huffDict[index], codeLength[index], output);
            }
    else
        for (int y = 0; y < bmp->h; y++)
            for (int x = 0; x < bmp->w; x++){
                color = getPixelSurface(x, y, bmp);
                index = RGBtoBWindex(color);
                huffmanCompresion(link_variable, huffDict[index], codeLength[index], output);
            }

    output.close();

    system("cls");

}

void WSS_to_BMP_conversion(std::string path)
{
    char signature[] = "   ";
    Uint16 biWidth; //szerokosc pobrana z pliku bmp
    Uint16 biHeight; //wysokosc pobrana z pliku bmp
    Uint8 colorMode;

    SDL_Color color; //zmienna tymczasowa przechowujaca kolor piksela

    palette_5bit palette[32]; // struktura przechowujaca palete ktora zostanie wpisana do pliku
    loop5BitPalette(palette); // zapetlenie wartosci 5 bitowych do 24 bitowych odpowiednikow
    Uint32 huffDict[32]; // slownik huffmana


    std::ifstream input(path, std::ios::binary);

    input.read((char*)&signature, sizeof(char)*3);
    input.read((char*)&biWidth, sizeof(Uint16));
    input.read((char*)&biHeight, sizeof(Uint16));
    input.read((char*)&colorMode, sizeof(Uint8));

    if((int)colorMode < 2)
        for(int i = 0; i < 32; i++){
            input.read((char*)&palette[i].originalC.r, sizeof(Uint8));
            input.read((char*)&palette[i].originalC.g, sizeof(Uint8));
            input.read((char*)&palette[i].originalC.b, sizeof(Uint8));
        }


    SDL_Surface* bmp = SDL_CreateRGBSurface(0, biWidth, biHeight, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);




    for(int i = 0; i < 32; i++)
        input.read((char*)&huffDict[i], sizeof(Uint32));

    bufor link_variable;
    int* codeLength = dictCodeLength(huffDict); //tablica przechowujaca dlugosci kodow slownika huffmana
    std::vector<Uint8> indexTab;
    Uint32 currBufor;

    int x = 0, y = 0;

    int count = 0;

    if((int)colorMode < 2){
        while(x < biWidth && y < biHeight){
            input.read((char*)&currBufor, sizeof(Uint32));
            huffmanDecompres(link_variable, currBufor, huffDict, codeLength, indexTab);

            while(!indexTab.empty()){
                color = palette[*indexTab.begin()].originalC;
                //setPixel(x, y, color.r, color.g, color.b, biWidth, biHeight, screen);
                setPixelSurface(x, y, color.r, color.g, color.b, biWidth, biHeight, bmp);
                indexTab.erase(indexTab.begin());
                x++;
                count++;
                if(x == biWidth){
                    y++;
                    if(y == biHeight && x == biWidth)
                        break;
                    x = 0;
                }
            }
        }
    }
    else{
        Uint8 index;

        while(x < biWidth && y < biHeight){
            input.read((char*)&currBufor, sizeof(Uint32));
            huffmanDecompres(link_variable, currBufor, huffDict, codeLength, indexTab);

            while(!indexTab.empty()){
                index = *indexTab.begin();
                index <<= 3;
                //setPixel(x, y, index, index, index, biWidth, biHeight, screen);
                setPixelSurface(x, y, index, index, index, biWidth, biHeight, bmp);
                indexTab.erase(indexTab.begin());
                x++;
                count++;
                if(x == biWidth){
                    y++;

                    if(y == biHeight)
                        break;
                    x = 0;
                }
            }
        }
    }


    std::string pathBMP;
    std::cout <<"\nPodaj nazwe dla pliku BMP: ";
    getline(std::cin, pathBMP);
    std::cin >> pathBMP;
    pathBMP += ".bmp";

    SDL_SaveBMP(bmp, pathBMP.c_str());

    input.close();
    system("cls");
}
