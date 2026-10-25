/****************************************************************

 pra.h

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 ****************************************************************/

#ifndef WASM_Z_PRA_H
#define WASM_Z_PRA_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
#define PRA_MAX_HOUSES 64u
#define PRA_MAX_ROOMS 16u
#define PRA_METRES_PER_UNIT 0.07
#define PRA_ROOFS 1u
#define PRA_YARDS 2u
#define PRA_NO_DOORS 4u
#define PRA_DOORS_ONLY 8u

    typedef struct pra_scene pra_scene;
    typedef struct
    {
        uint16_t seed[2];
        uint32_t houses;
        double origin[2];
    } pra_config;
    typedef struct
    {
        /* Triangle list: position xyz, normal xyz, linear color rgb (9 floats). */
        float *vertices;
        size_t vertex_count;
    } pra_mesh;
    typedef struct
    {
        uint32_t id, house;
        double center[2], width;
        int open, exterior;
    } pra_door;

    /* No mutable globals: callers own scenes and meshes. Invalid input, degenerate random
     * sequences, or OOM return NULL/0. */
    pra_scene *pra_create(const pra_config *config);
    void pra_destroy(pra_scene *scene);
    uint32_t pra_house_count(const pra_scene *scene);
    uint32_t pra_room_count(const pra_scene *scene, uint32_t house);
    /* Exact original transformed integer room record, centered at zero.
       Scale by PRA_METRES_PER_UNIT and add the house center for world placement. */
    int pra_room_points(const pra_scene *scene, uint32_t house, uint32_t room,
                        int64_t points[32][2]);
    uint32_t pra_door_count(const pra_scene *scene);
    int pra_get_door(const pra_scene *scene, uint32_t index, pra_door *door);
    int pra_set_door(pra_scene *scene, uint32_t id, int open);
    void pra_open_all(pra_scene *scene, int open);
    /* Optional world adapter: shape seeds derive from each tree’s absolute X/Z. */
    void pra_seed_trees_by_position(pra_scene *scene);
    /* Capsule query in absolute metres; invalid queries conservatively block. */
    int pra_blocked(const pra_scene *scene, double x, double z, double radius, double feet,
                    double height);
    int pra_sheltered(const pra_scene *scene, double x, double z);
    /* origin rebases the mesh, retaining precision at distant coordinates.
       Pass an empty output; free any existing mesh before reuse. Failure leaves it intact. */
    int pra_build_mesh(const pra_scene *scene, unsigned flags, const double origin[2],
                       pra_mesh *mesh);
    void pra_mesh_free(pra_mesh *mesh);
    /* Required bytes excluding NUL. NULL/0 measures size; short buffers truncate safely. */
    size_t pra_json(const pra_scene *scene, char *buffer, size_t capacity);
#ifdef __cplusplus
}
#endif
#endif
