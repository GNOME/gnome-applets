/*
 * GNOME panel printer applet module.
 * (C) 1998 The Free Software Foundation
 *
 * Author: Miguel de Icaza
 *
 */

#include <config.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"
#include <gdk_imlib.h>
#include "print.xpm"

GtkWidget *printer;
GtkWidget *applet = 0;

char *print_command = NULL;
char *print_title   = NULL;

void
position_label (GtkWidget *widget, void *l)
{
	GtkLabel *label = GTK_LABEL (l);
	int height;
	GtkWidget *parent = GTK_WIDGET (label)->parent;
	
	height = (GTK_WIDGET (label)->style->font->ascent +
		  GTK_WIDGET (label)->style->font->descent + 2);
	gtk_fixed_move (GTK_FIXED (parent), GTK_WIDGET (label),
		       (48 - gdk_string_width (GTK_WIDGET (label)->style->font, label->label)) / 2,
		       48 - height);
}

static void
execute (char *command)
{
	pid_t pid;

	pid = fork ();
        if (pid == 0){
		int i;

		for (i = 0; i < 255; i++)
			close (i);
		execl ("/bin/sh", "/bin/sh", "-c", command, NULL);
		
		_exit (127);
	}
}

static void
drop_data_available (GtkWidget *widget, GdkEventDropDataAvailable *event)
{
	char *p = event->data;
	int count = event->data_numbytes;
	int len, items;
	char *str;

	p = event->data;
	count = event->data_numbytes;
	do {
		len = 1 + strlen (p);
		count -= len;

		str = g_copy_strings (print_command, " ", p, NULL);
		execute (str);
		g_free (str);
		
		p += len;
		items++;
	} while (count > 0);
}

configure_dnd (void *_w)
{
	GtkWidget *w = GTK_WIDGET (_w);
	char *drop_types [] = { "url:ALL" };

	gtk_widget_dnd_drop_set (w, 1, drop_types, 1, FALSE);
	gtk_signal_connect (GTK_OBJECT (w), "drop_data_available_event",
			    GTK_SIGNAL_FUNC (drop_data_available), NULL);
}

GtkWidget *
printer_widget (void)
{
	GtkWidget *fixed;
	GtkWidget *label, *printer;
	int height;
	
	fixed   = gtk_fixed_new ();
	printer = gnome_pixmap_new_from_xpm_d (print_xpm);
	label   = gtk_label_new (print_title);

	gtk_fixed_put (GTK_FIXED (fixed), printer, 0, 0);
	gtk_fixed_put (GTK_FIXED (fixed), label, 0, 0);
	gtk_signal_connect (GTK_OBJECT (label), "realize",
			    GTK_SIGNAL_FUNC (position_label), label);
	gtk_signal_connect (GTK_OBJECT (printer), "realize",
			    GTK_SIGNAL_FUNC (configure_dnd), printer);
	gtk_signal_connect (GTK_OBJECT (fixed), "realize",
			    GTK_SIGNAL_FUNC (configure_dnd), fixed);
	gtk_widget_set_usize (fixed, 48, 48);
	gtk_widget_show_all (fixed);
	return fixed;
}

static gint
destroy_applet(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}

static gint
applet_session_save(GtkWidget *w, const char *cfgpath, const char *globcfgpath)
{
	char *query;

	query = g_copy_strings (cfgpath, "print_command", NULL);
	gnome_config_set_string (query, print_command ? print_command : "");
	g_free (query);

	gnome_config_sync ();
	gnome_config_drop_all ();

	return FALSE;
}

void
printer_properties (AppletWidget *applet, gpointer data)
{
}

int
main(int argc, char **argv)
{
	GtkWidget *mailcheck;
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sa.sa_flags   = SA_NOCLDSTOP;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGCHLD, &sa, NULL);
	
	panel_corba_register_arguments ();
	gnome_init("mailcheck_applet", NULL, argc, argv, 0, NULL);

	applet = applet_widget_new (argv[0]);
	if (!applet)
		g_error("Can't create applet!\n");

	if(APPLET_WIDGET(applet)->cfgpath && *(APPLET_WIDGET(applet)->cfgpath)) {
		gnome_config_push_prefix (APPLET_WIDGET(applet)->cfgpath);
		print_command = gnome_config_get_string ("print_command=lpr");
		print_title   = gnome_config_get_string ("title=Print");
	} else {
		print_title   = g_strdup ("Print");
		print_command = g_strdup ("lpr");
	}
	print_command = "lpr -P4";
	
	printer = printer_widget ();
	gtk_widget_show (printer);
	applet_widget_add (APPLET_WIDGET (applet), printer);
	gtk_widget_show (applet);
	gtk_signal_connect(GTK_OBJECT(applet),"destroy",
			   GTK_SIGNAL_FUNC(destroy_applet),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(applet),"session_save",
			   GTK_SIGNAL_FUNC(applet_session_save),
			   NULL);

	applet_widget_register_callback(APPLET_WIDGET(applet),
					"properties",
					_("Properties"),
					printer_properties,
					NULL);

	applet_widget_gtk_main ();

	return 0;
}
