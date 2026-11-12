#define _POSIX_C_SOURCE 200809L
#include "osal_posix.h"
#include <time.h>
#include <errno.h>
#include <stdint.h>
static int now_us(void *ctx, uint64_t *value)
{
    (void)ctx;
    struct timespec ts;
    if (!value || clock_gettime(CLOCK_MONOTONIC, &ts) || ts.tv_sec < 0)
        return ER_OSAL_ECLOCK;
    uint64_t sec = (uint64_t)ts.tv_sec;
    if (sec > (UINT64_MAX - 999999u) / 1000000u) return ER_OSAL_EOVERFLOW;
    *value = sec * 1000000u + (uint64_t)ts.tv_nsec / 1000u;
    return ER_OSAL_OK;
}
static int wait_us(void *ctx, uint64_t deadline)
{
    (void)ctx;
    uint64_t seconds = deadline / 1000000u;
    time_t sec = (time_t)seconds;
    if (sec < 0 || (uint64_t)sec != seconds) return ER_OSAL_EOVERFLOW;
    struct timespec ts = {sec, (long)(deadline % 1000000u) * 1000L};
    int rc;
    do { rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, 0); }
    while (rc == EINTR);
    return rc ? ER_OSAL_EWAIT : ER_OSAL_OK;
}
er_osal_clock_t er_osal_posix_clock(void)
{
    er_osal_clock_t clock = {now_us, wait_us, 0};
    return clock;
}
