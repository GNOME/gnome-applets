/*
 * GNOME cpumemusage panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Author: Radek Doulik
 *
 */

#include <stdio.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include "applet-lib.h"
#include "applet-widget.h"

#include "proc.h"
#include "procbar.h"

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

static ProcInfo   summary_info;
static ProcBar   *cpu, *mem, *swap = NULL;
static GtkWidget *cpumemusage;

static gint
update_values ()
{
	/* printf ("update\n"); */

	proc_read_cpu (&summary_info);
	procbar_set_values (cpu, summary_info.cpu);

	proc_read_mem (&summary_info);
	procbar_set_values (mem, summary_info.mem);

	if (swap)
		procbar_set_values (swap, summary_info.swap);

	return TRUE;
}

static GtkWidget *
cpumemusage_widget ()
{
	GtkWidget *vbox;

	proc_read_mem (&summary_info);

	vbox = gtk_vbox_new (TRUE, GNOME_PAD_SMALL >> 1);
	gtk_widget_set_usize (vbox, 80, 40);

	cpu  = procbar_new (NULL, PROC_CPU_SIZE-1, bar_cpu_colors,
			    update_values);
	mem  = procbar_new (NULL, PROC_MEM_SIZE-2, bar_mem_colors,
			    NULL);

	if (summary_info.swap [PROC_SWAP_TOTAL])
		swap = procbar_new (NULL, PROC_SWAP_SIZE-1, bar_swap_colors,
				    NULL);

	gtk_box_pack_start_defaults (GTK_BOX (vbox), cpu->hbox);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), mem->hbox);

	if (swap)
		gtk_box_pack_start_defaults (GTK_BOX (vbox), swap->hbox);

	gtk_widget_show (vbox);

	procbar_start (cpu, 200);

	return vbox;
}

static gint
destroy_applet(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}

int main(int argc, char **argv)
{
	GtkWidget *applet;

        panel_corba_register_arguments();

        gnome_init("cpumemusage_applet", NULL, argc, argv, 0, NULL);

	applet = applet_widget_new(argv[0]);
	if (!applet)
		g_error("Can't create applet!\n");

        cpumemusage = cpumemusage_widget();
        applet_widget_add( APPLET_WIDGET(applet), cpumemusage );
        gtk_widget_show(applet);
	gtk_signal_connect(GTK_OBJECT(applet),"destroy",
			   GTK_SIGNAL_FUNC(destroy_applet),
			   NULL);
	
	applet_widget_gtk_main();

        return 0;
}
