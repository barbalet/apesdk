#ifndef BRONZE_TASKS_H
#define BRONZE_TASKS_H
#include "../actors/bronze_actors.h"

const bronze_site *bronze_task_work_site(uint64_t id);
const bronze_site *bronze_task_move_site(uint64_t home, uint64_t workplace, const char *terrain);
void bronze_task_release_claim(BronzeLandscape *landscape, BronzeActor *actor);
uint64_t bronze_task_storage(uint64_t home);
BronzeTaskStatus bronze_task_gather(BronzeActor *actor, simulated_being *being,
                                   uint64_t target, int kind, double quantity, int commit);
BronzeTaskStatus bronze_task_deposit(BronzeActor *actor, simulated_being *being,
                                    uint64_t target, int kind, double quantity, int commit);
BronzeTaskStatus bronze_task_deposit_item(BronzeActor *actor, simulated_being *being,
                                         uint64_t target, int kind, double quantity, int commit);
BronzeTaskStatus bronze_task_craft(BronzeActor *actor, simulated_being *being, uint64_t target,
                                  const RecipeDef *recipe, const BronzeRecipePolicy *policy,
                                  unsigned batches, int commit);
BronzeTaskStatus bronze_task_store_transfer(BronzeActor *actor, simulated_being *being,
                                            uint64_t target, int kind, double quantity,
                                            int item, int household, int fetch, int commit);
void bronze_tasks_begin(simulated_group *group);
void bronze_tasks_steer(simulated_being *being);
void bronze_tasks_end(simulated_group *group);
const char *bronze_task_status_name(BronzeTaskStatus status);
#endif
