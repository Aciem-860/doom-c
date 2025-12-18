#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "context.h"
#include "texture_types.h"

typedef struct FloorTexture {
    SDL_Texture *texture;
    Uint32 *pixels;
    int width;
    int height;
} FloorTexture;

extern SDL_Texture *wall_bricks_texture;
extern SDL_Texture *wall_stone_texture;
extern SDL_Texture *wall_steel_texture;
extern SDL_Texture *wall_blue_texture;
extern FloorTexture floor_wood_texture;
extern FloorTexture floor_stone_texture;

Uint32 get_pixel(SDL_Surface* surface, int x, int y);
Uint32 correct_pixel(Uint32 pixel, SDL_Surface* surface);
void init_texture(Context* context);
void free_texture(void);
SDL_Texture *get_wall_texture(WallType type);
FloorTexture *get_floor_texture(FloorType floor_type);
    
#endif
