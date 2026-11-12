#include "actuator.h"
#include "er/devices.h"
#include <math.h>
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main(void)
{
    er_sim_t sim;
    er_sim_actuator_t context[ER_JOINTS];
    er_hal_actuator_t axes[ER_JOINTS];
    er_devices_t binding = {0};
    er_sim_init(&sim);
    CHECK(er_sim_actuator_init(&context[0], &sim, 0, true, &axes[0]) == ER_HAL_OK);
    CHECK(er_sim_actuator_init(&context[1], &sim, 1, false, &axes[1]) == ER_HAL_OK);
    CHECK(er_devices_bind(&binding, axes) == ER_HAL_ENOTSUP);
    float p = 42, v = 43, effort = 44;
    uint64_t stamp = 45;
    CHECK(axes[1].read_feedback(axes[1].ctx,&p,&v,&effort,&stamp) == ER_HAL_ENOTSUP);
    CHECK(p == 42 && v == 43 && effort == 44 && stamp == 45);
    CHECK(axes[1].set_velocity(axes[1].ctx,1) == ER_HAL_ENOTSUP);
    CHECK(axes[1].set_position(axes[1].ctx,-1,1) == ER_HAL_OK);
    CHECK(axes[0].set_velocity(axes[0].ctx,1) == ER_HAL_OK);
    CHECK(axes[0].enable(axes[0].ctx,true) == ER_HAL_OK);
    CHECK(axes[1].enable(axes[1].ctx,true) == ER_HAL_OK);
    CHECK(er_sim_actuators_advance(context,ER_JOINTS,10000) == ER_HAL_OK);
    CHECK(fabsf(sim.position[0]-.01f)<1e-6f && fabsf(sim.position[1]+.01f)<1e-6f);
    CHECK(axes[0].read_feedback(axes[0].ctx,&p,&v,&effort,&stamp)==ER_HAL_OK);
    CHECK(p==sim.position[0] && v==1 && stamp==10000 && isnan(effort));
    CHECK(axes[0].enable(axes[0].ctx,false)==ER_HAL_OK);
    CHECK(er_sim_actuators_advance(context,ER_JOINTS,10000)==ER_HAL_OK);
    CHECK(sim.position[0]==p && sim.velocity[0]==0 && sim.position[1]<-.01f);
    sim.sensor_valid=false;
    CHECK(axes[0].read_feedback(axes[0].ctx,&p,&v,&effort,&stamp)==ER_HAL_EIO);
    CHECK(axes[0].set_position(axes[0].ctx,NAN,1)==ER_HAL_EINVAL);
    CHECK(axes[0].set_position(axes[0].ctx,0,-1)==ER_HAL_EINVAL);
    CHECK(axes[0].set_velocity(axes[0].ctx,INFINITY)==ER_HAL_EINVAL);
    CHECK(er_sim_actuators_advance(context,ER_JOINTS,0)==ER_HAL_EINVAL);
    CHECK(er_sim_actuator_init(&context[1],&sim,ER_JOINTS,true,&axes[1])==ER_HAL_EINVAL);
    CHECK(er_sim_actuator_init(&context[1],&sim,1,true,&axes[1])==ER_HAL_OK);
    CHECK(er_devices_bind(&binding,axes)==ER_HAL_OK);
    puts("sim actuator capability/feedback/disable tests passed");
    return 0;
}
