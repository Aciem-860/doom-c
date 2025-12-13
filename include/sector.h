#ifndef SECTOR_H
#define SECTOR_H

#include <SDL2/SDL_image.h>
#include "../include/context.h"
#include "../include/player.h"

typedef struct {
    double start_x, end_x;
    double start_y, end_y;
    int portal; // < 0 = solid wall ; > 0 == tag of the next sector
} Wall;

typedef struct {
    // Missing:
    // Ceiling texture
    // Floor texture
    int tag;
    int cell_height;
    int floor_height;
    double brightness;
    int first_wall;
    int walls_number;
} Sector;

extern Sector sectors[10];
extern Wall walls[10];

extern int sector_number;
extern int wall_number;

extern SDL_Texture * wall_texture;

void init_texture(Context* context);
int load_level(const char *path);

void print_sector(Sector *);
void print_wall(Wall *);

void render_wall(Wall *, Position *, Context *);

#endif
