#include "bronze_apesdk_adapter.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{ BronzeApeSidecar sidecar; bronze_ape_sidecar_init(&sidecar); assert(bronze_ape_sidecar_add(&sidecar,BRONZE_APE_GRAIN,3)==3); assert(bronze_ape_sidecar_add(&sidecar,BRONZE_APE_GRAIN,-9)==0); assert(bronze_ape_day_for_cycle(0)==0); assert(bronze_ape_day_for_cycle(1439)==0); assert(bronze_ape_day_for_cycle(1440)==1); puts("Bronze ApeSDK adapter tests passed."); return 0; }
