/* Browser boundary for the shared ApeSDK simulation. */
#include "../../toolkit/toolkit.h"
#include "../../sim/sim.h"
#include "../../universe/universe.h"
#include "../../shared.h"
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define WEB_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define WEB_EXPORT
#endif

/* Keep this ABI strictly i32. Emscripten's C long can be i64, while the
 * browser UI deliberately passes ordinary JavaScript Numbers. */
WEB_EXPORT int apesdk_start(uint32_t seed)
{
    /* NUM_CONTROL is the processing view: it creates the simulation. */
    return (int)shared_init(WINDOW_PROCESSING, (n_uint)seed);
}
WEB_EXPORT void apesdk_cycle(uint32_t ticks)
{
    (void)shared_cycle((n_uint)ticks, WINDOW_PROCESSING);
}
WEB_EXPORT int apesdk_draw(int view, int width, int height)
{
    return (int)(intptr_t)shared_draw((n_int)view, (n_int)width, (n_int)height, 0);
}
WEB_EXPORT void apesdk_mouse(int view, int x, int y, int option)
{
    shared_mouseOption((n_byte)option);
    shared_mouseReceived((n_double)x, (n_double)y, view);
}
WEB_EXPORT void apesdk_mouse_up(void) { shared_mouseUp(); }
WEB_EXPORT void apesdk_key(int view, int key) { shared_keyReceived((n_int)key, (n_int)view); }
WEB_EXPORT void apesdk_key_up(void) { shared_keyUp(); }
WEB_EXPORT void apesdk_menu(int menu) { (void)shared_menu((n_int)menu); }
WEB_EXPORT int apesdk_population(void)
{
    simulated_group *group = sim_group();
    return group ? (int)group->num : 0;
}
WEB_EXPORT int apesdk_selected_x(void)
{
    simulated_group *group = sim_group();
    return (group && group->select) ? (int)being_location_x(group->select) : -1;
}
WEB_EXPORT int apesdk_selected_y(void)
{
    simulated_group *group = sim_group();
    return (group && group->select) ? (int)being_location_y(group->select) : -1;
}
WEB_EXPORT void apesdk_stop(void) { shared_close(); }
