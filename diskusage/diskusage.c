/*
 * GNOME diskusage panel applet
 * (C) 1997 The Free Software Foundation
 * 
 * Author: Bruno Widmann
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

#include "diskusage.h"
#include "procbar.h"

/**/
#define DU_DEBUG
/*/*/

static GdkColor bar_filesystem_colors [FS_SIZE-1] = {
	{0, 0xcfff, 0x5fff, 0x5fff},
	{0, 0, 0x8fff, 0},
};

static gint update_values ();

static DiskusageInfo   summary_info;

static ProcBar    **filesystem; 
static GtkWidget *diskusage;
static int	n_ProcBars;
static GtkWidget *my_box; /* hbox or vbox where the procbars reside in */
static GtkWidget *my_hbox; /* hbox where the procbars can reside in */
static GtkWidget *my_vbox; /* vbox where the procbars can reside in */
static GtkWidget *my_frame; /* the frame wherer hbox or vbox reside in */



/*
 * on initialization, create the procbar. Recreate them when
 * number of mounted filesystems changed, or panel position changed
 * from horizontal to vertical. Remove all procbars, and generate
 * new ones
 */
static void update_diskusage_widget ()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	int i;	
	int cn;		/* current number  of bars */
	gint bar_o;	/* bar orientation   	   */

#ifdef DU_DEBUG
	printf ("update_diskusage_widget \n");
#endif
	cn = summary_info.n_filesystems;
	

#ifdef DU_DEBUG
	printf ("removing %d old procbars \n", n_ProcBars);
#endif
	/*
	 * remove all previously generated procbars (if any)
	 */
	for (i = 0; i < n_ProcBars; i++)
			gtk_container_remove ((GtkContainer*) my_box, 
						filesystem[i]->box);

	/*
	 * remove previously used box
	 */
	if (my_vbox != NULL) {
		gtk_widget_destroy (my_vbox);
		my_vbox = NULL;
#ifdef DU_DEBUG
		printf ("destroying my_vbox \n");
#endif
	}
	
	if (my_hbox != NULL) {
		gtk_widget_destroy (my_hbox);
		my_hbox = NULL;
#ifdef DU_DEBUG
		printf ("destroying my_hbox \n");
#endif
	}


	/*
	 * when panel is vertical, set bars to horizontal, and vica verca
	 */
	if ((summary_info.orient == ORIENT_LEFT)
		|| (summary_info.orient == ORIENT_RIGHT)) {
		bar_o = PROCBAR_HORIZONTAL;
#ifdef DU_DEBUG
		printf ("setting bar_o to PROCBAR_HORIZONTAL \n");
		gtk_widget_set_usize (my_frame, DU_FRAME_WIDTH, cn*DU_BAR_HEIGHT);
#endif
	} else {
		bar_o = PROCBAR_VERTICAL;
#ifdef DU_DEBUG
		printf ("setting bar_o to PROCBAR_VERTIKAL \n");
		gtk_widget_set_usize (my_frame, cn*DU_BAR_WIDTH, DU_FRAME_HEIGHT);
#endif
	}



	/*
	 * check weather to use vbox or hbox.
	 * If the other box was previously used,
	 * then remove it
	 * if the box whe shall use was not previously used,
	 * add it to my_frame
	 */
	if (bar_o == PROCBAR_HORIZONTAL) { /* horizontal procbars: we need vbox */
		vbox = gtk_vbox_new (TRUE, GNOME_PAD_SMALL >> 1);
		gtk_container_add ((GtkContainer*) my_frame, vbox);
		gtk_widget_show (vbox);
		my_vbox = vbox;
		my_box = my_vbox;
#ifdef DU_DEBUG
		printf ("gtk_container_add of vbox \n");
#endif
	} else {			   /* vertical procbars: we need hbox */
		hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL >> 1);
		gtk_container_add ((GtkContainer*) my_frame, hbox);
		gtk_widget_show (hbox);
		my_hbox = hbox;
		my_box = my_hbox;
#ifdef DU_DEBUG
		printf ("gtk_container_add of hbox \n");
#endif
	}


#ifdef DU_DEBUG
	printf ("debug: checkpoint \n");
#endif

	/*
	 * create procbar for Filesystem 0
	 */
	filesystem[0]  = procbar_new (NULL, 
				bar_o,
				FS_SIZE-1, 
				bar_filesystem_colors, 
				update_values);

	gtk_box_pack_start_defaults (GTK_BOX (my_box), 
				filesystem[0]->box);

	/*
	 * creage procbars for the rest of the filesystems
	 */
	for (i=1; i < cn; i++) {
#ifdef DU_DEBUG
		printf ("init fs-%d\n", i);
#endif
		filesystem[i]  = procbar_new (NULL,
					bar_o,
					FS_SIZE-1,
					bar_filesystem_colors,
					NULL);

		gtk_box_pack_start_defaults (GTK_BOX (my_box), 
					filesystem[i]->box);

	}
	n_ProcBars = cn;
	
	
}

static void
applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	summary_info.orient = o;
	update_diskusage_widget();
}


static gint
update_values ()
{
	int i;
/*
#ifdef DU_DEBUG
	printf ("update\n");
#endif
*/
	diskusage_read (&summary_info);

	if (summary_info.previous_n_filesystems != summary_info.n_filesystems)
		update_diskusage_widget();
	summary_info.previous_n_filesystems = summary_info.n_filesystems; 
	
	for (i = 0; i < summary_info.n_filesystems; i++) {
		procbar_set_values (filesystem[i], 
				summary_info.filesystems[i].sizeinfo);
/*
#ifdef DU_DEBUG
		printf ("%u %u %u \n", 	summary_info.filesystems[i].sizeinfo[0],
			summary_info.filesystems[i].sizeinfo[1],
			summary_info.filesystems[i].sizeinfo[2]);
#endif
*/
	}

	return TRUE;
}

static GtkWidget *
diskusage_widget ()
{
	int i;
	
	GtkWidget *frame;

	/*
	 * default to horizontal panel, as long as we don't know for sure
	 */
	summary_info.orient = ORIENT_UP;

	diskusage_read (&summary_info);
	summary_info.previous_n_filesystems = summary_info.n_filesystems; 

	/*
	 * gtk_widget_set_usize (vbox, 80, 40);
	 */
	
	
	frame = gtk_frame_new (NULL);
	/*
	 * default to horizontal panel, as long as we don't know for sure
	 */
	gtk_widget_set_usize (frame,
				summary_info.n_filesystems * DU_BAR_WIDTH, 
				DU_FRAME_HEIGHT);

	my_frame = frame;

	/*
	 * neither vbox or hbox was used yet, so set my_box to NULL
	 */
	my_box = NULL;

	my_vbox = NULL;
	my_hbox = NULL;


#ifdef DU_DEBUG
	printf ("calloc \n");
#endif
	filesystem = calloc(DU_MAX_FS, sizeof (ProcBar*));

	/*
	 * let update funktion know that there are no
	 * progress bars added to the our applet yet
	 */
	n_ProcBars = 0;
	
	update_diskusage_widget ();


	gtk_widget_show (frame);

	procbar_start (filesystem[0], DU_UPDATE_TIME);


	return frame;
}

static void
about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[] = {
		"Bruno Widmann (bwidmann@tks.fh-sbg.ac.at)",
	  NULL
	  };

	about = gnome_about_new ( "The GNOME Disk Usage Applet", "0.0.2",
			"shows a bargraph with the used space "
			"on each mounted filesystem.  ",
			authors,
			"This applet is released under "
			"the terms and conditions of the "
			"GNU Public Licence.",
			NULL);
	gtk_widget_show (about);

	return;
}

int main(int argc, char **argv)
{
	GtkWidget *applet;

        applet_widget_init_defaults("diskusage_applet", NULL, argc, argv, 0,
				    NULL,argv[0]);

	applet = applet_widget_new();
	if (!applet)
		g_error("Can't create applet!\n");

        diskusage = diskusage_widget();

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

        applet_widget_add( APPLET_WIDGET(applet), diskusage );

	/*set tooltip over the applet, NULL to remove a tooltip*/
	applet_widget_set_tooltip	(APPLET_WIDGET(applet), "Disk Usage");

        gtk_widget_show(applet);
       	
	applet_widget_register_callback(APPLET_WIDGET(applet),
					     "about",
                                             _("About..."),
                                             about_cb,
                                             NULL);

	
	applet_widget_gtk_main();

	free (filesystem);

        return 0;
}
