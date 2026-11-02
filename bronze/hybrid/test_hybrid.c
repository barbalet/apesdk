#include "bronze_hybrid.h"
#include "../../universe/universe.h"
#include "../../gui/gui.h"
#include "../../shared.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static n_uint pixels;
static n_int dimension;
static void check_pixel(n_int x, n_int y, n_byte color, void *context)
{
    (void)color; (void)context;
    assert(x >= 0 && y >= 0 && x < dimension && y < dimension);
    pixels++;
}
int main(void)
{
    bronze_site first[10];
    const bronze_site *sites;
    n_uint count, i;
    n_byte2 seed[2] = {17,29}, location[2];
    simulated_group *group;
    assert(draw_offscreen(sim_init(KIND_START_UP, 0x42u, MAP_AREA + TERRAIN_WINDOW_AREA + CONTROL_WINDOW_AREA, 0)));
    sim_cycle();
    draw_window(MAP_DIMENSION, MAP_DIMENSION);
    weather_set_lightning(0, 3, 5, 0);
    draw_cycle(0, 1 << NUM_VIEW);
    {
        n_byte before = draw_weather_grayscale()[3 + 5 * MAP_DIMENSION];
        weather_set_lightning(0, 3, 5, 255);
        draw_cycle(0, 1 << NUM_VIEW);
        assert(draw_weather_grayscale()[3 + 5 * MAP_DIMENSION] == before);
    }
    assert(weather_lightning(3, 5) == 0);
    assert(weather_seven_values(MAPSPACE_TO_APESPACE(3), MAPSPACE_TO_APESPACE(5)) < WEATHER_SEVEN_LIGHTNING_DAY);
    weather_cycle();
    assert(weather_lightning(3, 5) == 0);
    sites = bronze_hybrid_sites(&count);
    assert(count == 10);
    memcpy(first, sites, sizeof(first));
    for (i = 0; i < count; i++)
    {
        assert(land_topography()[sites[i].x + sites[i].y * MAP_DIMENSION] > 150);
        assert(sites[i].kind == (bronze_site_kind)(i % 5));
    }
    bronze_hybrid_reset();
    assert(memcmp(first, sites, sizeof(first)) == 0);
    dimension = MAP_DIMENSION;
    bronze_hybrid_draw(dimension, check_pixel, NULL);
    assert(pixels > 100);
    pixels = 0; dimension = HI_RES_MAP_DIMENSION;
    bronze_hybrid_draw(dimension, check_pixel, NULL);
    assert(pixels > 100);
    group = sim_group();
    /* Use a living native being at a field, and verify bounded production
       without Bronze changing the being's position or population. */
    location[0] = MAPSPACE_TO_APESPACE(sites[1].x);
    location[1] = MAPSPACE_TO_APESPACE(sites[1].y);
    being_set_location(&group->beings[0], location);
    land_load_state(42, 720, seed);
    assert(being_awake(&group->beings[0]));
    bronze_hybrid_cycle(group);
    {
        double before = bronze_hybrid_economy()->grain;
        bronze_hybrid_cycle(group);
        assert(bronze_hybrid_economy()->grain == before);
    }
    for (i = 1; i < 220; i++)
    {
        land_load_state(42 + i / 24, (720 + (i % 24) * 60) % 1440, seed);
        bronze_hybrid_cycle(group);
    }
    assert(bronze_hybrid_economy()->grain > 0);
    assert(sites[1].stock >= 0 && sites[1].stock < sites[1].capacity);
    assert(being_location_x(&group->beings[0]) == location[0]);
    assert(being_location_y(&group->beings[0]) == location[1]);
    bronze_hybrid_reset();
    assert(bronze_hybrid_economy()->grain == 0);
    assert(bronze_hybrid_economy()->tools == 0);
    {
        n_byte4 *hud = calloc(512 * 512 + 2, sizeof(n_byte4));
        assert(hud);
        hud[0] = 123; hud[512 * 512 + 1] = 456;
        bronze_hybrid_draw_hud(hud + 1, 512, 512);
        assert(hud[0] == 123 && hud[512 * 512 + 1] == 456);
        assert(hud[1] == 0); /* HUD preserves map pixels above its panel. */
        assert(hud[512 * 512] != 0);
        free(hud);
    }
    sim_close();
    puts("Bronze hybrid terrain, symbols, bounded economy and reset checks passed.");
    return 0;
}
