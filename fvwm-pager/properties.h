/* fvwm-pager
 *
 * Copyright (C) 1998 Michael Lausch
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __properties_h__
#define __properties_h__ 1

#include <applet-widget.h>

typedef struct _Pager_props
{
  gint      width;
  gint      height;
  gchar*    active_desk_color;
  gchar*    inactive_desk_color;
  gchar*    active_win_color;
  gchar*    inactive_win_color;
  gchar*    default_cursor;
  gchar*    window_drag_cursor;
}PagerProps;

extern PagerProps pager_props;

void pager_properties_dialog(AppletWidget* applet, gpointer data);
void load_fvwmpager_properties(gchar* path, PagerProps* prop);
void save_fvwmpager_properties(gchar* path, PagerProps* prop);


#endif
