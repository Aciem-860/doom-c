#include "context.h"
#include <SDL2/SDL.h>

int set_color(SDL_Renderer* renderer, SDL_Color color)
{
    if (SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) < 0) {
        return -1;
    }
    return 0;
}
