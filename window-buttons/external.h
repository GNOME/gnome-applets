/***************************************************************************
 *            external.h
 *
 *  Wed Sep  29 01:56:39 2010
 *  Copyright  2010  Andrej Belcijan
 *  <{andrejx} at {gmail.com}>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <gconf/gconf-client.h>

#define GCONF_METACITY_THEME			"/apps/metacity/general/theme"
#define GCONF_METACITY_BUTTON_LAYOUT		"/apps/metacity/general/button_layout"
#define GCONF_COMPIZ_DECORATION_MATCH		"/apps/compiz-1/plugins/decor/screen0/options/decoration_match"
#define GCONF_COMPIZ_DECORATION_MATCH_OLD	"/apps/compiz/plugins/decoration/allscreens/options/decoration_match"
#define COMPIZ_DECORATION_MATCH			"!state=maxvert"
