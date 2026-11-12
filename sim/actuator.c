#include "actuator.h"
#include <math.h>
#include <string.h>

static er_hal_status_t enable(void *opaque, bool value)
{
    er_sim_actuator_t *c = opaque;
    if (!c || !c->sim) return ER_HAL_EINVAL;
    c->enabled = value;
    if (!value) c->speed = 0;
    return ER_HAL_OK;
}

static er_hal_status_t position(void *opaque, float target, float speed)
{
    er_sim_actuator_t *c = opaque;
    if (!c || !c->sim || !isfinite(target) || !isfinite(speed) ||
        target < -3 || target > 3 || speed <= 0 || speed > 2)
        return ER_HAL_EINVAL;
    c->target = target;
    c->speed = speed;
    c->position_mode = true;
    return ER_HAL_OK;
}

static er_hal_status_t velocity(void *opaque, float speed)
{
    er_sim_actuator_t *c = opaque;
    if (!c || !c->sim) return ER_HAL_EINVAL;
    if (!c->feedback) return ER_HAL_ENOTSUP;
    if (!isfinite(speed) || fabsf(speed) > 2) return ER_HAL_EINVAL;
    c->speed = speed;
    c->position_mode = false;
    return ER_HAL_OK;
}

static er_hal_status_t torque(void *opaque, float value)
{
    (void)opaque; (void)value;
    return ER_HAL_ENOTSUP;
}

static er_hal_status_t feedback(void *opaque, float *p, float *v,
                                float *effort, uint64_t *stamp)
{
    er_sim_actuator_t *c = opaque;
    if (!c || !c->sim) return ER_HAL_EINVAL;
    if (!c->feedback) return ER_HAL_ENOTSUP;
    if (!p || !v || !effort || !stamp) return ER_HAL_EINVAL;
    if (!c->sim->sensor_valid || !isfinite(c->sim->position[c->axis]) ||
        !isfinite(c->sim->velocity[c->axis])) return ER_HAL_EIO;
    *p = c->sim->position[c->axis];
    *v = c->sim->velocity[c->axis];
    *effort = NAN;
    *stamp = c->sim->now_us;
    return ER_HAL_OK;
}

er_hal_status_t er_sim_actuator_init(er_sim_actuator_t *c, er_sim_t *s,
    unsigned axis, bool has_feedback, er_hal_actuator_t *device)
{
    if (!c || !s || !device || axis >= ER_JOINTS) return ER_HAL_EINVAL;
    memset(c, 0, sizeof(*c));
    c->sim = s;
    c->axis = axis;
    c->feedback = has_feedback;
    *device = (er_hal_actuator_t){
        .capabilities = ER_HAL_ACT_POSITION | ER_HAL_ACT_ENABLE |
            (has_feedback ? ER_HAL_ACT_FEEDBACK | ER_HAL_ACT_VELOCITY : 0),
        .enable = enable, .set_position = position, .set_velocity = velocity,
        .set_torque = torque, .read_feedback = feedback, .ctx = c
    };
    return ER_HAL_OK;
}

er_hal_status_t er_sim_actuators_advance(er_sim_actuator_t *c,
    size_t count, uint32_t dt)
{
    if (!c || !count || count > ER_JOINTS || !c[0].sim || !dt ||
        dt > ER_MAX_STEP_US) return ER_HAL_EINVAL;
    unsigned seen = 0;
    er_output_t output = {0};
    for (size_t i = 0; i < count; ++i) {
        if (c[i].sim != c[0].sim || c[i].axis >= ER_JOINTS ||
            (seen & (1u << c[i].axis))) return ER_HAL_EINVAL;
        seen |= 1u << c[i].axis;
        if (!c[i].enabled) continue;
        output.enabled = true;
        float v = c[i].speed;
        if (c[i].position_mode) {
            v = (c[i].target - c[i].sim->position[c[i].axis]) * 10;
            v = fmaxf(-c[i].speed, fminf(c[i].speed, v));
        }
        output.velocity[c[i].axis] = v;
        output.target[c[i].axis] = c[i].target;
    }
    er_sim_advance(c[0].sim, &output, dt);
    return ER_HAL_OK;
}
