#ifndef _READ_BATTERY_H
#define _READ_BATTERY_H

#ifdef __linux__
typedef struct apm_info {
  const char driver_version[10];
  int        apm_version_major;
  int        apm_version_minor;
  int        apm_flags;
  int        ac_line_status;
  int        battery_status;
  int        battery_flags;
  int        battery_percentage;
  int        battery_time;
  int        using_minutes;
} apm_info;
#elif __FreeBSD__
#include <fcntl.h>
#include <machine/apm_bios.h>
#define APMDEV "/dev/apm"
#elif __NetBSD__ && defined(NETBSD_APM)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <machine/apmvar.h>
#define APMDEV "/dev/apm"
#endif

/* Prototypes */ 
gboolean battery_read_charge (int * percentage,
			      int * ac_online,
			      int * hours_remaining,
			      int * minutes_remaining);

#endif /* _READ_BATTERY_H */
