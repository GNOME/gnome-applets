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

// ACPI battery read-out functions for Linux >= 2.4.12
// October 2001 by Lennart Poettering <lennart@poettering.de>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __linux__

#include <stdio.h>
#include <apm.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

// Returns the first line of file f which starts with "field: " in
// tmp. Return value is a pointer to the end of the string
static gchar *al_get_field(FILE *f, const gchar *field, gchar *tmp, int s)
{
  g_assert(f && field && tmp && s);

  while (!feof(f))
    {
      gchar *c, *e;
      
      if (!fgets(tmp, s, f)) break;
      
      if ((c = strchr(tmp, ':'))) // Is a colon separated line
        {
          *c = 0;
          if (strcmp(tmp, field) == 0) // It is the right line
            {
              c++; // Find the beginning of the data
              if (*c)
                c ++;
              c = c + strspn(c, " \t");
              
              if ((e = strchr(c, 0)) > tmp) // Remove NL from end
                {
                  e--;
                  if (*e == '\n')
                    *e = 0;
                }

              return c;
            }
          
        }
    }
  
  return NULL;
}

// Returns the same as the function above, but converted into an integer
static int al_get_field_int(FILE *f, const gchar *field)
{
  gchar tmp[256];
  gchar *p;
  
  g_assert(f && field);
 
  if (p = al_get_field(f, field, tmp, sizeof(tmp)))
    return atoi(p);
  else
    return 0;
}

// Fills out a classic apm_info structure with the data gathered from
// the ACPI kernel interface in /proc
gboolean acpi_linux_read(struct apm_info *apminfo)
{
  guint32 max_capacity, low_capacity, critical_capacity, remain;
  gboolean charging, ac_online;
  FILE *f;

  // apminfo.ac_line_status must be one when on ac power
  // apminfo.battery_status must be 0 for high, 1 for low, 2 for critical, 3 for charging
  // apminfo.battery_percentage must contain batter charge percentage
  // apminfo.battery_flags & 0x8 must be nonzero when charging
  
  g_assert(apminfo);

  max_capacity = 0;
  low_capacity = 0;
  critical_capacity = 0;

  if ((f = fopen("/proc/acpi/battery/1/info", "r")))
    {
      max_capacity = al_get_field_int(f, "Design Capacity");
      low_capacity = al_get_field_int(f, "Design Capacity Warning");
      critical_capacity = al_get_field_int(f, "Design Capacity Low");
      
      fclose(f);
    }
  
  if (!max_capacity)
    return FALSE;
  
  charging = FALSE;
  remain = 0;

  if ((f = fopen("/proc/acpi/battery/1/status", "r")))
    {
      gchar *s;
      gchar tmp[256];

      if ((s = al_get_field(f, "State", tmp, sizeof(tmp))))
        charging = strcmp(s, "charging") == 0;
      
      remain = al_get_field_int(f, "Remaining Capacity");

      fclose(f);
    }
  
  ac_online = FALSE;
  
  if ((f = fopen("/proc/acpi/ac_adapter/0/status", "r")))
    {
      gchar *s;
      gchar tmp[256];      

      if ((s = al_get_field(f, "Status", tmp, sizeof(tmp))))
        ac_online = strcmp(s, "on-line") == 0;

      fclose(f);
    }

  apminfo->ac_line_status = ac_online ? 1 : 0;
  apminfo->battery_status = remain < low_capacity ? 1 : remain < critical_capacity ? 2 : 0;
  apminfo->battery_percentage = (int) (remain/(float)max_capacity*100);
  apminfo->battery_flags = charging ? 0x8 : 0;

  return TRUE;
}

#endif
