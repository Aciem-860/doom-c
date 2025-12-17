#include <stdlib.h>
#include "log.h"
#include "textures.h"
#include "sector.h"

SDL_Texture *wall_bricks_texture;
SDL_Texture *wall_stone_texture;
SDL_Texture *wall_steel_texture;
SDL_Texture *wall_blue_texture;
FloorTexture floor_wood_texture;
FloorTexture floor_stone_texture;

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

static FloorTexture init_floor_texture(Context *context, const char* path) {
    SDL_Surface* floor_surface = IMG_Load(path);
    
    FloorTexture floor_tex = { 0 };
    floor_tex.width = floor_surface->w;
    floor_tex.height = floor_surface->h;
    floor_tex.pixels = (Uint32*) malloc(floor_tex.width * floor_tex.height * sizeof(Uint32));
    floor_tex.texture = SDL_CreateTextureFromSurface(context->renderer, floor_surface);
    
    for (int x = 0; x < floor_surface->w; x++) {
        for (int y = 0; y < floor_surface->h; y++) {
            Uint32 pixel = get_pixel(floor_surface, x, y);
            pixel = correct_pixel(pixel, floor_surface);
            floor_tex.pixels[x + floor_surface->w * y] = pixel;
        }
    }
    
    SDL_FreeSurface(floor_surface);
    return floor_tex;
}

static void free_floor_texture(FloorTexture *tex) {
    free(tex->pixels);
    SDL_DestroyTexture(tex->texture);
}

void init_texture(Context* context) {
    wall_bricks_texture = IMG_LoadTexture(context->renderer, "img/bricks.png");
    wall_stone_texture  = IMG_LoadTexture(context->renderer, "img/stone.png");
    wall_steel_texture  = IMG_LoadTexture(context->renderer, "img/steel.png");
    wall_blue_texture   = IMG_LoadTexture(context->renderer, "img/blue.png");

    floor_wood_texture = init_floor_texture(context, "img/floor_wood.png");
    floor_stone_texture = init_floor_texture(context, "img/floor_stone.png");
}

void free_texture(void) {
    SDL_DestroyTexture(wall_bricks_texture);
    SDL_DestroyTexture(wall_stone_texture);
    SDL_DestroyTexture(wall_steel_texture);
    SDL_DestroyTexture(wall_blue_texture);
    free_floor_texture(&floor_wood_texture);
    free_floor_texture(&floor_stone_texture);
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
    default:
        return NULL;
    }
}

FloorTexture *get_floor_texture(FloorType floor_type) {
    switch (floor_type) {
    case FLOOR_PARQUET:
        return &floor_wood_texture;
    case FLOOR_STONE:
        return &floor_stone_texture;
    default:
        logg(ERROR, "Floor type not recognized");
        return NULL;
    }
}
