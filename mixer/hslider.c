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

#include "hslider.h"

/****************/
/* widget stuff */
/****************/


static void
hslider_class_init(HSliderClass *hsliderclass)
{
	GTK_SCALE_CLASS(hsliderclass)->slider_length = 7;
}

static void
hslider_init(HSlider *hslider)
{
}


guint
hslider_get_type(void)
{
	static guint hslider_type = 0;

	if (!hslider_type) {
		GtkTypeInfo hslider_info = {
			"HSlider",
			sizeof(HSlider),
			sizeof(HSliderClass),
			(GtkClassInitFunc) hslider_class_init,
			(GtkObjectInitFunc) hslider_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		hslider_type = gtk_type_unique(gtk_hscale_get_type(),
						    &hslider_info);
	}
	return hslider_type;
}

GtkWidget *
hslider_new(void)
{
        return GTK_WIDGET(gtk_type_new(hslider_get_type()));
}

