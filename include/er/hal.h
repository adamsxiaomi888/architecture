#ifndef ER_HAL_H
#define ER_HAL_H

/* Hardware/OS boundary. Implementations own driver state; the core never
 * dereferences a platform-specific handle. All functions are non-blocking
 * unless their contract explicitly says otherwise. */
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    ER_HAL_OK = 0, ER_HAL_EINVAL = -1, ER_HAL_EBUSY = -2,
    ER_HAL_ETIMEOUT = -3, ER_HAL_EIO = -4, ER_HAL_ENOTSUP = -5,
    ER_HAL_EFAULT = -6
} er_hal_status_t;

typedef struct {
    uint64_t (*now_us)(void *ctx); /* monotonic; never wall clock */
    void *ctx;
} er_hal_clock_t;

typedef struct {
    er_hal_status_t (*read)(void *ctx, uint64_t now_us, float *value,
                            uint64_t *sample_us);
    void *ctx;
} er_hal_sensor_t;

enum {
    ER_HAL_ACT_POSITION = 1u << 0,
    ER_HAL_ACT_VELOCITY = 1u << 1,
    ER_HAL_ACT_TORQUE = 1u << 2,
    ER_HAL_ACT_FEEDBACK = 1u << 3,
    ER_HAL_ACT_ENABLE = 1u << 4,
    ER_HAL_ACT_PWM = 1u << 5
};

typedef struct {
    uint32_t capabilities;
    er_hal_status_t (*enable)(void *ctx, bool enabled);
    er_hal_status_t (*set_position)(void *ctx, float position, float velocity);
    er_hal_status_t (*set_velocity)(void *ctx, float velocity);
    er_hal_status_t (*set_torque)(void *ctx, float torque);
    er_hal_status_t (*read_feedback)(void *ctx, float *position, float *velocity,
                                     float *effort, uint64_t *sample_us);
    void *ctx;
} er_hal_actuator_t;

typedef struct {
    er_hal_status_t (*read)(void *ctx, uint32_t offset, void *data, uint32_t size);
    er_hal_status_t (*write)(void *ctx, uint32_t offset, const void *data, uint32_t size);
    er_hal_status_t (*erase)(void *ctx, uint32_t offset, uint32_t size);
    uint32_t size_bytes;
    uint32_t erase_block_bytes;
    void *ctx;
} er_hal_storage_t;

typedef struct {
    er_hal_status_t (*feed)(void *ctx);
    er_hal_status_t (*stop)(void *ctx);
    uint32_t timeout_ms;
    void *ctx;
} er_hal_watchdog_t;

/* Contract: sample_us must be monotonic and no newer than now_us. Values are
 * SI units chosen by the device interface (radians, rad/s, amperes, etc.).
 * Real-time callers must not allocate, sleep, log, or access a filesystem. */
#endif
