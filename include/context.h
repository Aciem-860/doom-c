#ifndef CONTEXT_H
#define CONTEXT_H

#include <SDL2/SDL.h>

typedef struct Context {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width, height;
} Context;

int set_color(SDL_Renderer* renderer, SDL_Color color);

#endif
