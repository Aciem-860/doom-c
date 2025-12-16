#include <SDL2/SDL_render.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/param.h>

#include "sector.h"
#include "player.h"
#include "textures.h"
#include "context.h"
#include "log.h"

#define swap(x, y) do {                         \
        (x) ^= (y);                             \
        (y) ^= (x);                             \
        (x) ^= (y);                             \
    } while(0)

const double NEAR_PLANE = 1.0;
double tan_fov;

enum parser_state { SECTOR, WALL };
enum wall_cut_side { INTACT, LEFT, RIGHT };

Sector sectors[10];
Wall walls[10];

int sector_number;
int wall_number;
int current_sector;

Uint32 floor_pixels [64 * 64];

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

    logg(INFO, "Sector Number: %d", sector_number);
    logg(INFO, "Wall Number  : %d", wall_number);

    fclose(level_file);
    return 0;
}

static Uint32 get_pixel_from_texture(SDL_Texture *texture, int x, int y) {
    void* pixels;
    int pitch;

    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    Uint8* base = (Uint8*)pixels;
    Uint32* row = (Uint32*)(base + y * pitch);
    Uint32 color = row[x];
    printf("COLOR = %d\n", color);

    SDL_UnlockTexture(texture);

    return color;
}

static const double wall_height_real = 50.0; // TODO: remplacer par valeur du secteur

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


void render_floor(Context* context, Position* camera) {
    int texture_width, texture_height;
    SDL_QueryTexture(floor_texture, NULL, NULL, &texture_width, &texture_height);
    SDL_Texture* tex_floor_strip = SDL_CreateTexture(context->renderer,
                                                     SDL_PIXELFORMAT_ARGB8888,
                                                     SDL_TEXTUREACCESS_TARGET,
                                                     context->width,
                                                     context->height/2);

    Uint32 pixels_buffer[context->width * context->height/2];
    double z_pos = 19 * (double) context->height;
    
    for (int line = context->height/2+1; line < context->height; line++) {

        double y = (double) line - (double) context->height/2; // y > 0

        // --- Rayons gauche et droit ---
        double xl, yl;
        double xr, yr;
        double distance_row = z_pos / y;

        // Positions des rayons dans le monde réel
        //  => différent du monde de projection
        xl = distance_row;
        xr = distance_row;

        yl = tan_fov * distance_row;
        yr = -yl;

        rotate(xl, yl, -camera->angle, &xl, &yl);
        rotate(xr, yr, -camera->angle, &xr, &yr);

        xl += camera->x;
        xr += camera->x;
        yr -= camera->y;
        yl -= camera->y;
        

        // Taille du rayon perpendiculaire à la
        // direction de regard (rayon du faisceau)

        double step_x = (xr - xl) / context->width;
        double step_y = (yr - yl) / context->width;
        
        for (int x = 0; x < context->width; x++) {

            int x_ray = (int)(xl + x * step_x) % texture_width;
            int y_ray = (int)(yl + x * step_y) % texture_height;

            if (x_ray < 0) x_ray = (texture_width - x_ray) % texture_width;
            if (y_ray < 0) y_ray = (texture_height - y_ray) % texture_height;

            /* printf("X RAY = %d, Y RAY = %d\n", x_ray, y_ray); */

            pixels_buffer[x + context->width * (int) y] = floor_pixels[x_ray + texture_width * y_ray];
        }
    }
    
    SDL_SetRenderTarget(context->renderer, tex_floor_strip);
    SDL_UpdateTexture(tex_floor_strip, NULL, pixels_buffer,
                      sizeof(Uint32) * context->width);
    SDL_Rect dst_area = {0, context->height/2, context->width, context->height/2};
        
    SDL_SetRenderTarget(context->renderer, NULL);
    SDL_RenderCopy(context->renderer, tex_floor_strip, NULL, &dst_area);
    SDL_DestroyTexture(tex_floor_strip);        
}

static int get_projection(double x,
                          double y,
                          double distance_to_screen,
                          int* x_proj,
                          int* wall_height)
{
    if (x < NEAR_PLANE) return 1; // SHOULD NOT HAPPEN

    double angle = atan2(y, x);
    
    *x_proj = (int)((y / x) * distance_to_screen);
    *wall_height = (distance_to_screen * wall_height_real / x);
    return 0;
}


/// Vérifie que le mur n'est pas trop proche (depuis une de ses extrémités),
/// et renvoie les premières coordonnées qui sont suffisamment loins.
/// Si aucun point n'est trouvé, alors cela signifie que le mur est trop proche
/// du début à la fin et qu'il ne faut pas tenter de l'afficher à l'écran.
/// Renvoie 1 le cas échéant, 0 si un point a été trouvé.
static int check_wall_distance_and_find_another(Wall* wall, Wall* actual_wall) {
    double sx, ex, sy, ey;
    
    if (wall->start_x < NEAR_PLANE && wall->end_x < NEAR_PLANE) {
        return 1;
    } else if (wall->start_x < NEAR_PLANE && wall->end_x > NEAR_PLANE) {
        sx = wall->start_x;
        sy = wall->start_y;
        ex = wall->end_x;
        ey = wall->end_y;
        
    } else if (wall->start_x > NEAR_PLANE && wall->end_x < NEAR_PLANE) {
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
    
    double step = 0.05;
    int mx = (int) (1.0 / step);
    for (int i = 0; i < mx + 1; i++) {
        double t = step * i; // t inclus dans [0,1]
        int x = (int) (sx + t * (ex - sx));
        int y = (int) (sy + t * (ey - sy));

        if (x < NEAR_PLANE) continue;
            
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
    if (wall->portal >= 0) return; // portal to another sector
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
    if (sx < NEAR_PLANE && ex < NEAR_PLANE) return;
    
    
    // --- 4. Projection sur l'écran ---
    double x_center = (double)context->width / 2.0;

    int sx_p, ex_p;
    int start_wall_height, end_wall_height;

    if (check_wall_distance_and_find_another(&rotated_wall, &actual_wall)) return;
    if (get_projection(actual_wall.start_x, actual_wall.start_y,
                       camera->distance_to_screen, &sx_p, &start_wall_height)) return;
    if (get_projection(actual_wall.end_x, actual_wall.end_y,
                       camera->distance_to_screen, &ex_p, &end_wall_height)) return;

    // --- 5. Tracer le mur ---

    if (sx_p > ex_p) {
        swap(sx_p, ex_p);
        swap(start_wall_height, end_wall_height);
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
