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
#endif /* __linux__ */

/* Prototypes */ 
int battery_read_charge(char * percentage,
			char * ac_online,
			char * hours_remaining,
			char * minutes_remaining);

#endif /* _READ_BATTERY_H */
