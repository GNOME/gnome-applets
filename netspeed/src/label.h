/*
 * Copyright (C) 2015 Alberts Muktupāvels
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
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 */

#ifndef NETSPEED_LABEL_H
#define NETSPEED_LABEL_H

#include <gtk/gtk.h>

#define NETSPEED_TYPE_LABEL netspeed_label_get_type ()
G_DECLARE_FINAL_TYPE (NetspeedLabel, netspeed_label,
                      NETSPEED, LABEL,
                      GtkLabel)

GtkWidget *netspeed_label_new             (void);
void       netspeed_label_set_dont_shrink (NetspeedLabel *label,
                                           gboolean       dont_shrink);

#endif
