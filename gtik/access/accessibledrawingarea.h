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

#ifndef __ACCESSIBLE_DRAWING_AREA_H__
#define __ACCESSIBLE_DRAWING_AREA_H__

#include <gtk/gtkaccessible.h>
#include <libgail-util/gailtextutil.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ACCESSIBLE_TYPE_DRAWING_AREA                     (accessible_drawing_area_get_type ())
#define ACCESSIBLE_DRAWING_AREA(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), ACCESSIBLE_TYPE_DRAWING_AREA, AccessibleDrawingArea))
#define ACCESSIBLE_DRAWING_AREA_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), ACCESSIBLE_TYPE_DRAWING_AREA, AccessibleDrawingAreaClass))
#define ACCESSIBLE_IS_DRAWING_AREA(obj)                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ACCESSIBLE_TYPE_DRAWING_AREA))
#define ACCESSIBLE_IS_DRAWING_AREA_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE ((klass), ACCESSIBLE_TYPE_DRAWING_AREA))
#define ACCESSIBLE_DRAWING_AREA_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), ACCESSIBLE_TYPE_DRAWING_AREA, AccessibleDrawingAreaClass))

typedef struct _AccessibleDrawingArea        AccessibleDrawingArea;
typedef struct _AccessibleDrawingAreaClass   AccessibleDrawingAreaClass;

struct _AccessibleDrawingArea
{
	GtkAccessible parent;
	GailTextUtil *textutil;
};

struct _AccessibleDrawingAreaClass
{
	GtkAccessibleClass parent_class;
};

GType accessible_drawing_area_get_type (void);
AtkObject* accessible_drawing_area_new (GtkWidget *widget);
extern	gchar* gtik_get_text(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ACCESSIBLE_DRAWING_AREA_H__ */
