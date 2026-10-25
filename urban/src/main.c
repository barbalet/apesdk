/****************************************************************

 main.c

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

#include "pra.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int obj(const pra_scene *scene, const char *path, unsigned flags)
{
    pra_mesh mesh = {0};
    double origin[2] = {0, 0};
    if (!pra_build_mesh(scene, flags, origin, &mesh))
        return 0;
    FILE *f = fopen(path, "w");
    if (!f)
    {
        pra_mesh_free(&mesh);
        return 0;
    }
    fprintf(f, "# PRA: metres, Y up. %zu triangle vertices\n", mesh.vertex_count);
    for (size_t i = 0; i < mesh.vertex_count; i++)
    {
        float *v = mesh.vertices + 9 * i;
        fprintf(f, "v %.9g %.9g %.9g %.6g %.6g %.6g\n", v[0], v[1], v[2], v[6], v[7], v[8]);
    }
    for (size_t i = 0; i < mesh.vertex_count; i++)
    {
        float *v = mesh.vertices + 9 * i;
        fprintf(f, "vn %.6g %.6g %.6g\n", v[3], v[4], v[5]);
    }
    for (size_t i = 0; i < mesh.vertex_count; i += 3)
        fprintf(f, "f %zu//%zu %zu//%zu %zu//%zu\n", i + 1, i + 1, i + 2, i + 2, i + 3, i + 3);
    int ok = !ferror(f);
    if (fclose(f))
        ok = 0;
    pra_mesh_free(&mesh);
    return ok;
}
int main(int argc, char **argv)
{
    pra_config config = {{1234, 5678}, 1, {0, 0}};
    const char *path = NULL;
    int open = 0;
    unsigned flags = PRA_ROOFS | PRA_YARDS;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--cutaway"))
        {
            flags &= ~PRA_ROOFS;
            continue;
        }
        if (!strcmp(argv[i], "--open"))
        {
            open = 1;
            continue;
        }
        if (i + 1 >= argc)
        {
            fprintf(stderr, "Missing option value\n");
            return 2;
        }
        const char *option = argv[i++], *value = argv[i];
        if (!strcmp(option, "--obj"))
            path = value;
        else
        {
            errno = 0;
            char *end;
            unsigned long n = strtoul(value, &end, 10);
            if (errno || *end || *value == '-' || !*value || n > UINT32_MAX)
                return 2;
            if (!strcmp(option, "--seed"))
            {
                config.seed[0] = (uint16_t)n;
                config.seed[1] = (uint16_t)(n >> 16);
            }
            else if (!strcmp(option, "--houses"))
                config.houses = (uint32_t)n;
            else
                return 2;
        }
    }
    pra_scene *scene = pra_create(&config);
    if (!scene)
    {
        fprintf(stderr,
                "Invalid configuration, degenerate random sequence, or allocation failure\n");
        return 1;
    }
    pra_open_all(scene, open);
    if (path)
    {
        if (!obj(scene, path, flags))
        {
            pra_destroy(scene);
            return 1;
        }
    }
    else
    {
        size_t length = pra_json(scene, NULL, 0);
        char *json = malloc(length + 1);
        if (!json)
        {
            pra_destroy(scene);
            return 1;
        }
        pra_json(scene, json, length + 1);
        puts(json);
        free(json);
    }
    pra_destroy(scene);
    return 0;
}
