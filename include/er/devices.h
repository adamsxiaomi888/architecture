#ifndef ER_DEVICES_H
#define ER_DEVICES_H
#include "er/hal.h"
#include "er/runtime.h"

/* Single owner, static storage. Contexts must outlive this binding. This adapter
 * is for the feedback velocity-mode runtime, not every possible actuator.
 * Drivers must permit preloading velocity while disabled. Binding/rebinding is
 * a stopped-system operation; fault recovery is explicit rebind + runtime reset.
 * Disable is best effort: failure requires independent hardware protection. */
typedef struct {
    er_hal_actuator_t axes[ER_JOINTS];
    bool bound;
    bool faulted;
} er_devices_t;
er_hal_status_t er_devices_bind(er_devices_t *dev,
                                const er_hal_actuator_t axes[ER_JOINTS]);
er_hal_status_t er_devices_read(er_devices_t *dev, uint64_t now_us,
                                er_sensor_t *sensor);
er_hal_status_t er_devices_apply(er_devices_t *dev, const er_output_t *output);
er_hal_status_t er_devices_disable(er_devices_t *dev);
#endif
