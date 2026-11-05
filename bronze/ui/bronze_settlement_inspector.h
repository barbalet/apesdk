#ifndef BRONZE_SETTLEMENT_INSPECTOR_H
#define BRONZE_SETTLEMENT_INSPECTOR_H
/* Copied values, never pointers into a different session's registry. */
typedef struct {
    uint64_t township_id, building_id;
    unsigned members, housing, requested_housing, assigned_workers, capacity;
    unsigned connected, missing_connections;
    char building[64], occupation[64], access[128];
} BronzeSettlementInspection;
#endif
