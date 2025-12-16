#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "context.h"
#include "sector.h"

extern SDL_Texture *wall_bricks_texture;
extern SDL_Texture *wall_stone_texture;
extern SDL_Texture *wall_steel_texture;
extern SDL_Texture *wall_blue_texture;
extern SDL_Texture *floor_texture;
extern Uint32 floor_pixels[64 * 64];

Uint32 get_pixel(SDL_Surface* surface, int x, int y);
Uint32 correct_pixel(Uint32 pixel, SDL_Surface* surface);
void init_texture(Context* context);
void free_texture();
SDL_Texture *get_wall_texture(WallType type);

#endif
