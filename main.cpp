#include <exception>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "functions.h"

using namespace std;

int main(int argc, char* argv[]){

    bool flag = true;

    while(flag){
        flag = menu();
    }

    SDL_Quit();

    return 0;
}
