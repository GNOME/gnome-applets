/*
 * GNOME diskusage panel applet
 * (C) 1997 The Free Software Foundation
 * 
 * Author: Bruno Widmann
 * 	   Modified from code taken from 
 * 	   cpumemusage & cpuload applets.
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
#include "properties.h"
#include "mountlist.h"

/*#define DU_DEBUG*/

static GdkColor bar_filesystem_colors [FS_SIZE-1] = {
	{0, 0xcfff, 0x5fff, 0x5fff},
	{0, 0, 0x8fff, 0},
};

diskusage_properties props;

static gint update_values ();

static DiskusageInfo   summary_info;

static ProcBar **filesystem; /* a array of pointers to the procbars */
static GtkWidget *diskusage;
static GtkWidget *my_box;    /* hbox or vbox where the procbars reside */
static GtkWidget *my_hbox;   /* hbox where the procbars can reside in  */
static GtkWidget *my_vbox;   /* vbox where the procbars can reside in  */
static GtkWidget *my_frame;  /* the frame wherer hbox or vbox reside   */
static GtkWidget *my_applet;


/*
 * when panel is vertical, set bars to horizontal, and vica verca
 * also, this set's the widget's size, depending of the number of bars,
 * and the orientation
 */
void set_procbar_orientation ()
{
	int cn;		/* current number  of bars */
	cn = summary_info.n_filesystems;
	
	if ((summary_info.orient == ORIENT_LEFT)
		|| (summary_info.orient == ORIENT_RIGHT)) {
		summary_info.bar_o = PROCBAR_HORIZONTAL;
#ifdef DU_DEBUG
		printf ("setting bar_o to PROCBAR_HORIZONTAL \n");
#endif
		gtk_widget_set_usize (my_frame, DU_FRAME_WIDTH, cn*DU_BAR_HEIGHT);
	} else {
		summary_info.bar_o = PROCBAR_VERTICAL;
#ifdef DU_DEBUG
		printf ("setting bar_o to PROCBAR_VERTIKAL \n");
#endif
		gtk_widget_set_usize (my_frame, cn*DU_BAR_WIDTH, DU_FRAME_HEIGHT);
	}

}

void set_diskusage_tooltip ()
{

	int i;
	int l;
	int cn;		/* current number  of bars */
	gchar *tooltip;

	cn = summary_info.n_filesystems;
	l=0;
	for (i = 0; i < cn; i++)
		l += strlen(summary_info.filesystems[i].mount_dir);

	l += cn +1;

	tooltip = g_new (gchar, l);

	strcpy(tooltip, summary_info.filesystems[0].mount_dir );
	for (i = 1; i < cn; i++) {
		strcat(tooltip, "\n");
		strcat(tooltip, summary_info.filesystems[i].mount_dir );
	}

#ifdef DU_DEBUG
	printf ("%s \n", tooltip);
#endif
	applet_widget_set_tooltip (APPLET_WIDGET(my_applet), tooltip);

	free (tooltip);

}


/*
 * on initialization, create the procbars, and the box holding them 
 */
static void init_diskusage_widget ()
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	int i;	
	int cn;		/* current number  of bars */

#ifdef DU_DEBUG
	printf ("init_diskusage_widget \n");
#endif

	cn = summary_info.n_filesystems;

	set_procbar_orientation(); 

	/*
	 * check weather to use vbox or hbox.
	 * and add it to my_frame
	 */
	if (summary_info.bar_o == PROCBAR_HORIZONTAL) { /* horizontal procbars: we need vbox */
		vbox = gtk_vbox_new (TRUE, GNOME_PAD_SMALL >> 1);
		gtk_container_add ((GtkContainer*) my_frame, vbox);
		gtk_widget_show (vbox);
		my_vbox = vbox;
		my_box = my_vbox;
	} else {			   /* vertical procbars: we need hbox */
		hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL >> 1);
		gtk_container_add ((GtkContainer*) my_frame, hbox);
		gtk_widget_show (hbox);
		my_hbox = hbox;
		my_box = my_hbox;
	}


	/*
	 * create procbar for Filesystem 0
	 */
	filesystem[0]  = procbar_new (NULL, 
				summary_info.bar_o,
				FS_SIZE-1, 
				bar_filesystem_colors, 
				update_values);
	
	/*
	 * start the update timer
	 */
	procbar_start (filesystem[0], DU_UPDATE_TIME);


	gtk_box_pack_start_defaults (GTK_BOX (my_box), 
				filesystem[0]->box);

	/*
	 * creage procbars for the rest of the filesystems
	 */
	for (i=1; i < cn; i++) {
		filesystem[i]  = procbar_new (NULL,
					summary_info.bar_o,
					FS_SIZE-1,
					bar_filesystem_colors,
					NULL);

		gtk_box_pack_start_defaults (GTK_BOX (my_box), 
					filesystem[i]->box);

	}
	summary_info.n_ProcBars = cn;
	
	set_diskusage_tooltip();
	
}


/*
 * when number of mounted filesystems changed, or panel position changed
 * Remove / add procbars appropiatly
 */
static void update_diskusage_widget ()
{
	int i;
	int cn;		/* current number  of bars */
	int pn;		/* previous number  of bars */

#ifdef DU_DEBUG
	printf ("update_diskusage_widget \n");
#endif
	cn = summary_info.n_filesystems;
	pn = summary_info.previous_n_filesystems;

	/*
	 * this resizes the widget appropiatley
	 */
	set_procbar_orientation();
		
	if (cn > summary_info.n_ProcBars) {
		for (i=summary_info.n_ProcBars; i < cn; i++) {
			filesystem[i]  = procbar_new (NULL,
					summary_info.bar_o,
					FS_SIZE-1,
					bar_filesystem_colors,
					NULL);
			gtk_box_pack_start_defaults (GTK_BOX (my_box), 
					filesystem[i]->box);
		}	
		summary_info.n_ProcBars = cn;
	}
	
	if (cn < pn) {
		for (i = cn; i < pn; i++)
			gtk_widget_hide (filesystem[i]->box);
	} else if (cn > pn) {
		for (i = pn; i < cn; i++) {
			gtk_widget_show (filesystem[i]->box);
		}
	}
	set_diskusage_tooltip();
}

/*
 * when panel orientation changes, delete old procpars 
 * and remove the old v/h box then call init_diskusage again
 */
static void
applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	int i;

#ifdef DU_DEBUG
	printf ("entering applet_change_orient  \n");
#endif
	if (summary_info.orient != o) {

		summary_info.orient = o;

		procbar_stop (filesystem[0]);
		
		/*
		 * remove all  procbars from the frame
		 */
		for (i = 0; i < summary_info.n_ProcBars; i++)
			gtk_container_remove ((GtkContainer*) my_box, 
						filesystem[i]->box);
		summary_info.n_ProcBars = 0;

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
		init_diskusage_widget();
	}
		
#ifdef DU_DEBUG
	printf ("leaving applet_change_orient  \n");
#endif
}


/*
 * read new filesystem-info, and update all the bars. If # of
 * filesystems has changed, call update_diskusage_widget to create
 * the correct # of procbars, and to set the correct widget-size
 */
static gint
update_values ()
{
	int i;

#ifdef DU_DEBUG
	printf ("update\n");
#endif

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

#ifdef DU_DEBUG
	printf ("entering diskusage_widget  \n");
#endif

	/*
	 * default to horizontal panel, as long as we don't know for sure
	 */
	summary_info.orient = ORIENT_UP;

	diskusage_read (&summary_info);
	summary_info.previous_n_filesystems = summary_info.n_filesystems; 

	
	frame = gtk_frame_new (NULL);

	my_frame = frame;

	/*
	 * neither vbox or hbox was used yet, so set my_box to NULL
	 */
	my_box = NULL;
	my_vbox = NULL;
	my_hbox = NULL;

	filesystem = calloc(DU_MAX_FS, sizeof (ProcBar*));

	/*
	 * let update funktion know that there are no
	 * progress bars added to the our applet yet
	 */
	summary_info.n_ProcBars = 0;
	
	init_diskusage_widget ();


	gtk_widget_show (frame);

#ifdef DU_DEBUG
	printf ("leaving diskusage_widget  \n");
#endif


	return frame;
}

static void
about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[] = {
		"Bruno Widmann (bwidmann@tks.fh-sbg.ac.at)",
		"Martin Baulig (martin@home-of-linux.org)",
	  NULL
	  };

	about = gnome_about_new ( "The GNOME Disk Usage Applet", "0.0.3",
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

	my_applet = applet;
        
	load_properties(APPLET_WIDGET(applet)->cfgpath, &props);

        diskusage = diskusage_widget();

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);
#ifdef DU_DEBUG
	printf ("applet_widget_add \n");
#endif

        applet_widget_add( APPLET_WIDGET(applet), diskusage );

#ifdef DU_DEBUG
	printf ("applet_set_tooltip\n");
#endif



#ifdef DU_DEBUG
	printf ("applet_widget_show\n");
#endif

        gtk_widget_show(applet);
       	
	applet_widget_register_callback(APPLET_WIDGET(applet),
					     "about",
                                             _("About..."),
                                             about_cb,
                                             NULL);
       	
	applet_widget_register_callback(APPLET_WIDGET(applet),
					"properties",
                                        _("Properties..."),
                                        properties,
                                        NULL);

	
	applet_widget_gtk_main();

	free (filesystem);

        return 0;
}
