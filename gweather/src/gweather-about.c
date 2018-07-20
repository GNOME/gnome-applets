/*
 * Copyright (C) Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gweather-about.h"

void gweather_about_run (GWeatherApplet *gw_applet)
{
    
    static const gchar *authors[] = {
        "Todd Kulesza <fflewddur@dropline.net>",
        "Philip Langdale <philipl@mail.utexas.edu>",
        "Ryan Lortie <desrt@desrt.ca>",
        "Davyd Madeley <davyd@madeley.id.au>",
        "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
        "Kevin Vandersloot <kfv101@psu.edu>",
        NULL
    };

    const gchar *documenters[] = {
        "Dan Mueth <d-mueth@uchicago.edu>",
        "Spiros Papadimitriou <spapadim+@cs.cmu.edu>",
	"Sun GNOME Documentation Team <gdocteam@sun.com>",
	"Davyd Madeley <davyd@madeley.id.au>",
	NULL
    };

    gtk_show_about_dialog (NULL,
	"version",	VERSION,
	"copyright",	_("\xC2\xA9 1999-2005 by S. Papadimitriou and others"),
	"comments",	_("A panel application for monitoring local weather "
			  "conditions."),
	"authors",	authors,
	"documenters",	documenters,
	"translator-credits",	_("translator-credits"),
	"logo-icon-name",	"weather-storm",
	NULL);
}
