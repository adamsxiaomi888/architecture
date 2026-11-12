#include "er/osal.h"
#include <stdio.h>
#define CHECK(c) do {if(!(c)){fprintf(stderr,"line %d: %s\n",__LINE__,#c);return 1;}}while(0)
typedef struct {uint64_t now, extra; unsigned waits; int now_error,wait_error; bool early,rollback;} fake_t;
static int now(void *ctx,uint64_t *out){fake_t*f=ctx;if(f->now_error)return -1;*out=f->now;return 0;}
static int wait(void *ctx,uint64_t until){fake_t*f=ctx;f->waits++;if(f->wait_error)return -1;if(f->rollback)f->now=0;else f->now=until+f->extra-(f->early?1:0);return 0;}
int main(void)
{
    fake_t f={0}; er_osal_clock_t c={now,wait,&f}; er_periodic_t t; er_periodic_report_t r;
    CHECK(er_periodic_init(&t,&c,0)==ER_OSAL_EINVAL);
    CHECK(er_periodic_init(&t,&c,10)==0);
    CHECK(er_periodic_wait(&t,&r)==0 && r.deadline_us==10 && r.wake_us==10 && r.lateness_us==0);
    /* Work takes 3 us; absolute schedule stays at 20 rather than 23. */
    f.now+=3; CHECK(er_periodic_wait(&t,&r)==0 && r.wake_us==20 && t.next_us==30);
    f.now=65; unsigned waits=f.waits;
    CHECK(er_periodic_wait(&t,&r)==0 && f.waits==waits && r.lateness_us==35 && r.missed_periods==3 && t.next_us==70);
    CHECK(er_periodic_wait(&t,&r)==0 && r.wake_us==70);
    f.extra=25; CHECK(er_periodic_wait(&t,&r)==0 && r.wake_us==105 && r.missed_periods==2 && t.next_us==110);
    f.extra=0; f.now=104; CHECK(er_periodic_wait(&t,&r)==ER_OSAL_ECLOCK && !t.initialized);
    f.now=10; f.early=true; CHECK(er_periodic_init(&t,&c,10)==0);
    CHECK(er_periodic_wait(&t,&r)==ER_OSAL_EWAIT && !t.initialized);
    f.early=false; f.rollback=true; CHECK(er_periodic_init(&t,&c,10)==0);
    CHECK(er_periodic_wait(&t,&r)==ER_OSAL_ECLOCK);
    f.rollback=false; f.now=UINT64_MAX-5;
    CHECK(er_periodic_init(&t,&c,10)==ER_OSAL_EOVERFLOW);
    f.now=UINT64_MAX-10; CHECK(er_periodic_init(&t,&c,10)==0);
    CHECK(er_periodic_wait(&t,&r)==ER_OSAL_EOVERFLOW && !t.initialized);
    f.now=0; f.now_error=1; CHECK(er_periodic_init(&t,&c,10)==ER_OSAL_ECLOCK);
    f.now_error=0; CHECK(er_periodic_init(&t,&c,10)==0);
    f.wait_error=1; CHECK(er_periodic_wait(&t,&r)==ER_OSAL_EWAIT);
    f.wait_error=0; CHECK(er_periodic_init(&t,&c,10)==0);
    f.now_error=1; CHECK(er_periodic_wait(&t,&r)==ER_OSAL_ECLOCK);
    puts("OSAL deterministic cadence tests passed");return 0;
}
