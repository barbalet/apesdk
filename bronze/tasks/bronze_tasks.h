#ifndef BRONZE_TASKS_H
#define BRONZE_TASKS_H
#include "../actors/bronze_actors.h"

const bronze_site *bronze_task_work_site(uint64_t id);
const bronze_site *bronze_task_move_site(uint64_t home, uint64_t workplace, const char *terrain);
BronzeTaskStatus bronze_task_gather(uint64_t home, uint64_t target, const char *resource,
                                   double quantity, int commit);
void bronze_tasks_begin(simulated_group *group);
void bronze_tasks_end(simulated_group *group);
const char *bronze_task_status_name(BronzeTaskStatus status);
#endif
