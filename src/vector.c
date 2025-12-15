#include <math.h>

/// Angle in radians between vector v1 = (x1,y1) and v2 = (x2, y2);
static inline double angle_between(double x1, double y1, double x2, double y2)
{
    double phi_ref = atan2(y1, x1);
    double phi = atan2(x2, y2);
    double angle = phi - phi_ref;

    // Normalisation
    if (angle > M_PI) angle -= 2*M_PI;
    if (angle < -M_PI) angle += 2*M_PI;
    
    return angle;
}

void get_intersect(double x1, double y1,
                   double x2, double y2,
                   double x3, double y3,
                   double x4, double y4,
                   double* x, double* y)
{
    double t = ((x1-x3)*(y3-y4)-(y1-y3)*(x3-x4))/((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
    
    *x = x1 + t * (x2 - x1);
    *y = y1 + t * (y2 - y1);
}
