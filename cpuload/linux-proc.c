/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>

#include <glibtop.h>
#include <glibtop/cpu.h>

#include "linux-proc.h"

#define NCPUSTATES 4

static long cp_time[NCPUSTATES];
static long last[NCPUSTATES];

static unsigned needed_cpu_flags =
(1 << GLIBTOP_CPU_USER) +
(1 << GLIBTOP_CPU_NICE) +
(1 << GLIBTOP_CPU_SYS) +
(1 << GLIBTOP_CPU_IDLE);

void GetLoad(int Maximum, int *usr, int *nice, int *sys, int *free)
{
  glibtop_cpu cpu;
  int total;

  glibtop_get_cpu (&cpu);

  assert ((cpu.flags & needed_cpu_flags) == needed_cpu_flags);

  cp_time[0] = cpu.user;
  cp_time[1] = cpu.nice;
  cp_time[2] = cpu.sys;
  cp_time[3] = cpu.idle;

  *usr  = cp_time[0] - last[0];
  *nice = cp_time[1] - last[1];
  *sys  = cp_time[2] - last[2];
  *free = cp_time[3] - last[3];
  total = *usr + *nice + *sys + *free;

  last[0] = cp_time[0];
  last[1] = cp_time[1];
  last[2] = cp_time[2];
  last[3] = cp_time[3];

  *usr = rint(Maximum * (float)(*usr)   /total);
  *nice =rint(Maximum * (float)(*nice)  /total);
  *sys = rint(Maximum * (float)(*sys)   /total);
  *free = rint(Maximum * (float)(*free) /total);
}
