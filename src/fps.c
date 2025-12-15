#include "fps.h"
#include <SDL2/SDL_timer.h>

void init_fps(FPS *fps, int fps_per_second) {
    fps->frames = 0;
    fps->fps = 0;
    fps->last_time = SDL_GetTicks();
    fps->target_delay = 1000 / fps_per_second;
}

void compute_fps(FPS *fps) {
    fps->current_time = SDL_GetTicks();
    fps->frames++;

    if (fps->current_time - fps->last_time > 1000) {
        fps->fps = fps->frames * 1000.0f / (fps->current_time - fps->last_time);
        fps->frames = 0;
        fps->last_time = fps->current_time;
    }
}
