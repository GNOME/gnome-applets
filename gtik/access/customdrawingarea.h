/*
 * Copyright 2002 Sun Microsystems Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __CUSTOM_DRAWING_AREA_H__
#define __CUSTOM_DRAWING_AREA_H__

#include <glib-object.h>
#include <gtk/gtkdrawingarea.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CUSTOM_TYPE_DRAWING_AREA            (custom_drawing_area_get_type ())
#define CUSTOM_DRAWING_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUSTOM_TYPE_DRAWING_AREA, CustomDrawingArea))
#define CUSTOM_DRAWING_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CUSTOM_TYPE_DRAWING_AREA, CustomDrawingAreaClass))

typedef struct _CustomDrawingArea       CustomDrawingArea;
typedef struct _CustomDrawingAreaClass  CustomDrawingAreaClass;

struct _CustomDrawingArea
{
	GtkDrawingArea drawing_area;
};

struct _CustomDrawingAreaClass
{
	GtkDrawingAreaClass parent_class;
};


GType              custom_drawing_area_get_type   (void);
CustomDrawingArea* custom_drawing_area_new        (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CUSTOM_DRAWING_AREA_H__ */
