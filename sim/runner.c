#include "sim.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Fixed scenarios intentionally have no wall-clock or random dependencies. */
int main(int argc, char **argv)
{
    const char *name = argc == 2 ? argv[1] : "normal";
    bool stale = strcmp(name, "stale") == 0;
    bool sensor_fault = strcmp(name, "sensor") == 0;
    bool stall = strcmp(name, "stall") == 0;
    if (argc > 2 || (!stale && !sensor_fault && !stall && strcmp(name, "normal"))) {
        fprintf(stderr, "usage: er_sim_runner [normal|stale|sensor|stall]\n");
        return 2;
    }
    er_sim_t sim;
    er_runtime_t rt;
    er_limits_t lim = {{-3,-3},{3,3},{2,2}};
    er_command_t cmd = {0,1,{1.0f,-0.5f},{1,1}};
    er_sensor_t sensor;
    er_output_t out;
    er_sim_init(&sim);
    er_runtime_init(&rt, &lim);
    er_sim_set_stalled(&sim, 0, stall);
    if (er_runtime_submit_command(&rt, &cmd) != ER_OK || er_runtime_arm(&rt) != ER_OK) return 1;
    printf("{\"schema\":1,\"scenario\":\"%s\",\"dt_us\":10000,\"steps\":300}\n", name);
    for (unsigned n = 0; n < 300; ++n) {
        bool submitted = false;
        if (n && !stale && !rt.faults) {
            cmd.timestamp_us = sim.now_us;
            ++cmd.sequence;
            if (er_runtime_submit_command(&rt, &cmd) != ER_OK) return 1;
            submitted = true;
        }
        if (sensor_fault && n == 20) er_sim_set_sensor_valid(&sim, false);
        if (sensor_fault && n == 21) er_sim_set_sensor_valid(&sim, true);
        er_sim_sensor(&sim, &sensor);
        er_status_t status = er_runtime_step(&rt, sim.now_us, &sensor, &out);
        if (status != ER_OK && status != ER_EFAULT) return 1;
        printf("{\"tick\":%u,\"time_us\":%llu,\"submitted\":%s,\"sequence\":%u,\"target\":[1,-0.5],\"sensor_valid\":%s,\"stalled\":%s,\"position\":[%.9g,%.9g],\"output_velocity\":[%.9g,%.9g],\"enabled\":%s,\"faults\":%u}\n",
               n, (unsigned long long)sim.now_us, submitted || n == 0 ? "true" : "false",
               cmd.sequence, sensor.valid ? "true" : "false", stall ? "true" : "false",
               sensor.position[0], sensor.position[1], out.velocity[0], out.velocity[1],
               out.enabled ? "true" : "false", out.faults);
        if ((stale && n >= 11) || (sensor_fault && n >= 20)) {
            uint32_t expected = stale ? ER_FAULT_CMD_STALE : ER_FAULT_SENSOR_INVALID;
            if (out.enabled || out.faults != expected || out.velocity[0] != 0 || out.velocity[1] != 0) return 1;
        } else if (!out.enabled || out.faults) return 1;
        er_sim_advance(&sim, &out, 10000);
    }
    if (!stale && !sensor_fault) {
        if (fabsf(sim.position[1] + 0.5f) > 0.001f) return 1;
        if (stall ? sim.position[0] != 0 : fabsf(sim.position[0] - 1) > 0.001f) return 1;
        /* No stall detector exists yet: this checks the plant model, not protection. */
    }
    return 0;
}
