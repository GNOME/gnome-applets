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

#ifndef __HSLIDER_H__
#define __HSLIDER_H__

#include <gnome.h>

BEGIN_GNOME_DECLS

/* widget related macros and structures */
#define HSLIDER(obj)         GTK_CHECK_CAST(obj, hslider_get_type(), HSlider)
#define HSLIDER_CLASS(klass) GTK_CHECK_CAST_CLASS(klass, hslider_get_type(), HSliderClass)
#define IS_SLIDER(obj) GTK_CHECK_TYPE(obj, hslider_get_type())

typedef struct _HSlider       HSlider;
typedef struct _HSliderClass  HSliderClass;

struct _HSlider {
    GtkHScale parent;
};

struct _HSliderClass {
    GtkHScaleClass parent_class;
};




guint       hslider_get_type       (void);
GtkWidget  *hslider_new            (void);

END_GNOME_DECLS


#endif /* __GNOME_HELPWIN_H__ */


