/****************************************************************
 profession.h

 Civilian livelihood extension for ApeSDK's being simulation.
****************************************************************/
#ifndef SIMULATEDAPE_PROFESSION_H
#define SIMULATEDAPE_PROFESSION_H

#include "entity.h"

typedef enum
{
    PROFESSION_NONE = 0,
    PROFESSION_BAKER,
    PROFESSION_GROCER,
    PROFESSION_TAILOR,
    PROFESSION_MASON,
    PROFESSION_CARPENTER,
    PROFESSION_ELECTRICIAN,
    PROFESSION_PLUMBER,
    PROFESSION_MECHANIC,
    PROFESSION_DOCTOR,
    PROFESSION_NURSE,
    PROFESSION_TEACHER,
    PROFESSION_WATER_TECHNICIAN,
    PROFESSION_IMAM,
    PROFESSION_KHATIB,
    PROFESSION_COUNT
} simulated_profession_type;

/* This is deliberately separate from simulated_being so existing ApeSDK
   serialized populations remain binary compatible.  A host simulation owns
   one record per being and may persist it in its own schema. */
typedef struct
{
    simulated_profession_type type;
    n_byte2 skill;
    n_byte2 workplace;
    n_byte2 mentor;
} simulated_profession;

n_byte profession_is_service( simulated_profession_type type );
void profession_interaction( simulated_being *provider, simulated_being *recipient,
                             n_byte2 resource );
void profession_training( simulated_being *mentor, simulated_being *apprentice,
                          n_byte2 skill_gain );

#endif
