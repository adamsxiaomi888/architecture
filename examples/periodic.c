#include "er/osal.h"
#include "osal_posix.h"
#include <stdio.h>
#include <inttypes.h>
/* Demonstrates scheduling only. Console I/O is not a real-time control task. */
int main(void)
{
    er_osal_clock_t clock = er_osal_posix_clock();
    er_periodic_t periodic;
    er_periodic_report_t report;
    if (er_periodic_init(&periodic, &clock, 10000) != ER_OSAL_OK) return 1;
    for (unsigned i=0; i<10; ++i) {
        if (er_periodic_wait(&periodic, &report) != ER_OSAL_OK) return 1;
        printf("tick=%u lateness_us=%" PRIu64 " missed=%" PRIu64 "\n",
               i, report.lateness_us, report.missed_periods);
    }
    return 0;
}
