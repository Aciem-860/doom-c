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
#include "vector.h"
#include "player.h"
#include "textures.h"
#include "context.h"
#include "log.h"

#define swap(x, y) do {                         \
        (x) ^= (y);                             \
        (y) ^= (x);                             \
        (x) ^= (y);                             \
    } while(0)

const double NEAR_PLANE = 1;
double tan_fov;

double z_pos;

enum parser_state { SECTOR, WALL };
enum wall_cut_side { INTACT, LEFT, RIGHT };

Sector sectors[MAX_NB_SECTORS];
Wall walls[MAX_NB_WALLS];

int sector_number;
int wall_number;

Uint32 floor_pixels [64 * 64];

int load_level(const char* path) {
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

        if (strcmp(line, "\n") == 0 || line[0] == '#') {
            continue;
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
                    sector.floor_type = atoi(token);
                    break;
                default:
                    break;
                }
                
                token = strtok(NULL, " ");
                tok_id++;
            }


            DynamicArray_int walls_id = NewArray();
            walls_id.values = NULL;
            sector.walls_id = walls_id;
           
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
                case 6:
                    wall.sector = toki;
                    break;
                default:
                    continue;
                }

                token = strtok(NULL, " ");
                tok_id++;
            }
            walls[wall_id] = wall;
            da_append(&(sectors[wall.sector].walls_id), wall_id);
            wall_id++;
        }

        
    Next:
        line_nb++;
    }

    sector_number = sector_id;
    wall_number = wall_id;

    logg(INFO, "Sector Number: %d", sector_number);
    logg(INFO, "Wall Number  : %d", wall_number);

    for (int sec = 0; sec < sector_number; sec++) {
        logg(INFO, "< SECTOR %d >", sec);
        for (size_t i = 0; i < sectors[sec].walls_id.count; i ++) {
            logg(INFO, "   * Wall n°%d", sectors[sec].walls_id.values[i]);
        }
    }

    fclose(level_file);
    return 0;
}

static const double wall_height_real = 50.0; // TODO: remplacer par valeur du secteur

static void render_col(Context *context, int x, int wh, int ww,
                       double percent, WallType wall_type, double distance)
{
    if (wh <= 0 || distance < 1e-9) {
        return;
    }

    int texture_width, texture_height;
    SDL_Texture* wall_texture = get_wall_texture(wall_type);
    SDL_QueryTexture(wall_texture, NULL, NULL, &texture_width, &texture_height);

    int wall_col = (int)(percent * ww);
    int texture_col = wall_col % texture_width;

    int y_low = distance;
    int y_high = y_low - wh;

    SDL_Rect src_rect = { texture_col, 0, 1, texture_height };
    SDL_Rect dst_rect = { x, y_high, 1, wh };

    SDL_RenderCopy(context->renderer, wall_texture, &src_rect, &dst_rect);
}


static int find_current_sector(Position *position) {
    for (int s = 0; s < sector_number; s++) {
        if (is_point_in_sector(position, &(sectors[s]))) {
            return s;
        }
    }
    return -1;
}

void render_floor(Context* context, Position* camera)
{
    SDL_Texture* tex_floor_strip = SDL_CreateTexture(context->renderer,
                                                     SDL_PIXELFORMAT_ARGB8888,
                                                     SDL_TEXTUREACCESS_TARGET,
                                                     context->width,
                                                     context->height/2);
    z_pos = 20 * context->height;
    Uint32 pixels_buffer[context->width * context->height/2];
    
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

        Position pos = { 0, 0 };
        for (int x = 0; x < context->width; x++) {

            double x_real_world = (double)(xl + x * step_x);
            double y_real_world = (double)(yl + x * step_y);


            pos.x = x_real_world;
            pos.y = -y_real_world;
            
            int in_sector = find_current_sector(&pos);
            if (in_sector == -1) {
                pixels_buffer[x + context->width * (int) y] = 0x000000;
                continue;
            }

            FloorTexture *floor_tex = get_floor_texture(sectors[in_sector].floor_type);
            
            int x_ray = (int)x_real_world % floor_tex->width;
            int y_ray = (int)y_real_world % floor_tex->height;

            if (x_ray < 0) x_ray = (floor_tex->width - x_ray) % floor_tex->width;
            if (y_ray < 0) y_ray = (floor_tex->height - y_ray) % floor_tex->height;
            
            pixels_buffer[x + context->width * (int) y] = floor_tex->pixels[x_ray + floor_tex->width * y_ray];
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

static void get_projection(double x,
                           double y,
                           double distance_to_screen,
                           int* x_proj,
                           int* wall_height)
{
    *x_proj = (int)((y / x) * distance_to_screen);
    *wall_height = (distance_to_screen * wall_height_real / x);
}


/// Vérifie que le mur n'est pas trop proche (depuis une de ses extrémités),
/// et renvoie les premières coordonnées qui sont suffisamment loins.
/// Si aucun point n'est trouvé, alors cela signifie que le mur est trop proche
/// du début à la fin et qu'il ne faut pas tenter de l'afficher à l'écran.
/// Renvoie 1 le cas échéant, 0 si un point a été trouvé.
static int clip_wall_to_screen(Wall *in, Wall *out)
{
    double sx = in->start_x, sy = in->start_y;
    double ex = in->end_x,   ey = in->end_y;

    if (sx >= NEAR_PLANE && ex >= NEAR_PLANE) {
        memcpy(out, in, sizeof(*in));
        return 0;
    }

    if (sx < NEAR_PLANE && ex < NEAR_PLANE) {
        return 1; // totalement invisible
    }

    double t = (NEAR_PLANE - sx) / (ex - sx);

    double ix = sx + t * (ex - sx);
    double iy = sy + t * (ey - sy);

    *out = *in;
    if (sx < NEAR_PLANE) {
        out->start_x = ix;
        out->start_y = iy;
    } else {
        out->end_x = ix;
        out->end_y = iy;
    }

    return 0;
}

static void render_wall(Wall *wall, Position *camera, Context *context)
{
    const double EPS = 1e-9;

    if (wall->portal >= 0) return; // portail → rendu dans render_sector

    // --- 1. Coordonnées murales relatives à la caméra ---
    double sx = wall->start_x - camera->x;
    double sy = wall->start_y - camera->y;
    double ex = wall->end_x   - camera->x;
    double ey = wall->end_y   - camera->y;

    // --- 2. Rotation pour aligner caméra sur X+ ---
    rotate(sx, sy, -camera->angle, &sx, &sy);
    rotate(ex, ey, -camera->angle, &ex, &ey);

    Wall rotated_wall = { sx, ex, sy, ey };
    Wall actual_wall;

    // --- 3. Clipping near-plane ---
    if (clip_wall_to_screen(&rotated_wall, &actual_wall)) {
        return;
    }

    // --- 4. Utilisation de la tangente du début/fin du mur ---
    // ---    pour estimer si un mur sort de l'écran ou pas  ---
    double tan_left  = actual_wall.start_y / actual_wall.start_x;
    double tan_right = actual_wall.end_y / actual_wall.end_x;
    if (tan_left > tan_fov && tan_right > tan_fov) return;
    if (tan_left < -tan_fov && tan_right < -tan_fov) return;

    // --- 5. Projection ---
    int sx_p, ex_p, start_wall_height, end_wall_height;
    get_projection(actual_wall.start_x, actual_wall.start_y, camera->distance_to_screen, &sx_p, &start_wall_height);
    get_projection(actual_wall.end_x,   actual_wall.end_y,   camera->distance_to_screen, &ex_p, &end_wall_height);

    double x_center = (double)context->width / 2.0;
    int sx_screen = sx_p + x_center;
    int ex_screen = ex_p + x_center;

    int wall_width = ex_screen - sx_screen;
    if (wall_width <= 0) return;

    // --- 5. Boucle sur les colonnes ---
    double wx = actual_wall.end_x - actual_wall.start_x;
    double wy = actual_wall.end_y - actual_wall.start_y;
    double real_wall_width = sqrt(wx*wx + wy*wy);

    double start_distance = (double)context->height/2 + z_pos/actual_wall.start_x;
    double end_distance   = (double)context->height/2 + z_pos/actual_wall.end_x;

    for (int col = 0; col < wall_width; col++) {
        double t = (double)col / wall_width;
        int screen_x = sx_screen + col;

        if (screen_x < 0 || screen_x >= context->width) continue;

        double wall_height = start_wall_height + t * (end_wall_height - start_wall_height);
        if (wall_height < EPS) continue;

        double distance = start_distance + t * (end_distance - start_distance);
        double x = camera->distance_to_screen;
        double y = sx_p + col;

        double det = x * wy - y * wx;
        if (fabs(det) < EPS) continue;

        double u = (sx * y - sy * x) / det;
        if (u < 0.0 || u > real_wall_width) continue;

        render_col(context, screen_x, (int)wall_height, real_wall_width, u, wall->wall_type, distance);
    }
}


typedef struct Point {
    double x;
    double y;
} Point;

/// Retourne 1 si le point est dans le sector, 0 sinon
int is_point_in_sector(Position *position, Sector *sector) {
    for (int w = 0; w < sector->walls_id.count; w++) {
        Wall *wall = &(walls[sector->walls_id.values[w]]);
        Point wall_start = { .x = wall->start_x, .y = wall->start_y };
        Point wall_end = { .x = wall->end_x, .y = wall->end_y };
        Point player = { .x = position->x, .y = position->y };

        // FORMULE : det(Vi+1 - Vi, P - Vi) >= 0 ? Si oui continuer, sinon retourner 0
        if (det(wall_end.x - wall_start.x, wall_end.y - wall_start.y, player.x - wall_start.x, player.y - wall_start.y) < 0) return 0;
    }

    return 1;
}

static unsigned int visited_sectors[MAX_NB_SECTORS] = { 0 };

void render_sector(Context *context, Position *camera, Sector *sector) {
    // Vérifier si un mur contient un portail (vers un secteur NON DÉJÀ TRAITÉ)
    for (int w = 0; w < sector->walls_id.count; w++) {
        Wall *wall = &(walls[sector->walls_id.values[w]]);

        if (wall->portal == -1) continue;

        if (!visited_sectors[wall->portal]) {
            visited_sectors[wall->portal] = 1;
            render_sector(context, camera, &sectors[wall->portal]);
        }
    }

    
    for (int w = 0; w < sector->walls_id.count; w++) {
        render_wall(&(walls[sector->walls_id.values[w]]), camera, context);
    }
}

void reset_visited_sectors(void) {
    for (int s = 0; s < MAX_NB_SECTORS; s++) {
        visited_sectors[s] = 0;
    }
}
