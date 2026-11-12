#include "bridge.h"
#include "er/runtime.h"
#include "sim.h"
#include "actuator.h"
#include "er/devices.h"
#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define ER_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define ER_EXPORT
#endif
static er_runtime_t rt; static er_sim_t sim; static er_output_t out;
static er_devices_t devices;
static er_sim_actuator_t drivers[ER_JOINTS];
static er_hal_actuator_t axes[ER_JOINTS];
static uint32_t seq; static int ready; static char json[512];
ER_EXPORT void er_web_reset(void){ er_limits_t l={{-3,-3},{3,3},{2,2}}; memset(&out,0,sizeof out); er_sim_init(&sim); er_runtime_init(&rt,&l); for(unsigned i=0;i<ER_JOINTS;i++) (void)er_sim_actuator_init(&drivers[i],&sim,i,true,&axes[i]); (void)er_devices_bind(&devices,axes); seq=0; ready=0; json[0]=0; }
ER_EXPORT int er_web_command(float a,float b,float speed){ er_command_t c; if(!ready) ready=1; c.timestamp_us=sim.now_us;c.sequence=++seq;c.position[0]=a;c.position[1]=b;c.velocity_limit[0]=speed;c.velocity_limit[1]=speed; if(er_runtime_submit_command(&rt,&c)!=ER_OK){memset(&out,0,sizeof out);out.faults=er_runtime_faults(&rt);return -1;} return er_runtime_arm(&rt)==ER_OK?0:-1; }
ER_EXPORT int er_web_step(unsigned dt){ er_sensor_t s; if(!ready)return -1; (void)er_devices_read(&devices,sim.now_us,&s); int rc=er_runtime_step(&rt,sim.now_us,&s,&out); if(er_devices_apply(&devices,&out)!=ER_HAL_OK && out.enabled){memset(&out,0,sizeof out);er_runtime_disarm(&rt);rc=ER_EFAULT;} if(er_sim_actuators_advance(drivers,ER_JOINTS,dt)!=ER_HAL_OK)return -1; return rc; }
ER_EXPORT void er_web_set_fault(int invalid,int stall0,int stall1){er_sim_set_sensor_valid(&sim,!invalid);er_sim_set_stalled(&sim,0,stall0!=0);er_sim_set_stalled(&sim,1,stall1!=0);}
ER_EXPORT void er_web_stop(void){er_runtime_disarm(&rt);(void)er_devices_disable(&devices);memset(&out,0,sizeof out);}
ER_EXPORT const char *er_web_state(void){(void)snprintf(json,sizeof json,"{\"time_us\":%llu,\"position\":[%.5g,%.5g],\"target\":[%.5g,%.5g],\"velocity\":[%.5g,%.5g],\"enabled\":%d,\"faults\":%u}",(unsigned long long)sim.now_us,sim.position[0],sim.position[1],out.target[0],out.target[1],out.velocity[0],out.velocity[1],out.enabled?1:0,out.faults);return json;}
