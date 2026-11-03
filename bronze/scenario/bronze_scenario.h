#ifndef BRONZE_SCENARIO_H
#define BRONZE_SCENARIO_H
#include <stddef.h>
#include <stdint.h>
typedef struct BronzeScenario BronzeScenario;
BronzeScenario* bronze_scenario_create(void);
void bronze_scenario_destroy(BronzeScenario* scenario);
/* Transactional: failure preserves the previous active compiled configuration.
   The production caller supplies Bundle.main URLs; no implicit cwd search. */
int bronze_scenario_load(BronzeScenario* scenario, const char* path, const char* display_name);
const char* bronze_scenario_name(const BronzeScenario* scenario);
const char* bronze_scenario_diagnostics(const BronzeScenario* scenario);
uint32_t bronze_scenario_seed(const BronzeScenario* scenario);
size_t bronze_scenario_vocations(const BronzeScenario* scenario);
#endif
