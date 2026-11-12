#include "er/devices.h"
#include <math.h>
#include <string.h>

static er_hal_status_t disable_all(er_devices_t *d)
{
    er_hal_status_t result = ER_HAL_OK;
    for (int i=0; i<ER_JOINTS; ++i) {
        if (d->axes[i].enable) {
            er_hal_status_t rc=d->axes[i].enable(d->axes[i].ctx, false);
            if (rc!=ER_HAL_OK && result==ER_HAL_OK) result=rc;
        }
    }
    if (result!=ER_HAL_OK) d->faulted=true;
    return result;
}
static er_hal_status_t fail(er_devices_t *d, er_hal_status_t rc)
{
    d->faulted=true;
    (void)disable_all(d);
    return rc;
}
er_hal_status_t er_devices_bind(er_devices_t *d,
                                const er_hal_actuator_t axes[ER_JOINTS])
{
    if (!d || !axes) return ER_HAL_EINVAL;
    /* Caller must disable an existing binding before replacing its contexts. */
    memset(d,0,sizeof(*d));
    memcpy(d->axes,axes,sizeof(d->axes));
    const uint32_t required=ER_HAL_ACT_FEEDBACK|ER_HAL_ACT_VELOCITY|ER_HAL_ACT_ENABLE;
    for (int i=0; i<ER_JOINTS; ++i) {
        const er_hal_actuator_t *a=&d->axes[i];
        if ((a->capabilities & required)!=required)
            return fail(d,ER_HAL_ENOTSUP);
        if (!a->enable || !a->set_velocity || !a->read_feedback)
            return fail(d,ER_HAL_EINVAL);
    }
    er_hal_status_t rc=disable_all(d);
    if (rc!=ER_HAL_OK) return rc;
    d->bound=true;
    return ER_HAL_OK;
}
er_hal_status_t er_devices_disable(er_devices_t *d)
{
    if (!d) return ER_HAL_EINVAL;
    return disable_all(d);
}
er_hal_status_t er_devices_read(er_devices_t *d, uint64_t now, er_sensor_t *s)
{
    if (s) memset(s,0,sizeof(*s));
    if (!d) return ER_HAL_EINVAL;
    if (!s) return fail(d,ER_HAL_EINVAL);
    if (!d->bound || d->faulted) return fail(d,ER_HAL_EFAULT);
    s->timestamp_us=now;
    for (int i=0; i<ER_JOINTS; ++i) {
        float effort=0; uint64_t stamp=0;
        er_hal_actuator_t *a=&d->axes[i];
        er_hal_status_t rc=a->read_feedback(a->ctx,&s->position[i],&s->velocity[i],&effort,&stamp);
        if (rc!=ER_HAL_OK) return fail(d,rc);
        if (!isfinite(s->position[i]) || !isfinite(s->velocity[i]) ||
            stamp>now || now-stamp>ER_MAX_SENSOR_AGE_US)
            return fail(d,ER_HAL_EFAULT);
        /* Preserve oldest sample so aggregation never makes stale data fresh. */
        if (stamp<s->timestamp_us) s->timestamp_us=stamp;
    }
    s->valid=true;
    return ER_HAL_OK;
}
er_hal_status_t er_devices_apply(er_devices_t *d, const er_output_t *o)
{
    if (!d) return ER_HAL_EINVAL;
    if (!o) return fail(d,ER_HAL_EINVAL);
    if (!d->bound || d->faulted) return fail(d,ER_HAL_EFAULT);
    if (!o->enabled || o->faults) return disable_all(d);
    for (int i=0; i<ER_JOINTS; ++i)
        if (!isfinite(o->velocity[i])) return fail(d,ER_HAL_EINVAL);
    /* Preload every axis before enabling. This is not an atomic hardware
     * transaction: a driver error can leave transient partial output. */
    for (int i=0; i<ER_JOINTS; ++i) {
        er_hal_actuator_t *a=&d->axes[i];
        er_hal_status_t rc=a->set_velocity(a->ctx,o->velocity[i]);
        if (rc!=ER_HAL_OK) return fail(d,rc);
    }
    for (int i=0; i<ER_JOINTS; ++i) {
        er_hal_actuator_t *a=&d->axes[i];
        er_hal_status_t rc=a->enable(a->ctx,true);
        if (rc!=ER_HAL_OK) return fail(d,rc);
    }
    return ER_HAL_OK;
}
