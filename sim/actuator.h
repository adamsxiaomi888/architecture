#ifndef ER_SIM_ACTUATOR_H
#define ER_SIM_ACTUATOR_H
#include "er/hal.h"
#include "sim.h"
#include <stddef.h>

/* Caller owns contexts and device tables; keep them alive until unbound.
 * Single-owner synchronous API. radians, rad/s and monotonic microseconds.
 * This model stops immediately on disable; real drivers must define their own
 * physical safe action. No current/torque physics: feedback effort is NAN. */
typedef struct {
    er_sim_t *sim;
    unsigned axis;
    bool feedback, enabled, position_mode;
    float target, speed;
} er_sim_actuator_t;

er_hal_status_t er_sim_actuator_init(er_sim_actuator_t *ctx, er_sim_t *sim,
    unsigned axis, bool feedback, er_hal_actuator_t *device);
/* All contexts must refer to one sim with distinct axes. Unlisted axes stop.
 * Does not expose internal plant position as device feedback for open-loop
 * devices; plant state remains available separately for simulation inspection. */
er_hal_status_t er_sim_actuators_advance(er_sim_actuator_t *contexts,
    size_t count, uint32_t dt_us);
#endif
