#include "control_task.h"
#include "er/runtime.h"
#include "er/devices.h"
#include "actuator.h"
#include "sim.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

int er_control_task_run(const er_osal_clock_t *clock, const char *scenario)
{
    if (!clock || !scenario) return 2;
    bool stale=!strcmp(scenario,"stale"), sensor_fault=!strcmp(scenario,"sensor");
    if (!stale && !sensor_fault && strcmp(scenario,"normal")) return 2;
    er_periodic_t tick; er_periodic_report_t report;
    if (er_periodic_init(&tick,clock,10000)!=ER_OSAL_OK) return 3;
    er_sim_t sim; er_runtime_t rt; er_sim_actuator_t drv[ER_JOINTS];
    er_hal_actuator_t axes[ER_JOINTS]; er_devices_t devices;
    er_limits_t limits={{-3,-3},{3,3},{2,2}}; er_command_t cmd={0,1,{1,-.5f},{1,1}};
    er_sensor_t sensor; er_output_t output; uint64_t max_late=0, missed=0;
    er_sim_init(&sim); er_runtime_init(&rt,&limits);
    for (unsigned i=0;i<ER_JOINTS;i++)
        if (er_sim_actuator_init(&drv[i],&sim,i,true,&axes[i])!=ER_HAL_OK) return 4;
    if (er_devices_bind(&devices,axes)!=ER_HAL_OK) return 4;
    if (er_runtime_submit_command(&rt,&cmd)!=ER_OK || er_runtime_arm(&rt)!=ER_OK) return 4;
    printf("control scenario=%s period_us=10000 steps=40\n",scenario);
    for (unsigned n=0;n<40;n++) {
        if (er_periodic_wait(&tick,&report)!=ER_OSAL_OK) return 5;
        if (report.lateness_us>max_late) max_late=report.lateness_us;
        missed+=report.missed_periods;
        if (n && !stale && !rt.faults) { cmd.timestamp_us=sim.now_us; cmd.sequence++; if(er_runtime_submit_command(&rt,&cmd)!=ER_OK)return 6; }
        if(sensor_fault && n==10) er_sim_set_sensor_valid(&sim,false);
        if(er_devices_read(&devices,sim.now_us,&sensor)!=ER_HAL_OK) sensor.valid=false;
        er_status_t rc=er_runtime_step(&rt,sim.now_us,&sensor,&output);
        if(rc!=ER_OK && rc!=ER_EFAULT)return 7;
        if((stale && n>=11) || (sensor_fault && n>=10)) {
            uint32_t expected=stale?ER_FAULT_CMD_STALE:ER_FAULT_SENSOR_INVALID;
            if(output.enabled || output.faults!=expected)return 8;
        } else if(!output.enabled || output.faults)return 9;
        if(er_devices_apply(&devices,&output)!=ER_HAL_OK && output.enabled)return 10;
        if(er_sim_actuators_advance(drv,ER_JOINTS,10000)!=ER_HAL_OK)return 11;
    }
    printf("result=pass max_lateness_us=%llu missed_periods=%llu faults=0x%x\n",
      (unsigned long long)max_late,(unsigned long long)missed,rt.faults);
    return 0;
}
