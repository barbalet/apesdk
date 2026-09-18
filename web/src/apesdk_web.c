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

WEB_EXPORT n_int apesdk_start(n_uint seed)
{
    return shared_init(NUM_VIEW, seed);
}
WEB_EXPORT void apesdk_cycle(n_uint ticks)
{
    (void)shared_cycle(ticks, NUM_VIEW);
}
WEB_EXPORT n_int apesdk_draw(n_int view, n_int width, n_int height)
{
    return (n_int)(intptr_t)shared_draw(view, width, height, 0);
}
WEB_EXPORT void apesdk_mouse(n_int view, n_int x, n_int y, n_int option)
{
    shared_mouseOption((n_byte)option);
    shared_mouseReceived((n_double)x, (n_double)y, view);
}
WEB_EXPORT void apesdk_mouse_up(void) { shared_mouseUp(); }
WEB_EXPORT void apesdk_key(n_int view, n_int key) { shared_keyReceived(key, view); }
WEB_EXPORT void apesdk_key_up(void) { shared_keyUp(); }
WEB_EXPORT void apesdk_menu(n_int menu) { (void)shared_menu(menu); }
WEB_EXPORT n_int apesdk_population(void)
{
    simulated_group *group = sim_group();
    return group ? group->num : 0;
}
WEB_EXPORT n_int apesdk_selected_x(void)
{
    simulated_group *group = sim_group();
    return (group && group->select) ? being_location_x(group->select) : -1;
}
WEB_EXPORT n_int apesdk_selected_y(void)
{
    simulated_group *group = sim_group();
    return (group && group->select) ? being_location_y(group->select) : -1;
}
WEB_EXPORT void apesdk_stop(void) { shared_close(); }
