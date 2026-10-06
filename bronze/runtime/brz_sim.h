#ifndef BRZ_SIM_H
#define BRZ_SIM_H

#include "brz_dsl.h"
#include "brz_port.h"

/* Simulation runner.
   Executes vocations/rules/tasks over a number of cycles and prints
   interactions and key values over time. */
int brz_run(const ParsedConfig* cfg);
int brz_run_with_events(const ParsedConfig* cfg, const BronzeEventSink* events);

#endif /* BRZ_SIM_H */
