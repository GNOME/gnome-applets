/*  panel-menu-add.h
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

#ifndef _PANEL_MENU_ADD_H_
#define _PANEL_MENU_ADD_H_

#include <glib.h>
#include <libbonobo.h>
#include <libbonoboui.h>
#include <panel-applet.h>

#include "panel-menu.h"

G_BEGIN_DECLS

void applet_add_cb(BonoboUIComponent *uic, PanelMenu *panel_menu, const gchar *verbname);

G_END_DECLS

#endif
