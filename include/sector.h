#ifndef SECTOR_H
#define SECTOR_H

#include <SDL2/SDL_image.h>
#include "context.h"
#include "player.h"
#include "dynamic_array.h"
#include "texture_types.h"

#define MAX_NB_SECTORS 20
#define MAX_NB_WALLS 100

extern double tan_fov;

typedef struct Wall {
    double start_x, end_x;
    double start_y, end_y;
    int portal; // < 0 = solid wall ; > 0 == tag of the next sector
    WallType wall_type;
    int sector;
} Wall;

typedef struct Sector {
    // MISSING
    // Ceiling texture
    // Floor texture
    int tag;
    int cell_height;
    int floor_height;
    double brightness;
    FloorType floor_type;
    DynamicArray_int walls_id;
} Sector;


extern Sector sectors[MAX_NB_SECTORS];
extern Wall walls[MAX_NB_WALLS];

extern int sector_number;
extern int wall_number;

void init_texture(Context *context);
void free_texture(void);
int load_level(const char *path);

void print_sector(Sector *);
void print_wall(Wall *);

void render_floor(Context *context, Position *camera);
void render_sector(Context *context, Position *camera, Sector *sector);
void reset_visited_sectors();
int is_point_in_sector(Position *position, Sector *sector);
#endif
