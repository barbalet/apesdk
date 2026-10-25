/* Copyright 2026 Tom Barbalet. MIT; see ../LICENSE. */
#include "pra_internal.h"
#include <stdarg.h>
#include <stdio.h>

void u_world(const u_house *h, const double p[2], double q[2])
{
    q[0] = h->center[0] + p[0] * h->m[0] + p[1] * h->m[1];
    q[1] = h->center[1] + p[0] * h->m[1] - p[1] * h->m[0];
}
void u_local(const u_house *h, const double p[2], double q[2])
{
    double x = p[0] - h->center[0], z = p[1] - h->center[1];
    double d = h->m[0] * h->m[0] + h->m[1] * h->m[1];
    q[0] = (x * h->m[0] + z * h->m[1]) / d;
    q[1] = (x * h->m[1] - z * h->m[0]) / d;
}
double u_distance(const double p[2], const double a[2], const double b[2])
{
    double x = b[0] - a[0], z = b[1] - a[1], d = x * x + z * z;
    double t = d ? ((p[0] - a[0]) * x + (p[1] - a[1]) * z) / d : 0;
    t = fmax(0, fmin(1, t));
    return hypot(p[0] - a[0] - t * x, p[1] - a[1] - t * z);
}
void u_panel(const u_opening *o, double a[2], double b[2])
{
    int axis = o->axis;
    memcpy(a, o->center, 2 * sizeof(double));
    double sign = o->hinge ? 1 : -1;
    a[axis] += sign * o->width / 2;
    memcpy(b, a, 2 * sizeof(double));
    if (o->open)
    {
        b[0] += o->inward[0] * o->width;
        b[1] += o->inward[1] * o->width;
    }
    else
        b[axis] -= sign * o->width;
}
static double metric(const u_house *h, n_int v, int axis)
{
    return (v - h->raw.pivot.data[axis]) * U_SCALE;
}
static int cmp_int(const void *a, const void *b)
{
    n_int x = *(const n_int *)a, y = *(const n_int *)b;
    return (x > y) - (x < y);
}
static int add_wall(u_house *h, int axis, n_int constant, n_int a, n_int b)
{
    double p[2], q[2];
    p[axis] = metric(h, a, axis);
    q[axis] = metric(h, b, axis);
    p[1 - axis] = q[1 - axis] = metric(h, constant, 1 - axis);
    for (size_t i = 0; i < h->wall_count; i++)
    {
        u_wall *w = h->walls + i;
        if (!memcmp(w->a, p, sizeof p) && !memcmp(w->b, q, sizeof q))
        {
            w->owners++;
            return 1;
        }
    }
    if (h->wall_count == U_WALLS)
        return 0;
    u_wall *w = h->walls + h->wall_count++;
    memcpy(w->a, p, sizeof p);
    memcpy(w->b, q, sizeof q);
    w->owners = 1;
    return 1;
}
int u_plan(u_house *h, uint32_t house_index)
{
    n_int coords[2][2 * MAX_ROOMS];
    size_t count = (size_t)h->raw.roomcount * 2;
    for (n_int r = 0; r < h->raw.roomcount; r++)
        for (int a = 0; a < 2; a++)
        {
            coords[a][2 * r] = h->raw.local_room[r].points[0].data[a];
            coords[a][2 * r + 1] = h->raw.local_room[r].points[2].data[a];
        }
    for (int a = 0; a < 2; a++)
        qsort(coords[a], count, sizeof(n_int), cmp_int);
    for (n_int r = 0; r < h->raw.roomcount; r++)
    {
        simulated_room *room = h->raw.local_room + r;
        for (int axis = 0; axis < 2; axis++)
            for (int side = 0; side < 2; side++)
            {
                n_int low = room->points[0].data[axis], high = room->points[2].data[axis];
                n_int constant = room->points[side ? 2 : 0].data[1 - axis];
                n_int prev = low;
                for (size_t i = 0; i < count; i++)
                    if (coords[axis][i] > prev && coords[axis][i] <= high)
                    {
                        if (!add_wall(h, axis, constant, prev, coords[axis][i]))
                            return 0;
                        prev = coords[axis][i];
                    }
            }
        for (int index = 8; index < 32; index += (index < 16 ? 2 : 4))
        {
            int window = index < 16;
            if (!(window ? house_window_present(room->points + index)
                         : house_door_present(room->points + index)))
                continue;
            int side = window ? (index - 8) / 2 : (index - 16) / 4;
            int axis = side < 2 ? 1 : 0;
            double center[2] = {0, 0};
            int n = window ? 2 : 4;
            for (int k = 0; k < n; k++)
                for (int a = 0; a < 2; a++)
                    center[a] += metric(h, room->points[index + k].data[a], a) / n;
            center[1 - axis] =
                metric(h, room->points[(side == 1 || side == 2) ? 2 : 0].data[1 - axis], 1 - axis);
            double width = window ? fabs((double)(room->points[index + 1].data[axis] -
                                                  room->points[index].data[axis])) *
                                        U_SCALE
                                  : 10 * U_SCALE;
            int duplicate = 0;
            for (size_t i = 0; i < h->opening_count; i++)
            {
                u_opening *o = h->openings + i;
                if (o->axis == axis && o->window == window &&
                    fabs(o->center[0] - center[0]) < 1e-8 &&
                    fabs(o->center[1] - center[1]) < 1e-8 && fabs(o->width - width) < 1e-8)
                    duplicate = 1;
            }
            if (duplicate)
                continue;
            if (h->opening_count == U_OPENINGS)
                return 0;
            u_opening *o = h->openings + h->opening_count++;
            memcpy(o->center, center, sizeof center);
            o->axis = axis;
            o->width = width;
            o->window = window;
            o->inward[1 - axis] = (side == 0 || side == 3) ? 1 : -1;
            if (!window)
                o->id = house_index * U_OPENINGS + (uint32_t)h->opening_count;
            for (size_t i = 0; i < h->wall_count; i++)
                if (h->walls[i].owners == 1 &&
                    u_distance(center, h->walls[i].a, h->walls[i].b) < 1e-8)
                    o->exterior = 1;
            if (r == 0 && !window)
                memcpy(h->entry, center, sizeof center);
        }
    }
    /* Keep source integer records untouched. A few source doors meet a T wall
       exactly at its endpoint. Find the nearest usable portal on that wall. */
    for (size_t j = 0; j < h->opening_count; j++)
    {
        u_opening *o = h->openings + j;
        if (o->window)
            continue;
        double original = o->center[o->axis], best = original, best_distance = 1e30;
        for (size_t trial = 0; trial <= 2 * h->wall_count; trial++)
        {
            double candidate = original;
            if (trial)
            {
                const u_wall *w = h->walls + (trial - 1) / 2;
                if (fabs(w->a[o->axis] - w->b[o->axis]) > 1e-8)
                    continue;
                candidate = w->a[o->axis] +
                            ((trial & 1) ? -1 : 1) * (o->width / 2 + U_THICKNESS / 2 + .025 + 1e-6);
            }
            int valid = 1;
            for (size_t k = 0; k < h->wall_count; k++)
            {
                const u_wall *w = h->walls + k;
                if (fabs(w->a[o->axis] - w->b[o->axis]) < 1e-8 &&
                    o->center[1 - o->axis] >= w->a[1 - o->axis] - 1e-8 &&
                    o->center[1 - o->axis] <= w->b[1 - o->axis] + 1e-8 &&
                    fabs(candidate - w->a[o->axis]) < o->width / 2 + U_THICKNESS / 2 + .025)
                    valid = 0;
            }
            for (int side = -1; side <= 1; side += 2)
            {
                double p[2] = {o->center[0], o->center[1]};
                p[o->axis] = candidate + side * o->width / 2;
                int on_wall = 0;
                for (size_t k = 0; k < h->wall_count; k++)
                {
                    const u_wall *w = h->walls + k;
                    if (fabs(w->a[o->axis] - w->b[o->axis]) > 1e-8 &&
                        u_distance(p, w->a, w->b) < 1e-8)
                        on_wall = 1;
                }
                if (!on_wall)
                    valid = 0;
            }
            if (valid && fabs(candidate - original) < best_distance)
            {
                best = candidate;
                best_distance = fabs(candidate - original);
            }
        }
        if (best_distance == 1e30)
            return 0;
        o->center[o->axis] = best;
    }
    /* Swing shared doors into a room rather than the narrow central hall.
       This keeps simultaneous open panels clear of neighboring passages. */
    for (size_t j = 0; j < h->opening_count; j++)
    {
        u_opening *o = h->openings + j;
        if (o->window)
            continue;
        double best = -1e30;
        for (n_int r = 0; r < h->raw.roomcount; r++)
        {
            const simulated_room *room = h->raw.local_room + r;
            double lo[2] = {metric(h, room->points[0].x, 0), metric(h, room->points[0].y, 1)};
            double hi[2] = {metric(h, room->points[2].x, 0), metric(h, room->points[2].y, 1)};
            int normal = 1 - o->axis;
            if (o->center[o->axis] - o->width / 2 < lo[o->axis] - 1e-8 ||
                o->center[o->axis] + o->width / 2 > hi[o->axis] + 1e-8)
                continue;
            double direction = 0;
            if (fabs(o->center[normal] - lo[normal]) < 1e-8)
                direction = 1;
            if (fabs(o->center[normal] - hi[normal]) < 1e-8)
                direction = -1;
            if (!direction)
                continue;
            double score = (hi[0] - lo[0]) * (hi[1] - lo[1]);
            if (room->points[2].x - room->points[0].x == 16)
                score -= 1e6;
            if (score > best)
            {
                best = score;
                o->inward[0] = o->inward[1] = 0;
                o->inward[normal] = direction;
            }
        }
    }
    /* Pick the hinge that keeps the open leaf furthest from other doorways. */
    for (size_t j = 0; j < h->opening_count; j++)
    {
        u_opening *o = h->openings + j;
        if (o->window)
            continue;
        int best_hinge = 0;
        double best = -1;
        for (int hinge = 0; hinge < 2; hinge++)
        {
            o->hinge = hinge;
            o->open = 1;
            double a[2], b[2], clearance = 1e30;
            u_panel(o, a, b);
            for (size_t k = 0; k < h->opening_count; k++)
                if (k != j && !h->openings[k].window)
                    clearance = fmin(clearance, u_distance(h->openings[k].center, a, b));
            if (clearance > best)
            {
                best = clearance;
                best_hinge = hinge;
            }
        }
        o->hinge = best_hinge;
        o->open = 0;
    }
    /* First source door is the exterior entry. */
    for (size_t j = 0; j < h->opening_count; j++)
        if (!h->openings[j].window)
        {
            memcpy(h->entry, h->openings[j].center, sizeof h->entry);
            break;
        }
    /* Classify after every room has contributed shared wall ownership. */
    for (size_t j = 0; j < h->opening_count; j++)
    {
        u_opening *o = h->openings + j;
        o->exterior = 0;
        for (size_t i = 0; i < h->wall_count; i++)
            if (h->walls[i].owners == 1 &&
                u_distance(o->center, h->walls[i].a, h->walls[i].b) < 1e-8)
                o->exterior = 1;
    }
    double gap = h->entry[0];
    u_line fence[5] = {{{-28, -30}, {gap - 1.9, -30}},
                       {{gap + 1.9, -30}, {28, -30}},
                       {{-28, -30}, {-28, 30}},
                       {{28, -30}, {28, 30}},
                       {{-28, 30}, {28, 30}}};
    memcpy(h->fences, fence, sizeof fence);
    return 1;
}
pra_scene *pra_create(const pra_config *config)
{
    if (!config || !config->houses || config->houses > PRA_MAX_HOUSES ||
        !isfinite(config->origin[0]) || !isfinite(config->origin[1]) ||
        fabs(config->origin[0]) > 1e9 || fabs(config->origin[1]) > 1e9)
        return NULL;
    pra_scene *s = calloc(1, sizeof *s);
    if (!s)
        return NULL;
    s->config = *config;
    s->house = calloc(config->houses, sizeof *s->house);
    if (!s->house)
    {
        free(s);
        return NULL;
    }
    n_byte2 seed[2] = {config->seed[0], config->seed[1]};
    for (uint32_t i = 0; i < config->houses; i++)
    {
        u_house *h = s->house + i;
        n_vect2 zero = {.x = 0, .y = 0};
        memcpy(h->seed, seed, sizeof seed);
        house_create(&h->raw, seed, &zero);
        if (!h->raw.captured || h->raw.roomcount < 1 || h->raw.roomcount > MAX_ROOMS)
        {
            pra_destroy(s);
            return NULL;
        }
        h->center[0] = config->origin[0] + 100 * (i % 8);
        h->center[1] = config->origin[1] + 100 * (i / 8);
        h->m[0] = (double)h->raw.direction.x / SINE_MAXIMUM;
        h->m[1] = (double)h->raw.direction.y / SINE_MAXIMUM;
        if (!u_plan(h, i))
        {
            pra_destroy(s);
            return NULL;
        }
        /* Yard placement is an adapter extension; preserve tree_create profiles. */
        for (int t = 0; t < 4; t++)
        {
            n_vect2 p = {.x = (t & 1) ? 320 : -320, .y = (t & 2) ? 330 : -330};
            p.x += (math_random(seed) % 21) - 10;
            p.y += (math_random(seed) % 21) - 10;
            tree_create(h->trees + t, seed, &p);
        }
    }
    return s;
}
void pra_destroy(pra_scene *s)
{
    if (s)
    {
        free(s->house);
        free(s);
    }
}
uint32_t pra_house_count(const pra_scene *s)
{
    return s ? s->config.houses : 0;
}
uint32_t pra_room_count(const pra_scene *s, uint32_t h)
{
    return s && h < s->config.houses ? (uint32_t)s->house[h].raw.roomcount : 0;
}
int pra_room_points(const pra_scene *s, uint32_t h, uint32_t r, int64_t points[32][2])
{
    if (!points || r >= pra_room_count(s, h))
        return 0;
    for (int i = 0; i < 32; i++)
        for (int a = 0; a < 2; a++)
            points[i][a] = s->house[h].raw.room[r].points[i].data[a];
    return 1;
}
uint32_t pra_door_count(const pra_scene *s)
{
    uint32_t n = 0;
    if (s)
        for (uint32_t h = 0; h < s->config.houses; h++)
            for (size_t j = 0; j < s->house[h].opening_count; j++)
                n += !s->house[h].openings[j].window;
    return n;
}
int pra_get_door(const pra_scene *s, uint32_t index, pra_door *door)
{
    if (!s || !door)
        return 0;
    for (uint32_t h = 0; h < s->config.houses; h++)
        for (size_t j = 0; j < s->house[h].opening_count; j++)
        {
            const u_opening *o = s->house[h].openings + j;
            if (o->window)
                continue;
            if (index-- == 0)
            {
                door->id = o->id;
                door->house = h;
                u_world(s->house + h, o->center, door->center);
                door->width = o->width;
                door->open = o->open;
                door->exterior = o->exterior;
                return 1;
            }
        }
    return 0;
}
int pra_set_door(pra_scene *s, uint32_t id, int open)
{
    if (s)
        for (uint32_t h = 0; h < s->config.houses; h++)
            for (size_t j = 0; j < s->house[h].opening_count; j++)
            {
                u_opening *o = s->house[h].openings + j;
                if (!o->window && o->id == id)
                {
                    o->open = !!open;
                    return 1;
                }
            }
    return 0;
}
void pra_open_all(pra_scene *s, int open)
{
    if (s)
        for (uint32_t i = 0; i < pra_door_count(s); i++)
        {
            pra_door d;
            pra_get_door(s, i, &d);
            pra_set_door(s, d.id, open);
        }
}
static int wall_blocks(const u_house *h, const u_wall *w, const double p[2], double r, double feet,
                       double height)
{
    if (feet >= U_HEIGHT || feet + height <= 0 || u_distance(p, w->a, w->b) > r + U_THICKNESS / 2)
        return 0;
    int axis = fabs(w->a[0] - w->b[0]) > 1e-8 ? 0 : 1;
    for (size_t j = 0; j < h->opening_count; j++)
    {
        const u_opening *o = h->openings + j;
        if (o->window || !o->open || o->axis != axis ||
            fabs(o->center[1 - axis] - w->a[1 - axis]) > 1e-8 || feet < 0 ||
            feet + height > U_DOOR_TOP)
            continue;
        if (fabs(p[axis] - o->center[axis]) + r < o->width / 2)
            return 0;
    }
    return 1;
}
int pra_blocked(const pra_scene *s, double x, double z, double r, double feet, double height)
{
    if (!s || !isfinite(x) || !isfinite(z) || !isfinite(r) || !isfinite(feet) ||
        !isfinite(height) || !isfinite(feet + height) || r < 0 || height <= 0)
        return 1;
    double world[2] = {x, z};
    for (uint32_t i = 0; i < s->config.houses; i++)
    {
        const u_house *h = s->house + i;
        double p[2];
        u_local(h, world, p);
        double radius = r / hypot(h->m[0], h->m[1]);
        /* Floors and ceilings remain physical in a cutaway mesh view. */
        for (n_int room = 0; room < h->raw.roomcount; room++)
        {
            const simulated_room *plan = h->raw.local_room + room;
            int floor_hit = feet < -1e-8 && feet + height > -.12 + 1e-8;
            int roof_hit = feet < U_HEIGHT + .18 - 1e-8 && feet + height > U_HEIGHT + 1e-8;
            if (!floor_hit && !roof_hit)
                continue;
            double eaves = roof_hit ? .15 : 0;
            double x0 = metric(h, plan->points[0].x, 0) - eaves;
            double z0 = metric(h, plan->points[0].y, 1) - eaves;
            double x1 = metric(h, plan->points[2].x, 0) + eaves;
            double z1 = metric(h, plan->points[2].y, 1) + eaves;
            double dx = fmax(fmax(x0 - p[0], p[0] - x1), 0);
            double dz = fmax(fmax(z0 - p[1], p[1] - z1), 0);
            if (hypot(dx, dz) <= radius)
                return 1;
        }
        for (size_t j = 0; j < h->wall_count; j++)
            if (wall_blocks(h, h->walls + j, p, radius, feet, height))
                return 1;
        if (feet < U_DOOR_TOP && feet + height > 0)
            for (size_t j = 0; j < h->opening_count; j++)
            {
                const u_opening *o = h->openings + j;
                if (o->window)
                    continue;
                double a[2], b[2];
                u_panel(o, a, b);
                if (u_distance(p, a, b) <= radius + 0.025)
                    return 1;
            }
        if (feet < 1.1 && feet + height > 0)
            for (int j = 0; j < 5; j++)
                if (u_distance(p, h->fences[j].a, h->fences[j].b) <= radius + .06)
                    return 1;
        if (feet < 3 && feet + height > 0)
            for (int t = 0; t < 4; t++)
                if (hypot(p[0] - h->trees[t].center.x * U_SCALE,
                          p[1] - h->trees[t].center.y * U_SCALE) <= radius + .16)
                    return 1;
    }
    return 0;
}
int pra_sheltered(const pra_scene *s, double x, double z)
{
    if (!s || !isfinite(x) || !isfinite(z))
        return 0;
    double world[2] = {x, z};
    for (uint32_t i = 0; i < s->config.houses; i++)
    {
        const u_house *h = s->house + i;
        double p[2];
        u_local(h, world, p);
        for (n_int j = 0; j < h->raw.roomcount; j++)
        {
            const simulated_room *r = h->raw.local_room + j;
            if (p[0] >= metric(h, r->points[0].x, 0) && p[0] <= metric(h, r->points[2].x, 0) &&
                p[1] >= metric(h, r->points[0].y, 1) && p[1] <= metric(h, r->points[2].y, 1))
                return 1;
        }
    }
    return 0;
}
typedef struct
{
    char *buffer;
    size_t capacity, length;
} writer;
static void out(writer *w, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t left = w->length < w->capacity ? w->capacity - w->length : 0;
    int n = vsnprintf(left ? w->buffer + w->length : NULL, left, fmt, args);
    va_end(args);
    if (n > 0)
        w->length += (size_t)n;
}
size_t pra_json(const pra_scene *s, char *buffer, size_t capacity)
{
    writer w = {buffer, buffer ? capacity : 0, 0};
    if (!s)
    {
        out(&w, "null");
        return w.length;
    }
    out(&w,
        "{\"schema\":1,\"component\":\"PRA\",\"units\":\"metres\",\"seed\":[%u,%u],\"houses\":[",
        s->config.seed[0], s->config.seed[1]);
    for (uint32_t i = 0; i < s->config.houses; i++)
    {
        const u_house *h = s->house + i;
        out(&w, "%s{\"id\":%u,\"center\":[%.17g,%.17g],\"source_seed\":[%u,%u],\"rooms\":[",
            i ? "," : "", i, h->center[0], h->center[1], h->seed[0], h->seed[1]);
        for (n_int r = 0; r < h->raw.roomcount; r++)
        {
            out(&w, "%s[", r ? "," : "");
            for (int j = 0; j < 32; j++)
                out(&w, "%s[%lld,%lld]", j ? "," : "", (long long)h->raw.room[r].points[j].x,
                    (long long)h->raw.room[r].points[j].y);
            out(&w, "]");
        }
        out(&w, "],\"doors\":[");
        int comma = 0;
        for (size_t j = 0; j < h->opening_count; j++)
        {
            const u_opening *o = h->openings + j;
            if (o->window)
                continue;
            double p[2];
            u_world(h, o->center, p);
            out(&w,
                "%s{\"id\":%u,\"center\":[%.17g,%.17g],\"width\":%.17g,\"open\":%s,\"exterior\":%"
                "s}",
                comma++ ? "," : "", o->id, p[0], p[1], o->width, o->open ? "true" : "false",
                o->exterior ? "true" : "false");
        }
        out(&w, "]}");
    }
    out(&w, "]}");
    return w.length;
}

void pra_seed_trees_by_position(pra_scene *s)
{
    if (!s) return;
    for (uint32_t i=0;i<s->config.houses;i++) {
        u_house *h=s->house+i;
        for(int t=0;t<4;t++) {
            simulated_tree *tree=h->trees+t;
            double local[2]={tree->center.x*U_SCALE,tree->center.y*U_SCALE},world[2];
            u_world(h,local,world);
            uint32_t x=(uint32_t)(int64_t)llround(world[0]*100),z=(uint32_t)(int64_t)llround(world[1]*100);
            uint32_t hash=(x*UINT32_C(0x9e3779b9))^(z*UINT32_C(0x85ebca6b));
            hash^=hash>>16;hash*=UINT32_C(0x7feb352d);hash^=hash>>15;
            n_byte2 seed[2]={(uint16_t)hash,(uint16_t)(hash>>16)};
            n_vect2 center=tree->center;
            tree_create(tree,seed,&center);
        }
    }
}
