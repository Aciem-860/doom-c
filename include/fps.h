#ifndef FPS_H
#define FPS_H
#include <SDL2/SDL_stdinc.h>

typedef struct FPS {
    int frames;
    int fps;
    Uint32 last_time;
    Uint32 current_time;
    Uint32 frame_start;
    Uint32 target_delay;
} FPS;

void init_fps(FPS *fps, int fps_per_second);
void compute_fps(FPS *fps);

#endif
