/*   my version of a smaller slider
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <glib.h>
#include <gnome.h>

#include "vslider.h"

/****************/
/* widget stuff */
/****************/


static void
vslider_class_init(VSliderClass *vsliderclass)
{
	GTK_SCALE_CLASS(vsliderclass)->slider_length = 7;
}

static void
vslider_init(VSlider *vslider)
{
}


guint
vslider_get_type(void)
{
	static guint vslider_type = 0;

	if (!vslider_type) {
		GtkTypeInfo vslider_info = {
			"Slider",
			sizeof(VSlider),
			sizeof(VSliderClass),
			(GtkClassInitFunc) vslider_class_init,
			(GtkObjectInitFunc) vslider_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		vslider_type = gtk_type_unique(gtk_vscale_get_type(),
						    &vslider_info);
	}
	return vslider_type;
}

GtkWidget *
vslider_new(void)
{
        return GTK_WIDGET(gtk_type_new(vslider_get_type()));
}

