#include "apemovie_core.h"

#include <stddef.h>
#include <stdint.h>

#include "../shared.h"
#include "../sim/sim.h"
#include "../universe/universe.h"

static int apemovie_started = 0;
static int apemovie_size_changed = 1;
static uint32_t apemovie_ticks = 0;

static void apemovie_disable_event_logging(void)
{
    simulated_group *group = sim_group();

    if (group != 0L)
    {
        group->ext_birth = 0L;
        group->ext_death = 0L;
    }
}

int apemovie_start(uint32_t seed, const char *load_path, int load_is_script, int quiet_events)
{
    if (apemovie_started)
    {
        shared_close();
        apemovie_started = 0;
    }

    if (shared_init(WINDOW_PROCESSING, seed) < 0)
    {
        return 0;
    }

    if (quiet_events)
    {
        apemovie_disable_event_logging();
    }

    if ((load_path != 0L) && (load_path[0] != 0))
    {
        if (shared_openFileName(load_path, load_is_script) == 0)
        {
            shared_close();
            return 0;
        }

        if (quiet_events)
        {
            apemovie_disable_event_logging();
        }
    }

    apemovie_started = shared_simulation_started() != 0;
    apemovie_size_changed = 1;
    apemovie_ticks = seed;
    return apemovie_started;
}

int apemovie_render_frame_bgra(uint8_t *destination, int width, int height, int bytes_per_row, uint32_t cycles_per_frame)
{
    uint32_t cycle = 0;
    n_byte *source;

    if ((destination == 0L) || (width <= 0) || (height <= 0) || (bytes_per_row < (width * 4)) || !apemovie_started)
    {
        return 0;
    }

    while (cycle < cycles_per_frame)
    {
        shared_cycle(++apemovie_ticks, WINDOW_PROCESSING);
        cycle++;
    }

    source = shared_draw(NUM_TERRAIN, width, height, (n_byte)apemovie_size_changed);
    apemovie_size_changed = 0;

    if (source == 0L)
    {
        return 0;
    }

    for (int y = 0; y < height; y++)
    {
        const uint8_t *source_row = (const uint8_t *)source + ((size_t)y * (size_t)width * 4u);
        uint8_t *destination_row = destination + ((size_t)y * (size_t)bytes_per_row);

        for (int x = 0; x < width; x++)
        {
            const uint8_t *pixel = source_row + ((size_t)x * 4u);
            uint8_t *out_pixel = destination_row + ((size_t)x * 4u);

            out_pixel[0] = pixel[3];
            out_pixel[1] = pixel[2];
            out_pixel[2] = pixel[1];
            out_pixel[3] = 255;
        }
    }

    return 1;
}

void apemovie_finish(void)
{
    if (apemovie_started)
    {
        shared_close();
        apemovie_started = 0;
    }
}

uint32_t apemovie_minutes_per_day(void)
{
    return TIME_DAY_MINUTES;
}

uint32_t apemovie_land_date(void)
{
    return land_date();
}

uint32_t apemovie_land_time(void)
{
    return land_time();
}

int apemovie_being_count(void)
{
    simulated_group *group = sim_group();

    if (group == 0L)
    {
        return 0;
    }

    return (int)group->num;
}
