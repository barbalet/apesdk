
#ifndef BRZ_AGENT_H
#define BRZ_AGENT_H

#include "brz_types.h"
#include "brz_dsl.h"
#include "brz_world.h"
#include "brz_settlement.h"
#include "brz_util.h"
#include "brz_port.h"
#include <stddef.h>

typedef struct BrzAgent {
    uint32_t id;
    const VocationDef* voc;
    BrzPos pos;
    BrzPos target;
    int has_target;
    int home_settlement;
    double hunger;
    double fatigue;
    double* res_inv;   /* [res_n] */
    double* item_inv;  /* [item_n] */
    size_t res_n;
    size_t item_n;
} BrzAgent;

int  brz_agents_alloc_and_spawn(BrzAgent** out, int agent_n, const ParsedConfig* cfg,
                                const BrzSettlement* setts, int sett_n,
                                size_t res_n, size_t item_n, unsigned seed);
void brz_agents_free(BrzAgent* agents, int agent_n);

void brz_agent_step(BrzAgent* a, const ParsedConfig* cfg, BrzWorld* world,
                    BrzSettlement* setts, int sett_n, BrzRng* rng,
                    const BronzeEventSink* events, int day);

#endif
