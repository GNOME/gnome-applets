/*
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
#include <applet-widget.h>
#include "../panel.h"

#include "led.h"
#include "cdrom-interface.h"
#include "cdplayer.h"

#include "images/cdplayer-stop.xpm"
#include "images/cdplayer-play-pause.xpm"
#include "images/cdplayer-prev.xpm"
#include "images/cdplayer-next.xpm"
#include "images/cdplayer-eject.xpm"

#define APPLET_ID "Cdplayer"
#define TIMEOUT_VALUE 500

int applet_id=-1; /*this is our id we use to comunicate with the panel*/

static void
cd_panel_update(GtkWidget *cdplayer, CDPlayerData *cd)
{
  cdrom_device_status_t stat;
  int retval;

  if (cdrom_get_status(cd->cdrom_device, &stat) == DISC_NO_ERROR)
    {
      switch(stat.audio_status)
	{
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
    }
  else
    led_nodisc(cd->panel.time, cd->panel.track);
}

static int
cdplayer_play_pause(GtkWidget *w, gpointer data)
{
  CDPlayerData *cd = data;
  cdrom_device_status_t stat;

  if (cdrom_get_status(cd->cdrom_device, &stat) == DISC_NO_ERROR)
    {
      switch(stat.audio_status)
	{
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
cdplayer_stop(GtkWidget *w, gpointer data)
{
  CDPlayerData *cd = data;
  cdrom_stop(cd->cdrom_device);
  return 0;
}

static int 
cdplayer_prev(GtkWidget *w, gpointer data)
{
  CDPlayerData *cd = data;
  cdrom_prev(cd->cdrom_device);
  return 0;
}

static int 
cdplayer_next(GtkWidget *w, gpointer data)
{
  CDPlayerData *cd = data;
  cdrom_next(cd->cdrom_device);
  return 0;
}

static int 
cdplayer_eject(GtkWidget *w, gpointer data)
{
  CDPlayerData *cd = data;
  cdrom_eject(cd->cdrom_device);
  return 0;
}

static int
cdplayer_timeout_callback (gpointer data)
{
  GtkWidget *cdplayer;
  CDPlayerData *cd;

  cdplayer = data;
  cd = gtk_object_get_user_data(GTK_OBJECT(cdplayer));

  cd_panel_update (cdplayer, cd);
  return 1;
}

static GtkWidget *
control_button_factory(GtkWidget *window, GtkWidget *box_container, 
		       gchar *pixmap_data[], 
		       int (*func)(GtkWidget *, gpointer data), 
		       CDPlayerData *cd)
{
  GtkWidget *w, *pixmap;

  w = gtk_button_new();
  GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
  GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
  pixmap = gnome_create_pixmap_widget_d (window, w, pixmap_data);
  gtk_box_pack_start(GTK_BOX(box_container), w, FALSE, TRUE, 0);
  gtk_widget_show(pixmap);
  gtk_container_add (GTK_CONTAINER(w), pixmap);
  gtk_signal_connect(GTK_OBJECT(w), "clicked", 
		     GTK_SIGNAL_FUNC(func), 
		     (gpointer) cd);
  gtk_widget_show(w);
  return w;
}

static GtkWidget *
create_cdpanel_widget(GtkWidget *window, CDPlayerData *cd)
{
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *pixmap;

  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
  gtk_widget_show(frame);

  vbox = gtk_vbox_new(FALSE, FALSE);
  gtk_container_add(GTK_CONTAINER(frame), vbox);
  gtk_widget_show(vbox);

  led_create_widget(window, &cd->panel.time, &cd->panel.track);
  gtk_box_pack_start_defaults(GTK_BOX(vbox), cd->panel.time);
  gtk_widget_show(cd->panel.time);

  /* */
  hbox = gtk_hbox_new(FALSE, FALSE);
  gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
  gtk_widget_show(hbox);

  cd->panel.stop = control_button_factory(window, hbox, stop_xpm,
					  cdplayer_stop, cd);
  cd->panel.play_pause = control_button_factory(window, hbox, play_pause_xpm,
						cdplayer_play_pause, cd);
  cd->panel.eject = control_button_factory(window, hbox, eject_xpm,
					   cdplayer_eject, cd);
  
  /* */
  hbox = gtk_hbox_new(FALSE, FALSE);
  gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);
  gtk_widget_show(hbox);

  cd->panel.prev = control_button_factory(window, hbox, prev_xpm,
					  cdplayer_prev, cd);

  gtk_box_pack_start(GTK_BOX(hbox), cd->panel.track, TRUE, TRUE, 0);
  gtk_widget_show(cd->panel.track);

  cd->panel.next = control_button_factory(window, hbox, next_xpm,
					  cdplayer_next, cd);

  return frame;
}

static void
destroy_cdplayer (GtkWidget *widget, void *data)
{
  CDPlayerData *cd;

  cd = gtk_object_get_user_data(GTK_OBJECT(widget));
  gtk_timeout_remove (cd->timeout);
  cdrom_close(cd->cdrom_device);
  g_free(cd);
}

/*these are commands sent over corba:*/
void
change_orient(int id, int orient)
{
	PanelOrientType o = (PanelOrientType)orient;
	puts("CHANGE_ORIENT");
}

void
session_save(int id, int panel, int pos)
{
	/*FIXME: save the position*/
	puts("SESSION_SAVE");
}

static gint
applet_die(gpointer data)
{
	exit(0);
}

void
shutdown_applet(int id)
{
	puts("SHUTDOWN_APPLET");
	/*kill our window*/
	gtk_widget_unref(aw);
	gtk_timeout_add(100,applet_die,NULL);
	led_done();
}

static GtkWidget *
create_cdplayer_widget (GtkWidget *window)
{
  gchar *devpath;
  CDPlayerData    *cd;
  GtkWidget       *cdpanel;
  time_t           current_time;
  int err;

  cd = g_new(CDPlayerData, 1);

  devpath = gnome_config_get_string("/panel/cdplayer/devpath=/dev/cdrom");
  cd->cdrom_device = cdrom_open(devpath, &err);
  g_free(devpath);
  if (cd->cdrom_device == NULL)
    {
      printf("Error: Can't open cdrom(%s)\n", strerror(err));
      g_free(cd);
      return NULL;
    }

  cdpanel = create_cdpanel_widget(window, cd);

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

int
main(int argc, char **argv)
{
	GtkWidget *cdplayer;
	char *result;
	
	gnome_init("cdplayer_applet", &argc, &argv);

	if (!gnome_panel_applet_init_corba (&argc, &argv)){
		fprintf (stderr, "Could not comunicate with the panel\n");
		exit (1);
	}

	led_init();

	aw = applet_widget_new ();

	cdplayer = create_cdplayer_widget (GTK_WIDGET(panel));
	gtk_widget_show(cdplayer);
	applet_widget_add (APPLET_WIDGET (aw), cdplayer);
	gtk_widget_show (aw);

	/*FIXME: do session saving, find out panel and pos from the panel
		 so we can restore them on the next startup*/
	result = gnome_panel_prepare_and_transfer(aw,argv[0],&applet_id,0,0);
	printf ("Done\n");
	if (result){
		printf ("Could not talk to the Panel: %s\n", result);
		exit (1);
	}

	applet_corba_gtk_main ("IDL:GNOME/Applet:1.0");

	return 0;
}
