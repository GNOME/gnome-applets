/* battstat        A GNOME battery meter for laptops. 
 * Copyright (C) 2000 by Jörgen Pehrson <jp@spektr.eu.org>
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

/*
 * ACPI battery read-out functions for Linux >= 2.4.12
 * October 2001 by Lennart Poettering <lennart@poettering.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __linux__

#include <stdio.h>
#include <apm.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include "acpi-linux.h"

static GHashTable *
read_file (const char *file, char *buf, size_t bufsize)
{
  GHashTable *hash = NULL;

  int fd, len, i;
  char *key, *value;
  gboolean reading_key;

  fd = open (file, O_RDONLY);

  if (fd == -1) {
    return hash;
  }

  len = read (fd, buf, bufsize);

  close (fd);

  if (len < 0) {
    if (getenv ("BATTSTAT_DEBUG"))
      g_message ("Error reading %s: %s", file, g_strerror (errno));
    return hash;
  }

  hash = g_hash_table_new (g_str_hash, g_str_equal);

  for (i = 0, value = key = buf, reading_key = TRUE; i < len; i++) {
    if (buf[i] == ':' && reading_key) {
      reading_key = FALSE;
      buf[i] = '\0';
      value = buf + i + 1;
    } else if (buf[i] == '\n') {
      reading_key = TRUE;
      buf[i] = '\0';
      /* g_message ("Read: %s => %s\n", key, value); */
      g_hash_table_insert (hash, key, g_strstrip (value));
      key = buf + i + 1;
    } else if (reading_key) {
      /* in acpi 20020214 it switched to lower-case proc
       * entries.  fixing this up here simplifies the
       * code.
       */
      buf[i] = g_ascii_tolower (buf[i]);
    }
  }

  return hash;
}

#if 0
static gboolean
read_bool (GHashTable *hash, const char *key)
{
  char *s;

  g_return_val_if_fail (hash, FALSE);
  g_return_val_if_fail (key, FALSE);

  s = g_hash_table_lookup (hash, key);
  return s && (*s == 'y');
}
#endif

static long
read_long (GHashTable *hash, const char *key)
{
  char *s;

  g_return_val_if_fail (hash, 0);
  g_return_val_if_fail (key, 0);

  s = g_hash_table_lookup (hash, key);
  return s ? strtol (s, NULL, 10) : 0;
}

static gulong
read_ulong (GHashTable *hash, const char *key)
{
  char *s;

  g_return_val_if_fail (hash, 0);
  g_return_val_if_fail (key, 0);

  s = g_hash_table_lookup (hash, key);
  return s ? strtoul (s, NULL, 10) : 0;
}

static const char *
read_string (GHashTable *hash, const char *key)
{
  return g_hash_table_lookup (hash, key);
}

/*
 * Fills out a classic apm_info structure with the data gathered from
 * the ACPI kernel interface in /proc
 */
gboolean acpi_linux_read(struct apm_info *apminfo)
{
  guint32 max_capacity, low_capacity, critical_capacity, remain;
  gboolean charging, ac_online;
  gulong acpi_ver;
  char buf[BUFSIZ];
  GHashTable *hash;
  const char *ac_state_state, *charging_state;
  char batt_info[60], batt_state[60], ac_state[60];
  DIR * procdir;
  struct dirent * procdirentry;

  /*
   * apminfo.ac_line_status must be one when on ac power
   * apminfo.battery_status must be 0 for high, 1 for low, 2 for critical, 3 for charging
   * apminfo.battery_percentage must contain batter charge percentage
   * apminfo.battery_flags & 0x8 must be nonzero when charging
   */
  
  g_assert(apminfo);

  max_capacity = 0;
  low_capacity = 0;
  critical_capacity = 0;
  charging = FALSE;
  remain = 0;
  ac_online = FALSE;

  hash = read_file ("/proc/acpi/info", buf, sizeof (buf));
  if (!hash)
    return FALSE;

  acpi_ver = read_ulong (hash, "version");
  g_hash_table_destroy (hash);

  if (acpi_ver < (gulong)20020208) {
    ac_state_state = "status";
    charging_state = "state";
  } else {
    ac_state_state = "state";
    charging_state = "charging state";
  }

  procdir=opendir("/proc/acpi/battery/");
  if (!procdir)
    return FALSE;

  while ((procdirentry=readdir(procdir)))
   {
    if (procdirentry->d_name[0]!='.')
     {
      strcpy(batt_info,"/proc/acpi/battery/");
      strcat(batt_info,procdirentry->d_name);
      strcat(batt_info,"/info");
      hash = read_file (batt_info, buf, sizeof (buf));
      if (hash)
       {
        max_capacity += read_long (hash, "last full capacity");
        low_capacity += read_long (hash, "design capacity warning");
        critical_capacity += read_long (hash, "design capacity low");
        g_hash_table_destroy (hash);
       }
      strcpy(batt_state,"/proc/acpi/battery/");
      strcat(batt_state,procdirentry->d_name);
      strcat(batt_state,"/state");
      hash = read_file (batt_state, buf, sizeof (buf));
      if (hash)
       {
        const char *s;
        if (!charging)
         {
          s = read_string (hash, charging_state);
          charging = s ? (strcmp (s, "charging") == 0) : 0;
         }
        remain += read_long (hash, "remaining capacity");
        g_hash_table_destroy (hash);
       }
     }
   }
  closedir(procdir);

  if (!max_capacity)
    return FALSE;

  procdir=opendir("/proc/acpi/ac_adapter/");
  if (!procdir)
    return FALSE;

  while ((procdirentry=readdir(procdir)))
   {
    if (procdirentry->d_name[0]!='.')
     {
      strcpy(ac_state,"/proc/acpi/ac_adapter/");
      strcat(ac_state,procdirentry->d_name);
      strcat(ac_state,"/");
      strcat(ac_state,ac_state_state);
      hash = read_file (ac_state, buf, sizeof (buf));
      if (hash && !ac_online)
       {
        const char *s;
        s = read_string (hash, ac_state_state);
        ac_online = s ? (strcmp (s, "on-line") == 0) : 0;
        g_hash_table_destroy (hash);
       }
     }
   }
  closedir(procdir);

  apminfo->ac_line_status = ac_online ? 1 : 0;
  apminfo->battery_status = remain < low_capacity ? 1 : remain < critical_capacity ? 2 : 0;
  apminfo->battery_percentage = (int) (remain/(float)max_capacity*100);
  apminfo->battery_flags = charging ? 0x8 : 0;

  return TRUE;
}

#endif
