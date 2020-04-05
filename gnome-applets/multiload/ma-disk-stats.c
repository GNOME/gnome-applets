/*
 * Copyright (C) 2020 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "ma-disk-stats.h"

#include <stdio.h>

static void
get_usage (const char    *filename,
           unsigned long *sectors_read,
           unsigned long *sectors_written)
{
  FILE *file;
  int result;
  unsigned long read_ios;
  unsigned long read_sectors;
  unsigned long write_ios;
  unsigned long write_sectors;

  *sectors_read = 0;
  *sectors_written = 0;

  file = fopen (filename, "r");
  if (file == NULL)
    return;

  result = fscanf (file,
                   /* read I/Os, merges, sectors, ticks */
                   "%lu %*u %lu %*u "
                   /* write I/Os, merges, sectors, ticks */
                   "%lu %*u %lu %*u "
                   /* in flight, io ticks, time in queue */
                   "%*u %*u %*u "
                   /* discard I/Os, merges, sectors, ticks */
                   "%*u %*u %*u %*u "
                   /* flush I/Os, flush ticks, */
                   "%*u %*u",
                   &read_ios,
                   &read_sectors,
                   &write_ios,
                   &write_sectors);

  if (result == 4 && read_ios != 0 && write_ios != 0)
    {
      *sectors_read = read_sectors;
      *sectors_written = write_sectors;
    }

  fclose (file);
}

void
ma_disk_stats_get_usage (unsigned long *sectors_read,
                         unsigned long *sectors_written)
{
  GDir *dir;
  const char *name;

  *sectors_read = 0;
  *sectors_written = 0;

  dir = g_dir_open ("/sys/block", 0, NULL);
  if (dir == NULL)
    return;

  while ((name = g_dir_read_name (dir)) != 0)
    {
      char *filename;
      unsigned long block_sectors_read;
      unsigned long block_sectors_written;

      filename = g_strdup_printf ("/sys/block/%s/stat", name);
      get_usage (filename, &block_sectors_read, &block_sectors_written);
      g_free (filename);

      *sectors_read += block_sectors_read;
      *sectors_written += block_sectors_written;
    }

  g_dir_close (dir);
}
