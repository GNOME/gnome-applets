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

#include "customdrawingarea.h"

GType
custom_drawing_area_get_type (void)
{
	static GType drawing_area_type = 0;

	if (!drawing_area_type)
	{
		static const GTypeInfo drawing_area_info =
		{
			sizeof (CustomDrawingAreaClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			NULL,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (CustomDrawingArea),
			0,             /* n_preallocs */
			NULL,
		};

		drawing_area_type = g_type_register_static (GTK_TYPE_DRAWING_AREA,
				"CustomDrawingArea", &drawing_area_info, 0);
	}

	return drawing_area_type;
}

CustomDrawingArea *
custom_drawing_area_new (void)
{
	CustomDrawingArea *custom_draw_area;

	custom_draw_area = g_object_new (CUSTOM_TYPE_DRAWING_AREA, NULL);
	return CUSTOM_DRAWING_AREA (custom_draw_area);
}
