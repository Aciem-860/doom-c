#include <SDL2/SDL_ttf.h>

const char* main_font_path = "res/DooM.ttf";

TTF_Font *main_font_ttf;
/* D'autres polices d'écriture si nécessaire */


static int init_ttf_with_path(TTF_Font **ttf, const char* ttf_path, int font_size) {

    *ttf = TTF_OpenFont(ttf_path, font_size);
    if (*ttf == NULL) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        return 1;
    }
    return 0;
}

/// Initialise les différents TTF.
/// En cas d'erreur, renvoie 1, sinon 0.
int init_ttf() {    
    if (TTF_Init()) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        return 1;
    }
    return init_ttf_with_path(&main_font_ttf, main_font_path, 24);
}
