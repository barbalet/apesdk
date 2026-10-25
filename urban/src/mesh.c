/* Copyright 2026 Tom Barbalet. MIT; see ../LICENSE. */
#include "pra_internal.h"
#include "pra_plants.h"
#include <stdint.h>

typedef struct
{
    pra_mesh mesh;
    size_t capacity;
    int failed;
    const u_house *house;
    double origin[2];
} builder;
static const double plaster[3] = {.65, .61, .51}, timber[3] = {.28, .16, .08},
                    glass[3] = {.23, .46, .52}, floor_color[3] = {.43, .35, .24};
static void triangle(builder *b, const double a[3], const double c[3], const double d[3],
                     const double color[3])
{
    if (b->failed)
        return;
    if (b->mesh.vertex_count + 3 > b->capacity)
    {
        size_t n = b->capacity ? b->capacity * 2 : 4096;
        if (n > 4000000)
        {
            b->failed = 1;
            return;
        }
        float *p = realloc(b->mesh.vertices, n * 9 * sizeof(float));
        if (!p)
        {
            b->failed = 1;
            return;
        }
        b->mesh.vertices = p;
        b->capacity = n;
    }
    /* The SDK transform reflects the local plan. Reverse winding here. */
    const double *v[3] = {a, d, c};
    double p[3][3];
    for (int k = 0; k < 3; k++)
    {
        double local[2] = {v[k][0], v[k][2]}, world[2];
        u_world(b->house, local, world);
        p[k][0] = world[0] - b->origin[0];
        p[k][1] = v[k][1];
        p[k][2] = world[1] - b->origin[1];
    }
    double u[3], v2[3], normal[3];
    for (int k = 0; k < 3; k++)
    {
        u[k] = p[1][k] - p[0][k];
        v2[k] = p[2][k] - p[0][k];
    }
    normal[0] = u[1] * v2[2] - u[2] * v2[1];
    normal[1] = u[2] * v2[0] - u[0] * v2[2];
    normal[2] = u[0] * v2[1] - u[1] * v2[0];
    double length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    if (length < 1e-12)
        return;
    for (int k = 0; k < 3; k++)
        for (int j = 0; j < 9; j++)
            b->mesh.vertices[b->mesh.vertex_count * 9 + k * 9 + j] =
                (float)(j < 3   ? p[k][j]
                        : j < 6 ? normal[j - 3] / length
                                : color[j - 6]);
    b->mesh.vertex_count += 3;
}
static void quad(builder *b, const double a[3], const double c[3], const double d[3],
                 const double e[3], const double color[3])
{
    triangle(b, a, c, d, color);
    triangle(b, a, d, e, color);
}
static void prism(builder *b, const double a[2], const double c[2], double thickness, double low,
                  double high, const double color[3])
{
    double dx = c[0] - a[0], dz = c[1] - a[1], length = hypot(dx, dz);
    if (length < 1e-10 || high <= low || thickness <= 0)
        return;
    double nx = -dz / length * thickness / 2, nz = dx / length * thickness / 2;
    double p[4][2] = {{a[0] + nx, a[1] + nz},
                      {c[0] + nx, c[1] + nz},
                      {c[0] - nx, c[1] - nz},
                      {a[0] - nx, a[1] - nz}};
    double lo[4][3], hi[4][3];
    for (int i = 0; i < 4; i++)
    {
        lo[i][0] = hi[i][0] = p[i][0];
        lo[i][2] = hi[i][2] = p[i][1];
        lo[i][1] = low;
        hi[i][1] = high;
    }
    quad(b, hi[0], hi[1], hi[2], hi[3], color);
    quad(b, lo[3], lo[2], lo[1], lo[0], color);
    for (int i = 0; i < 4; i++)
    {
        int j = (i + 1) % 4;
        quad(b, lo[i], lo[j], hi[j], hi[i], color);
    }
}
static void rectangle(builder *b, double x0, double z0, double x1, double z1, double low,
                      double high, const double color[3])
{
    double a[2] = {(x0 + x1) / 2, z0}, c[2] = {(x0 + x1) / 2, z1};
    prism(b, a, c, x1 - x0, low, high, color);
}
static int compare_double(const void *a, const void *c)
{
    double x = *(const double *)a, y = *(const double *)c;
    return (x > y) - (x < y);
}
static void wall_mesh(builder *b, const u_wall *w)
{
    const u_house *h = b->house;
    int axis = fabs(w->a[0] - w->b[0]) > 1e-8 ? 0 : 1;
    double cuts[2 + 2 * U_OPENINGS];
    size_t count = 0;
    cuts[count++] = w->a[axis];
    cuts[count++] = w->b[axis];
    for (size_t i = 0; i < h->opening_count; i++)
    {
        const u_opening *o = h->openings + i;
        if (o->axis != axis || fabs(o->center[1 - axis] - w->a[1 - axis]) > 1e-8)
            continue;
        for (int k = -1; k <= 1; k += 2)
        {
            double x = o->center[axis] + k * o->width / 2;
            if (x > w->a[axis] && x < w->b[axis])
                cuts[count++] = x;
        }
    }
    qsort(cuts, count, sizeof(double), compare_double);
    for (size_t i = 1; i < count; i++)
    {
        double a[2] = {w->a[0], w->a[1]}, c[2] = {w->b[0], w->b[1]};
        a[axis] = cuts[i - 1];
        c[axis] = cuts[i];
        double mid = (cuts[i - 1] + cuts[i]) / 2;
        int hole = 0, window_only = 1;
        for (size_t j = 0; j < h->opening_count; j++)
        {
            const u_opening *o = h->openings + j;
            if (o->axis == axis && fabs(o->center[1 - axis] - w->a[1 - axis]) < 1e-8 &&
                fabs(mid - o->center[axis]) < o->width / 2 - 1e-8)
            {
                hole = 1;
                window_only &= o->window;
            }
        }
        if (!hole)
            prism(b, a, c, U_THICKNESS, 0, U_HEIGHT, plaster);
        else
        {
            if (window_only)
                prism(b, a, c, U_THICKNESS, 0, U_SILL, plaster);
            prism(b, a, c, U_THICKNESS, window_only ? U_WINDOW_TOP : U_DOOR_TOP, U_HEIGHT, plaster);
        }
    }
}
static void opening_mesh(builder *b, const u_opening *o)
{
    double a[2] = {o->center[0], o->center[1]}, c[2] = {o->center[0], o->center[1]};
    a[o->axis] -= o->width / 2;
    c[o->axis] += o->width / 2;
    if (!o->window)
    {
        u_panel(o, a, c);
        prism(b, a, c, .045, 0, U_DOOR_TOP, timber);
        return;
    }
    prism(b, a, c, .025, U_SILL, U_WINDOW_TOP, glass);
    prism(b, a, c, .16, U_SILL - .035, U_SILL + .035, timber);
    prism(b, a, c, .16, U_WINDOW_TOP - .035, U_WINDOW_TOP + .035, timber);
    int bars = (int)ceil(o->width / 1.0);
    for (int i = 0; i <= bars; i++)
    {
        double p[2] = {a[0] + (c[0] - a[0]) * i / bars, a[1] + (c[1] - a[1]) * i / bars};
        double q[2] = {p[0], p[1]};
        p[o->axis] -= .025;
        q[o->axis] += .025;
        prism(b, p, q, .13, U_SILL, U_WINDOW_TOP, timber);
    }
}
typedef struct {builder *builder; double x,z,scale;} yard_plant;
static void yard_plant_triangle(void *context,const double a[3],const double c[3],const double d[3],const double color[3])
{
    yard_plant *p=context;
    double points[3][3];const double *src[3]={a,c,d};
    for(int i=0;i<3;i++){
        points[i][0]=p->x+src[i][0]*p->scale;
        points[i][1]=src[i][1]*p->scale-.15;
        points[i][2]=p->z+src[i][2]*p->scale;
    }
    triangle(p->builder,points[0],points[1],points[2],color);
}
static void yard_mesh(builder *b)
{
    const u_house *h = b->house;
    const double grass[3] = {.18, .27, .10}, concrete[3] = {.48, .47, .43},
                 road[3] = {.13, .14, .15};
    rectangle(b, -28, -31, 28, 30, -.22, -.15, grass);
    rectangle(b, -28, -34, 28, -31, -.20, -.13, road);
    rectangle(b, h->entry[0] - 1.4, -34, h->entry[0] + 1.4, h->entry[1], -.14, -.05, concrete);
    for (int j = 0; j < 5; j++)
    {
        const u_line *f = h->fences + j;
        prism(b, f->a, f->b, .08, .4, .51, timber);
        prism(b, f->a, f->b, .08, .85, .96, timber);
        int posts = (int)ceil(hypot(f->b[0] - f->a[0], f->b[1] - f->a[1]) / 2);
        for (int k = 0; k <= posts; k++)
        {
            double x = f->a[0] + (f->b[0] - f->a[0]) * k / posts,
                   z = f->a[1] + (f->b[1] - f->a[1]) * k / posts;
            rectangle(b, x - .06, z - .06, x + .06, z + .06, -.15, 1.1, timber);
        }
    }
    for (int t = 0; t < 4; t++)
    {
        const simulated_tree *tree = h->trees + t;
        double x = tree->center.x * U_SCALE, z = tree->center.y * U_SCALE;
        yard_plant plant={b,x,z,.75+(double)tree->radius/30.0};
        pra_plant_build(t==3?PRA_PLANT_BUSH:PRA_PLANT_TREE,1,yard_plant_triangle,&plant);
    }
}
int pra_build_mesh(const pra_scene *s, unsigned flags, const double origin[2], pra_mesh *mesh)
{
    if (!s || !mesh || !origin || !isfinite(origin[0]) || !isfinite(origin[1]) ||
        (flags & ~(PRA_ROOFS | PRA_YARDS | PRA_NO_DOORS | PRA_DOORS_ONLY)))
        return 0;
    builder b = {0};
    memcpy(b.origin, origin, sizeof b.origin);
    for (uint32_t i = 0; i < s->config.houses; i++)
    {
        b.house = s->house + i;
        const u_house *h = b.house;
        if (flags & PRA_DOORS_ONLY) {
            for(size_t j=0;j<h->opening_count;j++)
                if(!h->openings[j].window)opening_mesh(&b,h->openings+j);
            continue;
        }
        if (flags & PRA_YARDS)
            yard_mesh(&b);
        for (n_int j = 0; j < h->raw.roomcount; j++)
        {
            const simulated_room *r = h->raw.local_room + j;
            double x0 = (r->points[0].x - h->raw.pivot.x) * U_SCALE,
                   z0 = (r->points[0].y - h->raw.pivot.y) * U_SCALE;
            double x1 = (r->points[2].x - h->raw.pivot.x) * U_SCALE,
                   z1 = (r->points[2].y - h->raw.pivot.y) * U_SCALE;
            rectangle(&b, x0, z0, x1, z1, -.12, 0, floor_color);
            if (flags & PRA_ROOFS)
            {
                const double roof[3] = {.24, .17, .13};
                rectangle(&b, x0 - .15, z0 - .15, x1 + .15, z1 + .15, U_HEIGHT, U_HEIGHT + .18,
                          roof);
            }
        }
        for (size_t j = 0; j < h->wall_count; j++)
            wall_mesh(&b, h->walls + j);
        for (size_t j = 0; j < h->opening_count; j++)
            if(h->openings[j].window || !(flags & PRA_NO_DOORS))opening_mesh(&b, h->openings + j);
    }
    if (b.failed)
    {
        free(b.mesh.vertices);
        return 0;
    }
    *mesh = b.mesh;
    return 1;
}
void pra_mesh_free(pra_mesh *m)
{
    if (m)
    {
        free(m->vertices);
        m->vertices = NULL;
        m->vertex_count = 0;
    }
}
