#ifndef ER_RUNTIME_H
#define ER_RUNTIME_H
#include <stdint.h>
#include <stdbool.h>
#define ER_JOINTS 2
#define ER_MAX_CMD_AGE_US 100000u
#define ER_MAX_STEP_US 20000u
#define ER_MAX_SENSOR_AGE_US 10000u

typedef enum { ER_OK=0, ER_EINVAL=-1, ER_ESTATE=-2, ER_EFAULT=-3 } er_status_t;
typedef enum { ER_FAULT_NONE=0, ER_FAULT_CMD_STALE=1u<<0, ER_FAULT_CMD_INVALID=1u<<1, ER_FAULT_SENSOR_INVALID=1u<<2, ER_FAULT_TIME=1u<<3 } er_fault_t;
typedef struct { uint64_t timestamp_us; uint32_t sequence; float position[ER_JOINTS]; float velocity_limit[ER_JOINTS]; } er_command_t;
typedef struct { uint64_t timestamp_us; float position[ER_JOINTS]; float velocity[ER_JOINTS]; bool valid; } er_sensor_t;
typedef struct { bool enabled; float target[ER_JOINTS]; float velocity[ER_JOINTS]; uint32_t faults; } er_output_t;
typedef struct { float min_position[ER_JOINTS], max_position[ER_JOINTS], max_velocity[ER_JOINTS]; } er_limits_t;
typedef struct { bool initialized, armed, have_step, have_command; uint64_t now_us, last_cmd_us; uint32_t last_sequence, faults; er_command_t cmd; er_limits_t limits; } er_runtime_t;

void er_runtime_init(er_runtime_t *rt, const er_limits_t *limits);
er_status_t er_runtime_arm(er_runtime_t *rt);
void er_runtime_disarm(er_runtime_t *rt);
er_status_t er_runtime_reset_faults(er_runtime_t *rt);
er_status_t er_runtime_submit_command(er_runtime_t *rt, const er_command_t *cmd);
er_status_t er_runtime_step(er_runtime_t *rt, uint64_t now_us, const er_sensor_t *sensor, er_output_t *out);
uint32_t er_runtime_faults(const er_runtime_t *rt);
#endif
