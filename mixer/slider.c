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

#include "slider.h"

/****************/
/* widget stuff */
/****************/


static void
slider_class_init(SliderClass *sliderclass)
{
	GTK_SCALE_CLASS(sliderclass)->slider_length = 7;
}

static void
slider_init(Slider *slider)
{
}


guint
slider_get_type(void)
{
	static guint slider_type = 0;

	if (!slider_type) {
		GtkTypeInfo slider_info = {
			"Slider",
			sizeof(Slider),
			sizeof(SliderClass),
			(GtkClassInitFunc) slider_class_init,
			(GtkObjectInitFunc) slider_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		slider_type = gtk_type_unique(gtk_vscale_get_type(),
						    &slider_info);
	}
	return slider_type;
}

GtkWidget *
slider_new(void)
{
        return GTK_WIDGET(gtk_type_new(slider_get_type()));
}

