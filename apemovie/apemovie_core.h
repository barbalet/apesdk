#ifndef APEMOVIE_CORE_H
#define APEMOVIE_CORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int apemovie_start(uint32_t seed, const char *load_path, int load_is_script, int quiet_events);
int apemovie_render_frame_bgra(uint8_t *destination, int width, int height, int bytes_per_row, uint32_t cycles_per_frame);
void apemovie_finish(void);

uint32_t apemovie_minutes_per_day(void);
uint32_t apemovie_land_date(void);
uint32_t apemovie_land_time(void);
int apemovie_being_count(void);

#ifdef __cplusplus
}
#endif

#endif /* APEMOVIE_CORE_H */
