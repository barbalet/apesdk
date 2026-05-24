#ifndef SIMULATED_UNIVERSE_BRIDGE_H
#define SIMULATED_UNIVERSE_BRIDGE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sua_context sua_context;

typedef enum
{
    SUA_ERROR_NONE = 0,
    SUA_ERROR_INITIALIZATION_FAILED = 1,
    SUA_ERROR_SIMULATION_UNAVAILABLE = 2,
    SUA_ERROR_INVALID_SELECTION = 3,
    SUA_ERROR_INVALID_TERRAIN_SAMPLE = 4,
    SUA_ERROR_TRANSFER_FAILED = 5,
    SUA_ERROR_SCRIPT_FAILED = 6,
    SUA_ERROR_UNSUPPORTED_CAPABILITY = 7,
    SUA_ERROR_SNAPSHOT_CONTRACT_MISMATCH = 8,
    SUA_ERROR_COMMAND_REJECTED = 9,
    SUA_ERROR_MEMORY_OWNERSHIP_VIOLATION = 10
} sua_error_code;

typedef enum
{
    SUA_CAPABILITY_APESCRIPT = 1 << 0,
    SUA_CAPABILITY_BRAIN = 1 << 1,
    SUA_CAPABILITY_BRAINCODE = 1 << 2,
    SUA_CAPABILITY_IMMUNE = 1 << 3,
    SUA_CAPABILITY_TERRITORY = 1 << 4,
    SUA_CAPABILITY_ALPHA_WEATHER_DRAW = 1 << 5,
    SUA_CAPABILITY_HIGH_RESOLUTION_TERRAIN = 1 << 6,
    SUA_CAPABILITY_TRANSFER_BINARY = 1 << 7,
    SUA_CAPABILITY_TRANSFER_JSON = 1 << 8
} sua_capability_flags;

typedef struct
{
    int32_t snapshot_contract_version;
    int32_t apesdk_version_number;
    uint32_t capability_flags;
} sua_version_info;

typedef struct
{
    int32_t transient_index;
    uint16_t gender_name;
    uint16_t family_name;
    int32_t date_of_birth;
} sua_being_identifier;

sua_error_code sua_context_create(uint32_t seed, sua_context **out_context);
void sua_context_destroy(sua_context **context);

sua_error_code sua_context_step(sua_context *context, uint32_t cycles);
sua_error_code sua_context_version(sua_context *context, sua_version_info *out_version);

sua_error_code sua_snapshot_universe_json(sua_context *context, char **out_json, size_t *out_size);
sua_error_code sua_snapshot_being_json(sua_context *context, sua_being_identifier identifier, char **out_json, size_t *out_size);
sua_error_code sua_snapshot_terrain_json(sua_context *context, int32_t x, int32_t y, char **out_json, size_t *out_size);
sua_error_code sua_snapshot_memory_json(sua_context *context, sua_being_identifier identifier, char **out_json, size_t *out_size);
sua_error_code sua_snapshot_social_json(sua_context *context, sua_being_identifier identifier, char **out_json, size_t *out_size);

void sua_bridge_free(void **pointer);

#ifdef __cplusplus
}
#endif

#endif
