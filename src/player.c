#include "player.h"

#include <math.h>

Position player_position;

void init_position(Position * p, double fov, double screen_width) {
    p->x = 50.0;
    p->y = 50.0;
    p->angle = 0.0;
    p->fov = fov;
    p->distance_to_screen = screen_width / (2 * tan(fov/2));
}

void move(Position* p, double dx, double dy)
{
    p->x += dx;
    p->y += dy;
}

void rotate(double x1, double y1, double angle, double* x2, double* y2)
{
    double _x, _y;
    _x = x1 * cos(angle) - y1 * sin(angle);
    _y = x1 * sin(angle) + y1 * cos(angle);

    *x2 = _x;
    *y2 = _y;
}
