#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_IMPL

#include "sector.h"
#include "context.h"
#include "log.h"

#define HEIGHT 720
#define WIDTH 1080
#define FOV (90*M_PI/180)

#define PLAYER_MOVE_STEP 1

extern Position player_position;

Context context;
Uint8 keyboard[SDL_NUM_SCANCODES];

static void update_keyboard(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.key.keysym.scancode < SDL_NUM_SCANCODES) {
            keyboard[event.key.keysym.scancode] = (event.type == SDL_KEYDOWN);
        }
    }
}


int main(void)
{
    init_logg(stdout, 1, NONE);
    load_level("levels/level.map");
    init_position(&player_position, FOV, WIDTH);

    int status = EXIT_FAILURE;
    if (0 != SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Erreur SDL_Init: %s", SDL_GetError());
        goto Quit;
    }


    SDL_Color colours[5] = {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255,255,0,255}, {255,0,255,255}};
    
    for (int w = 0; w < wall_number; w++) {
        logg(DEBUG, "WALL N°%d", w);
        print_wall(walls + w);
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
        goto Quit;
    }


    const SDL_Color WHITE = { 255, 255, 255, 255 };
    const SDL_Color BLACK = { 0, 0, 0, 255 };


    init_texture(&context);
    
    //  MAIN LOOP  //
    while (1) {
        SDL_RenderClear(context.renderer);

        set_color(context.renderer, WHITE);

        update_keyboard();
        if (keyboard[SDL_SCANCODE_Q]) {
            break;
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
        
        for (int w = 0; w < wall_number; w++) {
            set_color(context.renderer, colours[w]);
            render_wall(&walls[w], &player_position, &context);
        }
        set_color(context.renderer, BLACK);
        SDL_RenderPresent(context.renderer);
        SDL_Delay(10);
    }
    
    status = EXIT_SUCCESS;

 Quit:
    if (context.renderer)
        SDL_DestroyRenderer(context.renderer);
    if (context.window)
        SDL_DestroyWindow(context.window);
    
    SDL_Quit();
    return status;
}
