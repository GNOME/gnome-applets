/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: gkb-applet.c
 * Purpose: GNOME Keyboard switcher
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs Ban  <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 * Thanks for aid of George Lebl <jirka@5z.com> and solidarity
 * Balazs Nagy <js@lsc.hu>, Charles Levert <charles@comm.polymtl.ca>
 * and Emese Kovacs <emese@gnome.hu> for her docs and ideas.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>

#include <X11/keysym.h>
#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <sys/stat.h>

#include <panel-applet.h>

#include <gtk/gtk.h>
#include <libbonobo.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <panel-applet-gconf.h>
#include <egg-screen-help.h>
#include "gkb.h"

static void
cb_help (BonoboUIComponent *uic,
         GKB	 *gkb,
         const gchar	 *verbname)
{
	GError *error = NULL;

	egg_help_display_on_screen ( "gkb", NULL,
		gtk_widget_get_screen (gkb->applet), &error);
	/* FIXME: display error to the user */
}

static void
cb_about (BonoboUIComponent *uic,
          GKB     *gkb,
          const gchar     *verbname)
{
	static GtkWidget *about;
	GtkWidget *link;
	gchar *file;	
	GdkPixbuf *pixbuf;
	GError    *error = NULL;

	static gchar const *authors[] = {
		"Szabolcs Ban <shooby@gnome.hu>",
		"Chema Celorio <chema@celorio.com>",
		NULL
	};
	static gchar const *docauthors[] = {
		"Alexander Kirillov <kirillov@math.sunysb.edu>",
		"Emese Kovacs <emese@gnome.hu>",
		"David Mason <dcm@redhat.com>",
		"Szabolcs Ban <shooby@gnome.hu>",
		NULL
	};

	gchar const *translator_credits = _("translator_credits");

	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gkb-icon.png", FALSE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, &error);
	g_free (file);

	if (error) {
		g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		g_error_free (error);
	}

	about = gnome_about_new (_("Keyboard Layout Switcher"),
				 VERSION,
				 _("(C) 1998-2000 Free Software Foundation"),
				 _("This applet switches between keyboard maps "
				   "using setxkbmap or xmodmap.\n"
				   "Mail me your flag and keyboard layout "
				   "if you want support for your locale "
				   "(my email address is shooby@gnome.hu).\n"
				   "So long, and thanks for all the fish.\n"
				   "Thanks for Balazs Nagy (Kevin) "
				   "<julian7@iksz.hu> for his help "
				   "and Emese Kovacs <emese@gnome.hu> for "
				   "her solidarity, help from guys like KevinV."
				   "\nShooby Ban <shooby@gnome.hu>"),
				 authors, docauthors,
				 strcmp (translator_credits, "translator_credits") 
				 ? translator_credits : NULL,
				 pixbuf);

	link = gnome_href_new ("http://projects.gnome.hu/gkb",
			       _("GKB Home Page (http://projects.gnome.hu/gkb)"));

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (about)->vbox), link, TRUE,
			    FALSE, 0);

	if (pixbuf)
		g_object_unref (pixbuf);

	gtk_window_set_wmclass (GTK_WINDOW (about),
		"keyboard layout switcher",
		"Keyboard Layout Switcher");
	gtk_window_set_screen (GTK_WINDOW (about),
		gtk_widget_get_screen (gkb->applet));
	g_signal_connect (G_OBJECT(about),
		"destroy",
		(GCallback)gtk_widget_destroyed, &about);

	gtk_widget_show_all (about);
}

static BonoboUIVerb const gkb_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("GKBProperties",   properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("GKBHelp",	  cb_help),
	BONOBO_UI_UNSAFE_VERB ("GKBAbout",	  cb_about),
	BONOBO_UI_VERB_END
};

static void
cb_gkb_change_pixel_size (GtkWidget *w, gint new_size, GKB *gkb)
{
	gkb_update (gkb, FALSE);
}

static void
cb_gkb_change_orient (GtkWidget * w, PanelAppletOrient new_orient, GKB *gkb)
{
	if (gkb->orient != new_orient) {
		gkb->orient = new_orient;
		gkb_update (gkb, FALSE);
	}
}

static gint
cb_gkb_button_press_event (GtkWidget * widget, GdkEventButton * event, GKB *gkb)
{
	if (event->button != 1)	
		return FALSE;

	if (gkb->cur + 1 >= gkb->n) {
		gkb->cur = 0;
		gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
	} else
		gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);

	gkb_update (gkb, TRUE);
	return TRUE;
}

static void
create_gkb_widget (GKB *gkb)
{
	gkb->eventbox = gtk_event_box_new ();
	gtk_widget_show (gkb->eventbox);

	gkb->vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (gkb->vbox);
	gkb->hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (gkb->hbox);

	gtk_container_add (GTK_CONTAINER (gkb->eventbox), gkb->hbox);
	gtk_container_add (GTK_CONTAINER (gkb->hbox), gkb->vbox);

	gkb->image = gtk_image_new ();

	gkb->button_press_id = g_signal_connect (gkb->applet,
		"button_press_event",
		G_CALLBACK (cb_gkb_button_press_event), gkb);

	gkb->darea_frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (gkb->darea_frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (gkb->darea_frame), gkb->image);
	gtk_box_pack_start (GTK_BOX (gkb->vbox), gkb->darea_frame, TRUE, TRUE, 0);
#if 0
	g_print ("c\n");
	gtk_box_pack_start (GTK_BOX (gkb->hbox), gkb->vbox, TRUE, TRUE, 0);
	g_print ("d\n");
#endif

	gkb->label1 = gtk_label_new (_("GKB"));

	gkb->label_frame1 = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (gkb->label_frame1), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (gkb->label_frame1), gkb->label1);
	gtk_container_add (GTK_CONTAINER (gkb->vbox), gkb->label_frame1);

	gkb->label2 = gtk_label_new (_("GKB"));

	gkb->label_frame2 = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (gkb->label_frame2), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (gkb->label_frame2), gkb->label2);
	gtk_container_add (GTK_CONTAINER (gkb->hbox), gkb->label_frame2);

	gkb->tooltips = gtk_tooltips_new ();
}

static GdkFilterReturn
global_key_filter (GKB *gkb, GdkXEvent *gdk_xevent, GdkEvent *event)
{
	XKeyEvent * xkevent = (XKeyEvent *) gdk_xevent;
	if (strcmp (gkb->key, convert_keysym_state_to_string (XKeycodeToKeysym (GDK_DISPLAY (), xkevent->keycode, 0), xkevent->state)) == 0) {
		if (gkb->cur + 1 < gkb->n)
			gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);
		else {
			gkb->cur = 0;
			gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
		}

		gkb_update (gkb, TRUE);
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn
event_filter (GdkXEvent * gdk_xevent, GdkEvent * event, gpointer data)
{
	XEvent *xevent = (XEvent *) gdk_xevent;
	GKB *gkb = (GKB *)data;

	if (xevent->type == KeyRelease)
		return global_key_filter (gkb, gdk_xevent, event);
	return GDK_FILTER_CONTINUE;
}

static void
gkb_update_handlers (GKB *gkb, gboolean disconnect)
{
	GdkWindow *root_window;

	root_window = gdk_get_default_root_window ();

	if (disconnect) {
		if (gkb->button_press_id != -1)
			g_signal_handler_disconnect (gkb->applet, gkb->button_press_id);
		gkb->button_press_id = -1;

		gdk_window_remove_filter (root_window, event_filter, gkb);
	} else {
		gdk_window_add_filter (root_window, event_filter, gkb);

		if (gkb->button_press_id == -1)
			gkb->button_press_id = g_signal_connect (gkb->applet,
				"button_press_event",
				G_CALLBACK (cb_gkb_button_press_event), gkb);
	}
}

static void
gkb_applet_shutdown (GKB *gkb)
{
	if (gkb->propwindow)
		gtk_widget_destroy (gkb->propwindow);
	gkb_update_handlers (gkb, TRUE);
	g_free (gkb);
}

gboolean
fill_gkb_applet (PanelApplet *applet)
{
	GdkWindow *root_window;
	gint keycode, modifiers;
	GKB *gkb = gkb_new ();

	gkb->update = gkb_update_handlers;
	gkb->applet = GTK_WIDGET (applet);

	panel_applet_add_preferences (applet, "/schemas/apps/gkb-applet/prefs", NULL);
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	create_gkb_widget (gkb);

	gtk_widget_show (gkb->darea_frame);
	gtk_container_add (GTK_CONTAINER (gkb->applet), gkb->eventbox);

	keycode = XKeysymToKeycode(GDK_DISPLAY(), gkb->keysym);

	modifiers = gkb->state;

	root_window = gdk_get_default_root_window ();

	gkb->keycode = keycode; 
	/* Incase the key is already grabbed by another application */ 
	gdk_error_trap_push ();
	gkb_xgrab (keycode, modifiers, root_window);
	gdk_flush ();
	gdk_error_trap_pop ();
	gdk_window_add_filter (root_window, event_filter, gkb);

	g_object_set_data_full (G_OBJECT (applet),
		"GKB", gkb, (GDestroyNotify) gkb_applet_shutdown);

	g_signal_connect (G_OBJECT (gkb->applet),
			  "change_orient",
			  G_CALLBACK (cb_gkb_change_orient), gkb);

	g_signal_connect (G_OBJECT (gkb->applet),
		"change_size",
		G_CALLBACK (cb_gkb_change_pixel_size), gkb);

	panel_applet_setup_menu_from_file (PANEL_APPLET (gkb->applet),
					   NULL,
					   "GNOME_KeyboardApplet.xml",
					   NULL,
					   gkb_menu_verbs, 
					   gkb);
	if (panel_applet_get_locked_down (PANEL_APPLET (gkb->applet))) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (PANEL_APPLET (gkb->applet));

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/GKBProperties",
					      "hidden", "1",
					      NULL);
	}


	gtk_widget_show (GTK_WIDGET(gkb->applet));
	gkb_update (gkb,TRUE);

	return TRUE;
}

gboolean gail_loaded = FALSE;

gboolean
gkb_factory (PanelApplet *applet,
		const gchar *iid,
		gpointer     data)
{
	if (GTK_IS_ACCESSIBLE (gtk_widget_get_accessible (GTK_WIDGET (applet))))
		gail_loaded = TRUE;

	return !strcmp (iid, "OAFIID:GNOME_KeyboardApplet") &&
		fill_gkb_applet (applet);
}
