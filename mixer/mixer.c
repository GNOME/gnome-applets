/*
 * GNOME audio mixer module
 * (C) 1998 The Free Software Foundation
 *
 * based on:
 *
 * GNOME time/date display module.
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 *
 * Feel free to implement new look and feels :-)
 */

#include <stdio.h>
#include <config.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include "applet-lib.h"
#include "applet-widget.h"

#include "slider.h"

#ifdef HAVE_LINUX_SOUNDCARD_H
#include <linux/soundcard.h>
#else
#include <machine/soundcard.h>
#endif
 

typedef void (*MixerUpdateFunc) (GtkWidget *, gint);

typedef struct {
	gint mastervol;
	gint mute;
	gint timeout;
	MixerUpdateFunc update_func;
	PanelOrientType orient;
} MixerData;

typedef struct {
	GtkWidget *slider;
	GtkWidget *mutebutton;
	GtkAdjustment *adj;
} MixerWidget;

GtkWidget *applet = NULL;
GtkWidget *mixerw = NULL;

MixerData *md = NULL;

gint mixerfd = -1;

gint timeout_enabled = 0;

/********************** Mixer related Code *******************/
/*  Mostly based on the gmix code                            */
/*************************************************************/
static void
openMixer( gchar *device_name ) 
{
	gint res, ver;

	mixerfd = open(device_name, O_RDWR, 0);
	if (mixerfd < 0) {
		/* probably should die more gracefully */
		g_message("Couldn't open mixer device %s\n",device_name);
		exit(1);
	}

        /*
         * check driver-version
         */
#ifdef OSS_GETVERSION
        res=ioctl(mixerfd, OSS_GETVERSION, &ver);
        if ((res!=EINVAL) && (ver!=SOUND_VERSION)) {
                g_message("warning: this version of gmix was compiled "
			"with a different version of\nsoundcard.h.\n");
        }
#endif
}

/* only works with master vol currently */
static int
readMixer(void)
{
	gint vol, r, l;

	ioctl(mixerfd, MIXER_READ(SOUND_MIXER_VOLUME), &vol);

	l = vol & 0xff;
	r = (vol & 0xff00) >> 8;
/*	printf("vol=%d l=%d r=%d\n",vol, l, r); */

	return (r+l)/2;
}

static void
setMixer(gint vol)
{
	gint tvol;

	tvol = (vol << 8) + vol;
/*g_message("Saving mixer value of %d",tvol);*/
	ioctl(mixerfd, MIXER_WRITE(SOUND_MIXER_VOLUME), &tvol);
}

static void
free_data(GtkWidget * widget, gpointer data)
{
	g_free(data);
}

static void 
mute_cb (GtkWidget *widget, gpointer   data)
{
	static gint old_vol=-1;
	gint nvol;
	MixerData *p = data;
	MixerWidget *mx;

	mx = gtk_object_get_user_data(GTK_OBJECT(widget));

	if (GTK_TOGGLE_BUTTON (widget)->active) {
		/* turn off the timeout */
		timeout_enabled = 0;
		gtk_widget_set_sensitive(mx->slider, FALSE);
		p->mute = 1;
		old_vol = readMixer();
		setMixer(0);
	} else {
		p->mute = 0;
		if (old_vol >= 0)
			setMixer(old_vol);
		timeout_enabled = 1;
		gtk_widget_set_sensitive(mx->slider, TRUE);
	}
/* printf("In mute_cb: mute button state is %d\n",p->mute); */
	gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON(widget), p->mute);
}

static void
adj_cb(GtkWidget *widget, gpointer   data)
{
	gint vol;
	
	vol = -GTK_ADJUSTMENT(data)->value;
/*	printf("In adj_cb: value is %d\n", vol); */
 
	if (timeout_enabled)
		setMixer(vol);
}

static int
mixer_timeout_callback(gpointer data)
{
	gint mvol;

	if (!timeout_enabled)
		return 1;

	mvol = readMixer();

	(*md->update_func) (mixerw, mvol);

	return 1;
}

static void
mixer_update_func(GtkWidget *mixer, gint mvol)
{
	MixerWidget *mx;

	mx = gtk_object_get_user_data(GTK_OBJECT(mixer));
	
	gtk_adjustment_set_value(mx->adj, (gfloat)(-mvol));

	/* need code to read current mixer ? Or not since timeout cb does it */
	/* how do I move the scale value once its realized?? */
/*	gtk_scale_set_draw_value(GTK_SCALE(mx->slider), mvol); */
}

static void
create_computer_mixer_widget(GtkWidget ** mixer, MixerUpdateFunc * update_func)
{
        GtkWidget *vbox, *scale, *button, *bbox, *lab;
	GtkAdjustment *adj;
	MixerWidget *mx;

        vbox=gtk_hbox_new(FALSE,1);
/*	gtk_container_border_width(GTK_CONTAINER(vbox), 2); */

        /* Only have master volume, single slider */
	scale = slider_new();
        adj=(GtkAdjustment *) gtk_adjustment_new (-50, -100.0, 0.0, 1.0, 1.0, 0.0);
        gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   (GtkSignalFunc)adj_cb, (gpointer)adj);
	gtk_range_set_adjustment(GTK_RANGE(scale), adj);
        gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
        gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
        gtk_box_pack_start (GTK_BOX (vbox), scale, TRUE, TRUE, 2);
	gtk_widget_set_usize(GTK_WIDGET(scale), 10, 48);
        gtk_widget_show (scale);


	bbox = gtk_vbox_new(FALSE, 1);
	lab = gtk_label_new("M");
	gtk_widget_show(lab);
	gtk_box_pack_start(GTK_BOX(bbox), lab, TRUE, TRUE, 0);
	lab = gtk_label_new("u");
	gtk_widget_show(lab);
	gtk_box_pack_start(GTK_BOX(bbox), lab, TRUE, TRUE, 0);
	lab = gtk_label_new("t");
	gtk_widget_show(lab);
	gtk_box_pack_start(GTK_BOX(bbox), lab, TRUE, TRUE, 0);
	lab = gtk_label_new("e");
	gtk_widget_show(lab);
	gtk_box_pack_start(GTK_BOX(bbox), lab, TRUE, TRUE, 0);
	gtk_widget_show(bbox);

	button = gtk_toggle_button_new();
	gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON(button), 0);
        gtk_signal_connect(GTK_OBJECT(button), "toggled",
			   (GtkSignalFunc)mute_cb, (gpointer)md);
        gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 2);
	gtk_widget_set_usize(GTK_WIDGET(button), 15, 8);
	gtk_container_add(GTK_CONTAINER(button), bbox);
	gtk_widget_show (button);

	mx = g_new(MixerWidget, 1);
	mx->slider = scale;
	mx->mutebutton = button;
	mx->adj = adj;

	gtk_signal_connect(GTK_OBJECT(vbox), "destroy",
			   (GtkSignalFunc) free_data,
			   mx);

	gtk_object_set_user_data(GTK_OBJECT(vbox), mx);
	gtk_object_set_user_data(GTK_OBJECT(button), mx);

        *mixer = vbox;
	*update_func = mixer_update_func;

}

static void
destroy_mixer(GtkWidget * widget, void *data)
{
	gtk_timeout_remove(md->timeout);
	g_free(md);
}

static GtkWidget *
create_mixer_widget(void)
{
	GtkWidget *mixer;
	gint      curmvol;

	md = g_new(MixerData, 1);

	/*FIXME: different clock types here */
	create_computer_mixer_widget(&mixer, &md->update_func);

	/* Install timeout handler */

	md->timeout = gtk_timeout_add(100, mixer_timeout_callback, mixer);
	timeout_enabled = 1;
	md->orient = ORIENT_UP;
	md->mute   = 0;

	gtk_signal_connect(GTK_OBJECT(mixer), "destroy",
			   (GtkSignalFunc) destroy_mixer,
			   NULL);
	/* Call the mixer's update function so that it paints its first state */

	/* add code to read currect status of mixer */
	curmvol = readMixer(); /* replace with proper read */
	(*md->update_func) (mixer, curmvol);

	return mixer;
}

/*these are commands sent over corba: */
static void
applet_change_orient(GtkWidget *w, PanelOrientType o)
{
	gint mvol;

	mvol = readMixer();
	md->orient = o;
	(*md->update_func) (mixerw, mvol);
}

static gint
destroy_applet(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}

/*
void
test_callback(int id, gpointer data)
{
	puts("TEST");
}
*/


int
main(int argc, char **argv)
{
	openMixer("/dev/mixer");

	panel_corba_register_arguments();
	gnome_init("mixer_applet", NULL, argc, argv, 0, NULL);

	applet = applet_widget_new(argv[0]);
	if (!applet)
		g_error("Can't create applet!\n");

	mixerw = create_mixer_widget();
	gtk_widget_show(mixerw);

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

	applet_widget_add(APPLET_WIDGET(applet), mixerw);
	gtk_widget_show(applet);
	gtk_signal_connect(GTK_OBJECT(applet),"destroy",
			   GTK_SIGNAL_FUNC(destroy_applet),
			   NULL);

/*
	gnome_panel_applet_register_callback(applet_id,
					     "test",
					     "TEST CALLBACK",
					     test_callback,
					     NULL);
*/

	applet_widget_gtk_main();

	return 0;
}
