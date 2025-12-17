#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_IMPL

#include "sector.h"
#include "context.h"
#include "log.h"
#include "colors.h"
#include "ttf.h"
#include "fps.h"

#define HEIGHT 720
#define WIDTH 1080
#define FOV (90 * M_PI / 180)

#define PLAYER_MOVE_STEP 1

#define format_buffer(buffer, text, ...) do {   \
        sprintf(buffer, text, __VA_ARGS__);     \
    } while (0)

extern Position player_position;

FPS fps;
Context context;
Uint8 keyboard[SDL_NUM_SCANCODES];

int current_sector = 0;

static void update_keyboard(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.key.keysym.scancode < SDL_NUM_SCANCODES) {
            keyboard[event.key.keysym.scancode] = (event.type == SDL_KEYDOWN);
        }
    }
}

/// Returns -1 if should close
static int parse_keyboard() {
    if (keyboard[SDL_SCANCODE_Q]) {
        return -1;
    }

    if (keyboard[SDL_SCANCODE_W]) {
        // MOVE FORWARD
        player_position.x += cos(player_position.angle);
        player_position.y += sin(player_position.angle);
    }
        
    if (keyboard[SDL_SCANCODE_S]) {
        // MOVE BACKWARD
        player_position.x -= cos(player_position.angle);
        player_position.y -= sin(player_position.angle);
    }

    if (keyboard[SDL_SCANCODE_A]) {
        // MOVE LEFT
        player_position.x += cos(player_position.angle-M_PI/2);
        player_position.y += sin(player_position.angle-M_PI/2);
    }
        
    if (keyboard[SDL_SCANCODE_D]) {
        // MOVE RIGHT
        player_position.x += cos(player_position.angle+M_PI/2);
        player_position.y += sin(player_position.angle+M_PI/2);
    }

    if (keyboard[SDL_SCANCODE_O]) {
        player_position.angle -= 0.05;
    }

    if (keyboard[SDL_SCANCODE_P]) {
        player_position.angle += 0.05;
    }

    return 0;
}

static void render_floor_and_walls() {
    render_floor(&context, &player_position);

    // Only render the current sector
    if (current_sector != -1) {
        render_sector(&context, &player_position, &sectors[current_sector]);
        reset_visited_sectors();
    }
}

static void render_fps() {
    char buffer[64];
    format_buffer(buffer, "FPS: %d", fps.fps);
        
    SDL_Surface *surface_message = TTF_RenderText_Solid(main_font_ttf, buffer, YELLOW);
    SDL_Texture *texture_message = SDL_CreateTextureFromSurface(context.renderer, surface_message);
        
    SDL_Rect rect_message = {
        0, 0, 100, 25
    };
        
    SDL_RenderCopy(context.renderer, texture_message, NULL, &rect_message);
    
    SDL_FreeSurface(surface_message);
    SDL_DestroyTexture(texture_message);
}

static int initialize_global() {
    init_logg(stdout, 1, ALL);
    load_level("levels/level3.map");

    {
        for (int s = 0; s < sector_number; s++) {
            logg(INFO, "< SECTOR %d >", s);
            logg(INFO, "TAG: %d", sectors[s].tag);
            logg(INFO, "WALLS CONNECTED TO: %d", sectors[s].walls_id.count);
        }

        for (int w = 0; w < wall_number; w++) {
            logg(INFO, "< WALL %d >", w);
            logg(INFO, "(%lf, %lf) -> (%lf, %lf)", walls[w].start_x, walls[w].start_y, walls[w].end_x, walls[w].end_y);
            logg(INFO, "PORTAL -> %d", walls[w].portal);
        }
    }

    init_position(&player_position, FOV, WIDTH);
    tan_fov = tan(FOV/2);

    int status = EXIT_FAILURE;
    if (0 != SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Erreur SDL_Init: %s", SDL_GetError());
        return status;
    }

    context.width = WIDTH;
    context.height = HEIGHT;

    context.window = SDL_CreateWindow("SDL2",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      WIDTH,
                                      HEIGHT,
                                      SDL_WINDOW_SHOWN);

    context.renderer = SDL_CreateRenderer(context.window,
                                          -1,
                                          SDL_RENDERER_ACCELERATED);

    if (NULL == context.window) {
        fprintf(stderr, "Erreur SDL_CreateWindow: %s", SDL_GetError());
        return status;
    }

    init_texture(&context);
    if(init_ttf()) return status;
    
    init_fps(&fps, 300); // Aimed to 60 Hz
    return 0;
}

/// Retourne 1 si aucun secteur n'est trouvé
static int update_current_sector(Position *position) {
    for (int s = 0; s < sector_number; s++) {
        Sector *sector = &(sectors[s]);
        if (is_point_in_sector(position, sector)) {
            current_sector = s;
            return 0;
        }
    }
    return 1;
}

int main(void) {
    int status;
    if (initialize_global() == EXIT_FAILURE) {
        status = EXIT_FAILURE;
        goto Quit;
    }

    logg(INFO, "Current player sector: %d", current_sector);
    int last_sector = current_sector;
    
    // --- MAIN LOOP --- //
    while (1) {
        fps.frame_start = SDL_GetTicks();
        
        SDL_RenderClear(context.renderer);
        set_color(context.renderer, WHITE);

        update_keyboard();
        if (parse_keyboard()) break;

        if(update_current_sector(&player_position)) {
            if (last_sector != -1)
                logg(WARNING, "You are outside of any sector!");
            last_sector = -1;
            current_sector = -1;
        }
        if (last_sector != current_sector && current_sector != -1) {
            logg(INFO, "Current player sector: %d", current_sector);
            last_sector = current_sector;
        }
        render_floor_and_walls();
        compute_fps(&fps);
        render_fps();
    
        set_color(context.renderer, BLACK);
        SDL_RenderPresent(context.renderer);

        /// SDL_DELAY computation
        Uint32 frame_time = SDL_GetTicks() - fps.frame_start;
        if (fps.target_delay > frame_time) {
            SDL_Delay(fps.target_delay - frame_time);
        }
    }
    
    status = EXIT_SUCCESS;

 Quit:
    if (context.renderer) {
        free_texture();
        SDL_DestroyRenderer(context.renderer);
    }
    if (context.window)
        SDL_DestroyWindow(context.window);
    
    if (status == EXIT_SUCCESS) {
        for (int s = 0; s < sector_number; s++) {
            da_free(&(sectors[s].walls_id));
        }
    }    
    
    SDL_Quit();
    return status;
}
