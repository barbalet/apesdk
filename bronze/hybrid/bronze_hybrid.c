/* Bronze-specific sidecar layer. Native ApeSDK owns land, beings and movement. */
#include "bronze_hybrid.h"
#include "../../sim/sim.h"
#include <string.h>
#include <stdio.h>
#include "../../gui/gui.h"
#include "../work/bronze_work.h"

enum { SITE_COUNT = 10 };
static bronze_site sites[SITE_COUNT];
static bronze_economy economy;
static n_uint site_count;
static n_uint last_day;
static uint64_t last_tick;

void bronze_hybrid_reset(void)
{
    n_uint i;
    memset(sites, 0, sizeof(sites));
    memset(&economy, 0, sizeof(economy));
    site_count = 0;
    last_day = land_date();
    last_tick = UINT64_MAX;
    /* Fixed scan ordering makes placement repeatable for a given terrain.
       Keep every site above the maximum waterline, including tidal changes. */
    for (i = 0; i < SITE_COUNT; i++)
    {
        n_uint scan;
        n_int start = (n_int)((i * 7919u + 1237u) % MAP_AREA);
        for (scan = 0; scan < MAP_AREA; scan++)
        {
            n_int index = (start + scan) % MAP_AREA;
            n_int x = index % MAP_DIMENSION, y = index / MAP_DIMENSION;
            n_uint j;
            n_int valid = land_topography()[index] > 150;
            for (j = 0; valid && j < site_count; j++)
            {
                n_int dx = x - sites[j].x, dy = y - sites[j].y;
                if (dx * dx + dy * dy < 144) valid = 0;
            }
            if (valid)
            {
                bronze_site *site = &sites[site_count++];
                site->x = x; site->y = y;
                site->kind = (bronze_site_kind)(i % 5);
                site->capacity = 200;
                site->stock = site->capacity;
                break;
            }
        }
    }
}

void bronze_hybrid_cycle(simulated_group *group)
{
    n_uint i, j;
    /* One economic sampling interval per simulated hour, independent of frames. */
    uint64_t tick = bronze_native_tick();
    if (land_time() % 60 != 0 || tick == last_tick) return;
    last_tick = tick;
    if (land_date() != last_day)
    {
        for (j = 0; j < site_count; j++)
            if (sites[j].kind == BRONZE_FIELD || sites[j].kind == BRONZE_WOODLAND)
            {
                sites[j].stock += 5;
                if (sites[j].stock > sites[j].capacity) sites[j].stock = sites[j].capacity;
            }
        last_day = land_date();
    }
    for (i = 0; i < group->num; i++)
    {
        simulated_being *being = &group->beings[i];
        if (!being_awake(being)) continue;
        n_int x = APESPACE_TO_MAPSPACE(being_location_x(being));
        n_int y = APESPACE_TO_MAPSPACE(being_location_y(being));
        for (j = 0; j < site_count; j++)
        {
            bronze_site *site = &sites[j];
            n_int dx = x - site->x, dy = y - site->y;
            if (dx * dx + dy * dy > 100 || site->kind == BRONZE_TOWN || site->stock < 1) continue;
            if (!bronze_work_claim(being)) break;
            site->stock -= 1;
            switch (site->kind)
            {
                case BRONZE_FIELD: economy.grain += 1; break;
                case BRONZE_COPPER_MINE: economy.copper += 1; break;
                case BRONZE_TIN_MINE: economy.tin += 1; break;
                case BRONZE_WOODLAND: economy.charcoal += 1; break;
                default: break;
            }
            economy.work_events++;
            break;
        }
    }
    if (economy.copper >= 1 && economy.tin >= 1 && economy.charcoal >= 1)
    {
        economy.copper--; economy.tin--; economy.charcoal--; economy.tools++;
    }
}

void bronze_hybrid_draw(n_int dimension, bronze_plot plot, void *context)
{
    n_uint i;
    n_int scale = dimension / MAP_DIMENSION;
    if (!plot || scale < 1) return;
    for (i = 0; i < site_count; i++)
    {
        const bronze_site *site = &sites[i];
        n_int x, y, radius = site->kind == BRONZE_TOWN ? 4 : 3;
        /* White roofed town, dark mine entrance, blue field/woodland outline.
           Use the existing palette; don't change colors of native graphics. */
        n_byte color = site->kind == BRONZE_TOWN ? COLOR_WHITE :
                       (site->kind == BRONZE_COPPER_MINE || site->kind == BRONZE_TIN_MINE) ? COLOR_BLACK : COLOR_BLUE;
        for (y = -radius; y <= radius; y++)
            for (x = -radius; x <= radius; x++)
            {
                n_int ax = x < 0 ? -x : x;
                n_int edge;
                if (site->kind == BRONZE_TOWN)
                    edge = (ax == radius && y >= -radius / 2) || y == radius || y == -radius + ax / 2;
                else if (site->kind == BRONZE_WOODLAND)
                    edge = (y <= 1 && ax == (y + radius) / 2) || (x == 0 && y >= 1);
                else
                    edge = ax == radius || y == -radius || y == radius ||
                           (site->kind == BRONZE_FIELD && y == 0);
                n_int px = site->x * scale + x, py = site->y * scale + y;
                if (edge && px >= 0 && py >= 0 && px < dimension && py < dimension)
                    plot(px, py, color, context);
            }
        /* Tin mines have a central white point; copper mines a white cross. */
        if (site->kind == BRONZE_TIN_MINE || site->kind == BRONZE_COPPER_MINE)
        {
            plot(site->x * scale, site->y * scale, COLOR_WHITE, context);
            if (site->kind == BRONZE_COPPER_MINE)
                plot(site->x * scale + 1, site->y * scale, COLOR_WHITE, context);
        }
    }
}
const bronze_site *bronze_hybrid_sites(n_uint *count) { if (count) *count = site_count; return sites; }
const bronze_economy *bronze_hybrid_economy(void) { return &economy; }

typedef struct { n_byte4 *pixels; n_int width, height; } bronze_hud_buffer;
static n_byte bronze_hud_pixel(n_int x, n_int y, n_int dx, n_int dy, void *context)
{
    bronze_hud_buffer *buffer = context;
    (void)dx; (void)dy;
    if (x >= 0 && y >= 0 && x < buffer->width && y < buffer->height)
        buffer->pixels[y * buffer->width + x] = 0xffffffffu;
    return 0;
}
void bronze_hybrid_draw_hud(n_byte4 *rgba, n_int width, n_int height)
{
    char line[80];
    n_int x, y, chars;
    bronze_hud_buffer buffer = {rgba, width, height};
    n_join draw = {bronze_hud_pixel, &buffer};
    if (!rgba || width < 80 || height < 48) return;
    chars = (width - 16) / 8;
    if (chars > 79) chars = 79;
    for (y = height - 40; y < height; y++)
        for (x = 0; x < width; x++)
        {
            /* The shared UIKit renderer consumes XRGB bytes (skip-first,
               big-endian CGImage), regardless of the host integer order. */
            n_byte *pixel = (n_byte *)&rgba[y * width + x];
            pixel[0] = 255; pixel[1] = 24; pixel[2] = 24; pixel[3] = 24;
        }
    snprintf(line, sizeof(line), "BRONZE: TOWNS / MINES / FIELDS / WOODLAND");
    line[chars] = 0;
    draw_string(line, 8, height - 34, &draw);
    snprintf(line, sizeof(line), "FOOD %.0f CU %.0f SN %.0f FUEL %.0f TOOLS %.0f",
             economy.grain, economy.copper, economy.tin, economy.charcoal, economy.tools);
    line[chars] = 0;
    draw_string(line, 8, height - 18, &draw);
}
