#include "er/runtime.h"
#include <stdio.h>
int main(void) {
    er_limits_t lim={{-3,-3},{3,3},{2,2}}; er_runtime_t rt;
    er_runtime_init(&rt,&lim);
    er_command_t cmd={1000,1,{1.0f,-0.5f},{1.0f,1.0f}};
    er_sensor_t sen={1000,{0,0},{0,0},true}; er_output_t out;
    if (er_runtime_submit_command(&rt,&cmd)!=ER_OK || er_runtime_arm(&rt)!=ER_OK) return 1;
    if (er_runtime_step(&rt,1000,&sen,&out)!=ER_OK) return 1;
    printf("enabled=%d target=(%.2f, %.2f) velocity=(%.2f, %.2f) faults=0x%x\n",
      out.enabled,out.target[0],out.target[1],out.velocity[0],out.velocity[1],out.faults);
    return 0;
}
