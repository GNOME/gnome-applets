/*
 * Copyright (C) 2018-2020 Alberts Muktupāvels
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

#ifndef GA_COMMAND_H
#define GA_COMMAND_H

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GA_TYPE_COMMAND (ga_command_get_type ())
G_DECLARE_FINAL_TYPE (GaCommand, ga_command, GA, COMMAND, GObject)

GaCommand  *ga_command_new          (const char    *command,
                                     unsigned int   interval,
                                     GError       **error);

const char *ga_command_get_command  (GaCommand     *self);

void        ga_command_set_interval (GaCommand     *self,
                                     unsigned int   interval);

void        ga_command_start        (GaCommand     *self);

void        ga_command_stop         (GaCommand     *self);

void        ga_command_restart      (GaCommand     *self);

G_END_DECLS

#endif
