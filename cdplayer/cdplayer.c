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
#include "gnome.h"
#include "../panel_cmds.h"
#include "../applet_cmds.h"
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

static PanelCmdFunc panel_cmd_func;

gpointer applet_cmd_func(AppletCommand *cmd);

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

static GtkWidget *
create_cdplayer_widget (GtkWidget *window, char *params)
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

static void
create_instance (Panel *panel, char *params, int pos)
{
  PanelCommand cmd;
  GtkWidget *cdplayer;

  cdplayer = create_cdplayer_widget (panel->window, params);
  if (cdplayer == NULL)
    return;
  cmd.cmd = PANEL_CMD_REGISTER_TOY;
  cmd.params.register_toy.applet = cdplayer;
  cmd.params.register_toy.id     = APPLET_ID;
  cmd.params.register_toy.pos   = pos;
  cmd.params.register_toy.flags  = APPLET_HAS_PROPERTIES;

  (*panel_cmd_func) (&cmd);
}

gpointer
applet_cmd_func(AppletCommand *cmd)
{
  g_assert(cmd != NULL);

  switch (cmd->cmd) {
  case APPLET_CMD_QUERY:
    return APPLET_ID;

  case APPLET_CMD_INIT_MODULE:
    panel_cmd_func = cmd->params.init_module.cmd_func;
    led_init(cmd->panel->window);
    break;

  case APPLET_CMD_DESTROY_MODULE:
    led_done();
    break;

  case APPLET_CMD_GET_DEFAULT_PARAMS:
    return g_strdup("");

  case APPLET_CMD_CREATE_INSTANCE:
    create_instance(cmd->panel,
		    cmd->params.create_instance.params,
		    cmd->params.create_instance.pos);
    break;

  case APPLET_CMD_GET_INSTANCE_PARAMS:
    return g_strdup("");

  case APPLET_CMD_ORIENTATION_CHANGE_NOTIFY:
    break;

  case APPLET_CMD_PROPERTIES:
    fprintf(stderr, "Clock properties not yet implemented\n"); /* FIXME */
    break;

  default:
    fprintf(stderr,
	    APPLET_ID " applet_cmd_func: Oops, unknown command type %d\n",
	    (int) cmd->cmd);
    break;
  }

  return NULL;
}
