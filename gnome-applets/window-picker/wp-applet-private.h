/*
 * Copyright (C) 2014 Sebastian Geiger
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
 *     Sebastian Geiger <sbastig@gmx.net>
 */

#ifndef WP_APPLET_PRIVATE_H
#define WP_APPLET_PRIVATE_H

#include <libwnck/libwnck.h>
#include "wp-applet.h"

G_BEGIN_DECLS

WnckScreen *wp_applet_get_default_screen  (WpApplet *self);

G_END_DECLS

#endif
