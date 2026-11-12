#include "control_task.h"
#include "osal_posix.h"
#include <stdio.h>
int main(int argc,char **argv){const char *s=argc>1?argv[1]:"normal";er_osal_clock_t c=er_osal_posix_clock();int rc=er_control_task_run(&c,s);if(rc)fprintf(stderr,"control failed rc=%d\n",rc);return rc;}
