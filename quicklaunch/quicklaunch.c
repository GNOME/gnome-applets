/*
    The QuickLaunch Applet - Mini-icon launchers for the GNOME Panel
    QuickLaunch Homepage: http://members.xoom.com/fabiofb/quicklaunch

    Copyright (C) 1999  Fábio Gomes de Souza
    Copyright 2000 Helix Code, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Author: Fábio Gomes de Souza <fabiofb@altavista.net>
*/

#include <config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>

#include <gtk/gtkwidget.h>
#include <gdk_imlib.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <applet-widget.h>

#define UNKNOWN_ICON "gnome-unknown.png"

static void reload_data(void);

static GList *launcher_widgets = NULL;
static GList *launchers = NULL;
static GtkWidget *label;
static GtkWidget *launcher_table;
static GtkWidget *wnd;
static GtkWidget *handlebox;

char *directory = NULL;

static void
cb_about (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about;
	const gchar *authors[] = {
		"Fabio Gomes de Souza <fabiofb@altavista.net>", 
		NULL 
	};

	if (about) {
		gtk_widget_show (about);
		if(about->window)
			gdk_window_raise (about->window);
		return;
	}

	about = gnome_about_new(_("The QuickLaunch Applet"), VERSION,
				"(c) 1999 Fabio Gomes de Souza", authors,
				_("This applet adds some pretty small icons "
				  "for your launchers to your panel, like the "
				  "windowish QuickLaunch toolbar, avoiding "
				  "that annoying huge GNOME launchers."),
				NULL);
	/* null about when the box gets destroyed */
	gtk_signal_connect (GTK_OBJECT (about), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &about);

	gtk_widget_show (about);
}

static void
launch_app_cb (GtkWidget *widget, gpointer data)
{
	GnomeDesktopEntry *dentry = data;
	if(dentry) gnome_desktop_entry_launch (dentry);
}

static void
destroy_launcher_widgets (void)
{
	GList *p;
	p = launcher_widgets;

	while (p) {
		GtkWidget *w = p->data;
		p->data = NULL;

		gtk_widget_destroy (GTK_WIDGET (w));
		p = p->next;
	}
	if (launcher_widgets) {
		g_list_free (launcher_widgets);
		launcher_widgets = NULL;
	}
	if (label) {
		gtk_widget_destroy (GTK_WIDGET (label));
		label = NULL;
	}
}

static void 
cb_properties_apply (GtkWidget *widget, int page, gpointer data)
{
	GnomeDesktopEntry *dentry;
	char *location = data;
	GtkWidget *dedit;

	dedit = gtk_object_get_data(GTK_OBJECT(widget), "dedit");

	dentry = gnome_dentry_get_dentry (GNOME_DENTRY_EDIT (dedit));
	if (dentry) {
		g_free(dentry->location);
		dentry->location = g_strdup(location);

		gnome_desktop_entry_save (dentry);
		reload_data ();
	}
}

static GtkWidget *
create_properties_dialog (GnomeDesktopEntry *dentry)
{
	GtkWidget *dialog;
	GtkObject *dedit;

	dialog = gnome_property_box_new ();
	gtk_window_set_title (GTK_WINDOW (dialog), _("Launcher Properties"));
	gtk_window_set_policy (GTK_WINDOW (dialog), FALSE, FALSE, TRUE);
  
	dedit = gnome_dentry_edit_new_notebook (
		GTK_NOTEBOOK (GNOME_PROPERTY_BOX (dialog)->notebook));
	gtk_object_set_data(GTK_OBJECT(dialog), "dedit", dedit);

	gnome_dentry_edit_set_dentry (GNOME_DENTRY_EDIT (dedit), dentry);

	gtk_signal_connect_object (GTK_OBJECT (dedit),"changed",
				   GTK_SIGNAL_FUNC (gnome_property_box_changed),
				   GTK_OBJECT (dialog));

	gtk_signal_connect_full (GTK_OBJECT (dialog), "apply",
				 GTK_SIGNAL_FUNC (cb_properties_apply), NULL,
				 g_strdup(dentry->location),
				 (GtkDestroyNotify)g_free,
				 FALSE, FALSE);
	gtk_signal_connect_object (GTK_OBJECT (dialog), "destroy",
				   GTK_SIGNAL_FUNC (gtk_object_destroy),
				   GTK_OBJECT(dedit));
	return dialog;
}

static void
cb_launcher_properties (GtkWidget *widget, gpointer data)
{
	static GtkWidget *dialog;
	GtkWidget *button = data;
	GnomeDesktopEntry *dentry;

	if (dialog) {
		gtk_widget_show (dialog);
		if (dialog->window)
			gdk_window_raise (dialog->window);
		return;
	}

	dialog = gtk_object_get_data (GTK_OBJECT (button), "properties-box");
	dentry = gtk_object_get_data (GTK_OBJECT (button), "dentry");
	
	dialog = create_properties_dialog (dentry);
	gtk_object_set_data (GTK_OBJECT (button),
			     "properties-box", dialog);

	gtk_signal_connect (GTK_OBJECT (dialog),
			    "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &dialog);
			    
	gtk_widget_show_all (dialog);
}

static void
cb_launcher_delete (GtkWidget *widget, gpointer data)
{
	GtkWidget *button = data;
	GnomeDesktopEntry *dentry;
	dentry = gtk_object_get_data (GTK_OBJECT (button), "dentry");
  
	unlink (dentry->location);
	reload_data ();
}

static void
launcher_table_update (void)
{
	gint x=0;
	gint y=0;
	gint c,i,rows;
	gboolean vertical;
	GList *li;
	GtkWidget *button;
	GtkWidget *pixmap;
	GtkWidget *menu;
	GtkReliefStyle relief_style;
	GnomeDesktopEntry *dentry;
	GnomeUIInfo uinfo[5] = {
		GNOMEUIINFO_END,
		GNOMEUIINFO_END,
		GNOMEUIINFO_END,
		GNOMEUIINFO_END,
		GNOMEUIINFO_END,
	};

	switch(applet_widget_get_panel_orient(APPLET_WIDGET(wnd))) {
	case ORIENT_UP:
	case ORIENT_DOWN:
		vertical = FALSE;
		break;
	default:
		vertical = TRUE;
		break;
	}
	rows = applet_widget_get_panel_pixel_size(APPLET_WIDGET(wnd))/24;
	if(rows<1) rows = 1;

	if (gnome_preferences_get_toolbar_relief_btn())
		relief_style = GTK_RELIEF_NORMAL;
	else
		relief_style = GTK_RELIEF_NONE;
	
	destroy_launcher_widgets ();
	c = g_list_length (launchers);
	if (vertical) {
		gtk_table_resize (GTK_TABLE (launcher_table), c/rows, 1);
		gtk_handle_box_set_handle_position (GTK_HANDLE_BOX (handlebox),
						    GTK_POS_TOP);
	} else {
		gtk_table_resize (GTK_TABLE (launcher_table), 1, c/rows);
		gtk_handle_box_set_handle_position (GTK_HANDLE_BOX(handlebox),
						    GTK_POS_LEFT);
					 
	}
	if (c == 0) {
		label = gtk_label_new (_("drop launchers from\n"
					 "the menu here"));
		gtk_table_attach_defaults (GTK_TABLE (launcher_table), 
					   label, 0, 1, 0, 1);
		gtk_widget_show (label);
		return;
	}
	for (li = launchers, i=0; li && i < c; li = li->next, i++) {
		dentry = li->data;

		if (!dentry->icon)
			dentry->icon = gnome_pixmap_file (UNKNOWN_ICON);

		button = gtk_button_new();
		gtk_button_set_relief (GTK_BUTTON (button), relief_style);
		pixmap = gnome_stock_pixmap_widget_at_size (
			NULL, dentry->icon, 18, 18);
		gtk_container_set_border_width (GTK_CONTAINER (button), 0);
		gtk_container_add (GTK_CONTAINER (button), pixmap);
		gtk_widget_show (pixmap);
		if (!vertical)
			gtk_table_attach_defaults (GTK_TABLE (launcher_table),
						   button, x, x+1, y, y+1);
		else
			gtk_table_attach_defaults (GTK_TABLE (launcher_table),
						   button, y, y+1, x, x+1);
						   
		gtk_container_queue_resize (GTK_CONTAINER (launcher_table));
		gtk_container_clear_resize_widgets (
			GTK_CONTAINER (launcher_table));
		gtk_widget_show (button);
		GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);
		gtk_widget_realize (GTK_WIDGET (button));
		gtk_object_set_data (GTK_OBJECT(button),"dentry", dentry);
		gtk_signal_connect (GTK_OBJECT(button), "clicked",
				    GTK_SIGNAL_FUNC (launch_app_cb), dentry);
		applet_widget_set_widget_tooltip (APPLET_WIDGET(wnd),
						  button, dentry->name);
		launcher_widgets = g_list_append (launcher_widgets, button);

		uinfo[0].type            = GNOME_APP_UI_ITEM;
		uinfo[0].label           = _("Launcher properties...");
		uinfo[0].hint            = NULL;
		uinfo[0].moreinfo        = cb_launcher_properties;
		uinfo[0].user_data       = NULL;
		uinfo[0].unused_data     = NULL;
		uinfo[0].pixmap_type     = GNOME_APP_PIXMAP_NONE;
		uinfo[0].pixmap_info     = GNOME_STOCK_MENU_OPEN;
		uinfo[0].accelerator_key = 0;
		uinfo[0].ac_mods         = (GdkModifierType) 0;
		uinfo[0].widget          = NULL;

		uinfo[1].type            = GNOME_APP_UI_ITEM;
		uinfo[1].label           = _("Delete launcher");
		uinfo[1].hint            = NULL;
		uinfo[1].moreinfo        = cb_launcher_delete;
		uinfo[1].user_data       = NULL;
		uinfo[1].unused_data     = NULL;
		uinfo[1].pixmap_type     = GNOME_APP_PIXMAP_NONE;
		uinfo[1].pixmap_info     = GNOME_STOCK_MENU_OPEN;
		uinfo[1].accelerator_key = 0;
		uinfo[1].ac_mods         = (GdkModifierType) 0;
		uinfo[1].widget          = NULL;

		menu = gnome_popup_menu_new (uinfo);
		gnome_popup_menu_attach (menu, button, button);

		if (y == rows-1) {
			y=0;
			x++;
		} else
			y++;
	}
}

static void
add_launcher(const char *filename)
{
	GnomeDesktopEntry *dentry;

	dentry = gnome_desktop_entry_load (filename);
	if (dentry == NULL)
		return;
	launchers = g_list_append (launchers, dentry);

}

static void
scan_dir (char *dirname)
{
	struct dirent *dent;
	DIR *dir;
  
	dir=opendir(dirname);
	if (dir == NULL) 
		return;
	while ((dent = readdir(dir)) != NULL) 
	{
		char *file;
		if (dent->d_name[0] == '.') 
			continue;

		file = g_concat_dir_and_file (dirname, dent->d_name);
		add_launcher (file);
		g_free(file);
	}
	closedir (dir);
	launcher_table_update ();

}

static void
reload_data (void)
{
	destroy_launcher_widgets ();
	g_list_foreach (launchers, (GFunc)gnome_desktop_entry_free, NULL);
	g_list_free (launchers);
	launchers = NULL;
	scan_dir (directory);
}

static void
cb_applet_destroy (GtkWidget *applet, gpointer data)
{
	g_message ("Destroying applet");
	destroy_launcher_widgets ();
	gtk_main_quit ();
}

static GnomeDesktopEntry *
make_new_dentry (const char *filename)
{
	GnomeDesktopEntry *dentry;
	char *execv[] = { NULL, NULL };

	dentry = g_new0 (GnomeDesktopEntry, 1);
	dentry->name = g_strdup (filename);
	dentry->comment = g_strdup (filename);
	dentry->type = g_strdup ("Application");

	execv[0] = (char *)filename;
	dentry->exec = g_copy_vector(execv);
	dentry->exec_length = 1;

	return dentry;
}

static void
drop_launcher (gchar *filename)
{
	GnomeDesktopEntry *dentry = NULL;
	const char *mimetype;

	mimetype = gnome_mime_type(filename);

	if(mimetype &&
	   (strcmp(mimetype, "application/x-gnome-app-info") == 0 ||
	    strcmp(mimetype, "application/x-kde-app-info") == 0)) {
		dentry = gnome_desktop_entry_load (filename);
	} else {
		struct stat s;
		if(stat(filename, &s) != 0) {
			g_warning (_("File '%s' does not exist"), filename);
			return;
		}

		if(S_IEXEC & s.st_mode) /*executable?*/
			dentry = make_new_dentry (filename);
	}

	if (dentry == NULL) {
		g_warning (_("Don't know how to make a launcher out of: '%s'"),
			   filename);
		return;
	}

	if (dentry->is_kde) {
		/* FIXME: bad, but oh well, should really convert
		 * the exec string */
		dentry->is_kde = FALSE;
	}

	srandom (time (NULL));

	do {
		long foo = random ();

		g_free(dentry->location);
		dentry->location = g_strdup_printf ("%s/dropped-entry-%lx.desktop",
						    directory, foo);
	} while (g_file_exists (dentry->location));

	gnome_desktop_entry_save (dentry);
	gnome_desktop_entry_free (dentry);
}

static void
drag_data_received (GtkWidget *widget,
		    GdkDragContext *context,
		    gint x,
		    gint y,
		    GtkSelectionData *selection_data,
		    guint info,
		    guint time,
		    gpointer data)
{
	GList *names;
  
	names = gnome_uri_list_extract_filenames ((char *)selection_data->data);
	g_list_foreach (names, (GFunc)drop_launcher, NULL);
	gnome_uri_list_free_strings (names);
	reload_data ();
}

/* ignore first button click and translate it into a second button slick */
static int
ignore_1st_click(GtkWidget *widget, GdkEvent *event)
{
	GdkEventButton *buttonevent = (GdkEventButton *)event;

	if((event->type == GDK_BUTTON_PRESS &&
	    buttonevent->button == 1) ||
	   (event->type == GDK_BUTTON_RELEASE &&
	    buttonevent->button == 1)) {
		buttonevent->button = 2;
	}
	 
	return FALSE;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "quicklaunch_applet", "index.html"};
	gnome_help_display(NULL, &help_entry);
}

static void
init_quicklaunch (void)
{

	GtkWidget *frame;
	GtkWidget *msg;

	static GtkTargetEntry drop_types [] = {
		{ "text/uri-list",0,0 },
	};
	static gint n_drop_types = sizeof(drop_types) / sizeof(drop_types)[0];
	
	directory = gnome_util_home_file ("quicklaunch");
	if (!g_file_test (directory, G_FILE_TEST_ISDIR)) {
		if (mkdir(directory,S_IRWXU)) {
			char *s = _("Cannot create directory:\n"
				    "~/.gnome/quicklaunch.\n"
				    "Aborting");
			msg = gnome_message_box_new (s, 
						     GNOME_MESSAGE_BOX_ERROR);
			gtk_widget_show (msg);
			gtk_signal_connect (GTK_OBJECT(msg),"destroy",
					    GTK_SIGNAL_FUNC (gtk_main_quit),
					    NULL);
		}
	}

	wnd = applet_widget_new ("quicklaunch_applet");
	gtk_signal_connect( GTK_OBJECT (wnd), "destroy",
			    GTK_SIGNAL_FUNC (cb_applet_destroy), NULL);
	gtk_signal_connect (GTK_OBJECT (wnd), "change_orient",
			    GTK_SIGNAL_FUNC (launcher_table_update), NULL);
	gtk_signal_connect (GTK_OBJECT (wnd), "change_pixel_size",
			    GTK_SIGNAL_FUNC (launcher_table_update), NULL);
	launcher_table=gtk_table_new (0, 0, FALSE);
	gtk_container_set_resize_mode (GTK_CONTAINER (launcher_table),
				       GTK_RESIZE_QUEUE);
	frame = gtk_event_box_new ();
	handlebox = gtk_handle_box_new ();
	gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX (handlebox), GTK_SHADOW_NONE);
	gtk_signal_connect(GTK_OBJECT(handlebox),"event",
			   GTK_SIGNAL_FUNC(ignore_1st_click),NULL);
	gtk_container_add (GTK_CONTAINER (handlebox), launcher_table);
	gtk_container_add (GTK_CONTAINER (frame), handlebox);
	applet_widget_add (APPLET_WIDGET (wnd), frame);
	applet_widget_register_stock_callback (APPLET_WIDGET (wnd),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"), help_cb, NULL);
	applet_widget_register_stock_callback (APPLET_WIDGET (wnd),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       cb_about,
					       NULL);
	gtk_drag_dest_set (GTK_WIDGET (wnd),
			   GTK_DEST_DEFAULT_MOTION | 
			   GTK_DEST_DEFAULT_HIGHLIGHT |
			   GTK_DEST_DEFAULT_DROP,
			   drop_types, n_drop_types,
			   GDK_ACTION_COPY);

	gtk_signal_connect (GTK_OBJECT (wnd), "drag_data_received",
			    GTK_SIGNAL_FUNC (drag_data_received), NULL);

	gtk_widget_realize (wnd);
	gtk_widget_show (launcher_table);
	gtk_widget_show (frame);
	gtk_widget_show (handlebox);
	gtk_widget_show (wnd);

	gtk_widget_realize (GTK_WIDGET (wnd));
	reload_data ();
}

int 
main (int argc, char *argv[])
{

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("quicklaunch_applet", VERSION,
			    argc, argv, NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-quicklaunch.png");

	init_quicklaunch ();
	applet_widget_gtk_main ();
	return 0;
}
