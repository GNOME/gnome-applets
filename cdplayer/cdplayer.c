/*
 * FIXME: who wrote this ... this is the header from clock .... this isn't
 * clock :)
 * GNOME time/date display module.
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 *
 * Feel free to implement new look and feels :-)
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
#include <applet-lib.h>
#include "panel.h"
#include "mico-parse.h"

#include "led.h"
#include "cdrom-interface.h"
#include "cdplayer.h"

#include "images/cdplayer-stop.xpm"
#include "images/cdplayer-play-pause.xpm"
#include "images/cdplayer-prev.xpm"
#include "images/cdplayer-next.xpm"
#include "images/cdplayer-eject.xpm"

#define TIMEOUT_VALUE 500

GtkWidget *plug = NULL;

int applet_id = -1;/*this is our id we use to comunicate with the panel */

static void 
cd_panel_update(GtkWidget * cdplayer, CDPlayerData * cd)
{
	cdrom_device_status_t stat;
	int retval;

	if (cdrom_get_status(cd->cdrom_device, &stat) == DISC_NO_ERROR) {
		switch (stat.audio_status) {
		case DISC_PLAY:
			led_time(cd->panel.time,
				 stat.relative_address.minute,
				 stat.relative_address.second,
				 cd->panel.track,
				 stat.track);
			break;
		case DISC_PAUSED:
			led_paused(cd->panel.time,
				   stat.relative_address.minute,
				   stat.relative_address.second,
				   cd->panel.track,
				   stat.track);
			break;
		case DISC_COMPLETED:
			/* check for looping or ? */
			break;
		case DISC_STOP:
		case DISC_ERROR:
			led_stop(cd->panel.time, cd->panel.track);
		default:

		}
	} else
		led_nodisc(cd->panel.time, cd->panel.track);
}

static int 
cdplayer_play_pause(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	cdrom_device_status_t stat;

	if (cdrom_get_status(cd->cdrom_device, &stat) == DISC_NO_ERROR) {
		switch (stat.audio_status) {
		case DISC_PLAY:
			cdrom_pause(cd->cdrom_device);
			break;
		case DISC_PAUSED:
			cdrom_resume(cd->cdrom_device);
			break;
		case DISC_COMPLETED:
		case DISC_STOP:
		case DISC_ERROR:
			cdrom_read_track_info(cd->cdrom_device);
			cdrom_play(cd->cdrom_device, cd->cdrom_device->track0,
				   cd->cdrom_device->track1);
			break;
		}
	}
}

static int 
cdplayer_stop(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	cdrom_stop(cd->cdrom_device);
	return 0;
}

static int 
cdplayer_prev(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	cdrom_prev(cd->cdrom_device);
	return 0;
}

static int 
cdplayer_next(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	cdrom_next(cd->cdrom_device);
	return 0;
}

static int 
cdplayer_eject(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	cdrom_eject(cd->cdrom_device);
	return 0;
}

static int 
cdplayer_timeout_callback(gpointer data)
{
	GtkWidget *cdplayer;
	CDPlayerData *cd;

	cdplayer = data;
	cd = gtk_object_get_user_data(GTK_OBJECT(cdplayer));

	cd_panel_update(cdplayer, cd);
	return 1;
}

static GtkWidget *
control_button_factory(GtkWidget * box_container,
		       gchar * pixmap_data[],
		       int (*func) (GtkWidget *, gpointer data),
		       CDPlayerData * cd)
{
	GtkWidget *w, *pixmap;

	w = gtk_button_new();
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
	pixmap = gnome_pixmap_new_from_xpm_d (pixmap_data);
	gtk_box_pack_start(GTK_BOX(box_container), w, FALSE, TRUE, 0);
	gtk_widget_show(pixmap);
	gtk_container_add(GTK_CONTAINER(w), pixmap);
	gtk_signal_connect(GTK_OBJECT(w), "clicked",
			   GTK_SIGNAL_FUNC(func),
			   (gpointer) cd);
	gtk_widget_show(w);
	return w;
}

static GtkWidget *
create_cdpanel_widget(GtkWidget *window, CDPlayerData * cd)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
	gtk_widget_show(frame);

	vbox = gtk_vbox_new(FALSE, FALSE);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	led_init(window);

	led_create_widget(window,&cd->panel.time, &cd->panel.track);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), cd->panel.time);
	gtk_widget_show(cd->panel.time);

	/* */
	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);

	cd->panel.stop = control_button_factory(hbox, stop_xpm,
						cdplayer_stop, cd);
	cd->panel.play_pause = control_button_factory(hbox, play_pause_xpm,
						cdplayer_play_pause, cd);
	cd->panel.eject = control_button_factory(hbox, eject_xpm,
						 cdplayer_eject, cd);

	/* */
	hbox = gtk_hbox_new(FALSE, FALSE);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
	gtk_widget_show(hbox);

	cd->panel.prev = control_button_factory(hbox, prev_xpm,
						cdplayer_prev, cd);

	gtk_box_pack_start(GTK_BOX(hbox), cd->panel.track, TRUE, TRUE, 0);
	gtk_widget_show(cd->panel.track);

	cd->panel.next = control_button_factory(hbox, next_xpm,
						cdplayer_next, cd);

	return frame;
}

static void 
destroy_cdplayer(GtkWidget * widget, void *data)
{
	CDPlayerData *cd;

	cd = gtk_object_get_user_data(GTK_OBJECT(widget));
	gtk_timeout_remove(cd->timeout);
	cdrom_close(cd->cdrom_device);
	g_free(cd);
}


static GtkWidget *
create_cdplayer_widget(GtkWidget *window)
{
	gchar *devpath;
	CDPlayerData *cd;
	GtkWidget *cdpanel;
	time_t current_time;
	int err;

	cd = g_new(CDPlayerData, 1);

	devpath = gnome_config_get_string("/panel/cdplayer/devpath=/dev/cdrom");
	cd->cdrom_device = cdrom_open(devpath, &err);
	g_free(devpath);
	if (cd->cdrom_device == NULL) {
		printf("Error: Can't open cdrom(%s)\n", strerror(err));
		g_free(cd);
		return NULL;
	}
	cdpanel = create_cdpanel_widget(window,cd);

	/* Install timeout handler */

	cd->timeout = gtk_timeout_add(TIMEOUT_VALUE, cdplayer_timeout_callback,
				      cdpanel);

	gtk_object_set_user_data(GTK_OBJECT(cdpanel), cd);
	gtk_signal_connect(GTK_OBJECT(cdpanel), "destroy",
			   (GtkSignalFunc) destroy_cdplayer,
			   NULL);
	cd_panel_update(cdpanel, cd);

	return cdpanel;
}

/*these are commands sent over corba:*/
void
change_orient(int id, int orient)
{
	PanelOrientType o = (PanelOrientType)orient;
}

void
session_save(int id, const char *cfgpath, const char *globcfgpath)
{
	/*save the session here*/
}

static gint
destroy_plug(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}

int
main(int argc, char **argv)
{
	GtkWidget *cdplayer;
	char *result;
	char *cfgpath;
	char *globcfgpath;

	char *myinvoc;
	guint32 winid;

	myinvoc = get_full_path(argv[0]);
	if(!myinvoc)
		return 1;

	panel_corba_register_arguments ();
	gnome_init("cdplayer_applet", NULL, argc, argv, 0, NULL);

	if (!gnome_panel_applet_init_corba ())
		g_error ("Could not comunicate with the panel\n");

	result = gnome_panel_applet_request_id(myinvoc,&applet_id,
					       &cfgpath,&globcfgpath,
					       &winid);

	g_free(myinvoc);
	if (result)
		g_error ("Could not talk to the Panel: %s\n", result);

	/*use cfg path for loading up data!*/

	g_free(globcfgpath);
	g_free(cfgpath);

	plug = gtk_plug_new (winid);
	gtk_widget_realize(plug);

	cdplayer = create_cdplayer_widget (plug);
	gtk_widget_show(cdplayer);
	gtk_container_add (GTK_CONTAINER (plug), cdplayer);
	gtk_widget_show (plug);
	gtk_signal_connect(GTK_OBJECT(plug),"destroy",
			   GTK_SIGNAL_FUNC(destroy_plug),
			   NULL);


	result = gnome_panel_applet_register(plug,applet_id);
	if (result)
		g_error ("Could not talk to the Panel: %s\n", result);

	applet_corba_gtk_main ("IDL:GNOME/Applet:1.0");

	return 0;
}
