/* Browser demo ABI. Core scenes remain explicitly owned and reentrant. */
#include "pra.h"
#include <stdlib.h>
static pra_scene *scene;
static pra_mesh mesh;
static char *json;
void pra_demo_destroy(void)
{
    pra_mesh_free(&mesh);
    pra_destroy(scene);
    scene = NULL;
    free(json);
    json = NULL;
}
int pra_demo_init(unsigned seed0, unsigned seed1, unsigned houses)
{
    pra_config config = {{(uint16_t)seed0, (uint16_t)seed1}, houses, {0, 0}};
    pra_scene *next = pra_create(&config);
    if (!next)
        return 0;
    pra_demo_destroy();
    scene = next;
    return 1;
}
const float *pra_demo_mesh(unsigned flags)
{
    double origin[2] = {0, 0};
    pra_mesh_free(&mesh);
    return pra_build_mesh(scene, flags, origin, &mesh) ? mesh.vertices : NULL;
}
size_t pra_demo_vertex_count(void)
{
    return mesh.vertex_count;
}
const char *pra_demo_json(void)
{
    free(json);
    json = NULL;
    size_t n = pra_json(scene, NULL, 0);
    json = malloc(n + 1);
    if (json)
        pra_json(scene, json, n + 1);
    return json;
}
void pra_demo_open_all(int open)
{
    pra_open_all(scene, open);
}
int pra_demo_set_door(unsigned id, int open)
{
    return pra_set_door(scene, id, open);
}
int pra_demo_toggle(double x, double z)
{
    pra_door closest = {0};
    double best = 2.5 * 2.5;
    for (uint32_t i = 0; i < pra_door_count(scene); i++)
    {
        pra_door d;
        pra_get_door(scene, i, &d);
        double dx = d.center[0] - x, dz = d.center[1] - z, dist = dx * dx + dz * dz;
        if (dist < best)
        {
            best = dist;
            closest = d;
        }
    }
    return closest.id ? pra_set_door(scene, closest.id, !closest.open) : 0;
}
int pra_demo_blocked(double x, double z)
{
    return pra_blocked(scene, x, z, .30, 0, 1.8);
}
int pra_demo_sheltered(double x, double z)
{
    return pra_sheltered(scene, x, z);
}

int pra_demo_probe(double x, double z, double radius, double feet, double height)
{
    return pra_blocked(scene, x, z, radius, feet, height);
}
