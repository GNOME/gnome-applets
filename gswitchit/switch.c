/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gdk/gdkx.h>
#include <gnome.h>

#include "gswitchit-applet.h"

extern gboolean
gkb_factory (PanelApplet * applet, const gchar * iid, gpointer data);

static gboolean
GKBAppletNew (PanelApplet * applet)
{
	fprintf (stderr, "No xkb found, fallback to gkb\n");
	return gkb_factory (applet, "OAFIID:GNOME_KeyboardApplet", NULL);
}

static gboolean
CheckXKB (void)
{
	gboolean have_xkb = FALSE;
	int opcode, errorBase, major, minor, xkbEventBase;

	gdk_error_trap_push ();
	have_xkb = XkbQueryExtension (GDK_DISPLAY (),
				      &opcode, &xkbEventBase, &errorBase,
				      &major, &minor)
	    && XkbUseExtension (GDK_DISPLAY (), &major, &minor);
	XSync (GDK_DISPLAY (), FALSE);
	gdk_error_trap_pop ();

	return have_xkb;
}

static gboolean
KeyboardAppletFactory (PanelApplet * applet,
		       const gchar * iid, gpointer data)
{
	if (!strcmp (iid, "OAFIID:GNOME_KeyboardApplet")) {
		if (CheckXKB ())
			return GSwitchItAppletNew (applet);
		else
			return GKBAppletNew (applet);
	}
	return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_KeyboardApplet_Factory", PANEL_TYPE_APPLET, "command-line", "0", KeyboardAppletFactory, NULL)	//data
