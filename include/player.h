#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    double x, y; // Position in 2D
    double angle; // Current angle from X axis
    double fov;
    double distance_to_screen; // Based on FOV parameter
} Position;

extern Position player_position;

void init_position(Position *, double, double);
void move(Position *p, double dx, double dy);
void rotate(double x1, double y1, double angle, double* x2, double* y2);

#endif
