#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <sys/param.h>

#include "sector.h"
#include "player.h"
#include "context.h"
#include "log.h"

enum parser_state { SECTOR, WALL };
enum wall_cut_side { INTACT, LEFT, RIGHT };

Sector sectors[10];
Wall walls[10];

int sector_number;
int wall_number;

SDL_Texture *wall_bricks_texture;
SDL_Texture *wall_stone_texture;
SDL_Texture *wall_steel_texture;
SDL_Texture *wall_blue_texture;

void init_texture(Context* context) {
    wall_bricks_texture = IMG_LoadTexture(context->renderer, "img/bricks.png");
    wall_stone_texture = IMG_LoadTexture(context->renderer, "img/stone.png");
    wall_steel_texture = IMG_LoadTexture(context->renderer, "img/steel.png");
    wall_blue_texture = IMG_LoadTexture(context->renderer, "img/blue.png");
}

static SDL_Texture *get_wall_texture(WallType type) {
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

void print_sector(Sector * s)
{
    logg(DEBUG, "TAG : %d", s->tag);
    logg(DEBUG, "CELL_HEIGHT : %d", s->cell_height);
    logg(DEBUG, "FLOOR_HEIGHT : %d", s->floor_height);
    logg(DEBUG, "BRIGHTNESS : %f", s->brightness);
    logg(DEBUG, "FIRST_WALL : %d", s->first_wall);
    logg(DEBUG, "WALLS_NUMBER : %d", s->walls_number);
}

void print_wall(Wall * w)
{
    logg(DEBUG, "START_X : %f", w->start_x);
    logg(DEBUG, "END_X : %f", w->end_x);
    logg(DEBUG, "START_Y : %f", w->start_y);
    logg(DEBUG, "END_Y : %f", w->end_y);
    logg(DEBUG, "PORTAL : %d", w->portal);
}


int load_level(const char* path)
{
    FILE* level_file;
    level_file = fopen(path, "r");

    char line[100];


    enum parser_state state = SECTOR;

    int sector_id = 0;
    int wall_id = 0;

    int line_nb = 0;
    while (fgets(line, 100, level_file)) {
        if (line_nb == 0) {
            if (strcmp(line, "[SECTOR]\n") != 0) {
                fprintf(stderr,
                        "[%s] should start with the header [SECTOR], but find < %s >\n",
                        path,
                        line);
                return -1;
            }
            goto Next;
        }

        if (strcmp(line, "[WALLS]\n") == 0) {
            state = WALL;
            goto Next;
        }


        char* token = strtok(line, " ");
        int tok_id = 0;
        
        // Parsing the line
        if (state == SECTOR) {
            Sector sector;
            while (token) {
                switch (tok_id) {
                case 0:
                    sector.tag = atoi(token);
                    break;
                case 1:
                    sector.cell_height = atoi(token);
                    break;
                case 2:
                    sector.floor_height = atoi(token);
                    break;
                case 3:
                    sector.brightness = atof(token);
                    break;
                case 4:
                    sector.first_wall = atoi(token);
                    break;
                case 5:
                    sector.walls_number = atoi(token);
                    break;
                default:
                    continue;
                }

                token = strtok(NULL, " ");
                tok_id++;
            }
            sectors[sector_id] = sector;
            sector_id++;
        }

        if (state == WALL) {
            Wall wall;
            while (token) {
                double toki = atof(token);
                switch (tok_id) {
                case 0:
                    wall.start_x = toki;
                    break;
                case 1:
                    wall.start_y = toki;
                    break;
                case 2:
                    wall.end_x = toki;
                    break;
                case 3:
                    wall.end_y = toki;
                    break;
                case 4:
                    wall.portal = toki;
                    break;
                case 5:
                    wall.wall_type = toki;
                    break;
                default:
                    continue;
                }

                token = strtok(NULL, " ");
                tok_id++;
            }
            walls[wall_id] = wall;
            wall_id++;
        }

        
    Next:
        line_nb++;
    }

    sector_number = sector_id;
    wall_number = wall_id;

    fclose(level_file);
    return 0;
}

static const double wall_height_real = 40.0; // TODO: remplacer par valeur du secteur

static void render_col(Context *context, int x, int wh, int ww,
                       double percent, WallType wall_type)
{
    if (wh > 0) {
        int texture_width;
        int texture_height;
        SDL_Texture* wall_texture = get_wall_texture(wall_type);
        SDL_QueryTexture(wall_texture, NULL, NULL, &texture_width, &texture_height);
        
        int wall_col = (int) (percent * ww);

        int texture_col = wall_col % texture_width;
        
        SDL_Rect src_rect = { texture_col, 0, 1, texture_height };
        SDL_Rect dst_rect = { x, (context->height -  wh)/2, 1, wh+1 };

        SDL_RenderCopy(context->renderer, wall_texture, &src_rect, &dst_rect);
    }
}

static void get_projection(double x,
                           double y,
                           double distance_to_screen,
                           int* x_proj,
                           int* wall_height)
{
    const double MIN_DIST = 1e-6;
    if (x < MIN_DIST) x = MIN_DIST;
    
    double angle = atan2(y, x);
    
    *x_proj = (int)((y / x) * distance_to_screen);
    *wall_height = (distance_to_screen * wall_height_real / x);
}


/// Vérifie que le mur n'est pas trop proche (depuis une de ses extrémités),
/// et renvoie les premières coordonnées qui sont suffisamment loins.
/// Si aucun point n'est trouvé, alors cela signifie que le mur est trop proche
/// du début à la fin et qu'il ne faut pas tenter de l'afficher à l'écran.
/// Renvoie 1 le cas échéant, 0 si un point a été trouvé.
static int check_wall_distance_and_find_another(Wall* wall, Wall* actual_wall) {
    double const MIN_DIST = 1e-5;    // distance minimal au mur avant qu'il ne soit plus affiché
    double sx, ex, sy, ey;
    
    if (wall->start_x < MIN_DIST && wall->end_x < MIN_DIST) {
        return 1;
    } else if (wall->start_x < MIN_DIST && wall->end_x > MIN_DIST) {
        sx = wall->start_x;
        sy = wall->start_y;
        ex = wall->end_x;
        ey = wall->end_y;
        
    } else if (wall->start_x > MIN_DIST && wall->end_x < MIN_DIST) {
        // End est OK mais pas Start
        // On part de START
        sx = wall->end_x;
        sy = wall->end_y;
        ex = wall->start_x;
        ey = wall->start_y;
    } else {
        // Les deux sont OK, on n'a rien à faire
        memcpy(actual_wall, wall, sizeof(*wall));
        return 0;
    } 
    
    double step = 0.01;
    int mx = (int) (1.0 / step);
    for (int i = 0; i < mx + 1; i++) {
        double t = step * i; // t inclus dans [0,1]
        int x = (int) (sx + t * (ex - sx));
        int y = (int) (sy + t * (ey - sy));

        if (x < MIN_DIST) continue;
            
        actual_wall->start_x = x;
        actual_wall->start_y = y;
        actual_wall->end_x = ex;
        actual_wall->end_y = ey;
        return 0;
    }
    // Le mur est clairement trop proche pour être affiché
    return 1;    
}

void render_wall(Wall *wall, Position *camera, Context *context)
{
    const double EPS = 1e-6;         // tolérance numérique
    
    // --- 1. Coordonnées murales relatives à la caméra ---
    double sx = wall->start_x - camera->x;
    double sy = wall->start_y - camera->y;
    double ex = wall->end_x   - camera->x;
    double ey = wall->end_y   - camera->y;
    
    // --- 2. Rotation : mettre la caméra dans l'axe X+ (direction caméra = (1,0)) ---
    rotate(sx, sy, -camera->angle, &sx, &sy);
    rotate(ex, ey, -camera->angle, &ex, &ey);

    Wall rotated_wall = { sx, ex, sy, ey };
    Wall actual_wall;

    // --- 3. Si point derrière la caméra => ne pas rendre ---
    if (sx < 0 && ex < 0) return;
    
    
    // --- 4. Projection sur l'écran ---
    double x_center = (double)context->width / 2.0;

    int sx_p, ex_p;
    int start_wall_height, end_wall_height;


    if (check_wall_distance_and_find_another(&rotated_wall, &actual_wall)) return;
    
    get_projection(actual_wall.start_x, actual_wall.start_y, camera->distance_to_screen, &sx_p, &start_wall_height);
    get_projection(actual_wall.end_x, actual_wall.end_y, camera->distance_to_screen, &ex_p, &end_wall_height);

    // --- 5. Tracer le mur ---

    if (sx_p > ex_p) {
        int t = sx_p;
        sx_p = ex_p;
        ex_p = t;

        t = start_wall_height;
        start_wall_height = end_wall_height;
        end_wall_height = t;
    }

    int sx_d = sx_p + x_center;
    int ex_d = ex_p + x_center;
    int wall_width = abs(ex_p - sx_p);

    if (wall_width <= 0) return;

    double wx = ex - sx;
    double wy = ey - sy;

    double real_wall_width = sqrt(wx * wx + wy * wy);
    
    for (int col = 0; col < wall_width; col++) {
        int screen_x = col + sx_d;
        
        if (screen_x < 0 || screen_x >= context->width)
            continue;
        
        double wh = (((double) (col * (end_wall_height - start_wall_height)) / (double)(2 * wall_width)) + (double)start_wall_height / 2);
        
        if (wh < EPS) continue;
        
        double x = camera->distance_to_screen;
        double y = sx_p + col;


        double det = x * wy - y * wx;
        if (fabs(det) < 1e-9) return;

        double u = (sx * y - sy * x) / det;
        render_col(context, screen_x, (int)(2*wh), real_wall_width, u, wall->wall_type);
    }
}
