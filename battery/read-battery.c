#include <stdio.h>
#include <gnome.h>
#include <string.h>
#include "read-battery.h"

int
battery_read_charge(char * percentage,
		    char * ac_online,
		    char * hours_remaining,
		    char * minutes_remaining)
{
#ifdef __linux__
/* This was mainly lifted out of the APM sample interface functions that
   came with the APM package. */ 
  char buffer[256], units[10];
  apm_info i;
  FILE * f;
  
  if ((f = fopen("/proc/apm", "r")) == NULL)
    {
      g_error(_("Cannot open /proc/apm!  Make sure that you built APM "
		"support into your kernel.\n"));
      return FALSE;
    }

  fgets( buffer, sizeof( buffer ) - 1, f );

  buffer[ sizeof( buffer ) - 1 ] = '\0';

  sscanf( buffer, "%s %d.%d %x %x %x %x %d%% %d %s\n",
	  (char *)i.driver_version,
	  &i.apm_version_major,
	  &i.apm_version_minor,
	  &i.apm_flags,
	  &i.ac_line_status,
	  &i.battery_status,
	  &i.battery_flags,
	  &i.battery_percentage,
	  &i.battery_time,
	  units );

  i.using_minutes = !strncmp( units, "min", 3 ) ? 1 : 0;

  if (i.using_minutes)
    {
      *hours_remaining = i.battery_time / 60;
      *minutes_remaining = i.battery_time % 60;
    }
  else /* Otherwise, the battery_time is in seconds */
    {
      *hours_remaining = i.battery_time / 3600;
      i.battery_time %= 3600;
      *minutes_remaining = i.battery_time / 60;
    }
  
  /* Fix possible kernel bug -- percentage set to 0xff (==255) instead
     of -1. */
  if (i.battery_percentage > 100) i.battery_percentage = 0;

  /* If we cannot read the battery percentage (indicated by a reading
     of -1), then just set it to zero.  It will sometimes appear as -1
     if, for example, the battery is unplugged, so this might be a
     FIXME if we want to provide that kind of information later */
  if (i.battery_percentage < 0) i.battery_percentage = 0;

  fclose(f);

  *percentage = i.battery_percentage;
  *ac_online = i.ac_line_status;
  
  return TRUE;
#elif __FreeBSD__  /* was #ifdef __linux__ */
  struct apm_info aip;
  int fd;

  fd = open(APMDEV, O_RDWR);
  if (fd == -1)
    {
      g_error (_("Cannot open /dev/apm; can't get data."));
      return FALSE;
    }

  if (ioctl(fd, APMIO_GETINFO, &aip) == -1) {
    g_error(_("ioctl failed on /dev/apm."));
    return FALSE;
  }

  /* We cannot read these under FreeBSD. */
  *hours_remaining = -1;
  *minutes_remaining = 1;

  /* if APM is not turned on */
  if (!aip.ai_status)
    {
      g_error(_("APM is disabled!  Cannot read battery charge information."));
    }

  *ac_online = aip.ai_acline;
  *percentage = aip.ai_batt_life;

  close(fd);
  return TRUE;
#else /* ! ( __linux__ || __FreeBSD__) */
  /* Assume always connected to power.  */
  *ac_online = 1;
  *percentage = 100;
  *hours_remaining = -1;
  *minutes_remaining = 1;
#endif /* __linux__ || __FreeBSD__ */
} /* battery_read_charge */
