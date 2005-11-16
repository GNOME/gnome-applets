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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <sys/types.h>
#include <libgail-util/gailmisc.h>
#include <atk/atkobject.h>

#include "accessibledrawingarea.h"
#include "customdrawingarea.h"

static void accessible_drawing_area_class_init       (AccessibleDrawingAreaClass *klass);

static gint accessible_drawing_area_get_n_children   (AtkObject *obj);

static void accessible_drawing_area_real_initialize  (AtkObject *obj, gpointer data);
static void accessible_drawing_area_finalize         (GObject *object);

static void atk_text_interface_init                  (AtkTextIface *iface);

/* AtkText Interfaces */
static gchar* accessible_drawing_area_get_text (AtkText *text,
						      gint    start_pos,
						      gint    end_pos);
static gchar* accessible_drawing_area_get_text_before_offset (AtkText *text,
						      gint offset,
						      AtkTextBoundary bound_type,
						      gint *start_offset,
						      gint *end_offset);
static gchar* accessible_drawing_area_get_text_after_offset (AtkText *text,
						      gint offset,
						      AtkTextBoundary bound_type,
						      gint *start_offset,
						      gint *end_offset);
static gchar* accessible_drawing_area_get_text_at_offset (AtkText *text,
						      gint offset,
						      AtkTextBoundary boundary_type,
						      gint *start_offset,
						      gint *end_offset);
static gint accessible_drawing_area_get_character_count (AtkText *text);
static gunichar accessible_drawing_area_get_character_at_offset (AtkText *text,
						      gint offset);

static gpointer parent_class = NULL;

GType
accessible_drawing_area_get_type (void)
{
	static GType type = 0;

	if (!type)
	{
		static GTypeInfo tinfo =
		{
			0, /* class size */
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) accessible_drawing_area_class_init, /* class init */
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			0, /* instance size */
			0, /* nb preallocs */
			(GInstanceInitFunc)NULL, /* instance init */
			NULL /* value table */
		};

		static const GInterfaceInfo atk_text_info =
		{
			(GInterfaceInitFunc) atk_text_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		/*
		 * Figure out the size of the class and instance
		 * we are deriving from
		 */
		AtkObjectFactory *factory;
		GType derived_type;
		GTypeQuery query;
		GType derived_atk_type;

		derived_type = g_type_parent (CUSTOM_TYPE_DRAWING_AREA);
		factory = atk_registry_get_factory (atk_get_default_registry(),
					 derived_type);
		derived_atk_type = atk_object_factory_get_accessible_type (factory);
		g_type_query (derived_atk_type, &query);
		tinfo.class_size = query.class_size;
		tinfo.instance_size = query.instance_size;

		type = g_type_register_static (derived_atk_type,
					 "AccessibleDrawingArea", &tinfo, 0);

		g_type_add_interface_static (type, ATK_TYPE_TEXT, &atk_text_info);

	}
	
	return type;
}

static void
accessible_drawing_area_class_init (AccessibleDrawingAreaClass         *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

	g_return_if_fail (class != NULL);
	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = accessible_drawing_area_finalize;

	class->get_n_children = accessible_drawing_area_get_n_children;
	class->initialize = accessible_drawing_area_real_initialize;
}

static void
accessible_drawing_area_real_initialize (AtkObject         *obj,
				gpointer         data)
{
	AccessibleDrawingArea *accessible_drawing_area;
	CustomDrawingArea *custom_draw_area;
	GtkAccessible *accessible;

	g_return_if_fail (obj != NULL);

	accessible_drawing_area = ACCESSIBLE_DRAWING_AREA(obj);

	custom_draw_area = CUSTOM_DRAWING_AREA(data);
	g_return_if_fail (custom_draw_area != NULL);

	accessible = GTK_ACCESSIBLE (obj);
	g_return_if_fail (accessible != NULL);
	accessible->widget = GTK_WIDGET (custom_draw_area);

	accessible_drawing_area->textutil = gail_text_util_new();
}

AtkObject *
accessible_drawing_area_new (GtkWidget *widget)
{
	GObject *object;
	AtkObject *accessible;

	object = g_object_new (ACCESSIBLE_TYPE_DRAWING_AREA, NULL);
	g_return_val_if_fail (object != NULL, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, widget);

	accessible->role = ATK_ROLE_DRAWING_AREA;

	return accessible;
}

/*
 * Report the number of children as 0
 */
static gint
accessible_drawing_area_get_n_children (AtkObject* obj)
{
	return 0;
}

static void
accessible_drawing_area_finalize (GObject *object)
{
	AccessibleDrawingArea *drawingarea = ACCESSIBLE_DRAWING_AREA(object);

	g_object_unref (drawingarea->textutil);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar*
accessible_drawing_area_get_text (AtkText         *text,
				gint         start_pos,
				gint         end_pos)
{
	AccessibleDrawingArea *access_draw_area;
	gchar *text_string;
	gchar *utf8;

	g_return_val_if_fail (ACCESSIBLE_IS_DRAWING_AREA (text), NULL);
	access_draw_area = ACCESSIBLE_DRAWING_AREA (text);
	g_return_val_if_fail (access_draw_area->textutil, NULL);

	text_string = gtik_get_text ();
	utf8 = g_locale_to_utf8 (text_string, -1, NULL, NULL, NULL);
	gail_text_util_text_setup (access_draw_area->textutil, utf8);
	g_free (text_string);
	g_free (utf8);

 	return gail_text_util_get_substring (access_draw_area->textutil,
				start_pos, end_pos);
}

static gchar*
accessible_drawing_area_get_text_before_offset(AtkText         *text,
				gint            offset,
				AtkTextBoundary boundary_type,
				gint            *start_offset,
				gint            *end_offset)
{
	return gail_text_util_get_text (ACCESSIBLE_DRAWING_AREA (text)->textutil,
				NULL, GAIL_BEFORE_OFFSET,
				boundary_type, offset,
				start_offset, end_offset);
}

static gchar*
accessible_drawing_area_get_text_after_offset (AtkText         *text,
				gint            offset,
				AtkTextBoundary boundary_type,
				gint            *start_offset,
				gint            *end_offset)
{
	return gail_text_util_get_text (ACCESSIBLE_DRAWING_AREA (text)->textutil,
				NULL, GAIL_AFTER_OFFSET,
				boundary_type, offset,
				start_offset, end_offset);
}

static gchar*
accessible_drawing_area_get_text_at_offset (AtkText         *text,
				gint            offset,
				AtkTextBoundary boundary_type,
				gint            *start_offset,
				gint            *end_offset)
{
	return gail_text_util_get_text (ACCESSIBLE_DRAWING_AREA (text)->textutil,
				NULL, GAIL_AT_OFFSET,
				boundary_type, offset,
				start_offset, end_offset);
}

static gint
accessible_drawing_area_get_character_count (AtkText         *text)
{
	gchar *text_string;
	gchar *utf8;
	gint len;

	text_string = gtik_get_text ();
	utf8 = g_locale_to_utf8 (text_string, -1, NULL, NULL, NULL);
	len = g_utf8_strlen (utf8, -1);
	g_free (text_string);
	g_free (utf8);
	
	return len;
}

static gunichar
accessible_drawing_area_get_character_at_offset (AtkText         *text,
				gint         offset)
{
	gchar *text_string;
	gchar *index;
	gchar *utf8;
	gunichar c;

	text_string = gtik_get_text ();
	utf8 = g_locale_to_utf8 (text_string, -1, NULL, NULL, NULL);
	index = g_utf8_offset_to_pointer (utf8, offset);
	c = g_utf8_get_char (index);
	g_free (text_string);
	g_free (utf8);

	return c;
}

static void
atk_text_interface_init (AtkTextIface         *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_text = accessible_drawing_area_get_text;
	iface->get_text_before_offset = accessible_drawing_area_get_text_before_offset;
	iface->get_text_after_offset = accessible_drawing_area_get_text_after_offset;
	iface->get_text_at_offset = accessible_drawing_area_get_text_at_offset;
	iface->get_character_count = accessible_drawing_area_get_character_count;
	iface->get_character_at_offset = accessible_drawing_area_get_character_at_offset;
}
