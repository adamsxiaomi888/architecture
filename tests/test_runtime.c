#include "er/runtime.h"
#include <assert.h>
#include <stdio.h>
int main(void){er_runtime_t r;er_limits_t l={{-3,-3},{3,3},{2,2}};er_runtime_init(&r,&l);assert(er_runtime_arm(&r)==ER_OK);er_command_t c={1000,1,{1,1},{1,1}};assert(er_runtime_submit_command(&r,&c)==ER_OK);er_sensor_t s={1000,{0,0},{0,0},true};er_output_t o;assert(er_runtime_step(&r,1000,&s,&o)==ER_OK&&o.enabled);assert(er_runtime_step(&r,101001,&s,&o)==ER_EFAULT);assert(er_runtime_faults(&r)!=0);assert(er_runtime_reset_faults(&r)==ER_OK);c.sequence=2;c.timestamp_us=102000;assert(er_runtime_submit_command(&r,&c)==ER_OK);assert(er_runtime_arm(&r)==ER_OK);s.valid=false;assert(er_runtime_step(&r,102000,&s,&o)==ER_EFAULT);printf("runtime tests passed\n");}
