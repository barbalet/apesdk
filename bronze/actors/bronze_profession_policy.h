#ifndef BRONZE_PROFESSION_POLICY_H
#define BRONZE_PROFESSION_POLICY_H

enum { BRONZE_VOCATION_LIMIT = 64, BRONZE_VOCATION_NAME_LIMIT = 64 };

typedef enum {
    BRONZE_JOB_UNSPECIFIED, BRONZE_JOB_FIELD, BRONZE_JOB_WOODLAND,
    BRONZE_JOB_COPPER, BRONZE_JOB_TIN, BRONZE_JOB_SERVICE, BRONZE_JOB_COAST
} BronzeJobPlace;

typedef struct {
    char name[BRONZE_VOCATION_NAME_LIMIT];
    unsigned weight;
    BronzeJobPlace workplace;
} BronzeVocation;

typedef struct {
    unsigned count, min_age_days, site_capacity, retry_minutes;
    unsigned skill_start, skill_max, skill_gain, mentor_skill;
    BronzeVocation vocations[BRONZE_VOCATION_LIMIT];
} BronzeProfessionPolicy;

#endif
