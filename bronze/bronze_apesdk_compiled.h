#ifndef BRONZE_APESDK_COMPILED_H
#define BRONZE_APESDK_COMPILED_H

#include "bronze_apesdk_adapter.h"
#include "dsl/brz_dsl.h"

/* Executes top-level compiled task operations through the ApeSDK adapter. */
int bronze_ape_execute_compiled_task(const ParsedConfig* config,
                                     const char* vocation_name,
                                     const char* task_name,
                                     const ScenarioWorldPort* world,
                                     const ScenarioActorPort* actor,
                                     BronzeApeSidecar* sidecar,
                                     BronzeApeSettlement* settlement,
                                     ScenarioEvent* events, int event_capacity);

#endif
