/* fvwm-pager
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
#ifndef __pager_win_h__
#define __pager_win_h__ 1

#include <gnome.h>

#define GNOME_TYPE_CANVAS_WIN             (gnome_canvas_win_get_type ())
#define GNOME_CANVAS_WIN(obj)             (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_WIN, GnomeCanvasWin))
#define GNOME_CANVAS_WIN_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_WIN, GnomeCanvasWinClass))
#define GNOME_IS_CANVAS_WIN(obj)          (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_WIN))
#define GNOME_IS_CANVAS_WIN_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_WIN))


typedef struct _GnomeCanvasWin GnomeCanvasWin;
typedef struct _GnomeCanvasWinClass GnomeCanvasWinClass;

struct _GnomeCanvasWin {
  GnomeCanvasRect re;
  double real_x1;
  double real_y1;
  
  double real_x2;
  double real_y2;
  gint   xid;
};

struct _GnomeCanvasWinClass {
	GnomeCanvasRect parent_class;
};

GtkType    gnome_canvas_win_get_type (void);


#endif
