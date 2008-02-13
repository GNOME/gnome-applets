/*
 * trash-monitor.h: monitor the state of the trash directories.
 *
 * Copyright © 2008 Ryan Lortie, Matthias Clasen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __TRASH_MONITOR_H__
#define __TRASH_MONITOR_H__

#include <gio/gio.h>

typedef struct _TrashMonitor TrashMonitor;
typedef struct _TrashMonitorClass TrashMonitorClass;

#define         TRASH_TYPE_MONITOR       (trash_monitor_get_type ())
#define         TRASH_MONITOR(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                          TRASH_TYPE_MONITOR, TrashMonitor))

GType           trash_monitor_get_type          (void);
TrashMonitor   *trash_monitor_new               (void);

void            trash_monitor_empty_trash       (TrashMonitor *monitor,
				                 GCancellable *cancellable,
				                 gpointer      func,
				                 gpointer      user_data);

#endif
