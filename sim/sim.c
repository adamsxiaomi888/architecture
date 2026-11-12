#include "sim.h"
#include <string.h>
#include <math.h>
void er_sim_init(er_sim_t *s){memset(s,0,sizeof(*s));s->sensor_valid=true;}
void er_sim_advance(er_sim_t *s,const er_output_t *o,uint32_t dt){if(!s||!o)return;float d=(float)dt/1000000.0f;s->now_us+=(uint64_t)dt;for(int i=0;i<ER_JOINTS;i++){float v=(o->enabled&&!s->stalled[i])?o->velocity[i]:0.0f;s->velocity[i]=v;s->position[i]+=v*d;if(!isfinite(s->position[i]))s->sensor_valid=false;}}
void er_sim_sensor(const er_sim_t*s,er_sensor_t*out){if(!s||!out)return;out->timestamp_us=s->now_us;for(int i=0;i<ER_JOINTS;i++){out->position[i]=s->position[i];out->velocity[i]=s->velocity[i];}out->valid=s->sensor_valid;}
void er_sim_set_sensor_valid(er_sim_t*s,bool v){if(s)s->sensor_valid=v;}
void er_sim_set_stalled(er_sim_t*s,int j,bool v){if(s&&j>=0&&j<ER_JOINTS)s->stalled[j]=v;}
