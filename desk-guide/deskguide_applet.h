/* GNOME Desktop Guide
 * Copyright (C) 1999 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#define G_LOG_DOMAIN "DeskGuide-Applet"

#include "applet-widget.h"
#include "gwmh.h"


#define DEFAULT_SIZE 38
#define MAX_DESKTOPS 32

#define DESK            (gwmh_desk_get_config ())
#define N_DESKTOPS      (DESK->n_desktops)
#define N_HAREAS        (DESK->n_hareas)
#define N_VAREAS        (DESK->n_Vareas)
#define CURRENT_DESKTOP (DESK->current_desktop)

typedef struct _ConfigItem ConfigItem;
struct _ConfigItem
{
  gchar   *path;
  gpointer value;
  gint     min;
  gint     max;
  gchar   *name;
  gpointer tmp_value;
};

/* internationalization macro */
#define TRANSL(stuff)	_ (stuff)

#define CONFIG_PAGE(name)				{ NULL, 0, -2, -2, name, 0 }
#define CONFIG_SECTION(path, name)			{ #path, 0, -2, -2, name, 0 }
#define CONFIG_BOOL(path, default, name)		{ #path, GINT_TO_POINTER (default), -1, -1, name, GINT_TO_POINTER (default) }
#define	CONFIG_RANGE(path, default, min, max, name)	{ #path, GINT_TO_POINTER (default), min, max, name, GINT_TO_POINTER (default) }

#define BOOL_CONFIG(path)	GPOINTER_TO_INT (gp_config_find_value (#path))
#define RANGE_CONFIG(path)	GPOINTER_TO_INT (gp_config_find_value (#path))


