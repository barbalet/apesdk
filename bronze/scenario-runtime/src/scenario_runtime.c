#include "scenario_runtime.h"

#include <string.h>

void scenario_event_init(ScenarioEvent* event, ScenarioTick tick,
                         ScenarioActorId actor_id, ScenarioActorId counterpart_id,
                         ScenarioPlaceId place_id, const char* action_id,
                         double requested_amount, double completed_amount,
                         ScenarioResult result)
{
    if(!event) return;
    event->version=SCENARIO_RUNTIME_VERSION;
    event->tick=tick;
    event->actor_id=actor_id;
    event->counterpart_id=counterpart_id;
    event->place_id=place_id;
    event->action_id=action_id;
    event->requested_amount=requested_amount;
    event->completed_amount=completed_amount;
    event->result=result;
}

int scenario_event_equivalent(const ScenarioEvent* left, const ScenarioEvent* right)
{
    if(!left || !right || left->version!=SCENARIO_RUNTIME_VERSION ||
       right->version!=SCENARIO_RUNTIME_VERSION) return 0;
    return left->tick==right->tick && left->actor_id==right->actor_id &&
           left->counterpart_id==right->counterpart_id && left->place_id==right->place_id &&
           left->requested_amount==right->requested_amount &&
           left->completed_amount==right->completed_amount && left->result==right->result &&
           ((!left->action_id && !right->action_id) ||
            (left->action_id && right->action_id && strcmp(left->action_id,right->action_id)==0));
}

int scenario_service_fixture_run(const ScenarioWorldPort* world,
                                 const ScenarioActorPort* recipient,
                                 const ScenarioPlacePort* place,
                                 ScenarioActorId provider_id,
                                 const char* service_action,
                                 double requested_amount,
                                 ScenarioResult service_result,
                                 ScenarioServiceFixture* fixture)
{
    ScenarioTick tick;
    ScenarioActorId recipient_id;
    ScenarioPlaceId place_id;
    ScenarioPosition place_position;
    double delivered;
    if(!world || !recipient || !place || !fixture || !world->tick || !recipient->id ||
       !place->id || !place->position || !service_action || provider_id==SCENARIO_ID_NONE ||
       requested_amount<0) return 0;
    tick=world->tick(world->context);
    recipient_id=recipient->id(recipient->context);
    place_id=place->id(place->context);
    if(recipient_id==SCENARIO_ID_NONE || place_id==SCENARIO_ID_NONE) return 0;
    place_position=place->position(place->context);
    delivered=service_result==SCENARIO_RESULT_COMPLETED ? requested_amount : 0.0;
    scenario_event_init(&fixture->events[0],tick,recipient_id,SCENARIO_ID_NONE,place_id,
                        "travel",1.0,1.0,SCENARIO_RESULT_COMPLETED);
    scenario_event_init(&fixture->events[1],tick+1,provider_id,recipient_id,place_id,
                        service_action,requested_amount,delivered,service_result);
    fixture->recipient_position=place_position;
    fixture->recipient_place_id=place_id;
    fixture->recipient_arrived=1;
    fixture->delivered_amount=delivered;
    return 1;
}

int scenario_service_fixture_equivalent(const ScenarioServiceFixture* left,
                                        const ScenarioServiceFixture* right)
{
    if(!left || !right) return 0;
    return scenario_event_equivalent(&left->events[0],&right->events[0]) &&
           scenario_event_equivalent(&left->events[1],&right->events[1]) &&
           left->recipient_position.x==right->recipient_position.x &&
           left->recipient_position.y==right->recipient_position.y &&
           left->recipient_place_id==right->recipient_place_id &&
           left->recipient_arrived==right->recipient_arrived &&
           left->delivered_amount==right->delivered_amount;
}
