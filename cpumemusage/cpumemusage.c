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
#include <gdk/gdkx.h>
#include <applet-widget.h>

#include "proc.h"
#include "procbar.h"


/* Milliseconds between updates */
#define CPU_UPDATE_MSEC 200
#define MEM_UPDATE_MSEC 1000

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

static ProcInfo   summary_info;
static ProcBar   *cpu, *mem, *swap = NULL;
static GtkWidget *cpumemusage;
static GtkWidget *applet;

static gint
update_cpu_values (void)
{
	/* printf ("update\n"); */

	proc_read_cpu (&summary_info);
	procbar_set_values (cpu, summary_info.cpu);

	return TRUE;
}

static gint
update_mem_values (void)
{
	proc_read_mem (&summary_info);
	procbar_set_values (mem, summary_info.mem);

	if (swap)
		procbar_set_values (swap, summary_info.swap);

	return TRUE;
}

static GtkWidget *
pack_procbars(gboolean vertical)
{
	GtkWidget *box;
	
	if (vertical) {
		box = gtk_hbox_new (TRUE, GNOME_PAD_SMALL >> 1);
		gtk_widget_set_usize (box, 40, 80);
	} else {
		box = gtk_vbox_new (TRUE, GNOME_PAD_SMALL >> 1);
		gtk_widget_set_usize (box, 80, 40);
	}

	procbar_set_orient (cpu, vertical);
	gtk_box_pack_start_defaults (GTK_BOX (box), cpu->hbox);
	procbar_set_orient (mem, vertical);
	gtk_box_pack_start_defaults (GTK_BOX (box), mem->hbox);
	
	if (swap) {
		procbar_set_orient (swap, vertical);
		gtk_box_pack_start_defaults (GTK_BOX (box), swap->hbox);
	}

	gtk_widget_show (box);

	return box;
}

static GtkWidget *
cpumemusage_widget ()
{
	GtkWidget *box;
	
	proc_read_mem (&summary_info);

	cpu  = procbar_new (NULL, PROC_CPU_SIZE-1, bar_cpu_colors,
			    update_cpu_values);
	mem  = procbar_new (NULL, PROC_MEM_SIZE-2, bar_mem_colors,
			    update_mem_values);

	if (summary_info.swap [PROC_SWAP_TOTAL])
		swap = procbar_new (NULL, PROC_SWAP_SIZE-1, bar_swap_colors,
				    NULL);

	box = pack_procbars (FALSE);
	procbar_start (cpu, CPU_UPDATE_MSEC);
	procbar_start (mem, MEM_UPDATE_MSEC);

	return box;
}



static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	GtkWidget *box;
	gboolean vertical;
	
	switch (o) {
	case ORIENT_UP:
	case ORIENT_DOWN:
		vertical = FALSE;
		break;
	case ORIENT_LEFT:
	case ORIENT_RIGHT:
		vertical = TRUE;
		break;
	}
		
	if (swap) {
		gtk_widget_ref (swap->hbox);
		gtk_container_remove (GTK_CONTAINER(cpumemusage), swap->hbox);
	}
	gtk_widget_ref (cpu->hbox);
	gtk_container_remove (GTK_CONTAINER(cpumemusage), cpu->hbox);
	gtk_widget_ref (mem->hbox);
	gtk_container_remove (GTK_CONTAINER(cpumemusage), mem->hbox);
	
	box = pack_procbars (vertical);

	if (swap)
		gtk_widget_unref (swap->hbox);
	gtk_widget_unref (cpu->hbox);
	gtk_widget_unref (mem->hbox);

	gtk_container_remove (GTK_CONTAINER (applet), cpumemusage);
	gtk_container_add (GTK_CONTAINER (applet), box);
	cpumemusage = box;
}

int main(int argc, char **argv)
{
        applet_widget_init_defaults("cpumemusage_applet", VERSION, argc, argv,
				    NULL, 0, NULL);

	applet = applet_widget_new("cpumemusage_applet");
	if (!applet)
		g_error("Can't create applet!\n");

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

        cpumemusage = cpumemusage_widget();
        applet_widget_add( APPLET_WIDGET(applet), cpumemusage );
        gtk_widget_show(applet);

	/* Be nice */
	nice (NICE_VALUE);
	applet_widget_gtk_main();

        return 0;
}
