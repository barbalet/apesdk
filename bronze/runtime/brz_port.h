#ifndef BRZ_PORT_H
#define BRZ_PORT_H

/* Stable Bronze domain ports.  These are intentionally narrower than either
   BrzWorld or apeSDK's simulated_being so an apeSDK adapter can live outside
   its generic engine directories. */
#include "brz_types.h"
#include <stdint.h>
#include <stddef.h>

typedef struct BronzeWorldPort BronzeWorldPort;
typedef struct BronzeActorPort BronzeActorPort;
typedef struct BronzeSettlementPort BronzeSettlementPort;

typedef enum {
    BRZ_EVENT_GATHERED, BRZ_EVENT_CRAFTED, BRZ_EVENT_TRADED,
    BRZ_EVENT_RESTED, BRZ_EVENT_OCCUPATION_SELECTED, BRZ_EVENT_DEPOSITED
} BronzeEventKind;

typedef struct {
    BronzeEventKind kind;
    uint32_t actor_id;
    int settlement_id;
    const char* subject;
    double amount;
    int day;
} BronzeEvent;

typedef void (*BronzeEventFn)(void* context, const BronzeEvent* event);
typedef struct { void* context; BronzeEventFn emit; } BronzeEventSink;

void bronze_world_port_init(BronzeWorldPort* port, void* world, size_t resource_count);
void bronze_actor_port_init(BronzeActorPort* port, void* agent);
void bronze_settlement_port_init(BronzeSettlementPort* port, const void* settlements, int count);
void bronze_world_port_destroy(BronzeWorldPort* port);
void bronze_settlement_port_destroy(BronzeSettlementPort* port);

struct BronzeWorldPort {
    void* context;
    uint16_t (*tags_at)(void* context, BrzPos pos);
    double (*take)(void* context, BrzPos pos, int resource_id, double amount);
    BrzPos (*nearest_tag)(void* context, BrzPos from, uint16_t tag, int max_radius);
    void (*step_regen)(void* context);
};

struct BronzeActorPort {
    void* context;
    uint32_t (*id)(void* context);
    BrzPos (*position)(void* context);
    void (*set_position)(void* context, BrzPos position);
    double (*need)(void* context, const char* name);
};

struct BronzeSettlementPort {
    void* context;
    int (*nearest)(void* context, BrzPos position);
};

void bronze_event_emit(const BronzeEventSink* sink, BronzeEventKind kind,
                       uint32_t actor_id, int settlement_id, const char* subject,
                       double amount, int day);

#endif
