/* GNOME FVWM PAger
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

#include "pager-win.h"

enum {
  ARG_0,
  ARG_WX1,
  ARG_WY1,
  ARG_WX2,
  ARG_WY2,
  ARG_XID
};


static void gnome_canvas_win_set_arg    (GtkObject          *object,
					GtkArg             *arg,
					guint               arg_id);
static void gnome_canvas_win_get_arg    (GtkObject          *object,
					GtkArg             *arg,
					guint               arg_id);

static void gnome_canvas_win_class_init (GnomeCanvasWinClass *class);

static void   gnome_canvas_win_draw   (GnomeCanvasItem *item, GdkDrawable *drawable, int x, int y, int width, int height);
static double gnome_canvas_win_point  (GnomeCanvasItem *item, double x, double y, int cx, int cy,
				       GnomeCanvasItem **actual_item);


GtkType
gnome_canvas_win_get_type (void)
{
	static GtkType win_type = 0;

	if (!win_type) {
		GtkTypeInfo win_info = {
			"GnomeCanvasWin",
			sizeof (GnomeCanvasWin),
			sizeof (GnomeCanvasWinClass),
			(GtkClassInitFunc) gnome_canvas_win_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		win_type = gtk_type_unique (gnome_canvas_rect_get_type (), &win_info);
	}

	return win_type;
}

static void
gnome_canvas_win_class_init (GnomeCanvasWinClass *class)
{
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	gtk_object_add_arg_type("GnomeCanvasWin::wx1", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_WX1);
	gtk_object_add_arg_type("GnomeCanvasWin::wx2", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_WX2);
	gtk_object_add_arg_type("GnomeCanvasWin::wy1", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_WY1);
	gtk_object_add_arg_type("GnomeCanvasWin::wy2", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_WY2);
	gtk_object_add_arg_type("GnomeCanvasWin::xid", GTK_TYPE_INT,    GTK_ARG_READWRITE, ARG_XID);
	item_class = (GnomeCanvasItemClass *) class;

	object_class->set_arg = gnome_canvas_win_set_arg;
}

static void
gnome_canvas_win_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
  GnomeCanvasItem *item;
  GnomeCanvasWin  *re;
  
  item = GNOME_CANVAS_ITEM (object);
  re = GNOME_CANVAS_WIN (object);
  
  switch (arg_id) {
  case ARG_WX1:
    re->real_x1 = GTK_VALUE_DOUBLE (*arg);
    gnome_canvas_item_set(item, "GnomeCanvasRE::x1", re->real_x1, NULL);
    break;
  case ARG_WX2:
    re->real_x2 = GTK_VALUE_DOUBLE (*arg);
    gnome_canvas_item_set(item, "GnomeCanvasRE::x2", re->real_x2, NULL);
    break;
  case ARG_WY1:
    re->real_y1 = GTK_VALUE_DOUBLE (*arg);
    gnome_canvas_item_set(item, "GnomeCanvasRE::y1", re->real_y1, NULL);
    break;
  case ARG_WY2:
    re->real_y2 = GTK_VALUE_DOUBLE (*arg);
    gnome_canvas_item_set(item, "GnomeCanvasRE::y2", re->real_y2, NULL);
    break;
  case ARG_XID:
    re->xid = GTK_VALUE_INT(*arg);
    break;
  }
}
