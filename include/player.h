#ifndef PLAYER_H
#define PLAYER_H

struct position {
    double x, y; // Position in 2D
    double angle; // Current angle from X axis
    double fov;
    double distance_to_screen; // Based on FOV parameter
};

extern struct position player_position;

void init_position(struct position *, double, double);
void move(struct position *p, double dx, double dy);
void rotate(double x1, double y1, double angle, double* x2, double* y2);

#endif
