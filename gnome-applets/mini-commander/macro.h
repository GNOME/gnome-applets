/*
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MACRO_H__
#define __MACRO_H__

#include <glib.h>

G_BEGIN_DECLS

#include "mini-commander-applet-private.h"

void      mc_macro_expand_command    (MCData *mc,
				      char   *command);
int       mc_macro_prefix_len        (MCData *mc,
				      char   *command);
int       mc_macro_prefix_len_wspace (MCData *mc,
				      char   *command);
char     *mc_macro_get_prefix        (MCData *mc,
				      char   *command);

G_END_DECLS

#endif /* __MACRO_H__ */
