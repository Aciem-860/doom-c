#ifndef SECTOR_H
#define SECTOR_H

#include "../include/context.h"
#include "../include/player.h"

typedef struct wall {
    double start_x, end_x;
    double start_y, end_y;
    int portal; // < 0 = solid wall ; > 0 == tag of the next sector
} Wall;

typedef struct sector {
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

extern struct sector sectors[10];
extern struct wall walls[10];

extern int sector_number;
extern int wall_number;

int load_level(const char *path);

void print_sector(struct sector *);
void print_wall(struct wall *);

void render_wall(struct wall *, struct position *, Context *);

#endif
