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
#include <stdlib.h>
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

#include "led.h"
#include "cdrom-interface.h"
#include "cdplayer.h"

#include "images/cdplayer-stop.xpm"
#include "images/cdplayer-play-pause.xpm"
#include "images/cdplayer-prev.xpm"
#include "images/cdplayer-next.xpm"
#include "images/cdplayer-eject.xpm"

#define TIMEOUT_VALUE 500

void cdpanel_realized(GtkWidget *cdpanel, CDPlayerData *cd);
    
GtkWidget *applet = NULL;

static gboolean
cd_try_open(CDPlayerData *cd)
{
	int err;
	if(cd->cdrom_device == NULL) {
		cd->cdrom_device = cdrom_open(cd->devpath, &err);
		return cd->cdrom_device != NULL;
	}
	return TRUE;
}

static void
cd_close (CDPlayerData *cd)
{
	if (cd->cdrom_device != NULL) {
		cdrom_close(cd->cdrom_device);
		cd->cdrom_device = NULL;
	}
}

static void 
cd_panel_update(GtkWidget * cdplayer, CDPlayerData * cd)
{
	cdrom_device_status_t stat;
	/* int retval; */

	if (cd_try_open(cd)) {
		if (cdrom_get_status(cd->cdrom_device, &stat) == DISC_NO_ERROR) {
			switch (stat.audio_status) {
			case DISC_PLAY:
				led_time(cd->panel.time,
					 stat.relative_address.minute,
					 stat.relative_address.second,
					 cd->panel.track_control.display,
					 stat.track);
				break;
			case DISC_PAUSED:
				led_paused(cd->panel.time,
					   stat.relative_address.minute,
					   stat.relative_address.second,
					   cd->panel.track_control.display,
					   stat.track);
				break;
			case DISC_COMPLETED:
				/* check for looping or ? */
				break;
			case DISC_STOP:
			case DISC_ERROR:
				led_stop(cd->panel.time, cd->panel.track_control.display);
				break;
			default:
				break;
			}
		}
		cd_close(cd);
	} else {
		led_nodisc(cd->panel.time, cd->panel.track_control.display);
	}
	return;
	cdplayer = NULL;
}

static void 
cdplayer_play_pause(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	cdrom_device_status_t stat;
	int status;

	if(!cd_try_open(cd))
		return;

	status = cdrom_get_status(cd->cdrom_device, &stat);
	if (status == DISC_NO_ERROR) {
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
	} else if(status == DISC_TRAY_OPEN) {
		cdrom_load(cd->cdrom_device);
		cdrom_read_track_info(cd->cdrom_device);
		cdrom_play(cd->cdrom_device, cd->cdrom_device->track0,
			   cd->cdrom_device->track1);
	}

	cd_close(cd);

	return;
	w = NULL;
}

static void
start_gtcd_cb(AppletWidget *w, gpointer data)
{
	gnome_execute_shell(NULL, "gtcd");
}

static void
help_cb (AppletWidget *w, gpointer data)
{
        GnomeHelpMenuEntry help_entry = { "cdplayer_applet",
                                          "index.html" };
        gnome_help_display(NULL, &help_entry);
}

static void
about_cb(AppletWidget *w, gpointer data)
{
        static GtkWidget   *about     = NULL;
        static const gchar *authors[] =
        {
                "Miguel de Icaza <miguel@kernel.org>",
                "Federico Mena <quartic@gimp.org>",
                NULL
        };

        if (about != NULL)
        {
                gdk_window_show(about->window);
                gdk_window_raise(about->window);
                return;
        }
        
        about = gnome_about_new (_("CD Player Applet"), VERSION,
                                 _("(c) 1997 The Free Software Foundation"),
                                 authors,
                                 _("The CD Player applet is a simple audio CD player for your panel"),
                                 NULL);
        gtk_signal_connect (GTK_OBJECT(about), "destroy",
                            GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
        gtk_widget_show (about);
}

static void 
cdplayer_stop(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	if(!cd_try_open(cd))
		return;
	cdrom_stop(cd->cdrom_device);

	cd_close(cd);
	return;
        w = NULL;
}

static void 
cdplayer_prev(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	if(!cd_try_open(cd))
		return;
	cdrom_prev(cd->cdrom_device);

	cd_close(cd);
	return;
        w = NULL;
}

static void 
cdplayer_next(GtkWidget * w, gpointer data)
{
	CDPlayerData *cd = data;
	if(!cd_try_open(cd))
		return;
	cdrom_next(cd->cdrom_device);

	cd_close(cd);
	return;
        w = NULL;
}

static void 
cdplayer_eject(GtkWidget * w, gpointer data)
{
	cdrom_device_status_t stat;
	CDPlayerData *cd = data;
	if(!cd_try_open(cd))
		return;
	if(cdrom_get_status(cd->cdrom_device, &stat)==DISC_TRAY_OPEN)
		/*FIXME: if there is no disc, we get TRAY_OPEN even
		  if the tray is closed, is this a kernel bug?? or
		  is this the inteded behaviour, we don't support this
		  on solaris anyway, but unless we have a good way to
		  find out if the tray is actually open, we can't do this*/
		/*cdrom_load(cd->cdrom_device);*/
		/*in the meantime let's just do eject so that at least that
		  works*/
		cdrom_eject(cd->cdrom_device);
	else
		cdrom_eject(cd->cdrom_device);
	cd_close(cd);
	return;
        w = NULL;
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
control_button_factory(gchar * pixmap_data[],
		       void (*func) (GtkWidget *, gpointer data),
		       CDPlayerData * cd)
{
	GtkWidget *w, *pixmap;

	w = gtk_button_new();
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
	pixmap = gnome_pixmap_new_from_xpm_d (pixmap_data);
	gtk_widget_show(pixmap);
	gtk_container_add(GTK_CONTAINER(w), pixmap);
	gtk_signal_connect(GTK_OBJECT(w), "clicked",
			   GTK_SIGNAL_FUNC(func), cd);
	gtk_widget_show(w);
	gtk_widget_ref(w);
	return w;
}

static GtkWidget *
create_cdpanel_widget(GtkWidget *window, CDPlayerData * cd)
{
	cd->panel.frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(cd->panel.frame), GTK_SHADOW_IN);
	gtk_widget_show(cd->panel.frame);
	
	led_init(window);

	/* time & track widgets */ 
	led_create_widget(window,&cd->panel.time, &cd->panel.track_control.display);
	gtk_widget_show(cd->panel.time);
	gtk_object_ref(GTK_OBJECT(cd->panel.time));

	cd->panel.play_control.stop = 
	  control_button_factory(stop_xpm, cdplayer_stop, cd);

	cd->panel.play_control.play_pause = 
	  control_button_factory(play_pause_xpm, cdplayer_play_pause, cd);

	cd->panel.play_control.eject = 
	  control_button_factory(eject_xpm, cdplayer_eject, cd);
	
	/* Track control.  */
	cd->panel.track_control.prev = 
	  control_button_factory(prev_xpm, cdplayer_prev, cd);

	gtk_widget_show(cd->panel.track_control.display);
	gtk_widget_ref(cd->panel.track_control.display);

	cd->panel.track_control.next = 
	  control_button_factory(next_xpm, cdplayer_next, cd);
	
	return cd->panel.frame;
}

static void 
destroy_cdplayer(GtkWidget * widget, gpointer data)
{
	CDPlayerData *cd;

	cd = gtk_object_get_user_data(GTK_OBJECT(widget));
	if (cd->timeout != 0)
		gtk_timeout_remove(cd->timeout);
	cd->timeout = 0;
	cd_close (cd);

	g_free (cd->devpath);
	cd->devpath = NULL;

	g_free(cd);

	return;
	data = NULL;
}

void
cdpanel_realized(GtkWidget *cdpanel, CDPlayerData *cd)
{
	cd_panel_update(cdpanel, cd);
}

static void
ref_and_remove(GtkWidget *w)
{
	GtkWidget *parent = w->parent;
	if(parent) {
		gtk_widget_ref(w);
		gtk_container_remove(GTK_CONTAINER(parent), w);
	}
}

static void
destroy_box(CDPlayerData* cd)
{
	g_return_if_fail(GTK_IS_CONTAINER(cd->panel.box));

	ref_and_remove(cd->panel.time);

	ref_and_remove(cd->panel.play_control.stop);
	ref_and_remove(cd->panel.play_control.play_pause);
	ref_and_remove(cd->panel.play_control.eject);

	ref_and_remove(cd->panel.track_control.prev);
	ref_and_remove(cd->panel.track_control.display);
	ref_and_remove(cd->panel.track_control.next);

	gtk_widget_destroy(cd->panel.box);
	cd->panel.box = NULL;
}

static GtkWidget *
pack_make_hbox(CDPlayerData* cd)
{
	GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(cd->panel.box), hbox,
			   TRUE, TRUE, 0);
	return hbox;
}

static void
pack_thing(GtkWidget *box, GtkWidget *w, gboolean expand)
{
	gtk_box_pack_start(GTK_BOX(box), w, expand, TRUE, 0);
	gtk_widget_unref(w);
}


static void
setup_box(CDPlayerData* cd)
{
	GtkWidget *hbox;

	if(cd->panel.box)
		destroy_box(cd);

	if ((cd->orient == ORIENT_DOWN || cd->orient == ORIENT_UP) &&
	    cd->size < 36  ) {
		/* tiny horizontal panel */
		cd->panel.box = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame),
				  cd->panel.box);
		gtk_widget_show(cd->panel.box);

		pack_thing(cd->panel.box, cd->panel.track_control.prev, FALSE);
		pack_thing(cd->panel.box,
			   cd->panel.track_control.display, TRUE);
		pack_thing(cd->panel.box, cd->panel.track_control.next, FALSE);

		pack_thing(cd->panel.box, cd->panel.time, TRUE);

		pack_thing(cd->panel.box, cd->panel.play_control.stop, FALSE);
		pack_thing(cd->panel.box,
			   cd->panel.play_control.play_pause, FALSE);
		pack_thing(cd->panel.box, cd->panel.play_control.eject, FALSE);
	} else if ((cd->orient == ORIENT_DOWN || cd->orient == ORIENT_UP) &&
		   cd->size < 48  ) {
		/* small horizontal panel */
		cd->panel.box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame),
				  cd->panel.box);
		gtk_widget_show(cd->panel.box);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.time, TRUE);
		pack_thing(hbox, cd->panel.track_control.display, TRUE);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.play_control.stop, FALSE);
		pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
		pack_thing(hbox, cd->panel.play_control.eject, FALSE);
		pack_thing(hbox, cd->panel.track_control.prev, FALSE);
		pack_thing(hbox, cd->panel.track_control.next, FALSE);
	} else if ((cd->orient == ORIENT_LEFT || cd->orient == ORIENT_RIGHT) &&
		   cd->size < 48  ) {
		/* small vertical panel (won't work too great on tiny verticals
		 * but oh well) */
		cd->panel.box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame),
				  cd->panel.box);
		gtk_widget_show(cd->panel.box);

		pack_thing(cd->panel.box, cd->panel.time, TRUE);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.play_control.stop, FALSE);
		pack_thing(hbox, cd->panel.play_control.play_pause, TRUE);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.play_control.eject, FALSE);
		pack_thing(hbox, cd->panel.track_control.display, TRUE);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.track_control.prev, TRUE);
		pack_thing(hbox, cd->panel.track_control.next, TRUE);
	} else {
		/* other panel sizes/orientations should go here */
		cd->panel.box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame),
				  cd->panel.box);
		gtk_widget_show(cd->panel.box);

		pack_thing(cd->panel.box, cd->panel.time, TRUE);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.play_control.stop, FALSE);
		pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
		pack_thing(hbox, cd->panel.play_control.eject, FALSE);

		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.track_control.prev, FALSE);
		pack_thing(hbox, cd->panel.track_control.display, TRUE);
		pack_thing(hbox, cd->panel.track_control.next, FALSE);
	}
}

static GtkWidget *
create_cdplayer_widget(GtkWidget *window, char *globcfgpath)
{
	CDPlayerData *cd;
	GtkWidget *cdpanel;
	int err;

	cd = g_new0(CDPlayerData, 1);

	gnome_config_push_prefix(globcfgpath);
	/* NOTE THAT devpath is GLOBAL!, EEK! */

	/* default to the right thing on solaris */
#if defined(sun) || defined(__sun__)
#if defined(SVR4) || defined(__svr4__)
	cd->devpath = gnome_config_get_string("cdplayer/devpath=/vol/dev/aliases/cdrom0");
#else
	cd->devpath = gnome_config_get_string("cdplayer/devpath=/dev/rcd0");
#endif
#else
	/* everything else including linux */
	cd->devpath = gnome_config_get_string("cdplayer/devpath=/dev/cdrom");
#endif
	gnome_config_pop_prefix();
	cd->cdrom_device = cdrom_open(cd->devpath, &err);
	
	cd->size = APPLET_WIDGET(window)->size;
	cd->orient = APPLET_WIDGET(window)->orient;

	cdpanel = create_cdpanel_widget(window, cd);
	cd->panel.box = NULL;
	setup_box(cd);

	/* Install timeout handler */

	cd->timeout = gtk_timeout_add(TIMEOUT_VALUE, cdplayer_timeout_callback,
				      cdpanel);

	gtk_object_set_user_data (GTK_OBJECT(cdpanel), cd);
	gtk_signal_connect (GTK_OBJECT(cdpanel), "destroy",
			    GTK_SIGNAL_FUNC (destroy_cdplayer),
			    NULL);
	gtk_signal_connect (GTK_OBJECT (cdpanel), "realize",
			    GTK_SIGNAL_FUNC (cdpanel_realized), cd);

	return cdpanel;
}


static void
applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
        CDPlayerData *cd = data;

        cd->size = size;

	setup_box(cd);

        cd_panel_update(cd->panel.frame, cd);
}

static void
applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
        CDPlayerData *cd = data;
        
        cd->orient = o;
        
	setup_box(cd);

        cd_panel_update(cd->panel.frame, cd);
}



int
main(int argc, char **argv)
{
	GtkWidget *cdplayer;
	CDPlayerData *cd;


	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	applet_widget_init ("cdplayer_applet", VERSION, argc, argv,
			    NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/mc/i-cdrom.png");

	applet = applet_widget_new("cdplayer_applet");
	if (!applet)
		g_error("Can't create applet!\n");

	gtk_widget_realize(applet);
	cdplayer = create_cdplayer_widget (applet,
					   APPLET_WIDGET(applet)->globcfgpath);

	if (cdplayer == NULL) {
		applet_widget_abort_load(APPLET_WIDGET(applet));
		return 1;
	}

	cd = (CDPlayerData*)gtk_object_get_user_data(GTK_OBJECT(cdplayer));
	
	if (cd == NULL) {
	        applet_widget_abort_load(APPLET_WIDGET(applet));
		return 1;
	}

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient), cd);
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
			   GTK_SIGNAL_FUNC(applet_change_pixel_size), cd);

        applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "run_gtcd",         
					       GNOME_STOCK_MENU_CDROM,
					       _("Run CD Player..."),
					       start_gtcd_cb,
					       NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"),
					       help_cb,
					       NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       about_cb,
					       NULL);
       

	gtk_widget_show(cdplayer);
	applet_widget_add (APPLET_WIDGET (applet), cdplayer);
	gtk_widget_show (applet);

	applet_widget_gtk_main ();

	return 0;
}
