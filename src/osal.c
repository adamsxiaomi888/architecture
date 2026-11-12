#include "er/osal.h"
#include <string.h>

static int fail(er_periodic_t *t, int error)
{
    t->initialized=false;
    return error;
}
int er_periodic_init(er_periodic_t *t,const er_osal_clock_t *clock,uint64_t period)
{
    if (!t) return ER_OSAL_EINVAL;
    /* Copy first to permit reinitialization using &timer->clock. */
    er_osal_clock_t copy={0};
    if (clock) copy=*clock;
    memset(t,0,sizeof(*t));
    if (!clock || !copy.now || !copy.wait_until || !period) return ER_OSAL_EINVAL;
    uint64_t now;
    if (copy.now(copy.ctx,&now)!=0) return ER_OSAL_ECLOCK;
    if (period>UINT64_MAX-now) return ER_OSAL_EOVERFLOW;
    t->clock=copy; t->period_us=period; t->next_us=now+period;
    t->last_us=now; t->initialized=true;
    return ER_OSAL_OK;
}
int er_periodic_wait(er_periodic_t *t,er_periodic_report_t *r)
{
    if (r) memset(r,0,sizeof(*r));
    if (!t || !r) return ER_OSAL_EINVAL;
    if (!t->initialized) return ER_OSAL_EINVAL;
    uint64_t now;
    if (t->clock.now(t->clock.ctx,&now)!=0 || now<t->last_us)
        return fail(t,ER_OSAL_ECLOCK);
    const uint64_t deadline=t->next_us;
    if (now<deadline) {
        const uint64_t before=now;
        if (t->clock.wait_until(t->clock.ctx,deadline)!=0)
            return fail(t,ER_OSAL_EWAIT);
        if (t->clock.now(t->clock.ctx,&now)!=0 || now<before)
            return fail(t,ER_OSAL_ECLOCK);
        if (now<deadline) return fail(t,ER_OSAL_EWAIT);
    }
    const uint64_t late=now-deadline, missed=late/t->period_us;
    /* Equivalent to deadline+(missed+1)*period, without multiplication overflow. */
    const uint64_t increment=t->period_us-late%t->period_us;
    if (increment>UINT64_MAX-now) return fail(t,ER_OSAL_EOVERFLOW);
    r->deadline_us=deadline; r->wake_us=now; r->lateness_us=late;
    r->missed_periods=missed;
    t->last_us=now; t->next_us=now+increment;
    return ER_OSAL_OK;
}
