#include "er/devices.h"
#include <math.h>
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {bool enabled;int disabled_calls;er_hal_status_t read_error,write_error,disable_error;float velocity;uint64_t stamp;} mock_t;
static er_hal_status_t enable(void *ctx,bool e){mock_t*m=ctx;if(!e){m->disabled_calls++;if(m->disable_error)return m->disable_error;}m->enabled=e;return ER_HAL_OK;}
static er_hal_status_t write_velocity(void*ctx,float v){mock_t*m=ctx;if(m->write_error)return m->write_error;m->velocity=v;return ER_HAL_OK;}
static er_hal_status_t read_feedback(void*ctx,float*p,float*v,float*eff,uint64_t*t){mock_t*m=ctx;if(m->read_error)return m->read_error;*p=0.5f;*v=m->velocity;*eff=NAN;*t=m->stamp;return ER_HAL_OK;}
static er_hal_actuator_t axis(mock_t*m){er_hal_actuator_t a={0};a.capabilities=ER_HAL_ACT_FEEDBACK|ER_HAL_ACT_VELOCITY|ER_HAL_ACT_ENABLE;a.enable=enable;a.set_velocity=write_velocity;a.read_feedback=read_feedback;a.ctx=m;return a;}
int main(void){
    mock_t m[2]={{0}};er_hal_actuator_t axes[2]={axis(&m[0]),axis(&m[1])};er_devices_t d;er_sensor_t s;er_output_t o={.enabled=true,.velocity={1,-1}};
    CHECK(er_devices_bind(&d,axes)==ER_HAL_OK);
    CHECK(m[0].disabled_calls==1&&m[1].disabled_calls==1);
    CHECK(er_devices_apply(&d,&o)==ER_HAL_OK);
    CHECK(m[0].enabled&&m[1].enabled&&m[1].velocity==-1);
    m[0].stamp=100;m[1].stamp=200;
    CHECK(er_devices_read(&d,300,&s)==ER_HAL_OK&&s.valid&&s.timestamp_us==100);
    m[1].read_error=ER_HAL_ETIMEOUT;
    CHECK(er_devices_read(&d,300,&s)==ER_HAL_ETIMEOUT&&!s.valid);
    CHECK(!m[0].enabled&&!m[1].enabled&&d.faulted);
    m[1].read_error=ER_HAL_OK;
    CHECK(er_devices_apply(&d,&o)==ER_HAL_EFAULT);
    CHECK(er_devices_bind(&d,axes)==ER_HAL_OK);
    m[1].write_error=ER_HAL_EIO;
    CHECK(er_devices_apply(&d,&o)==ER_HAL_EIO);
    CHECK(!m[0].enabled&&!m[1].enabled);
    m[1].write_error=ER_HAL_OK;
    axes[1].capabilities &= ~ER_HAL_ACT_FEEDBACK;
    CHECK(er_devices_bind(&d,axes)==ER_HAL_ENOTSUP&&!d.bound);
    axes[1]=axis(&m[1]);axes[1].read_feedback=NULL;
    CHECK(er_devices_bind(&d,axes)==ER_HAL_EINVAL);
    axes[1]=axis(&m[1]);CHECK(er_devices_bind(&d,axes)==ER_HAL_OK);
    CHECK(er_devices_read(&d,100000,&s)==ER_HAL_EFAULT&&!s.valid);
    CHECK(er_devices_bind(&d,axes)==ER_HAL_OK);
    o.velocity[0]=NAN;CHECK(er_devices_apply(&d,&o)==ER_HAL_EINVAL);
    o.velocity[0]=1;CHECK(er_devices_bind(&d,axes)==ER_HAL_OK);
    CHECK(er_devices_apply(&d,&o)==ER_HAL_OK);
    m[0].disable_error=ER_HAL_EIO;
    CHECK(er_devices_disable(&d)==ER_HAL_EIO);
    CHECK(!m[1].enabled&&d.faulted); /* Other axes are attempted despite error. */
    puts("device binding tests passed");return 0;
}
