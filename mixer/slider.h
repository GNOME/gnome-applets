/*
 *   smaller version of the slider widget
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SLIDER_H__
#define __SLIDER_H__

#include <gnome.h>

BEGIN_GNOME_DECLS

/* widget related macros and structures */
#define SLIDER(obj)         GTK_CHECK_CAST(obj, slider_get_type(), Slider)
#define SLIDER_CLASS(klass) GTK_CHECK_CAST_CLASS(klass, slider_get_type(), SliderClass)
#define IS_SLIDER(obj) GTK_CHECK_TYPE(obj, slider_get_type())

typedef struct _Slider       Slider;
typedef struct _SliderClass  SliderClass;

struct _Slider {
    GtkVScale parent;
};

struct _SliderClass {
    GtkVScaleClass parent_class;
};




guint       slider_get_type       (void);
GtkWidget  *slider_new            (void);

END_GNOME_DECLS


#endif /* __GNOME_HELPWIN_H__ */


