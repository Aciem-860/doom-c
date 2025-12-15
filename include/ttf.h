#ifndef TTF_H
#define TTF_H

#include <SDL2/SDL_ttf.h>

extern TTF_Font *main_font_ttf;

/// Initialise les différents TTF.
/// En cas d'erreur, renvoie 1, sinon 0.
int init_ttf();

#endif
