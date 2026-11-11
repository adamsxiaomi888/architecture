#include "er/runtime.h"
#include <math.h>
#include <string.h>

static void latch_fault(er_runtime_t *r, uint32_t code)
{
    r->faults |= code;
    r->armed = false;
}

void er_runtime_init(er_runtime_t *r, const er_limits_t *limits)
{
    if (!r) return;
    memset(r, 0, sizeof(*r));
    if (!limits) return;
    for (int i = 0; i < ER_JOINTS; ++i) {
        if (!isfinite(limits->min_position[i]) || !isfinite(limits->max_position[i]) ||
            !isfinite(limits->max_velocity[i]) || limits->min_position[i] >= limits->max_position[i] ||
            limits->max_velocity[i] <= 0) return;
    }
    r->limits = *limits;
    r->initialized = true;
}

er_status_t er_runtime_arm(er_runtime_t *r)
{
    if (!r || !r->initialized) return ER_ESTATE;
    if (r->faults) return ER_EFAULT;
    r->armed = true;
    return ER_OK;
}

void er_runtime_disarm(er_runtime_t *r)
{
    if (r) { r->armed = false; r->have_command = false; }
}

er_status_t er_runtime_reset_faults(er_runtime_t *r)
{
    if (!r || !r->initialized || r->armed) return ER_ESTATE;
    r->faults = 0;
    r->have_command = false;
    return ER_OK;
}

uint32_t er_runtime_faults(const er_runtime_t *r)
{
    return r ? r->faults : ER_FAULT_TIME;
}

er_status_t er_runtime_submit_command(er_runtime_t *r, const er_command_t *c)
{
    if (!r || !c || !r->initialized) return ER_EINVAL;
    if (r->faults) return ER_EFAULT;
    /* Sequence zero is reserved. At UINT32_MAX, restart a session while disarmed. */
    if (c->sequence <= r->last_sequence ||
        (r->have_command && c->timestamp_us < r->last_cmd_us) ||
        (r->have_step && c->timestamp_us < r->now_us &&
         r->now_us - c->timestamp_us > ER_MAX_CMD_AGE_US)) {
        latch_fault(r, ER_FAULT_CMD_INVALID);
        return ER_EFAULT;
    }
    for (int i = 0; i < ER_JOINTS; ++i) {
        if (!isfinite(c->position[i]) || !isfinite(c->velocity_limit[i]) ||
            c->position[i] < r->limits.min_position[i] ||
            c->position[i] > r->limits.max_position[i] ||
            c->velocity_limit[i] <= 0 || c->velocity_limit[i] > r->limits.max_velocity[i]) {
            latch_fault(r, ER_FAULT_CMD_INVALID);
            return ER_EFAULT;
        }
    }
    r->cmd = *c;
    r->last_sequence = c->sequence;
    r->last_cmd_us = c->timestamp_us;
    r->have_command = true;
    return ER_OK;
}

er_status_t er_runtime_step(er_runtime_t *r, uint64_t now,
                            const er_sensor_t *sensor, er_output_t *out)
{
    /* Even API misuse must not leave an old enabled output in caller storage. */
    if (out) memset(out, 0, sizeof(*out));
    if (!r || !sensor || !out || !r->initialized) return ER_EINVAL;
    if (r->have_step && (now <= r->now_us ||
        (r->armed && now - r->now_us > ER_MAX_STEP_US))) latch_fault(r, ER_FAULT_TIME);
    if (!r->have_step || now > r->now_us) r->now_us = now;
    r->have_step = true;
    if (!sensor->valid || sensor->timestamp_us > now ||
        now - sensor->timestamp_us > ER_MAX_SENSOR_AGE_US) latch_fault(r, ER_FAULT_SENSOR_INVALID);
    for (int i = 0; i < ER_JOINTS; ++i) {
        if (!isfinite(sensor->position[i]) || !isfinite(sensor->velocity[i]) ||
            sensor->position[i] < r->limits.min_position[i] ||
            sensor->position[i] > r->limits.max_position[i]) latch_fault(r, ER_FAULT_SENSOR_INVALID);
    }
    if (r->armed && (!r->have_command || now < r->last_cmd_us ||
        now - r->last_cmd_us > ER_MAX_CMD_AGE_US)) latch_fault(r, ER_FAULT_CMD_STALE);
    out->faults = r->faults;
    if (r->faults) return ER_EFAULT;
    if (!r->armed) return ER_OK;
    for (int i = 0; i < ER_JOINTS; ++i) {
        /* Educational velocity-mode P controller; hardware owns inner servo loops. */
        float velocity = (r->cmd.position[i] - sensor->position[i]) * 10.0f;
        float cap = r->cmd.velocity_limit[i];
        if (velocity > cap) velocity = cap;
        if (velocity < -cap) velocity = -cap;
        out->target[i] = r->cmd.position[i];
        out->velocity[i] = velocity;
    }
    out->enabled = true;
    return ER_OK;
}
