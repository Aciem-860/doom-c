#include "textures.h"
#include "sector.h"

SDL_Texture *wall_bricks_texture;
SDL_Texture *wall_stone_texture;
SDL_Texture *wall_steel_texture;
SDL_Texture *wall_blue_texture;
SDL_Texture *floor_texture;

Uint32 get_pixel(SDL_Surface* surface, int x, int y) {
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
    Uint32 pixel;

    switch (bpp) {
    case 1:
        pixel = *p;
        break;
    case 2:
        pixel = *(Uint16*)p;
        break;
    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            pixel = p[0] << 16 | p[1] << 8 | p[2];
        } else {
            pixel = p[0] | p[1] << 8 | p[2] << 16;
        }
        break;
    case 4:
        pixel = *(Uint32*)p;
        break;
    default:
        pixel = 0; // Format not handled
        break;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
    return pixel;
}

Uint32 correct_pixel(Uint32 pixel, SDL_Surface* surface) {
    Uint8 r, g, b;
    SDL_GetRGB(pixel, surface->format, &r, &g, &b);
    Uint32 ret = b + (g << 8) + (r << 16);
    return ret;
}

void init_texture(Context* context) {
    wall_bricks_texture = IMG_LoadTexture(context->renderer, "img/bricks.png");
    wall_stone_texture  = IMG_LoadTexture(context->renderer, "img/stone.png");
    wall_steel_texture  = IMG_LoadTexture(context->renderer, "img/steel.png");
    wall_blue_texture   = IMG_LoadTexture(context->renderer, "img/blue.png");

    SDL_Surface* floor_surface = IMG_Load("img/floor.png");

    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            Uint32 pixel = get_pixel(floor_surface, x, y);
            pixel = correct_pixel(pixel, floor_surface);
            floor_pixels[x + 64 * y] = pixel;
        }
    }
    
    floor_texture = SDL_CreateTextureFromSurface(context->renderer, floor_surface);
    SDL_FreeSurface(floor_surface);
}

void free_texture() {
    SDL_DestroyTexture(wall_bricks_texture);
    SDL_DestroyTexture(wall_stone_texture);
    SDL_DestroyTexture(wall_steel_texture);
    SDL_DestroyTexture(wall_blue_texture);
    SDL_DestroyTexture(floor_texture);
}

SDL_Texture *get_wall_texture(WallType type) {
    switch (type) {
    case WALL_BRICKS:
        return wall_bricks_texture;
    case WALL_STONE:
        return wall_stone_texture;
    case WALL_STEEL:
        return wall_steel_texture;
    case WALL_BLUE:
        return wall_blue_texture;
    }
}
