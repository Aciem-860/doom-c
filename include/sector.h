#ifndef SECTOR_H
#define SECTOR_H

#include <SDL2/SDL_image.h>
#include "../include/context.h"
#include "../include/player.h"

typedef enum {
    WALL_BRICKS,
    WALL_STONE,
    WALL_STEEL,
    WALL_BLUE
} WallType;


typedef struct {
    double start_x, end_x;
    double start_y, end_y;
    int portal; // < 0 = solid wall ; > 0 == tag of the next sector
    WallType wall_type;
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

void init_texture(Context* context);
int load_level(const char *path);

void print_sector(Sector *);
void print_wall(Wall *);

void render_wall(Wall *, Position *, Context *);

#endif
