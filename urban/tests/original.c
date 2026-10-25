/****************************************************************

 original.c

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

/* Compare extracted code with separately compiled original Urban + full ApeSDK. */
#include "pra_internal.h"
#include "reference.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    uint16_t fixed_original[2] = {20600, 18}, fixed_extracted[2] = {20600, 18};
    for (unsigned i = 0; i < 1000; i++)
    {
        assert(reference_random(fixed_original) == 18);
        assert(math_random(fixed_extracted) == 18);
        assert(fixed_original[0] == 20600 && fixed_original[1] == 18);
        assert(!memcmp(fixed_original, fixed_extracted, sizeof fixed_original));
    }
    for (uint32_t i = 0; i <= 10000; i++)
    {
        n_byte2 a[2] = {(n_byte2)(i * 7919), (n_byte2)(i * 104729 + 17)};
        if (i == 10000)
        {
            a[0] = 130;
            a[1] = 10;
        }
        uint16_t b[2] = {a[0], a[1]};
        simulated_building x = {0};
        reference_house y = {0};
        n_vect2 center = {.x = (n_int)i - 5000, .y = 3000 - (n_int)i};
        int64_t p[2] = {center.x, center.y};
        house_create(&x, a, &center);
        reference_create_house(b, p, &y);
        assert(a[0] == b[0] && a[1] == b[1]);
        assert(x.roomcount == y.rooms);
        for (int g = 0; g < 64; g++)
            assert(x.house[g] == y.genetics[g]);
        for (n_int r = 0; r < x.roomcount; r++)
            for (int j = 0; j < 32; j++)
                for (int k = 0; k < 2; k++)
                    assert(x.room[r].points[j].data[k] == y.points[r][j][k]);
        simulated_tree t = {0};
        reference_tree u = {0};
        tree_create(&t, a, &center);
        reference_create_tree(b, p, &u);
        assert(a[0] == b[0] && a[1] == b[1]);
        assert(t.radius == u.radius && t.center.x == u.center[0] && t.center.y == u.center[1]);
        for (int j = 0; j < 32; j++)
            assert(t.points[j] == u.points[j]);
        simulated_fence f[8] = {0};
        int64_t g[8][2][2] = {0};
        fence_init(a, (n_byte)i, &center, f);
        reference_create_fence(b, (uint8_t)i, p, g);
        assert(a[0] == b[0] && a[1] == b[1]);
        for (int j = 0; j < 8; j++)
            for (int k = 0; k < 2; k++)
                for (int n = 0; n < 2; n++)
                    assert(f[j].points[k].data[n] == g[j][k][n]);
    }
    puts("PRA original-C parity passed: 10000 seeds plus ten-room regression, exact "
         "room/genetics/final-seed/tree/fence comparison against full ApeSDK");
    return 0;
}
