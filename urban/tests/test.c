/* Copyright 2026 Tom Barbalet. MIT; see ../LICENSE. */
#include "pra_internal.h"
#include <assert.h>
#include <stdio.h>
static void doors(pra_scene *s)
{
    for (uint32_t i = 0; i < pra_door_count(s); i++)
    {
        pra_door d;
        assert(pra_get_door(s, i, &d));
        assert(d.width > .69 && d.width < .71);
        assert(pra_blocked(s, d.center[0], d.center[1], .30, 0, 1.8));
        assert(pra_set_door(s, d.id, 1));
        if (pra_blocked(s, d.center[0], d.center[1], .30, 0, 1.8))
        {
            fprintf(stderr, "Blocked open door %u seed %u %u at %.6g %.6g\n", d.id,
                    s->config.seed[0], s->config.seed[1], d.center[0], d.center[1]);
            for (size_t j = 0; j < s->house[d.house].opening_count; j++)
            {
                u_opening *o = s->house[d.house].openings + j;
                if (!o->window)
                    fprintf(stderr, "door %u axis%d %.4g %.4g open%d\n", o->id, o->axis,
                            o->center[0], o->center[1], o->open);
            }
            double local[2];
            u_local(s->house + d.house, d.center, local);
            for (size_t j = 0; j < s->house[d.house].wall_count; j++)
            {
                u_wall *w = s->house[d.house].walls + j;
                if (u_distance(local, w->a, w->b) < .361)
                    fprintf(stderr, "wall %zu %.4g %.4g -> %.4g %.4g owners%d\n", j, w->a[0],
                            w->a[1], w->b[0], w->b[1], w->owners);
            }
            abort();
        }
        assert(pra_blocked(s, d.center[0], d.center[1], .30, 0, 2.2));
        assert(pra_set_door(s, d.id, 0));
    }
}
int main(void)
{
    pra_config cfg = {{1234, 5678}, 1, {0, 0}};
    pra_config rejected = {{20600, 18}, 1, {0, 0}};
    assert(!pra_create(&rejected));
    pra_config largest = {{130, 10}, 1, {0, 0}};
    pra_scene *ten = pra_create(&largest);
    assert(ten);
    assert(pra_room_count(ten, 0) == 10);
    doors(ten);
    pra_open_all(ten, 1);
    for (uint32_t i = 0; i < pra_door_count(ten); i++)
    {
        pra_door d;
        assert(pra_get_door(ten, i, &d));
        assert(!pra_blocked(ten, d.center[0], d.center[1], .30, 0, 1.8));
    }
    pra_destroy(ten); /* Original SDK fixed point rejected forever. */
    assert(!pra_create(NULL));
    cfg.houses = 0;
    assert(!pra_create(&cfg));
    cfg.houses = 65;
    assert(!pra_create(&cfg));
    cfg.houses = 1;
    cfg.origin[0] = NAN;
    assert(!pra_create(&cfg));
    cfg.origin[0] = 0;
    pra_scene *s = pra_create(&cfg);
    assert(s);
    assert(pra_house_count(s) == 1);
    assert(pra_room_count(s, 0) >= 3);
    doors(s);
    pra_open_all(s, 1);
    for (uint32_t i = 0; i < pra_door_count(s); i++)
    {
        pra_door d;
        assert(pra_get_door(s, i, &d));
        assert(!pra_blocked(s, d.center[0], d.center[1], .30, 0, 1.8));
    }
    pra_open_all(s, 0);
    assert(!pra_set_door(s, UINT32_MAX, 1));
    assert(!pra_get_door(s, pra_door_count(s), &(pra_door){0}));
    assert(pra_blocked(s, NAN, 0, .2, 0, 1.8));
    assert(pra_blocked(s, 0, 0, -1, 0, 1.8));
    assert(!pra_sheltered(s, 1e6, 1e6));
    for (n_int i = 0; i < s->house[0].raw.roomcount; i++)
    {
        u_house *h = s->house;
        simulated_room *r = h->raw.local_room + i;
        double p[2] = {((r->points[0].x + r->points[2].x) / 2.0 - h->raw.pivot.x) * U_SCALE,
                       ((r->points[0].y + r->points[2].y) / 2.0 - h->raw.pivot.y) * U_SCALE},
               w[2];
        u_world(h, p, w);
        assert(pra_sheltered(s, w[0], w[1]));
        assert(!pra_blocked(s, w[0], w[1], .30, 0, 2.6));
        assert(pra_blocked(s, w[0], w[1], .30, 1.0, 1.8));
        assert(pra_blocked(s, w[0], w[1], .30, -.05, 1.8));
        assert(!pra_blocked(s, w[0], w[1], .30, 2.88, 1.8));
    }
    size_t size = pra_json(s, NULL, 0);
    char *json = malloc(size + 1);
    assert(json);
    assert(pra_json(s, json, size + 1) == size);
    assert(strlen(json) == size);
    for (size_t n = 1; n < 30; n++)
    {
        char buf[32];
        memset(buf, 42, sizeof buf);
        assert(pra_json(s, buf, n) == size);
        assert(buf[n - 1] == 0);
        assert(buf[n] == 42);
        assert(!memcmp(buf, json, n - 1));
    }
    free(json);
    pra_mesh m = {0};
    double origin[2] = {0, 0};
    assert(pra_build_mesh(s, PRA_YARDS | PRA_ROOFS, origin, &m));
    assert(m.vertex_count > 1000 && m.vertex_count % 3 == 0);
    for (size_t i = 0; i < m.vertex_count; i++)
    {
        float *v = m.vertices + 9 * i;
        for (int j = 0; j < 9; j++)
            assert(isfinite(v[j]));
        assert(fabs(sqrt(v[3] * v[3] + v[4] * v[4] + v[5] * v[5]) - 1) < 1e-5);
    }
    cfg.origin[0] = 1e8;
    cfg.origin[1] = -1e8;
    pra_scene *far = pra_create(&cfg);
    assert(far);
    pra_mesh rebased = {0};
    assert(pra_build_mesh(far, PRA_YARDS | PRA_ROOFS, cfg.origin, &rebased));
    assert(m.vertex_count == rebased.vertex_count);
    for (size_t i = 0; i < m.vertex_count * 9; i++)
        assert(fabs(m.vertices[i] - rebased.vertices[i]) < 1e-5);
    pra_mesh_free(&m);
    pra_mesh_free(&m);
    pra_mesh_free(&rebased);
    pra_destroy(far);
    pra_destroy(s);
    unsigned no_hallway = 0;
    size_t max_rooms = 0;
    for (uint32_t i = 0; i < 2000; i++)
    {
        cfg = (pra_config){{(uint16_t)(i * 7919), (uint16_t)(i * 104729 + 17)}, 1, {0, 0}};
        s = pra_create(&cfg);
        assert(s);
        u_house *h = s->house;
        int hallway = 0;
        for (n_int r = 0; r < h->raw.roomcount; r++)
        {
            simulated_room *room = h->raw.local_room + r;
            if (room->points[2].x - room->points[0].x == 16)
                hallway = 1;
        }
        no_hallway += !hallway;
        if ((size_t)h->raw.roomcount > max_rooms)
            max_rooms = (size_t)h->raw.roomcount;
        doors(s);
        pra_open_all(s, 1);
        for (uint32_t k = 0; k < pra_door_count(s); k++)
        {
            pra_door d;
            assert(pra_get_door(s, k, &d));
            if (pra_blocked(s, d.center[0], d.center[1], .30, 0, 1.8))
            {
                fprintf(stderr, "all-open blocked seed %u %u door %u\n", cfg.seed[0], cfg.seed[1],
                        d.id);
                double p[2];
                u_house *house = s->house + d.house;
                u_local(house, d.center, p);
                for (size_t j = 0; j < house->opening_count; j++)
                {
                    u_opening *o = house->openings + j;
                    if (o->window)
                        continue;
                    double a[2], b[2];
                    u_panel(o, a, b);
                    if (u_distance(p, a, b) < .326)
                        fprintf(stderr,
                                "blocked by panel %u at %.4g %.4g -> %.4g %.4g (query %.4g %.4g)\n",
                                o->id, a[0], a[1], b[0], b[1], p[0], p[1]);
                }
                abort();
            }
        }
        pra_destroy(s);
    }
    cfg.houses = PRA_MAX_HOUSES;
    s = pra_create(&cfg);
    assert(s);
    assert(pra_house_count(s) == PRA_MAX_HOUSES);
    assert(pra_room_count(s, PRA_MAX_HOUSES) == 0);
    assert(!pra_room_points(s, 0, 99, (int64_t[32][2]){{0}}));
    assert(!pra_build_mesh(s, 16, origin, &m));
    assert(pra_build_mesh(s, PRA_ROOFS | PRA_YARDS, cfg.origin, &m));
    assert(m.vertex_count > 0 && m.vertex_count < 4000000);
    pra_mesh_free(&m);
    pra_destroy(s);
    assert(no_hallway > 0);
    printf("PRA C checks passed: 2000 seeds, %u no-hallway layouts, max %zu rooms; "
           "door/collision/mesh/JSON/rebase checks\n",
           no_hallway, max_rooms);
    return 0;
}
