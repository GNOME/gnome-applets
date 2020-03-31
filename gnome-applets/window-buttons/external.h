/*
 * Copyright (C) 2010 Andrej Belcijan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Andrej Belcijan <{andrejx} at {gmail.com}>
 */

#include <glib.h>
#include <gio/gio.h>

#define GSETTINGS_METACITY_SCHEMA			"org.gnome.desktop.wm.preferences"
#define CFG_METACITY_BUTTON_LAYOUT			"button-layout"

#define GSETTINGS_COMPIZ_SCHEMA				"org.compiz"
#define CFG_COMPIZ_CURRENT_PROFILE			"current-profile"
#define GSETTINGS_COMPIZ_DECOR_SCHEMA		"org.compiz.decor"
#define GSETTINGS_COMPIZ_DECOR_PATH			"/org/compiz/profiles/%s/plugins/decor/"
#define CFG_COMPIZ_DECORATION_MATCH			"decoration-match"
#define CFG_COMPIZ_DECORATION_MATCH_VALUE	"!state=maxvert"
