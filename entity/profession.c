/****************************************************************
 profession.c
****************************************************************/
#include "profession.h"
#include "entity_internal.h"

n_byte profession_is_service( simulated_profession_type type )
{
    return ( type == PROFESSION_DOCTOR || type == PROFESSION_NURSE ||
             type == PROFESSION_TEACHER || type == PROFESSION_WATER_TECHNICIAN ||
             type == PROFESSION_IMAM || type == PROFESSION_KHATIB );
}

void profession_interaction( simulated_being *provider, simulated_being *recipient,
                             n_byte2 resource )
{
    if ( provider == 0L || recipient == 0L ) return;
    episodic_interaction( provider, recipient, EVENT_PROFESSION_SERVICE,
                          EPISODIC_AFFECT_ZERO, resource );
    episodic_interaction( recipient, provider, EVENT_PROFESSION_SERVICE_BY,
                          AFFECT_RECEIVE, resource );
}

void profession_training( simulated_being *mentor, simulated_being *apprentice,
                          n_byte2 skill_gain )
{
    if ( mentor == 0L || apprentice == 0L ) return;
    episodic_interaction( mentor, apprentice, EVENT_PROFESSION_TRAIN,
                          AFFECT_CHAT, skill_gain );
    episodic_interaction( apprentice, mentor, EVENT_PROFESSION_TRAINED,
                          AFFECT_CHAT, skill_gain );
}
