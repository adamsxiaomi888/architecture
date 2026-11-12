#ifndef ER_OSAL_H
#define ER_OSAL_H
#include <stdbool.h>
#include <stdint.h>

/* Single-owner clock adapter. Both operations use the same monotonic epoch,
 * in microseconds. wait_until returns only at/after its absolute deadline;
 * interruptions must be handled by the port. Zero means success. */
typedef struct {
    int (*now)(void *ctx, uint64_t *time_us);
    int (*wait_until)(void *ctx, uint64_t deadline_us);
    void *ctx;
} er_osal_clock_t;
enum { ER_OSAL_OK=0, ER_OSAL_EINVAL=-1, ER_OSAL_ECLOCK=-2,
       ER_OSAL_EOVERFLOW=-3, ER_OSAL_EWAIT=-4 };
typedef struct {
    er_osal_clock_t clock;
    uint64_t period_us, next_us, last_us;
    bool initialized;
} er_periodic_t;
typedef struct {
    uint64_t deadline_us, wake_us, lateness_us;
    /* Additional cadence slots expired beyond the scheduled deadline. */
    uint64_t missed_periods;
} er_periodic_report_t;
/* Clock, wait and arithmetic errors invalidate the timer. Explicit
 * reinitialization is required. Invalid pointer arguments return EINVAL.
 * Init schedules first release one period after current time. A late wake
 * advances directly to the next strictly future cadence slot (no burst).
 * Reports describe scheduling only; callers decide the safety policy. */
int er_periodic_init(er_periodic_t *timer, const er_osal_clock_t *clock,
                     uint64_t period_us);
int er_periodic_wait(er_periodic_t *timer, er_periodic_report_t *report);
#endif
