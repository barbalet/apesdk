#include "bronze_apesdk_compiled.h"

#include <string.h>

static int resource_id(const char* name)
{
    static const char* names[]={"grain","fish","wood","copper","tin","charcoal"};
    int i;
    for(i=0;i<BRONZE_APE_RESOURCE_COUNT;i++) if(name && strcmp(name,names[i])==0) return i;
    return -1;
}

static BronzeApeAction action_for(const OpDef* op)
{
    if(op->action_code==BRZ_ACTION_GATHER) return BRONZE_APE_GATHER;
    if(op->action_code==BRZ_ACTION_CRAFT) return BRONZE_APE_CRAFT;
    if(op->action_code==BRZ_ACTION_DEPOSIT) return BRONZE_APE_DEPOSIT;
    if(op->action_code==BRZ_ACTION_EAT) return BRONZE_APE_EAT;
    if(op->action_code==BRZ_ACTION_REST) return BRONZE_APE_REST;
    if(op->action_code==BRZ_ACTION_MOVE_TO || op->action_code==BRZ_ACTION_ROAM ||
       op->action_code==BRZ_ACTION_WANDER) return BRONZE_APE_MOVE;
    return (BronzeApeAction)-1;
}

int bronze_ape_execute_compiled_task(const ParsedConfig* config,
                                     const char* vocation_name,
                                     const char* task_name,
                                     const ScenarioWorldPort* world,
                                     const ScenarioActorPort* actor,
                                     BronzeApeSidecar* sidecar,
                                     BronzeApeSettlement* settlement,
                                     ScenarioEvent* events, int event_capacity)
{
    VocationDef* vocation;
    TaskDef* task;
    int count=0;
    size_t i;
    if(!config || !vocation_name || !task_name || !events || event_capacity<1) return -1;
    vocation=NULL;
    for(i=0;i<config->vocations.len;i++) {
        VocationDef* candidate=(VocationDef*)brz_vec_at((BrzVec*)&config->vocations,i);
        if(candidate && candidate->name && strcmp(candidate->name,vocation_name)==0) { vocation=candidate; break; }
    }
    task=vocation ? brz_voc_find_task(vocation,task_name) : NULL;
    if(!task) return -1;
    for(i=0;i<task->stmts.len;i++) {
        StmtDef* statement=(StmtDef*)brz_vec_at(&task->stmts,i);
        BronzeApeAction action;
        int resource;
        double amount;
        if(!statement || statement->kind!=ST_OP) return -1;
        action=action_for(&statement->as.op);
        if(action<BRONZE_APE_MOVE || action>BRONZE_APE_REST || count>=event_capacity) return -1;
        resource=resource_id(statement->as.op.a0);
        amount=statement->as.op.has_n0 ? statement->as.op.n0 : 1.0;
        bronze_ape_action_run(world,actor,sidecar,settlement,action,resource,amount,&events[count++]);
    }
    return count;
}
