/*
 * Mixer (volume control) applet.
 * 
 * (C) Copyright 2001, Richard Hult
 *
 * Author: Richard Hult <rhult@codefactory.se>
 *
 *
 * Loosely based on the mixer applet:
 *
 * GNOME audio mixer module
 * (C) 1998 The Free Software Foundation
 *
 * Author: Michael Fulbright <msf@redhat.com>:
 *
 * Based on:
 *
 * GNOME time/date display module.
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 *
 */

/*
 * Contact Dave Larson <davlarso@acm.org> for questions of
 * bugs concerning the Solaris audio api code.
 *
 */	

#include <config.h>

#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libgnomeui/gnome-about.h>
#include <panel-applet.h>

#ifdef HAVE_LINUX_SOUNDCARD_H
#include <linux/soundcard.h>
#define OSS_API
#elif HAVE_MACHINE_SOUNDCARD_H
#include <machine/soundcard.h>
#define OSS_API
#elif HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#define OSS_API
#elif HAVE_SOUNDCARD_H
#include <soundcard.h>
#define OSS_API
#elif HAVE_SYS_AUDIOIO_H
#include <sys/audioio.h>
#define SUN_API
#elif HAVE_SYS_AUDIO_IO_H
#include <sys/audio.io.h>
#define SUN_API
#elif HAVE_SUN_AUDIOIO_H
#include <sun/audioio.h>
#define SUN_API
#elif HAVE_DMEDIA_AUDIO_H
#define IRIX_API
#include <dmedia/audio.h>
#else
#error No soundcard defenition!
#endif /* SOUNDCARD_H */

#ifdef OSS_API
#define VOLUME_MAX 100
#endif
#ifdef SUN_API
#define VOLUME_MAX 255
#endif
#ifdef IRIX_API
#define VOLUME_MAX 255
#endif

#include "volume-zero.xpm"
#include "volume-min.xpm"
#include "volume-medium.xpm"
#include "volume-max.xpm"
#include "volume-mute.xpm"

#define IS_PANEL_HORIZONTAL(o) (o == PANEL_APPLET_ORIENT_UP || o == PANEL_APPLET_ORIENT_DOWN)

typedef struct {
	PanelAppletOrient  orientation;

	gint               timeout;
	gboolean           mute;
	gint               vol;
	gint               vol_before_popup;

	GtkAdjustment     *adj;

	GtkWidget         *applet;
	GtkWidget         *event_box;
	GtkWidget         *frame;
	GtkWidget         *image;
	
	/* The popup window and scale. */
	GtkWidget         *popup;
	GtkWidget         *scale;

	GdkPixbuf         *zero_pixbuf;
	GdkPixbuf         *min_pixbuf;
	GdkPixbuf         *medium_pixbuf;
	GdkPixbuf         *max_pixbuf;
	GdkPixbuf         *mute_pixbuf;
} MixerData;

static void mixer_update_slider (MixerData *data);
static void mixer_update_image  (MixerData *data);
static void mixer_popup_show    (MixerData *data);
static void mixer_popup_hide    (MixerData *data, gboolean revert);


static gint       mixerfd = -1;

#ifdef OSS_API
static int mixerchannel;
#endif

#ifdef IRIX_API
/*
 * Note: we are using the obsolete API to increase portability.
 * /usr/sbin/audiopanel provides many more options...
 */
#define MAX_PV_BUF	4
long pv_buf[MAX_PV_BUF] = { 
  AL_LEFT_SPEAKER_GAIN, 0L, AL_RIGHT_SPEAKER_GAIN, 0L 
};
#endif

/********************** Mixer related Code *******************/
/*  Mostly based on the gmix code                            */
/*************************************************************/
static void
openMixer( gchar *device_name ) 
{
	gint res, ver;
#ifdef OSS_API
	int devmask;

	mixerfd = open(device_name, O_RDWR, 0);
#endif
#ifdef SUN_API
	mixerfd = open(device_name, O_RDWR);
#endif
#ifdef IRIX_API
	/* 
	 * This is a result code, not a file descriptor, and we ignore
	 * the values read.  But the call is useful to see if we can
	 * access the default output port.
	 */
	mixerfd = ALgetparams(AL_DEFAULT_DEVICE, pv_buf, MAX_PV_BUF);
#endif
	if (mixerfd < 0) {
		/* probably should die more gracefully */
		char *s = g_strdup_printf(_("Couldn't open mixer device %s\n"),
					  device_name);
		gnome_error_dialog(s);
		g_warning(s);
		g_free(s);
		return;
	}

        /*
         * check driver-version
         */
#ifdef OSS_GETVERSION
        res=ioctl(mixerfd, OSS_GETVERSION, &ver);
        if ((res==0) && (ver!=SOUND_VERSION)) {
                g_message(_("warning: this version of gmix was compiled "
			"with a different version of\nsoundcard.h.\n"));
        }
#endif
#ifdef OSS_API
	/*
	 * Check whether this mixer actually supports the channel
	 * we're going to try to monitor.
	 */
	res = ioctl(mixerfd, MIXER_READ(SOUND_MIXER_DEVMASK), &devmask);
	if (res != 0) {
		char *s = g_strdup_printf(_("Querying available channels of mixer device %s failed\n"), device_name);
		gnome_error_dialog(s);
		g_free(s);
		return;
	} else if (devmask & SOUND_MASK_VOLUME) {
		mixerchannel = SOUND_MIXER_VOLUME;
	} else if (devmask & SOUND_MASK_PCM) {
		g_message(_("warning: mixer has no volume channel - using PCM instead.\n"));
		mixerchannel = SOUND_MIXER_PCM;
	} else {
		char *s = g_strdup_printf(_("Mixer device %s has neither volume nor PCM channels.\n"), device_name);
		gnome_error_dialog(s);
		g_free(s);
		return;
	}
#endif		
}

/* only works with master vol currently */
static int
readMixer(void)
{
	gint vol, r, l;
#ifdef OSS_API
	/* if we couldn't open the mixer */
	if (mixerfd < 0) return 0;

	ioctl(mixerfd, MIXER_READ(mixerchannel), &vol);

	l = vol & 0xff;
	r = (vol & 0xff00) >> 8;
/*	printf("vol=%d l=%d r=%d\n",vol, l, r); */

	return (r+l)/2;
#endif
#ifdef SUN_API
 	audio_info_t ainfo;
        AUDIO_INITINFO (&ainfo);	
 	ioctl (mixerfd, AUDIO_GETINFO, &ainfo);
 	return (ainfo.play.gain);
#endif
#ifdef IRIX_API
	/*
	 * Average the current gain settings.  If we can't read the
	 * current levels use the values from the previous read.
	 */
	(void) ALgetparams(AL_DEFAULT_DEVICE, pv_buf, MAX_PV_BUF);
	return (pv_buf[1] + pv_buf[3]) / 2;
#endif
}

static void
setMixer(gint vol)
{
	gint tvol;
#ifdef OSS_API
	/* if we couldn't open the mixer */
	if (mixerfd < 0) return;

	tvol = (vol << 8) + vol;
/*g_message("Saving mixer value of %d",tvol);*/
	ioctl(mixerfd, MIXER_WRITE(mixerchannel), &tvol);
	ioctl(mixerfd, MIXER_WRITE(SOUND_MIXER_SPEAKER), &tvol);
#endif
#ifdef SUN_API
 	audio_info_t ainfo;
	AUDIO_INITINFO (&ainfo);
 	ainfo.play.gain = vol;
 	ioctl (mixerfd, AUDIO_SETINFO, &ainfo);
#endif
#ifdef IRIX_API
	if (vol < 0) 
	  tvol = 0;
	else if (vol > VOLUME_MAX)
	  tvol = VOLUME_MAX;
	else
	  tvol = vol;

	pv_buf[1] = pv_buf[3] = tvol;
	(void) ALsetparams(AL_DEFAULT_DEVICE, pv_buf, MAX_PV_BUF);
#endif
}

static void
mixer_value_changed_cb (GtkAdjustment *adj, MixerData *data)
{
	gint vol;

	if (IS_PANEL_HORIZONTAL (data->orientation)) {
		vol = -GTK_ADJUSTMENT (adj)->value;
	} else {
		vol = -(-VOLUME_MAX - GTK_ADJUSTMENT (adj)->value);
	}

	data->vol = vol;

	mixer_update_image (data);
	
	if (!data->mute) {
		setMixer (vol);
	}
}

static gboolean
mixer_timeout_cb (MixerData *data)
{
	BonoboUIComponent *component;
	gint               vol;

	vol = readMixer ();

	/* Some external program changed the volume, get out of mute mode. */
	if (data->mute && (vol > 0)) {
		data->mute = FALSE;

		/* Sync the menu. */
		component = panel_applet_get_popup_component (PANEL_APPLET (data->applet));

		bonobo_ui_component_set_prop (component,
					      "/commands/Mute",
					      "state",
					      "0",
					      NULL);
	}

	if (!data->mute && vol != data->vol) {
		data->vol = vol;
		mixer_update_slider (data);
	}
	
	return TRUE;
}

static void
mixer_update_slider (MixerData *data)
{
	gint vol = data->vol;

	if (IS_PANEL_HORIZONTAL (data->orientation)) {
		vol = -vol;
	} else {
		vol = vol - VOLUME_MAX;
	}
	
	gtk_adjustment_set_value (data->adj, vol);
}

static void
mixer_update_image (MixerData *data)
{
	gint       vol;
	GdkPixbuf *pixbuf;

	vol = data->vol;
	
	if (vol <= 0) {
		pixbuf = data->zero_pixbuf;
	}
	else if (vol <= VOLUME_MAX / 3) {
		pixbuf = data->min_pixbuf;
	}
	else if (vol <= 2 * VOLUME_MAX / 3) {
		pixbuf = data->medium_pixbuf;
	}
	else {
		pixbuf = data->max_pixbuf;
	}

	if (!data->mute) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (data->image), pixbuf);
	} else {
		GdkPixbuf *copy = gdk_pixbuf_copy (pixbuf);

		gdk_pixbuf_composite (data->mute_pixbuf,
				      copy,
				      0,
				      0,
				      gdk_pixbuf_get_width (data->mute_pixbuf),
				      gdk_pixbuf_get_height (data->mute_pixbuf),
				      0,
				      0,
				      1.0,
				      1.0,
				      GDK_INTERP_BILINEAR,
				      127);

		gtk_image_set_from_pixbuf (GTK_IMAGE (data->image), copy);
		g_object_unref (copy);
	}
}

static gboolean
scale_key_press_event_cb (GtkWidget *widget, GdkEventKey *event, MixerData *data)
{
	switch (event->keyval) {
	case GDK_Escape:
		/* Revert. */
		mixer_popup_hide (data, TRUE);
		return TRUE;

	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		/* Apply. */
		mixer_popup_hide (data, FALSE);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}
static gboolean
scale_button_release_event_cb (GtkWidget *widget, GdkEventButton *event, MixerData *data)
{
	if (event->button == 1 && data->popup) {
		mixer_popup_hide (data, FALSE);
		return TRUE;
	}

	return FALSE;
}

static gboolean
event_box_button_press_event_cb (GtkWidget *widget, GdkEventButton *event, MixerData *data)
{
	if (event->button == 1) {
		if (!data->popup) {
			mixer_popup_show (data);
		}
		else {
			mixer_popup_hide (data, FALSE);
		}
	}

	g_signal_stop_emission_by_name (widget, "button_press_event");

	if (event->button == 2) {
		g_print ("Button2: Why won't the panel get this event?\n");
	}

	return FALSE;
}

static void
mixer_popup_show (MixerData *data)
{
	GtkRequisition  req;
	GtkWidget      *frame;
	GtkWidget      *inner_frame;
	gint            x, y;
	gint            width, height;
	GdkGrabStatus   pointer, keyboard;

	data->popup = gtk_window_new (GTK_WINDOW_POPUP);

	data->vol_before_popup = readMixer ();
	
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_widget_show (frame);
	
	inner_frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (inner_frame), 0);
	gtk_frame_set_shadow_type (GTK_FRAME (inner_frame), GTK_SHADOW_NONE);
	gtk_widget_show (inner_frame);
	
	if (IS_PANEL_HORIZONTAL (data->orientation)) {
		data->scale = gtk_vscale_new (data->adj);
		gtk_widget_set_usize (data->scale, -1, 100);			
	} else {
		data->scale = gtk_hscale_new (data->adj);
		gtk_widget_set_usize (data->scale, 100, -1);
	}
	
	g_signal_connect (data->scale,
			  "button-release-event",
			  (GCallback) scale_button_release_event_cb,
			  data);

	g_signal_connect (data->scale,
			  "key-press-event",
			  (GCallback) scale_key_press_event_cb,
			  data);
	
	gtk_widget_show (data->scale);
	gtk_scale_set_draw_value (GTK_SCALE (data->scale), FALSE);
	
	gtk_range_set_update_policy (GTK_RANGE (data->scale),
				     GTK_UPDATE_CONTINUOUS);
	
	gtk_container_add (GTK_CONTAINER (data->popup), frame);
	
	gtk_container_add (GTK_CONTAINER (frame), inner_frame);
	gtk_container_add (GTK_CONTAINER (inner_frame), data->scale);

	gtk_widget_show_all (inner_frame);
	
	/*
	 * Position the popup right next to the button.
	 */
	gtk_widget_size_request (data->popup, &req);
	
	gdk_window_get_origin (data->image->window, &x, &y);
	gdk_window_get_size (data->image->window, &width, &height);
	
	switch (data->orientation) {
	case PANEL_APPLET_ORIENT_DOWN:
		x += (width - req.width) / 2;
		y += height;
		break;
		
	case PANEL_APPLET_ORIENT_UP:
		x += (width - req.width) / 2;
		y -= req.height;
		break;
		
	case PANEL_APPLET_ORIENT_LEFT:
		x -= req.width;
		y += (height - req.height) / 2;			
		break;
		
	case PANEL_APPLET_ORIENT_RIGHT:
		x += width;
		y += (height - req.height) / 2;			
		break;
		
	default:
		break;
	}
	
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
	
	gtk_window_move (GTK_WINDOW (data->popup), x, y);
	
	gtk_widget_show (data->popup);
	
	/*
	 * Grab focus and pointer.
	 */
	gtk_widget_grab_focus (data->scale);
	gtk_grab_add (data->scale);

	pointer = gdk_pointer_grab (data->scale->window,
				    TRUE,
				    (GDK_BUTTON_PRESS_MASK
				     | GDK_BUTTON_RELEASE_MASK
				     | GDK_POINTER_MOTION_MASK),
				    NULL, NULL, GDK_CURRENT_TIME);
	
	keyboard = gdk_keyboard_grab (data->scale->window,
				      TRUE,
				      GDK_CURRENT_TIME);
	
	if (pointer != GDK_GRAB_SUCCESS || keyboard != GDK_GRAB_SUCCESS) {
		/* We could not grab. */
		gtk_grab_remove (data->scale);
		gtk_widget_destroy (data->popup);
		data->popup = NULL;
		data->scale = NULL;
		
		if (pointer == GDK_GRAB_SUCCESS) {
			gdk_keyboard_ungrab (GDK_CURRENT_TIME);
		}
		if (keyboard == GDK_GRAB_SUCCESS) {
			gdk_pointer_ungrab (GDK_CURRENT_TIME);
		}
		
		g_warning ("volume-control-applet: Could not grab X server.");
		return;
	}

	gtk_frame_set_shadow_type (GTK_FRAME (data->frame), GTK_SHADOW_IN);
}

static void
mixer_popup_hide (MixerData *data, gboolean revert)
{
	gint vol;
	
	if (data->popup) {
		gtk_grab_remove (data->scale);
		gdk_pointer_ungrab (GDK_CURRENT_TIME);		
		gdk_keyboard_ungrab (GDK_CURRENT_TIME);

		/* Ref the adjustment or it will get destroyed
		 * when the scale goes away.
		 */
		gtk_object_ref (GTK_OBJECT (data->adj));

		gtk_widget_destroy (GTK_WIDGET (data->popup));

		data->popup = NULL;
		data->scale = NULL;

		if (revert) {
			vol = data->vol_before_popup;
			setMixer (vol);
		} else {
			/* Get a soft beep like sound to play here, like
			 * on MacOS X :) */
			/*gnome_sound_play ("blipp.wav");*/
		}

		gtk_frame_set_shadow_type (GTK_FRAME (data->frame), GTK_SHADOW_NONE);
	}
}

static void
destroy_mixer_cb (GtkWidget *widget, MixerData *data)
{
	if (data->timeout != 0) {
		gtk_timeout_remove (data->timeout);
		data->timeout = 0;
	}

	if (data->zero_pixbuf) {
		g_object_unref (data->zero_pixbuf);
		data->zero_pixbuf = NULL;
	}
	if (data->min_pixbuf) {
		g_object_unref (data->min_pixbuf);
		data->min_pixbuf = NULL;
	}
	if (data->medium_pixbuf) {
		g_object_unref (data->medium_pixbuf);
		data->medium_pixbuf = NULL;
	}
	if (data->max_pixbuf) {
		g_object_unref (data->max_pixbuf);
		data->max_pixbuf = NULL;
	}
	if (data->mute_pixbuf) {
		g_object_unref (data->mute_pixbuf);
		data->mute_pixbuf = NULL;
	}
	
	g_free (data);

	return;
}

static void
applet_change_orient_cb (GtkWidget *w, PanelAppletOrient o, MixerData *data)
{
	gint vol;
	
	mixer_popup_hide (data, FALSE);

	data->orientation = o;
}

static void
applet_change_size_cb (GtkWidget *w, gint size, MixerData *data)
{
	gint vol;
	
	mixer_popup_hide (data, FALSE);

	/* Really only needed to fit on the ultra small panel,
	 * but we could scale up for the bigger ones...
	 */
	if (IS_PANEL_HORIZONTAL (data->orientation)) {
		gtk_widget_set_size_request (GTK_WIDGET (data->event_box),
					     23,
					     MIN (23, size));
	} else {
		gtk_widget_set_size_request (GTK_WIDGET (data->event_box),
					     MIN (23, size),
					     23);
	}
}

static void
mixer_start_gmix_cb (BonoboUIComponent *uic,
		     MixerData         *data,
		     const gchar       *verbname)
{
	gnome_execute_shell (NULL, "gmix");
}

static void
mixer_about_cb (BonoboUIComponent *uic,
		MixerData         *data,
		const gchar       *verbname)
{
        static GtkWidget   *about     = NULL;
        static const gchar *authors[] = {
		"Richard Hult <rhult@codefactory.se>",
                NULL
        };

        if (about != NULL) {
                gdk_window_show (about->window);
                gdk_window_raise (about->window);
                return;
        }

        about = gnome_about_new (_("Volume Control"), VERSION,
                                 _("(C) 2001 Richard Hult"),
                                 _("The volume control lets you set the volume level for your desktop."),
				 authors,
				 NULL,
				 NULL,
                                 NULL);

        gtk_signal_connect (GTK_OBJECT (about), "destroy",
                            GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &about);
	
        gtk_widget_show (about);
}

static void
mixer_help_cb (BonoboUIComponent *uic,
	       MixerData         *data,
	       const gchar       *verbname)
{
#if 0
	GnomeHelpMenuEntry help_entry = {
		"volume-control-applet",
		"index.html"
	};

	gnome_help_display (NULL, &help_entry);
#endif
}

/* Dummy callback to get rid of a warning, for now. */
static void
mixer_mute_cb (BonoboUIComponent *uic,
	       MixerData         *data,
	       const gchar       *verbname)
{
}

static void
mixer_ui_component_event (BonoboUIComponent            *comp,
			  const gchar                  *path,
			  Bonobo_UIComponent_EventType  type,
			  const gchar                  *state_string,
			  MixerData                    *data)
{
	gboolean state;
	
	if (!strcmp (path, "Mute")) {
		if (!strcmp (state_string, "1")) {
			state = TRUE;
		} else {
			state = FALSE;
		}

		if (state == data->mute) {
			return;
		}
	       
		data->mute = state;

		if (data->mute) {
			setMixer (0);
			mixer_update_image (data);
		}
		else {
			setMixer (data->vol);
			mixer_update_image (data);
		}
	}
}

static const BonoboUIVerb mixer_applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("RunMixer", mixer_start_gmix_cb),
	BONOBO_UI_UNSAFE_VERB ("Mute",     mixer_mute_cb),
	BONOBO_UI_UNSAFE_VERB ("Help",     mixer_help_cb),
	BONOBO_UI_UNSAFE_VERB ("About",    mixer_about_cb),

        BONOBO_UI_VERB_END
};

static const char mixer_applet_menu_xml [] =
"<popup name=\"button3\">\n"
"   <menuitem name=\"RunMixer\" verb=\"RunMixer\" _label=\"Run Audio Mixer...\"\n"
"             />" /*pixtype=\"stock\" pixname=\"gtk-volume\"/>\n"*/
"   <menuitem name=\"Mute\" verb=\"Mute\" type=\"toggle\" _label=\"Mute\"\n"
"             />" /*pixtype=\"stock\" pixname=\"gtk-volume\"/>\n"*/
"   <menuitem name=\"Help\" verb=\"Help\" _label=\"Help\"\n"
"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
"   <menuitem name=\"About\" verb=\"About\" _label=\"About ...\"\n"
"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
"</popup>\n";

static BonoboObject *
mixer_applet_new (void)
{
	MixerData         *data;
	GtkWidget         *applet;
	gint               vol;
	BonoboUIComponent *component;

#ifdef OSS_API
	openMixer("/dev/mixer");
#endif
#ifdef SUN_API
	openMixer("/dev/audioctl");
#endif

	data = g_new0 (MixerData, 1);

	data->zero_pixbuf = gdk_pixbuf_new_from_xpm_data (volume_zero_xpm);
	data->min_pixbuf = gdk_pixbuf_new_from_xpm_data (volume_min_xpm);
	data->medium_pixbuf = gdk_pixbuf_new_from_xpm_data (volume_medium_xpm);
	data->max_pixbuf = gdk_pixbuf_new_from_xpm_data (volume_max_xpm);
	data->mute_pixbuf = gdk_pixbuf_new_from_xpm_data (volume_mute_xpm);
	
	data->event_box = gtk_event_box_new ();
	
	data->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (data->frame), GTK_SHADOW_NONE);

	gtk_container_add (GTK_CONTAINER (data->event_box), data->frame);
	
	data->image = gtk_image_new ();

	g_signal_connect (data->event_box,
			  "button-press-event",
			  (GCallback) event_box_button_press_event_cb,
			  data);
	
	gtk_container_add (GTK_CONTAINER (data->frame), data->image);

	gtk_widget_show_all (data->event_box);
	
        data->adj = GTK_ADJUSTMENT (
		gtk_adjustment_new (-50,
				    -VOLUME_MAX,
				    0.0, 
				    VOLUME_MAX/100,
				    VOLUME_MAX/10,
				    0.0));

	g_signal_connect (GTK_OBJECT (data->adj),
			  "value-changed",
			  (GCallback) mixer_value_changed_cb,
			  data);

	/* Get the initial volume. */
	vol = readMixer ();
	data->vol = vol;

	applet = panel_applet_new (data->event_box);
	
	data->applet = applet;
	
	/* Install timeout handler, that keeps the applet up-to-date if the
	 * volume is changed somewhere else.
	 */
	data->timeout = gtk_timeout_add (500,
					 (GSourceFunc) mixer_timeout_cb,
					 data);

	data->mute = FALSE;

	g_signal_connect (applet,
			  "destroy",
			  (GCallback) destroy_mixer_cb,
			  data);

	g_signal_connect (applet,
			  "change_orient",
			  G_CALLBACK (applet_change_orient_cb),
			  data);

	g_signal_connect (applet,
			  "change_size",
			  G_CALLBACK (applet_change_size_cb),
			  data);

	panel_applet_setup_menu (PANEL_APPLET (applet),
				 mixer_applet_menu_xml,
				 mixer_applet_menu_verbs,
				 data);

	component = panel_applet_get_popup_component (PANEL_APPLET (applet));

	g_signal_connect (component,
			  "ui-event",
			  (GCallback) mixer_ui_component_event,
			  data);
	
	applet_change_orient_cb (applet,
				 panel_applet_get_orient (PANEL_APPLET (applet)),
				 data);
	applet_change_size_cb (applet,
			       panel_applet_get_size (PANEL_APPLET (applet)),
			       data);
	
	mixer_update_slider (data);
	mixer_update_image (data);

	gtk_widget_show (applet);

	return BONOBO_OBJECT (panel_applet_get_control (PANEL_APPLET (applet)));
}

static BonoboObject *
mixer_applet_factory (BonoboGenericFactory *this,
		      const gchar          *iid,
		      gpointer              data)
{
	BonoboObject *applet = NULL;
	
	if (!strcmp (iid, "OAFIID:GNOME_MixerApplet"))
		applet = mixer_applet_new ();
	
	return applet;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MixerApplet_Factory",
			     "Volume control applet",
			     "0",
			     mixer_applet_factory,
			     NULL)
