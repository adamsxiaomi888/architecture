#ifndef ER_WEB_BRIDGE_H
#define ER_WEB_BRIDGE_H
#ifdef __cplusplus
extern "C" {
#endif
void er_web_reset(void);
int er_web_command(float target0, float target1, float velocity);
int er_web_step(unsigned dt_us);
void er_web_set_fault(int sensor_invalid, int stall0, int stall1);
void er_web_stop(void);
const char *er_web_state(void);
#ifdef __cplusplus
}
#endif
#endif
