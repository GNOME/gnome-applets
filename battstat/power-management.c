/* battstat        A GNOME battery meter for laptops. 
 * Copyright (C) 2000 by Jörgen Pehrson <jp@spektr.eu.org>
 * Copyright (C) 2002-2005 Free Software Foundation
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
 $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <gnome.h> // needed to make it through battstat.h
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "battstat.h"

#define ERR_ACPID _("Can't access ACPI events in /var/run/acpid.socket! "    \
                    "Make sure the ACPI subsystem is working and "           \
                    "the acpid daemon is running.")

#define ERR_OPEN_APMDEV _("Can't open the APM device!\n\n"                  \
                          "Make sure you have read permission to the\n"     \
                          "APM device.")

#define ERR_APM_E _("The APM Management subsystem seems to be disabled.\n"  \
                    "Try executing \"apm -e 1\" (FreeBSD) and see if \n"    \
                    "that helps.\n")

#define ERR_NO_SUPPORT _("Your platform is not supported!  The battery\n"   \
                         "monitor applet will not work on your system.\n")

static const char *apm_readinfo (BatteryStatus *status);

/*
 * What follows is a series of platform-specific apm_readinfo functions
 * that take care of the quirks of getting battery information for different
 * platforms.  Each function takes a BatteryStatus pointer and fills it
 * then returns a const char *.
 *
 * In the case of success, NULL is returned.  In case of failure, a
 * localised error message is returned to give the user hints about what
 * the problem might be.  This error message is not to be freed.
 */

#ifdef __FreeBSD__

#include <machine/apm_bios.h>

static const char *
apm_readinfo (BatteryStatus *status)
{
  /* This is how I read the information from the APM subsystem under
     FreeBSD.  Each time this functions is called (once every second)
     the APM device is opened, read from and then closed.
  */
  struct apm_info apminfo;
  int fd;

  if (DEBUG) g_print("apm_readinfo() (FreeBSD)\n");

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1)
    return ERR_OPEN_APMDEV;

  if (ioctl(fd, APMIO_GETINFO, &apminfo) == -1)
    err(1, "ioctl(APMIO_GETINFO)");

  close(fd);

  if(apminfo.ai_status == 0)
    return ERR_APM_E;

  status->present = TRUE;
  status->on_ac_power = apminfo.ai_acline ? 1 : 0;
  status->state = apminfo.ai_batt_stat;
  status->percent = apminfo.ai_batt_life;
  status->charging = (status->state == 3) ? TRUE : FALSE;
  status->minutes = apminfo.ai_batt_time;

  return NULL;
}

#elif defined(__NetBSD__) || defined(__OpenBSD__)

#include <sys/param.h>
#include <machine/apmvar.h>

#if defined(__NetBSD__)
#define APMDEVICE "/dev/apm"
#endif

static const char *
apm_readinfo (BatteryStatus *status)
{
  /* Code for OpenBSD by Joe Ammond <jra@twinight.org>. Using the same
     procedure as for FreeBSD.
  */
  struct apm_info apminfo;
  int fd;

#if defined(__NetBSD__)
  if (DEBUG) g_print("apm_readinfo() (NetBSD)\n");
#else /* __OpenBSD__ */
  if (DEBUG) g_print("apm_readinfo() (OpenBSD)\n");
#endif

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1)
    return ERR_OPEN_APMDEV;
  if (ioctl(fd, APM_IOC_GETPOWER, &apminfo) == -1)
    err(1, "ioctl(APM_IOC_GETPOWER)");
  close(fd);

  status->present = TRUE;
  status->on_ac_power = apminfo.ac_state ? 1 : 0;
  status->state = apminfo.battery_state;
  status->percent = apminfo.battery_life;
  status->charging = (status->state == 3) ? TRUE : FALSE;
  status->minutes = apminfo.minutes_left;

  return NULL;
}

#elif __linux__

#include <apm.h>
#include "acpi-linux.h"

struct acpi_info acpiinfo;
gboolean using_acpi;
int acpi_count;
int acpiwatch;
struct apm_info apminfo;

// Declared in acpi-linux.c
gboolean acpi_linux_read(struct apm_info *apminfo, struct acpi_info *acpiinfo);

gboolean acpi_callback (GIOChannel * chan, GIOCondition cond, gpointer data)
{
  if (cond & (G_IO_ERR | G_IO_HUP)) {
    acpi_linux_cleanup(&acpiinfo);
    apminfo.battery_percentage = -1;
    return FALSE;
  }
  
  if (acpi_process_event(&acpiinfo)) {
    acpi_linux_read(&apminfo, &acpiinfo);
  }
  return TRUE;
}

static const char *
apm_readinfo (BatteryStatus *status)
{
  /* Code for Linux by Thomas Hood <jdthood@mail.com>. apm_read() will
     read from /proc/... instead and we do not need to open the device
     ourselves.
  */
  if (DEBUG) g_print("apm_readinfo() (Linux)\n");

  /* ACPI support added by Lennart Poettering <lennart@poettering.de> 10/27/2001
   * Updated by David Moore <dcm@acm.org> 5/29/2003 to poll less and
   *   use ACPI events. */
  if (using_acpi && acpiinfo.event_fd >= 0) {
    if (acpi_count <= 0) {
      /* Only call this one out of 30 calls to apm_readinfo() (every 30 seconds)
       * since reading the ACPI system takes CPU cycles. */
      acpi_count=30;
      acpi_linux_read(&apminfo, &acpiinfo);
    }
    acpi_count--;
  }
  /* If we lost the file descriptor with ACPI events, try to get it back. */
  else if (using_acpi) {
      if (acpi_linux_init(&acpiinfo)) {
          acpiwatch = g_io_add_watch (acpiinfo.channel,
              G_IO_IN | G_IO_ERR | G_IO_HUP,
              acpi_callback, NULL);
          acpi_linux_read(&apminfo, &acpiinfo);
      }
  }
  else
    apm_read(&apminfo);

  status->present = TRUE;
  status->on_ac_power = apminfo.ac_line_status ? 1 : 0;
  status->state = apminfo.battery_status;
  status->percent = (guint) apminfo.battery_percentage;
  status->charging = (apminfo.battery_flags & 0x8) ? TRUE : FALSE;
  status->minutes = apminfo.battery_time;

  return NULL;
}

#else

static const char *
apm_readinfo (BatteryStatus *status)
{
  status->present = FALSE;
  status->on_ac_power = 1;
  status->state = BATTERY_HIGH;
  status->percent = 100;
  status->charging = FALSE;
  status->minutes = 0;

  return ERR_NO_SUPPORT;
}

#endif

/*
 * End platform-specific code.
 */

/*
 * power_management_getinfo
 *
 * Main interface to the power management code.  Fills 'status' for you so
 * you don't have to worry about platform-specific details.
 *
 * In the case of success, NULL is returned.  In case of failure, a
 * localised error message is returned to give the user hints about what
 * the problem might be.  This error message is not to be freed.
 */
const char *
power_management_getinfo( BatteryStatus *status )
{
  const char *retval;

  retval = apm_readinfo( status );

  if(status->state > 3) {
    status->state = 0;
    status->present = FALSE;
  }

  if(status->percent == (guint)-1) {
    status->percent = 0;
    status->present = FALSE;
  }

  if(status->percent > 100)
    status->percent = 100;

  if(status->percent == 100)
    status->charging = FALSE;

  if(!status->on_ac_power)
    status->charging = FALSE;

  return retval;
}

/*
 * power_management_initialise
 *
 * Initialise the power management code.  Call this before you call anything
 * else.
 *
 * In the case of success, NULL is returned.  In case of failure, a
 * localised error message is returned to give the user hints about what
 * the problem might be.  This error message is not to be freed.
 */
const char *
power_management_initialise( void )
{
#ifdef __linux__
  struct stat statbuf;

  if (acpi_linux_init(&acpiinfo)) {
    using_acpi = TRUE;
    acpi_count = 0;
  }
  else
    using_acpi = FALSE;

  /* If neither ACPI nor APM could be read, but ACPI does seem to be
   * installed, warn the user how to get ACPI working. */
  if (!using_acpi && (apm_exists() == 1) &&
          (stat("/proc/acpi", &statbuf) == 0)) {
    using_acpi = TRUE;
    acpi_count = 0;
    return ERR_ACPID;
  }

  /* Watch for ACPI events and handle them immediately with acpi_callback(). */
  if (using_acpi && acpiinfo.event_fd >= 0) {
    acpiwatch = g_io_add_watch (acpiinfo.channel,
        G_IO_IN | G_IO_ERR | G_IO_HUP,
        acpi_callback, NULL);
  }
#endif

  return NULL;
}

/*
 * power_management_cleanup
 *
 * Perform any cleanup that might be required.
 */
void
power_management_cleanup( void )
{
#ifdef __linux__
  if (using_acpi)
  {
    if (acpiwatch != 0)
      g_source_remove(acpiwatch);
     acpiwatch = 0;
     acpi_linux_cleanup(&acpiinfo);
  }
#endif
}
