/*  panel-menu-config.h
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _PANEL_MENU_CONFIG_H_
#define _PANEL_MENU_CONFIG_H_

#include <glib.h>
#include <libbonobo.h>
#include <libbonoboui.h>
#include <panel-applet.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "panel-menu.h"

G_BEGIN_DECLS

gboolean panel_menu_config_load_prefs (PanelMenu *panel_menu);
gboolean panel_menu_config_load_layout (PanelMenu *panel_menu);
void panel_menu_config_load (PanelMenu *panel_menu, const gchar *layout);
void panel_menu_config_save_prefs (PanelMenu *panel_menu);
void panel_menu_config_save_layout (PanelMenu *panel_menu);
void panel_menu_config_save (PanelMenu *panel_menu);
void panel_applet_gconf_set_string_list (PanelApplet *applet,
					 const char *key,
					 GList *strings);
GList *panel_applet_gconf_get_string_list (PanelApplet *applet,
					   const char *key);
void _gconf_client_clean_dir (GConfClient *client, const gchar *dir);

G_END_DECLS

#endif
