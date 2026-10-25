/* Copyright 2026 Tom Barbalet. MIT; see ../LICENSE.
 * Shared branch/crown construction for PRA yards and instanced vegetation. */
#ifndef PRA_PLANTS_H
#define PRA_PLANTS_H
#include <math.h>
#include <stddef.h>

typedef void (*pra_plant_triangle)(void *, const double[3], const double[3], const double[3],
                                   const double[3]);
enum
{
    PRA_PLANT_TREE,
    PRA_PLANT_BUSH
};

static inline void pra_plant_point(unsigned kind, const double p[3], double q[3])
{
    q[0] = p[0];
    q[1] = p[1];
    q[2] = p[2];
    if (kind == PRA_PLANT_BUSH)
    {
        q[0] *= .62;
        q[2] *= .62;
        q[1] = p[1] < 2.6 ? p[1] * .10 : .26 + (p[1] - 2.6) * .32;
    }
}
static inline void pra_plant_face(unsigned kind, pra_plant_triangle emit, void *context,
                                  const double a[3], const double b[3], const double c[3],
                                  const double color[3])
{
    double p[3], q[3], r[3];
    pra_plant_point(kind, a, p);
    pra_plant_point(kind, b, q);
    pra_plant_point(kind, c, r);
    emit(context, p, q, r, color);
}
static inline void pra_plant_limb(unsigned kind, unsigned sides, const double a[3],
                                  const double b[3], double r0, double r1, pra_plant_triangle emit,
                                  void *context)
{
    const double bark[3] = {.28, .19, .105};
    double d[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    double length = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    for (unsigned k = 0; k < 3; k++)
        d[k] /= length;
    double u[3] = {d[1], -d[0], 0};
    double n = hypot(u[0], u[1]);
    if (n < .001)
    {
        u[0] = 1;
        u[1] = 0;
        n = 1;
    }
    for (unsigned k = 0; k < 3; k++)
        u[k] /= n;
    double v[3] = {d[1] * u[2] - d[2] * u[1], d[2] * u[0] - d[0] * u[2], d[0] * u[1] - d[1] * u[0]};
    for (unsigned j = 0; j < sides; j++)
    {
        double points[4][3];
        for (unsigned end = 0; end < 2; end++)
            for (unsigned edge = 0; edge < 2; edge++)
            {
                double angle = 6.283185307179586 * (double)(j + edge) / (double)sides;
                for (unsigned k = 0; k < 3; k++)
                    points[end * 2 + edge][k] =
                        (end ? b[k] : a[k]) +
                        (end ? r1 : r0) * (cos(angle) * u[k] + sin(angle) * v[k]);
            }
        pra_plant_face(kind, emit, context, points[0], points[1], points[3], bark);
        pra_plant_face(kind, emit, context, points[0], points[3], points[2], bark);
    }
}
static inline void pra_plant_crown(unsigned kind, unsigned tier, const double center[3],
                                   const double radius[3], unsigned shade, pra_plant_triangle emit,
                                   void *context)
{
    unsigned sides = tier == 0 ? 8 : tier == 1 ? 6 : 4;
    unsigned rings = tier == 0 ? 3 : 2;
    double color[3] = {.13 + .012 * (shade % 3), .28 + .016 * (shade % 4),
                       .085 + .009 * (shade % 3)};
    for (unsigned ring = 0; ring < rings; ring++)
        for (unsigned j = 0; j < sides; j++)
        {
            double p[4][3];
            for (unsigned row = 0; row < 2; row++)
                for (unsigned edge = 0; edge < 2; edge++)
                {
                    double latitude =
                        -1.5707963267948966 + 3.141592653589793 * (ring + row) / rings;
                    double angle = 6.283185307179586 * (j + edge) / (double)sides;
                    double radial = cos(latitude);
                    p[row * 2 + edge][0] = center[0] + radius[0] * radial * cos(angle);
                    p[row * 2 + edge][1] = center[1] + radius[1] * sin(latitude);
                    p[row * 2 + edge][2] = center[2] + radius[2] * radial * sin(angle);
                }
            if (ring > 0)
                pra_plant_face(kind, emit, context, p[0], p[3], p[1], color);
            if (ring + 1 < rings)
                pra_plant_face(kind, emit, context, p[0], p[2], p[3], color);
        }
}
static inline void pra_plant_build(unsigned kind, unsigned tier, pra_plant_triangle emit,
                                   void *context)
{
    unsigned limbs = tier == 0 ? 6 : tier == 1 ? 4 : 3;
    unsigned sides = tier == 0 ? 6 : tier == 1 ? 4 : 3;
    pra_plant_limb(kind, sides, (double[3]){0, 0, 0}, (double[3]){.18, 6.9, .12}, .24, .065, emit,
                   context);
    for (unsigned i = 0; i < limbs; i++)
    {
        double angle = 6.283185307179586 * i / limbs + .24;
        double end[3] = {cos(angle) * 2.65, 5.1 + (i % 3) * .65, sin(angle) * 2.65};
        double start[3] = {.07, 2.5 + i * .32, .04};
        pra_plant_limb(kind, sides, start, end, .135, .035, emit, context);
        if (tier == 0)
            for (unsigned fork = 0; fork < 2; fork++)
            {
                double root[3], tip[3];
                for (unsigned k = 0; k < 3; k++)
                    root[k] = start[k] + (end[k] - start[k]) * .62;
                tip[0] = end[0] + cos(angle + (fork ? 1.1 : -1.1)) * .8;
                tip[1] = end[1] + .8;
                tip[2] = end[2] + sin(angle + (fork ? 1.1 : -1.1)) * .8;
                pra_plant_limb(kind, 4, root, tip, .055, .014, emit, context);
            }
        double center[3] = {end[0], end[1] + .85, end[2]};
        pra_plant_crown(kind, tier, center, (double[3]){1.85, 1.55, 1.7}, i, emit, context);
    }
    pra_plant_crown(kind, tier, (double[3]){.18, 7.25, .12}, (double[3]){2.1, 1.9, 2.0}, 7, emit,
                    context);
}
#endif
