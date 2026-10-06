#include "bronze_apesdk_adapter.h"

static n_int bronze_terrain_height(n_int x, n_int y)
{
    return land_location(x, y);
}

static n_byte bronze_is_water(n_int x, n_int y)
{
    return (n_byte)(land_location(x, y) < WATER_MAP);
}

void bronze_ape_actor_init(bronze_ape_actor *actor, simulated_being *being)
{
    if(actor) actor->being=being;
}

bronze_ape_position bronze_ape_actor_position(const bronze_ape_actor *actor)
{
    bronze_ape_position position={0,0};
    if(actor && actor->being){
        position.x=being_location_x(actor->being);
        position.y=being_location_y(actor->being);
    }
    return position;
}

void bronze_ape_actor_set_position(bronze_ape_actor *actor, bronze_ape_position position)
{
    n_byte2 location[2];
    if(!actor || !actor->being) return;
    location[0]=(n_byte2)position.x; location[1]=(n_byte2)position.y;
    being_set_location(actor->being,location);
}

n_int bronze_ape_actor_energy(const bronze_ape_actor *actor)
{
    return (actor && actor->being) ? being_energy(actor->being) : 0;
}

n_byte bronze_ape_actor_fatigue(const bronze_ape_actor *actor)
{
    return (actor && actor->being) ? being_drive(actor->being,DRIVE_FATIGUE) : 0;
}

void bronze_ape_world_init(bronze_ape_world *world)
{
    if(!world) return;
    world->terrain_height=bronze_terrain_height;
    world->is_water=bronze_is_water;
    world->date=land_date;
    world->time=land_time;
}
