/*
 * GNOME cpumemusage panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Author: Radek Doulik
 *         Orientation switching: Owen Taylor <otaylor@redhat.com>
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>

#include "proc.h"


/* Milliseconds between updates */
#define CPU_UPDATE_MSEC 200
#define MEM_UPDATE_MSEC 1000

#define WIDTH 80

/* Nice value to avoid eating CPU when it is needed */
#define NICE_VALUE 5


static GdkColor bar_cpu_colors [PROC_CPU_SIZE-1] = {
	{0, 0xffff, 0xffff, 0x4fff},
	{0, 0xdfff, 0xdfff, 0xdfff},
	{0, 0xafff, 0xafff, 0xafff},
	{0, 0, 0, 0},
};

static GdkColor bar_mem_colors [PROC_MEM_SIZE-1] = {
	{0, 0xbfff, 0xbfff, 0x4fff},
	{0, 0xefff, 0xefff, 0x4fff},
	{0, 0xafff, 0xafff, 0xafff},
	{0, 0, 0x8fff, 0},
};

static GdkColor bar_swap_colors [PROC_SWAP_SIZE-1] = {
	{0, 0xcfff, 0x5fff, 0x5fff},
	{0, 0, 0x8fff, 0},
};

static ProcInfo  summary_info;
static GtkWidget *cpu, *mem, *swap = NULL;
static GtkWidget *cpumemusage;
static GtkWidget *applet;

static gint size;
static gint spacing;

static void applet_change_pixel_size(GtkWidget *widget, int s);

static gint
update_cpu_values (void)
{
	/* printf ("update\n"); */

	proc_read_cpu (&summary_info);
	gnome_proc_bar_set_values (GNOME_PROC_BAR (cpu), summary_info.cpu);

	return TRUE;
}

static gint
update_mem_values (void)
{
	proc_read_mem (&summary_info);
	gnome_proc_bar_set_values (GNOME_PROC_BAR (mem), summary_info.mem);

	if (swap)
		gnome_proc_bar_set_values
			(GNOME_PROC_BAR (swap), summary_info.swap);

	return TRUE;
}

static GtkWidget *
pack_procbars(gboolean vertical)
{
	GtkWidget *box;
	int s;
	
	if (vertical) {
		box = gtk_hbox_new (TRUE, 0);
		/* gtk_widget_set_usize (box, 40, 80); */
	} else {
		box = gtk_vbox_new (TRUE, 0);
		/* gtk_widget_set_usize (box, 80, 40); */
	}

	gnome_proc_bar_set_orient (GNOME_PROC_BAR (cpu), vertical);
	gtk_box_pack_start_defaults (GTK_BOX (box), cpu);
	gnome_proc_bar_set_orient (GNOME_PROC_BAR (mem), vertical);
	gtk_box_pack_start_defaults (GTK_BOX (box), mem);
	
	if (swap) {
		gnome_proc_bar_set_orient (GNOME_PROC_BAR (swap), vertical);
		gtk_box_pack_start_defaults (GTK_BOX (box), swap);
	}

	s = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));
	applet_change_pixel_size (GTK_WIDGET(applet),s);

	gtk_widget_show_all (box);

	return box;
}

static GtkWidget *
cpumemusage_widget (void)
{
	GtkWidget *box;
	GNOME_Panel_OrientType orient;
	
	proc_read_mem (&summary_info);

	cpu  = gnome_proc_bar_new (NULL, PROC_CPU_SIZE-1, bar_cpu_colors,
				   update_cpu_values);
	mem  = gnome_proc_bar_new (NULL, PROC_MEM_SIZE-2, bar_mem_colors,
				   update_mem_values);

	if (summary_info.swap [PROC_SWAP_TOTAL])
		swap = gnome_proc_bar_new
			(NULL, PROC_SWAP_SIZE-1, bar_swap_colors, NULL);

	update_cpu_values ();
	update_mem_values ();

	gnome_proc_bar_start (GNOME_PROC_BAR (cpu), CPU_UPDATE_MSEC, NULL);
	gnome_proc_bar_start (GNOME_PROC_BAR (mem), MEM_UPDATE_MSEC, NULL);

	orient = applet_widget_get_panel_orient (APPLET_WIDGET (applet));

	switch (orient) {
	case GNOME_Panel_ORIENT_LEFT:
	case GNOME_Panel_ORIENT_RIGHT:
	    box = pack_procbars (TRUE);
	    break;
	default:
	    box = pack_procbars (FALSE);
	    break;
	}

	return box;
}

static void
cpumemusage_about (AppletWidget *applet, gpointer data)
{
        static GtkWidget   *about     = NULL;
        static const gchar *authors[] =
        {
		"Radek Doulik <rodo@ucw.cz>",
                NULL
        };

        if (about != NULL)
        {
                gdk_window_show(about->window);
                gdk_window_raise(about->window);
                return;
        }
        
        about = gnome_about_new (_("CPU/Mem Usage Applet"), VERSION,
                                 _("(c) 1997 the Free Software Foundation"),
                                 authors,
                                 _("The CPU/Mem usage applet displays your system resources with 3 colorful stacked bar graphs (processor, memory, swap)"),
                                 NULL);
        gtk_signal_connect (GTK_OBJECT(about), "destroy",
                            GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
        gtk_widget_show (about);
}

static void
applet_change_orient(GtkWidget *w, PanelOrientType o)
{
	GtkWidget *box;
	gboolean vertical;

	switch (o) {
	case ORIENT_LEFT:
	case ORIENT_RIGHT:
		vertical = TRUE;
		break;
	default:
		vertical = FALSE;
		break;
	}

	if (swap) {
		gtk_widget_ref (swap);
		gtk_container_remove (GTK_CONTAINER(cpumemusage), swap);
	}
	gtk_widget_ref (cpu);
	gtk_container_remove (GTK_CONTAINER(cpumemusage), cpu);
	gtk_widget_ref (mem);
	gtk_container_remove (GTK_CONTAINER(cpumemusage), mem);
	
	box = pack_procbars (vertical);

	if (swap)
		gtk_widget_unref (swap);
	gtk_widget_unref (cpu);
	gtk_widget_unref (mem);

	gtk_container_remove (GTK_CONTAINER (applet), cpumemusage);
	gtk_container_add (GTK_CONTAINER (applet), box);
	cpumemusage = box;

	gtk_box_set_spacing (GTK_BOX (cpumemusage), spacing);
	if (o == ORIENT_LEFT || o == ORIENT_RIGHT)
		gtk_widget_set_usize (cpumemusage, size, WIDTH);
	else
		gtk_widget_set_usize (cpumemusage, WIDTH, size);
	return;
}

static void
applet_change_pixel_size(GtkWidget *widget, int s)
{
	GNOME_Panel_OrientType orient;

	if(s<PIXEL_SIZE_STANDARD) {
		spacing = 0;
		size = s;
	} else if(s<PIXEL_SIZE_LARGE) {
		spacing = 1;
		size = s - 4;
	} else if(s<PIXEL_SIZE_HUGE) {
		spacing = 2;
		size = s - 6;
	} else {
		spacing = 3;
		size = s - 8;
	}

	orient = applet_widget_get_panel_orient (APPLET_WIDGET (applet));

	if(!cpumemusage)
		return;

	gtk_box_set_spacing (GTK_BOX (cpumemusage), spacing);
	if (orient == GNOME_Panel_ORIENT_LEFT || orient == GNOME_Panel_ORIENT_RIGHT)
		gtk_widget_set_usize (cpumemusage, size, WIDTH);
	else
		gtk_widget_set_usize (cpumemusage, WIDTH, size);
	return;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
    GnomeHelpMenuEntry help_entry = { "cpumemusage_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
}

int
main(int argc, char **argv)
{

	/* Initialize gettext support */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
	
        applet_widget_init("cpumemusage_applet", VERSION, argc, argv,
			   NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-cpu-mem.png");

	applet = applet_widget_new("cpumemusage_applet");
	if (!applet)
		g_error("Can't create applet!\n");

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
			   GTK_SIGNAL_FUNC(applet_change_pixel_size),
			   NULL);

        cpumemusage = cpumemusage_widget();
        applet_widget_add( APPLET_WIDGET(applet), cpumemusage );

	applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"),
					       help_cb, NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       cpumemusage_about,
					       NULL);

	/* Be nice */
	nice (NICE_VALUE);

        gtk_widget_show(applet);
	applet_widget_gtk_main();

        return 0;
}
