/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 * GNOME 2 Action:
 * 			Chris Phelps
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <gdk/gdkx.h>
#include <panel-applet.h>

#include "led.h"
#include "cdrom-interface.h"
#include "cdplayer.h"

#include "images/cdplayer-stop.xpm"
#include "images/cdplayer-play-pause.xpm"
#include "images/cdplayer-prev.xpm"
#include "images/cdplayer-next.xpm"
#include "images/cdplayer-eject.xpm"

#define TIMEOUT_VALUE 500

/* Function prototypes */
static BonoboObject *applet_factory (BonoboGenericFactory *this, const gchar *iid, gpointer data);
static BonoboObject *applet_new (void);

static void cdplayer_destroy(GtkWidget * widget, gpointer data);
static void cdplayer_realize(GtkWidget *cdplayer, CDPlayerData *cd);
static int cdplayer_timeout_callback(gpointer data);

static void start_gtcd_cb(GtkWidget *w, gpointer data);
static void help_cb (GtkWidget *w, gpointer data);
static void about_cb(GtkWidget *w, gpointer data);

static void applet_change_size(GtkWidget *w, int size, gpointer data);
static void applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data);
static void ui_component_event (BonoboUIComponent *comp, const gchar *path,	Bonobo_UIComponent_EventType type, const gchar *state_string, CDPlayerData *data);

static void setup_box(CDPlayerData* cd);
static GtkWidget *control_button_factory(gchar * pixmap_data[], GCallback func, CDPlayerData * cd);
static void destroy_box(CDPlayerData* cd);
static void ref_and_remove(GtkWidget *w);
static GtkWidget *pack_make_hbox(CDPlayerData* cd);
static void pack_thing(GtkWidget *box, GtkWidget *w, gboolean expand);

static gboolean cd_try_open(CDPlayerData *cd);
static void cd_close (CDPlayerData *cd);

static void cd_panel_update(GtkWidget * cdplayer, CDPlayerData * cd);
static void cdplayer_play_pause(GtkWidget * w, gpointer data);
static void cdplayer_stop(GtkWidget * w, gpointer data);
static void cdplayer_prev(GtkWidget * w, gpointer data);
static void cdplayer_next(GtkWidget * w, gpointer data);
static void cdplayer_eject(GtkWidget * w, gpointer data);

/* Bonobo Verbs for our popup menu */
static const BonoboUIVerb applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("RunGTCD", start_gtcd_cb),
	BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
	BONOBO_UI_UNSAFE_VERB ("About", about_cb),
    BONOBO_UI_VERB_END
};

/* and the XML definition for the popup menu */
static const char applet_menu_xml [] =
"<popup name=\"button3\">\n"
"   <menuitem name=\"RunGTCD\" verb=\"RunGTCD\" _label=\"Run GTCD...\"\n"
"             pixtype=\"stock\" pixname=\"gtk-cdrom\"/>\n"
"   <menuitem name=\"Help\" verb=\"Help\" _label=\"Help\"\n"
"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
"   <menuitem name=\"About\" verb=\"About\" _label=\"About ...\"\n"
"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
"</popup>\n";

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_CDPlayerApplet_Factory",
							 "CD Player applet",
							 "0",
							  applet_factory,
							  NULL)

static BonoboObject *
applet_factory (BonoboGenericFactory *this, const gchar *iid, gpointer data)
{
	BonoboObject *applet = NULL;

	if (!strcmp (iid, "OAFIID:GNOME_CDPlayerApplet"))
		applet = applet_new ();
	return applet;
}


static BonoboObject *
applet_new ()
{
	GtkWidget *applet;
	GtkWidget *cdplayer;
	CDPlayerData *cd;
	BonoboUIComponent *component;
	int err;

	cd = g_new0(CDPlayerData, 1);

	/* default to the right thing on solaris */
#if defined(sun) || defined(__sun__)
#if defined(SVR4) || defined(__svr4__)
	/*cd->devpath = gnome_config_get_string("cdplayer/devpath=/vol/dev/aliases/cdrom0");*/
#else
	/*cd->devpath = gnome_config_get_string("cdplayer/devpath=/dev/rcd0");*/
#endif
#else
	/* everything else including linux */
	/*cd->devpath = gnome_config_get_string("cdplayer/devpath=/dev/cdrom");*/
	cd->devpath = g_strdup("/dev/cdrom");
#endif
    /*
      My intention here is to make a config dialog with ONE preference, which is the device path
      This will be a gconf based settings, and will be PER applet, so you could have multiple applets
      on a panel for people with multiple cd drives.
     */

	cd->cdrom_device = cdrom_open(cd->devpath, &err);

	/* the rest of the widgets go in here */
	cdplayer = cd->panel.frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(cdplayer), GTK_SHADOW_IN);
	g_object_set_data (G_OBJECT(cdplayer), "cd-info", cd);
	g_signal_connect (G_OBJECT(cdplayer), "destroy", G_CALLBACK (cdplayer_destroy), NULL);
	g_signal_connect_after (G_OBJECT (cdplayer), "realize", G_CALLBACK (cdplayer_realize), cd);
	gtk_widget_show(cdplayer);

	cd->panel.box = NULL;
	cd->panel.play_control.stop = control_button_factory(stop_xpm, G_CALLBACK(cdplayer_stop), cd);
	cd->panel.play_control.play_pause = control_button_factory(play_pause_xpm, G_CALLBACK(cdplayer_play_pause), cd);
	cd->panel.play_control.eject = control_button_factory(eject_xpm, G_CALLBACK(cdplayer_eject), cd);
	cd->panel.track_control.prev = control_button_factory(prev_xpm, G_CALLBACK(cdplayer_prev), cd);
	cd->panel.track_control.next = control_button_factory(next_xpm, G_CALLBACK(cdplayer_next), cd);
	led_init();
	led_create_widgets(&cd->panel.time, &cd->panel.track_control.display, (gpointer)cd);

	applet = panel_applet_new (cdplayer);

    cd->orient = panel_applet_get_orient (PANEL_APPLET (applet));
    cd->size = panel_applet_get_size (PANEL_APPLET (applet));

	g_signal_connect (applet, "destroy", G_CALLBACK (cdplayer_destroy), cd);
	g_signal_connect (applet, "change_orient", G_CALLBACK (applet_change_orient), cd);
	g_signal_connect (applet, "change_size", G_CALLBACK (applet_change_size), cd);

	panel_applet_setup_menu (PANEL_APPLET (applet),
							 applet_menu_xml,
							 applet_menu_verbs,
							 cd);

	component = panel_applet_get_popup_component (PANEL_APPLET (applet));

	g_signal_connect (component, "ui-event", G_CALLBACK (ui_component_event), cd);
	gtk_widget_show (applet);
	return BONOBO_OBJECT (panel_applet_get_control (PANEL_APPLET (applet)));
}

static void
cdplayer_destroy(GtkWidget * widget, gpointer data)
{
	CDPlayerData *cd;

	cd = g_object_get_data(G_OBJECT(widget), "cd-info");
	if (cd->timeout != 0)
		gtk_timeout_remove(cd->timeout);
	cd->timeout = 0;
	cd_close (cd);

	g_free (cd->devpath);
	cd->devpath = NULL;
	g_free(cd);
    led_done();
}

static void
cdplayer_realize(GtkWidget *cdplayer, CDPlayerData *cd)
{
	/* Add all of the widgets to the box in the right order for the Orientation/Size types */
	setup_box(cd);
	cd->timeout = gtk_timeout_add(TIMEOUT_VALUE, cdplayer_timeout_callback, (gpointer)cdplayer);
}

static int 
cdplayer_timeout_callback(gpointer data)
{
	GtkWidget *cdplayer;
	CDPlayerData *cd;

	cdplayer = (GtkWidget *)data;
	cd = g_object_get_data(G_OBJECT(cdplayer), "cd-info");
	cd_panel_update(cdplayer, cd);
	return(1);
}

/* Signal handlers for the cdplayer popup menu*/
static void
start_gtcd_cb(GtkWidget *w, gpointer data)
{
	gnome_execute_shell(NULL, "gtcd");
}

static void
help_cb (GtkWidget *w, gpointer data)
{

}

static void
about_cb(GtkWidget *w, gpointer data)
{
    static GtkWidget   *about     = NULL;

    static const gchar *authors[] =
    {
        "Miguel de Icaza <miguel@kernel.org>",
        "Federico Mena <quartic@gimp.org>",
        "Chris Phelps <chicane@renient.com>",
        NULL
    };

    if (about != NULL)
    {
        gdk_window_show(about->window);
        gdk_window_raise(about->window);
        return;
    }
    
    about = gnome_about_new (_("CD Player Applet"), VERSION,
                             _("(C) 1997 The Free Software Foundation\n" \
                               "(C) 2001 Chris Phelps (GNOME 2 Port)"),
                             _("The CD Player applet is a simple audio CD player for your panel"),
                             authors,  NULL, NULL, NULL);

    g_signal_connect (G_OBJECT(about), "destroy",
                      G_CALLBACK(gtk_widget_destroyed), &about);
    gtk_widget_show (about);
}

static void
applet_change_size(GtkWidget *w, int size, gpointer data)
{
	CDPlayerData *cd;

	cd = (CDPlayerData *)data;
	cd->size = size;
	setup_box(cd);
	cd_panel_update(cd->panel.frame, cd);
}

static void
applet_change_orient(GtkWidget *w, PanelAppletOrient o, gpointer data)
{
	CDPlayerData *cd;

	cd = (CDPlayerData *)data;
	cd->orient = o;
	setup_box(cd);
	cd_panel_update(cd->panel.frame, cd);
}

static void
ui_component_event (BonoboUIComponent *comp,
					const gchar *path,
					Bonobo_UIComponent_EventType  type,
					const gchar *state_string,
			        CDPlayerData *data)
{
	gboolean state;

	g_print("(ui-component-event) path is %s and state is %s.\n", path, state);
}

static void
setup_box(CDPlayerData* cd)
{
	GtkWidget *hbox;

	if(cd->panel.box)
		destroy_box(cd);

	if ((cd->orient == PANEL_APPLET_ORIENT_DOWN || cd->orient == PANEL_APPLET_ORIENT_UP) && cd->size < 36  ) {
		/* tiny horizontal panel */
		cd->panel.box = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
		pack_thing(cd->panel.box, cd->panel.track_control.prev, FALSE);
		pack_thing(cd->panel.box, cd->panel.track_control.display, TRUE);
		pack_thing(cd->panel.box, cd->panel.track_control.next, FALSE);
		pack_thing(cd->panel.box, cd->panel.time, TRUE);
		pack_thing(cd->panel.box, cd->panel.play_control.stop, FALSE);
		pack_thing(cd->panel.box, cd->panel.play_control.play_pause, FALSE);
		pack_thing(cd->panel.box, cd->panel.play_control.eject, FALSE);
		gtk_widget_show(cd->panel.box);

	} else if ((cd->orient == PANEL_APPLET_ORIENT_DOWN || cd->orient == PANEL_APPLET_ORIENT_UP) && cd->size < 48  ) {
		/* small horizontal panel */
		cd->panel.box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.time, TRUE);
		pack_thing(hbox, cd->panel.track_control.display, TRUE);
		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.play_control.stop, FALSE);
		pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
		pack_thing(hbox, cd->panel.play_control.eject, FALSE);
		pack_thing(hbox, cd->panel.track_control.prev, FALSE);
		pack_thing(hbox, cd->panel.track_control.next, FALSE);
		gtk_widget_show(cd->panel.box);
	} else if ((cd->orient == PANEL_APPLET_ORIENT_LEFT || cd->orient == PANEL_APPLET_ORIENT_RIGHT) && cd->size < 48  ) {
		cd->panel.box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
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
		gtk_widget_show(cd->panel.box);
	} else {
		/* other panel sizes/orientations should go here */
		cd->panel.box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(cd->panel.frame), cd->panel.box);
		pack_thing(cd->panel.box, cd->panel.time, TRUE);
		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.play_control.stop, FALSE);
		pack_thing(hbox, cd->panel.play_control.play_pause, FALSE);
		pack_thing(hbox, cd->panel.play_control.eject, FALSE);
		hbox = pack_make_hbox(cd);
		pack_thing(hbox, cd->panel.track_control.prev, FALSE);
		pack_thing(hbox, cd->panel.track_control.display, TRUE);
		pack_thing(hbox, cd->panel.track_control.next, FALSE);
		gtk_widget_show(cd->panel.box);
	}
	gtk_widget_show_all(cd->panel.frame);
}

static GtkWidget *
control_button_factory(gchar * pixmap_data[], GCallback func, CDPlayerData * cd)
{
	GtkWidget *w;
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	w = gtk_button_new();
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(w, GTK_CAN_FOCUS);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)pixmap_data);
	image = gtk_image_new_from_pixbuf(pixbuf);
	gtk_widget_show(image);
	gtk_container_add(GTK_CONTAINER(w), image);
	g_signal_connect(G_OBJECT(w), "clicked",
					 G_CALLBACK(func), cd);
	gtk_widget_show(w);
	gtk_widget_ref(w);
	return(w);
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

static void
ref_and_remove(GtkWidget *w)
{
	GtkWidget *parent = w->parent;
	if(parent) {
		g_object_ref(G_OBJECT(w));
		gtk_container_remove(GTK_CONTAINER(parent), w);
	}
}

static GtkWidget *
pack_make_hbox(CDPlayerData* cd)
{
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(cd->panel.box), hbox, TRUE, TRUE, 0);
	return hbox;
}

static void
pack_thing(GtkWidget *box, GtkWidget *w, gboolean expand)
{
	gtk_box_pack_start(GTK_BOX(box), w, expand, TRUE, 0);
	g_object_unref(G_OBJECT(w));
}

/* Deal with the hardware stuff */
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


/* Control button callbacks */
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
		/*
		  FIXME: if there is no disc, we get TRAY_OPEN even
		  if the tray is closed, is this a kernel bug?? or
		  is this the inteded behaviour, we don't support this
		  on solaris anyway, but unless we have a good way to
		  find out if the tray is actually open, we can't do this
		*/
		/* cdrom_load(cd->cdrom_device); */
		/*in the meantime let's just do eject so that at least that works */
		cdrom_eject(cd->cdrom_device);
	else
		cdrom_eject(cd->cdrom_device);
	cd_close(cd);
	return;
        w = NULL;
}
