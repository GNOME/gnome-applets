/*--------------------------------*-C-*---------------------------------*
 *
 *  Copyright 1999, Nat Friedman <nat@nat.org>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 *
 *----------------------------------------------------------------------*/

/*
 * File: applets/battery/read-battery.c
 *
 * This file contains the routines which read the battery data from
 * the hardware.
 *
 * Author: Nat Friedman <nat@nat.org>
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <gnome.h>
#include "read-battery.h"

int
battery_read_charge(char * percentage,
		    char * ac_online,
		    char * hours_remaining,
		    char * minutes_remaining)
{
#ifdef __linux__
  static FILE * apm_file = NULL;

  char buffer[256], units[10];
  apm_info i;

  /*
   * (1) Open /proc/apm
   */
  if (apm_file == NULL)
    {
      apm_file = fopen("/proc/apm", "r");

      if (apm_file == NULL)
	{
	  g_warning (_("Cannot open /proc/apm!  Make sure that you built APM "
		      "support into your kernel.\n"));
	  *ac_online = 1;
	  *percentage = 100;
	  *hours_remaining = -1;

	  return FALSE;
	}
    }
  else
    {
      /*
       * If we are here, then this routine has been called before and
       * apm_file is valid.  We do not want to call open() over and
       * over again, since each call to open() requires an access to
       * the root inode, and disk accesses are bad on laptops.  An
       * alternative would be to chdir() into /proc, but then core
       * dumps will disappear, and that's not good either.
       *
       * Unfortunately, we can't just lseek() to the beginning of
       * /proc/apm and re-read the data, either.  It ends up always
       * giving you the same data.  So we dup() the fd associated with
       * apm_file, close the old one, and use the duplicate.  This
       * gives us up-to-date apm information and never touches the
       * filesystem.
       *
       */

      int dup_fd;

      dup_fd = dup( fileno(apm_file) );
      if (dup_fd == -1)
	{
	  g_error(_("Could not dup() APM file descriptor: %s\n"),
		  g_strerror(errno));
	  abort();
	}

      fclose(apm_file);

      apm_file = fdopen(dup_fd, "r");
      fseek(apm_file, 0, SEEK_SET);
    }

  /*
   * (2) Read the battery charge information
   */
  fgets( buffer, sizeof( buffer ) - 1, apm_file );

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

  /*
   * (3) Interpret it and return.
   */
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
  
  if (i.battery_percentage > 100)
    i.battery_percentage = 0;

  /*
   * If we cannot read the battery percentage (indicated by a reading
   * of -1), then just set it to zero.  It will sometimes appear as -1
   * if, for example, the battery is unplugged, so this might be a
   * FIXME if we want to provide that kind of information later.
   */
  if (i.battery_percentage < 0) i.battery_percentage = 0;

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
