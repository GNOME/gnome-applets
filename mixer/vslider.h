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
#define VSLIDER(obj)         GTK_CHECK_CAST(obj, vslider_get_type(), VSlider)
#define VSLIDER_CLASS(klass) GTK_CHECK_CAST_CLASS(klass, vslider_get_type(), VSliderClass)
#define IS_VSLIDER(obj) GTK_CHECK_TYPE(obj, vslider_get_type())

typedef struct _VSlider       VSlider;
typedef struct _VSliderClass  VSliderClass;

struct _VSlider {
    GtkVScale parent;
    
};

struct _VSliderClass {
    GtkVScaleClass parent_class;
};




guint       vslider_get_type       (void);
GtkWidget  *vslider_new            (void);

END_GNOME_DECLS


#endif /* __GNOME_HELPWIN_H__ */


