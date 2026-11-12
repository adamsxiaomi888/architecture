#ifndef ER_SIM_H
#define ER_SIM_H
#include "er/runtime.h"
typedef struct { uint64_t now_us; float position[ER_JOINTS]; float velocity[ER_JOINTS]; bool sensor_valid; bool stalled[ER_JOINTS]; } er_sim_t;
void er_sim_init(er_sim_t *sim);
void er_sim_advance(er_sim_t *sim, const er_output_t *out, uint32_t dt_us);
void er_sim_sensor(const er_sim_t *sim, er_sensor_t *sensor);
void er_sim_set_sensor_valid(er_sim_t *sim, bool valid);
void er_sim_set_stalled(er_sim_t *sim, int joint, bool stalled);
#endif
