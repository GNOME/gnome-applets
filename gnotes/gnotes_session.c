/*
Session management code for gnotes
By Russell Steinthal <rms39@columbia.edu> 1999

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include <config.h>
#include <gnome.h>
#include "gnote.h"
#include "gnotes_applet.h"

GnomeClient *newGnomeClient(void);

static int save_state (GnomeClient *client, gint phase, 
		       GnomeRestartStyle save_style,
		       gint shutdown, 
		       GnomeInteractStyle interact_style,
		       gint fast, gpointer client_data);

static void session_die (gpointer client_data);
/* shamelessly stolen from http://www.gnome.org/devel/start/sm.shtml 
*/

GnomeClient *newGnomeClient()
{
  gchar buf[1024];

  GnomeClient *client;

  client = gnome_client_new();

  if (!client)
    return NULL;

  getcwd((char *)buf,sizeof(buf));
  gnome_client_set_current_directory(client, (char *)buf);

  gtk_object_ref(GTK_OBJECT(client));
  gtk_object_sink(GTK_OBJECT(client));

  gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
		      GTK_SIGNAL_FUNC (save_state), NULL);
  gtk_signal_connect (GTK_OBJECT (client), "die",
		      GTK_SIGNAL_FUNC (session_die), NULL);
  return client;
}

/* nothing spectacular here - does it need to do anything else?? */

static void
session_die (gpointer client_data)
{
  gtk_main_quit ();
}

static int
save_state (GnomeClient        *client,
	    gint                phase,
	    GnomeRestartStyle   save_style,
	    gint                shutdown,
	    GnomeInteractStyle  interact_style,
	    gint                fast,
	    gpointer            client_data)
{
  gnotes_save(0, 0);
  return TRUE;
}
