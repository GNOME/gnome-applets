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
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-about.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gconf/gconf-client.h>
#include <panel-applet-gconf.h>
#include <egg-screen-exec.h>
#include <egg-screen-help.h>

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

#define VOLUME_STOCK_MUTE             "volume-mute"
#define VOLUME_STOCK_ZERO	      "volume-zero"
#define VOLUME_STOCK_MIN              "volume-min"
#define VOLUME_STOCK_MED              "volume-med"
#define VOLUME_STOCK_MAX              "volume-max"

#define VOLUME_DEFAULT_ICON_SIZE 48
static GtkIconSize volume_icon_size = 0;
static gboolean icons_initialized = FALSE;
static gboolean device_present = TRUE;

#define IS_PANEL_HORIZONTAL(o) (o == PANEL_APPLET_ORIENT_UP || o == PANEL_APPLET_ORIENT_DOWN)

#ifdef OSS_API

typedef struct {
	gint channel;
	gchar *name;
} ChannelData;

static gchar *channel_names [] = {
	N_("Main Volume"), N_("Bass"), N_("Treble"), N_("Synth"), 
	N_("Pcm"), N_("Speaker"), N_("Line"), 
	N_("Microphone"), N_("CD"), N_("Mix"), N_("Pcm2"), 
	N_("Recording Level"), N_("Input Gain"), N_("Output Gain"), 
	N_("Line1"), N_("Line2"), N_("Line3"), N_("Digital1"), 
	N_("Digital2"), N_("Digital3"), 
	N_("Phone Input"), N_("Phone Output"), N_("Video"), N_("Radio"), N_("Monitor")
};
#endif

typedef struct {
	PanelAppletOrient  orientation;

	gint               timeout;
	gboolean       mute;
	gint               vol;
	gint               vol_before_popup;
	gint		    channel;
	int 		    mixerchannel;
	GList		    *channels;
	gchar		    *device;

	GtkAdjustment     *adj;

	GtkWidget         *applet;
	GtkWidget         *frame;
	GtkWidget         *image;
	GtkWidget 		*prefdialog;
	
	/* The popup window and scale. */
	GtkWidget         *popup;
	GtkWidget         *scale;
	
	GtkWidget 		*error_dialog;

	GdkPixbuf	  *zero;
	GdkPixbuf	  *min;
	GdkPixbuf	  *max;
	GdkPixbuf         *med;
	GdkPixbuf	  *muted;

	GtkTooltips	  *tooltips;
} MixerData;

static void mixer_update_slider (MixerData *data);
static void mixer_update_image  (MixerData *data);
static void mixer_popup_show    (MixerData *data);
static void mixer_popup_hide    (MixerData *data, gboolean revert);

static void mixer_start_gmix_cb (BonoboUIComponent *uic,
				 MixerData         *data,
				 const gchar       *verbname);
static void mixer_ui_component_event (BonoboUIComponent *comp,
			  const gchar                  *path,
			  Bonobo_UIComponent_EventType  type,
			  const gchar                  *state_string,
			  MixerData                    *data);
void add_atk_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc);

static gint mixerfd = -1;
static gchar *run_mixer_cmd = NULL;

static const gchar *access_name = N_("Volume Control");     
static const gchar *access_name_mute = N_("Volume Control (muted)");
static const gchar *access_name_nodevice = N_("No audio device");
gboolean gail_loaded = FALSE;  


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

static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}

#ifdef OSS_API
static void
get_channels (MixerData *data)
{
	gint res, i;
	int devmask;
	gboolean valid = FALSE;
	
	res = ioctl(mixerfd, MIXER_READ(SOUND_MIXER_DEVMASK), &devmask);
	if (res != 0) {
		return;
	} 
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		ChannelData *cdata;
		if (devmask & (1 << i)) {
			cdata = g_new0 (ChannelData, 1);
			cdata->channel = i;
			cdata->name = channel_names[i];
			data->channels = g_list_append (data->channels, cdata);
			if (data->mixerchannel == i)
				valid = TRUE;
		}
	}
	/* selected channel does not work. Set it to something that does */
	if (!valid) {
		ChannelData *cdata = data->channels->data;
		data->mixerchannel = cdata->channel;
		if (key_writable (PANEL_APPLET (data->applet), "channel"))
			panel_applet_gconf_set_int (PANEL_APPLET (data->applet), "channel", 
						    data->mixerchannel, NULL);
	}
}
#endif

/********************** Mixer related Code *******************/
/*  Mostly based on the gmix code                            */
/*************************************************************/
static gboolean
openMixer(const gchar *device_name, MixerData *data) 
{
	gint res, ver;
#ifdef OSS_API
	

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
		return FALSE;
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
	 get_channels (data);

#endif	
 	return TRUE;	
}

/* only works with master vol currently */
static int
readMixer(MixerData *data)
{
	gint vol, r, l;
#ifdef OSS_API
	/* if we couldn't open the mixer */
	if (mixerfd < 0) return 0;

	ioctl(mixerfd, MIXER_READ(data->mixerchannel), &vol);

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
setMixer(gint vol, MixerData *data)
{
	gint tvol;
#ifdef OSS_API
	/* if we couldn't open the mixer */
	if (mixerfd < 0) return;

	tvol = (vol << 8) + vol;
/*g_message("Saving mixer value of %d",tvol);*/
	ioctl(mixerfd, MIXER_WRITE(data->mixerchannel), &tvol);
	
/* SOUND_MIXER_SPEAKER is output level on Mac, but input level on PC. #96639 */
#ifdef __powerpc__
	ioctl(mixerfd, MIXER_WRITE(SOUND_MIXER_SPEAKER), &tvol);
#endif
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

	vol = -adj->value;
	
	data->vol = vol;

	mixer_update_image (data);

	if (!data->mute) {
		setMixer (vol, data);
	}
}

#ifdef SUN_API
static gboolean
set_mute_status (gboolean mute_state)
{
	audio_info_t ainfo;
	AUDIO_INITINFO (&ainfo);
	ainfo.output_muted = mute_state;
	ioctl (mixerfd, AUDIO_SETINFO, &ainfo);
}

static gboolean
get_mute_status ()
{
	audio_info_t ainfo;
	AUDIO_INITINFO (&ainfo);
	ioctl (mixerfd, AUDIO_GETINFO, &ainfo);
	return (ainfo.output_muted);
}
#endif

static gboolean
mixer_timeout_cb (MixerData *data)
{
	BonoboUIComponent *component;
	gint               vol;
	gboolean state;
	
	if (!device_present) {
		data->mute = TRUE;
		mixer_update_image (data);

		gtk_tooltips_set_tip (data->tooltips,
				      data->applet,
				      _(access_name_nodevice),
				      NULL);
		if (gail_loaded) {
			add_atk_namedesc (data->applet,
					  _(access_name_nodevice),
					  NULL);
		}
		return TRUE;
	}

#ifdef SUN_API
	state = get_mute_status ();

	if (state != data->mute) {
		data->mute = state;
		if (data->mute) {
			mixer_update_image (data);


			gtk_tooltips_set_tip (data->tooltips,
					      data->applet,
					      _(access_name_mute),
					      NULL);
			if (gail_loaded) {
				add_atk_namedesc (data->applet,
						  _(access_name_mute),
						  NULL);
			}
			component = panel_applet_get_popup_component (PANEL_APPLET (data->applet));
			bonobo_ui_component_set_prop (component,
						      "/commands/Mute",
						      "state",
						      "1",
						      NULL);

		} else {
			mixer_update_image (data);

			gtk_tooltips_set_tip (data->tooltips,
					      data->applet,
					      _(access_name),
					      NULL);
			if (gail_loaded) {
				add_atk_namedesc (data->applet,
						  _(access_name),
						  NULL);
			}
			component = panel_applet_get_popup_component (PANEL_APPLET (data->applet));
			bonobo_ui_component_set_prop (component,
						      "/commands/Mute",
						      "state",
						      "0",
						      NULL);

		}
	}
#endif
	vol = readMixer (data);

#ifndef SUN_API
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
		
		gtk_tooltips_set_tip (data->tooltips,
				      data->applet,
				      _(access_name),
			              NULL); 
		if (gail_loaded) {
			add_atk_namedesc (data->applet, _(access_name), NULL);
		}
	}
#endif

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

	vol = -vol;
	
	gtk_adjustment_set_value (data->adj, vol);
}

static void
mixer_load_volume_images (MixerData *data) {
	if (data->zero)
		g_object_unref (data->zero);
	if (data->min)
		g_object_unref (data->min);
	if (data->med)
		g_object_unref (data->med);
	if (data->max)
		g_object_unref (data->max);
	if (data->muted)
		g_object_unref (data->muted);

	data->zero = gtk_widget_render_icon (data->applet, VOLUME_STOCK_ZERO,
					     volume_icon_size, NULL);
	data->min = gtk_widget_render_icon (data->applet, VOLUME_STOCK_MIN,
					     volume_icon_size, NULL);
	data->med = gtk_widget_render_icon (data->applet, VOLUME_STOCK_MED,
					     volume_icon_size, NULL);
	data->max = gtk_widget_render_icon (data->applet, VOLUME_STOCK_MAX,
					     volume_icon_size, NULL);
	data->muted = gtk_widget_render_icon (data->applet, VOLUME_STOCK_MUTE,
					     volume_icon_size, NULL);
}

static void
mixer_update_image (MixerData *data)
{
	gint vol, size;
	GdkPixbuf *pixbuf, *copy, *scaled = NULL;
	vol = data->vol;

	size = panel_applet_get_size (PANEL_APPLET (data->applet));
	
	if (vol <= 0)
		pixbuf = gdk_pixbuf_copy (data->zero);
	else if (vol <= VOLUME_MAX / 3)
		pixbuf = gdk_pixbuf_copy (data->min);
	else if (vol <= 2 * VOLUME_MAX / 3)
		pixbuf = gdk_pixbuf_copy (data->med);
	else
		pixbuf = gdk_pixbuf_copy (data->max);

	if (data->mute) {
		GdkPixbuf *mute = gdk_pixbuf_copy (data->muted);

		gdk_pixbuf_composite (mute,
				      pixbuf,
				      0,
				      0,
				      gdk_pixbuf_get_width (mute),
				      gdk_pixbuf_get_height (mute),
				      0,
				      0,
				      1.0,
				      1.0,
				      GDK_INTERP_BILINEAR,
				      255);
		g_object_unref (mute);
	}

	size = MAX (11, size);
	scaled = gdk_pixbuf_scale_simple (pixbuf, size, size, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf (GTK_IMAGE (data->image), scaled);
	g_object_unref (scaled);	
	g_object_unref (pixbuf);
	
	if (key_writable (PANEL_APPLET (data->applet), "vol"))
		panel_applet_gconf_set_int (PANEL_APPLET (data->applet), "vol", data->vol,
					    NULL);
	if (key_writable (PANEL_APPLET (data->applet), "mute"))
		panel_applet_gconf_set_bool (PANEL_APPLET (data->applet), "mute", data->mute,
					     NULL);
			
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

static void
error_response (GtkDialog *dialog, gint id, MixerData *data)
{
	gtk_widget_destroy (data->error_dialog);
	data->error_dialog = NULL;
}

static void
show_error_dialog (MixerData *data)
{	
	if (data->error_dialog) {
		gtk_window_present (GTK_WINDOW (data->error_dialog));
		return;
	}
	data->error_dialog = gtk_message_dialog_new (NULL,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					 ("Couldn't open mixer device %s\n"),
					 data->device, NULL);

	gtk_window_set_screen (GTK_WINDOW (data->error_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (data->applet)));
	g_signal_connect (GTK_OBJECT (data->error_dialog), 
                             "response", 
                             G_CALLBACK (error_response),
                             data);
        gtk_widget_show_all (data->error_dialog);
}

static gboolean
scale_button_release_event_cb (GtkWidget *widget, GdkEventButton *event, MixerData *data)
{

	if (data->popup != NULL) {
		mixer_popup_hide (data, FALSE);
		return TRUE;
	}

	return FALSE;
}

static gboolean
scale_button_event  (GtkWidget *widget, GdkEventButton *event, MixerData *data)
{
	return TRUE;
}

static gboolean
applet_button_release_event_cb (GtkWidget *widget, GdkEventButton *event, MixerData *data)
{

	if (event->button == 1) {
		if (data->popup == NULL && device_present) {
			mixer_popup_show (data);
			return TRUE;
		} else if (!device_present)
			show_error_dialog (data);
	}

	return FALSE;
}

static gboolean
applet_key_press_event_cb (GtkWidget *widget, GdkEventKey *event, MixerData *data)
{
	switch (event->keyval) {
	case GDK_Escape:
		/* Revert. */
		mixer_popup_hide (data, TRUE);
		return TRUE;
	case GDK_m:
		if (event->state == GDK_CONTROL_MASK) {
			Bonobo_UIComponent_EventType  type;
			if (data->mute)
				mixer_ui_component_event (NULL, "Mute", type, "0", data);
			else
				mixer_ui_component_event (NULL, "Mute", type, "1", data);
			return TRUE;
		}
		break;
	case GDK_o:
		if (event->state == GDK_CONTROL_MASK) {
			mixer_start_gmix_cb (NULL, data, NULL);
			return TRUE;
		}
		break;
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		/* Apply. */
		if (data->popup != NULL)
			mixer_popup_hide (data, FALSE);
		else if(device_present)
			mixer_popup_show (data);
		else
			show_error_dialog (data);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}


static gboolean
applet_scroll_event_cb (GtkWidget *widget, GdkEventScroll *event, MixerData *data)
{
	gint direction; 
	if (event->type != GDK_SCROLL ) {
		return FALSE;
	}

	direction = event->direction;

	switch(direction) {
	case GDK_SCROLL_UP:
		data->vol += 5;
		break;
	case GDK_SCROLL_DOWN:
		data->vol -= 5;
		break;
	default:
		break;
	}

	if (data->vol > VOLUME_MAX) data->vol = VOLUME_MAX;
	if (data->vol < 0) data->vol = 0;

	mixer_update_slider(data);

	return TRUE;
}

static void
mixer_popup_show (MixerData *data)
{
	GtkRequisition  req;
	GtkWidget      *frame;
	GtkWidget      *inner_frame;
	GtkWidget      *pluslabel, *minuslabel;
	GtkWidget 	     *event;
	GtkWidget 	     *box;
	gint            x, y;
	gint            width, height;
	GdkGrabStatus   pointer, keyboard;

	data->popup = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_screen (GTK_WINDOW (data->popup),
			       gtk_widget_get_screen (data->applet));

	data->vol_before_popup = readMixer (data);
	
	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_widget_show (frame);
	
	inner_frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (inner_frame), 0);
	gtk_frame_set_shadow_type (GTK_FRAME (inner_frame), GTK_SHADOW_NONE);
	gtk_widget_show (inner_frame);
	
	event = gtk_event_box_new ();
	/* This signal is to not let button press close the popup when the press is
	** in the scale */
	g_signal_connect_after (event, "button_press_event",
				  G_CALLBACK (scale_button_event), data);
	
	if (IS_PANEL_HORIZONTAL (data->orientation)) {
		box = gtk_vbox_new (FALSE, 0);
		data->scale = gtk_vscale_new (data->adj);
		gtk_widget_set_size_request (data->scale, -1, 100);			
	} else {
		box = gtk_hbox_new (FALSE, 0);
		data->scale = gtk_hscale_new (data->adj);
		gtk_widget_set_size_request (data->scale, 100, -1);
		gtk_range_set_inverted (GTK_RANGE (data->scale), TRUE);
	}
	
	if (gail_loaded) {
		add_atk_namedesc (data->scale, 
				_("Volume Controller"),
				_("Use Up/Down arrow keys to change volume"));
	}
	g_signal_connect_after (data->popup,
			  "button-press-event",
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
	
	/* Translators - The + and - refer to increasing and decreasing the volume. 
	** I don't know if there are sensible alternatives in other languages */
	pluslabel = gtk_label_new (_("+"));	
	minuslabel = gtk_label_new (_("-"));
	
	if (IS_PANEL_HORIZONTAL (data->orientation)) {
		gtk_box_pack_start (GTK_BOX (box), pluslabel, FALSE, FALSE, 0);
		gtk_box_pack_end (GTK_BOX (box), minuslabel, FALSE, FALSE, 0);
	} else {
		gtk_box_pack_start (GTK_BOX (box), minuslabel, FALSE, FALSE, 0);
		gtk_box_pack_end (GTK_BOX (box), pluslabel, FALSE, FALSE, 0);
	}
	gtk_box_pack_start (GTK_BOX (box), data->scale, TRUE, TRUE, 0);
	
	gtk_container_add (GTK_CONTAINER (event), box);
	gtk_container_add (GTK_CONTAINER (inner_frame), event);
	
	gtk_widget_show_all (inner_frame);
	
	/*
	 * Position the popup right next to the button.
	 */
	gtk_widget_size_request (data->popup, &req);
	
	gdk_window_get_origin (data->image->window, &x, &y);
	gdk_drawable_get_size (data->image->window, &width, &height);
	
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
	gtk_widget_grab_focus (data->popup);
	gtk_grab_add (data->popup);

	pointer = gdk_pointer_grab (data->popup->window,
				    TRUE,
				    (GDK_BUTTON_PRESS_MASK |
				    GDK_BUTTON_RELEASE_MASK
				     | GDK_POINTER_MOTION_MASK),
				    NULL, NULL, GDK_CURRENT_TIME);
	
	keyboard = gdk_keyboard_grab (data->popup->window,
				      TRUE,
				      GDK_CURRENT_TIME);
	
	if (pointer != GDK_GRAB_SUCCESS || keyboard != GDK_GRAB_SUCCESS) {
		/* We could not grab. */
		gtk_grab_remove (data->popup);
		gtk_widget_destroy (data->popup);
		data->popup = NULL;
		data->scale = NULL;
		
		if (pointer == GDK_GRAB_SUCCESS) {
			gdk_keyboard_ungrab (GDK_CURRENT_TIME);
		}
		if (keyboard == GDK_GRAB_SUCCESS) {
			gdk_pointer_ungrab (GDK_CURRENT_TIME);
		}
		
		g_warning ("mixer_applet2: Could not grab X server.");
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
		g_object_ref (G_OBJECT (data->adj));

		gtk_widget_destroy (GTK_WIDGET (data->popup));

		data->popup = NULL;
		data->scale = NULL;

		if (revert) {
			vol = data->vol_before_popup;
			setMixer (vol, data);
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
	if (data->error_dialog)
		gtk_widget_destroy (data->error_dialog);
	g_free (data);
}

static void
applet_change_background_cb (PanelApplet               *applet,
			     PanelAppletBackgroundType  type,
			     GdkColor                  *color,
			     const gchar               *pixmap,
			     MixerData                 *data)
{
	if (type == PANEL_NO_BACKGROUND) {
		GtkRcStyle *rc_style = gtk_rc_style_new ();

		gtk_widget_modify_style (data->applet, rc_style);
		gtk_rc_style_unref (rc_style);
	}
	else if (type == PANEL_COLOR_BACKGROUND) {
		gtk_widget_modify_bg (data->applet,
				      GTK_STATE_NORMAL,
				      color);
	} else { /* pixmap */
		/* FIXME: Handle this when the panel support works again */
	}
}

static void
applet_change_orient_cb (GtkWidget *w, PanelAppletOrient o, MixerData *data)
{
	mixer_popup_hide (data, FALSE);

	data->orientation = o;
}

static void
applet_change_size_cb (GtkWidget *w, gint size, MixerData *data)
{	
	mixer_popup_hide (data, FALSE);
	mixer_update_image (data);
}

static void
mixer_start_gmix_cb (BonoboUIComponent *uic,
		     MixerData         *data,
		     const gchar       *verbname)
{
	GError *error = NULL;

	if (!run_mixer_cmd)
		return;

	egg_screen_execute_command_line_async (
			gtk_widget_get_screen (data->applet), run_mixer_cmd, &error);
	if (error) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_OK,
						 _("There was an error executing '%s': %s"),
						 run_mixer_cmd,
						 error->message);

		g_signal_connect (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (data->applet));
		gtk_widget_show (dialog);

		g_error_free (error);
	}
}

static void
mixer_about_cb (BonoboUIComponent *uic,
		MixerData         *data,
		const gchar       *verbname)
{
        static GtkWidget   *about     = NULL;
        GdkPixbuf *pixbuf             = NULL;
        gchar *file;
        static const gchar *authors[] = {
		"Richard Hult <rhult@codefactory.se>",
                NULL
        };

	const gchar *documenters[] = {
		NULL
	};

	const gchar *translator_credits = _("translator_credits");
	
        if (about) {
		gtk_window_set_screen (GTK_WINDOW (about),
				       gtk_widget_get_screen (data->applet));
                gtk_window_present (GTK_WINDOW (about));
                return;
        }
        
        file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "mixer/gnome-mixer-applet.png", TRUE, NULL);
        pixbuf = gdk_pixbuf_new_from_file (file, NULL);
        g_free (file);

        about = gnome_about_new (_("Volume Control"), VERSION,
                                 _("(C) 2001 Richard Hult"),
                                 _("The volume control lets you set the volume level for your desktop."),
				 authors,
				 documenters,
				 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                                 pixbuf);

	gtk_window_set_screen (GTK_WINDOW (about),
			       gtk_widget_get_screen (data->applet));
	gtk_window_set_wmclass (GTK_WINDOW(about), "volume control", "Volume Control");
	gnome_window_icon_set_from_file (GTK_WINDOW (about), GNOME_ICONDIR"/mixer/gnome-mixer-applet.png");
        g_signal_connect (G_OBJECT (about), "destroy",
                          G_CALLBACK (gtk_widget_destroyed),
	                  &about);
	
        gtk_widget_show (about);
}

static void
mixer_help_cb (BonoboUIComponent *uic,
	       MixerData         *data,
	       const gchar       *verbname)
{
        GError *error = NULL;

	egg_help_display_on_screen (
		"mixer_applet2", NULL,
		gtk_widget_get_screen (data->applet),
		&error);

	if (error) { /* FIXME: the user needs to see this error */
		g_print ("%s \n", error->message);
		g_error_free (error);
		error = NULL;
	}
}

/* Dummy callback to get rid of a warning, for now. */
static void
mixer_mute_cb (BonoboUIComponent *uic,
	       gpointer           data,
	       const gchar       *verbname)
{
}

#ifdef OSS_API

static gboolean
cb_row_selected (GtkTreeSelection *selection, MixerData *data)
{
	PanelApplet *applet = PANEL_APPLET (data->applet);
	GtkTreeIter iter;
	GtkTreeModel *model;
	ChannelData *cdata;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) 
		return FALSE;
		
	gtk_tree_model_get (model, &iter, 1, &cdata, -1);
	if (cdata->channel == data->mixerchannel)
		return FALSE;		
	
	data->mixerchannel = cdata->channel;
	panel_applet_gconf_set_int (applet, "channel", data->mixerchannel, NULL);
}

static void
show_help_cb (GtkWindow *dialog)
{
	GError *error = NULL;
	static GnomeProgram *applet_program = NULL;
	
	if (!applet_program) {
		int argc = 1;
		char *argv[2] = { "mixer_applet2" };
		applet_program = gnome_program_init ("mixer_applet2", VERSION,
						      LIBGNOME_MODULE, argc, argv,
     						      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	}

	egg_help_display_desktop_on_screen (
			applet_program, "mixer_applet2", "mixer_applet2", NULL,
			gtk_widget_get_screen (GTK_WIDGET (dialog)),
			&error);

	if (error) {
		GtkWidget *error_dialog;

		error_dialog = gtk_message_dialog_new (
				NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				_("There was an error displaying help: %s"),
				error->message);

		g_signal_connect (error_dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (error_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (dialog)));
		gtk_widget_show (error_dialog);
		g_error_free (error);
	}
}

static void
dialog_response (GtkDialog *dialog, gint id, MixerData *data)
{
	if (id == GTK_RESPONSE_HELP) {
		show_help_cb (GTK_WINDOW (dialog));
		return;
	}
	
	gtk_widget_destroy (GTK_WIDGET (dialog));
	data->prefdialog = NULL;
	
}

static void
mixer_pref_cb (BonoboUIComponent *uic,
	       MixerData         *data,
	       const gchar       *verbname)
{
	GtkWidget *dialog;
	GtkWidget *vbox, *vbox1, *vbox2, *vbox3;
	GtkWidget *hbox, *hbox2;
	GtkWidget *label;
	GtkWidget *tree;
	GtkListStore *model;
	GtkWidget *scrolled;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	gchar *tmp;
	GList *list = data->channels;
	
	if (data->prefdialog) {
		gtk_window_present (GTK_WINDOW (data->prefdialog));
		return;
	}
	
	dialog = gtk_dialog_new_with_buttons (_("Volume Control Preferences"), NULL,
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                                   GTK_STOCK_HELP, GTK_RESPONSE_HELP,
                                                   NULL);
	gnome_window_icon_set_from_file (GTK_WINDOW (dialog), GNOME_ICONDIR"/mixer/gnome-mixer-applet.png");	
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_window_set_screen (GTK_WINDOW (dialog),
                           gtk_widget_get_screen (GTK_WIDGET (data->applet)));
        gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
        gtk_window_set_default_size (GTK_WINDOW (dialog), 300, 300);
        data->prefdialog = dialog;
                           
	vbox = GTK_DIALOG (dialog)->vbox;
	gtk_box_set_spacing (GTK_BOX (vbox), 2);
	
	vbox1 = gtk_vbox_new (FALSE, 18);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);
	gtk_box_pack_start (GTK_BOX (vbox), vbox1, TRUE, TRUE, 0);
	
	vbox2 = gtk_vbox_new (FALSE, 6);
        gtk_box_pack_start (GTK_BOX (vbox1), vbox2, TRUE, TRUE, 0);
        
        tmp = g_strdup_printf ("<b>%s</b>", _("Audio Channels"));
        label = gtk_label_new (NULL);
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
        gtk_label_set_markup (GTK_LABEL (label), tmp);
        g_free (tmp);
        gtk_box_pack_start (GTK_BOX (vbox2), label, FALSE, FALSE, 0);
        
        hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox, TRUE, TRUE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	vbox3 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox3, TRUE, TRUE, 0);
	
	label = gtk_label_new_with_mnemonic (_("_Select channel to control:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (vbox3), label, FALSE, FALSE, 0);
	
	scrolled = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox3), scrolled, TRUE, TRUE, 0);
	
	model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), tree);
	gtk_container_add (GTK_CONTAINER (scrolled), tree);
	g_object_unref (G_OBJECT (model));
	cell = gtk_cell_renderer_text_new ();
  	column = gtk_tree_view_column_new_with_attributes ("hello",
						    	   cell,
						     	   "text", 0,
						     	   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
        gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
        
        while (list) {
        	ChannelData *cdata = list->data;
        	GtkTreeIter iter;
        	
		gtk_list_store_append (GTK_LIST_STORE (model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, _(cdata->name), 1, cdata, -1);
		if (cdata->channel == data->mixerchannel) {
			GtkTreePath *path;
			path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (tree), path, NULL, FALSE);
		}
        	list = g_list_next (list);
        }

	if ( ! key_writable (PANEL_APPLET (data->applet), "channel")) {
		gtk_widget_set_sensitive (tree, FALSE);
		gtk_widget_set_sensitive (label, FALSE);
	}

        g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree))), 
			  	  "changed",
			  	   G_CALLBACK (cb_row_selected), data);
			  	   
	g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (dialog_response), data);
        
        gtk_widget_show_all (dialog);
        
}
#endif

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
#ifdef SUN_API
			set_mute_status (TRUE);
#else
			setMixer (0, data);
#endif
			mixer_update_image (data);
		
				
			gtk_tooltips_set_tip (data->tooltips,
					      data->applet,
					      _(access_name_mute),
					      NULL); 
			if (gail_loaded) {
				add_atk_namedesc (data->applet,
						  _(access_name_mute),
						  NULL);
			}
		}
		else {
#ifdef SUN_API
			set_mute_status (FALSE);
#else
			setMixer (data->vol, data);
#endif
			mixer_update_image (data);
			
			
			gtk_tooltips_set_tip (data->tooltips,
					      data->applet,
					      _(access_name),
					      NULL); 
			if (gail_loaded) {
				add_atk_namedesc (data->applet,
						  _(access_name),
						  NULL);
			}
		}
	}
}

static void
get_channel (MixerData *data)
{	
#ifdef OSS_API
	PanelApplet *applet = PANEL_APPLET (data->applet);
	gint num;
	
	num = panel_applet_gconf_get_int (applet, "channel", NULL);
	num = CLAMP (num, 0,  SOUND_MIXER_NRDEVICES-1);	
	data->mixerchannel = num;
#endif

}

static void
get_prefs (MixerData *data)
{
	PanelApplet *applet = PANEL_APPLET (data->applet);
	BonoboUIComponent *component;
	gboolean mute;
	
	data->vol = panel_applet_gconf_get_int (applet, "vol", NULL);
	mute = panel_applet_gconf_get_bool (applet, "mute", NULL);
	
	if (data->vol < 0)
		return;
		
	data->mute = mute;
	if (data->mute) {
#ifdef SUN_API
		set_mute_status (TRUE);
#else
		setMixer (0, data);
#endif
		mixer_update_image (data);
		
				
		gtk_tooltips_set_tip (data->tooltips,
					      data->applet,
					      _(access_name_mute),
					      NULL); 
		if (gail_loaded) {
				add_atk_namedesc (data->applet,
						  _(access_name_mute),
						  NULL);
		}
	}
	else {
#ifdef SUN_API
		set_mute_status (FALSE);
#else
		setMixer (data->vol, data);
#endif
		mixer_update_image (data);
			
		
		gtk_tooltips_set_tip (data->tooltips,
				      data->applet,
				      _(access_name),
				      NULL); 
		if (gail_loaded) {
			add_atk_namedesc (data->applet,
					  _(access_name),
					  NULL);
		}
	}
}

typedef struct
{
	char *stock_id;
	const guint8 *icon_data;
} MixerStockIcon;
    
static MixerStockIcon items[] =
    {
        { VOLUME_STOCK_MUTE, GNOME_ICONDIR"/mixer/volume-mute.png" },
	{ VOLUME_STOCK_ZERO, GNOME_ICONDIR"/mixer/volume-zero.png" },
	{ VOLUME_STOCK_MIN, GNOME_ICONDIR"/mixer/volume-min.png" },
	{ VOLUME_STOCK_MED, GNOME_ICONDIR"/mixer/volume-medium.png" },
	{ VOLUME_STOCK_MAX, GNOME_ICONDIR"/mixer/volume-max.png" }
};

static void
register_mixer_stock_icons (GtkIconFactory *factory)
{
    GtkIconSource *source;
    int i;

    source = gtk_icon_source_new ();

    for (i = 0; i < G_N_ELEMENTS (items); i++) {
	GtkIconSet *icon_set;

	gtk_icon_source_set_filename (source, items[i].icon_data);

	icon_set = gtk_icon_set_new ();
	gtk_icon_set_add_source (icon_set, source);

	gtk_icon_factory_add (factory, items[i].stock_id, icon_set);

	gtk_icon_set_unref (icon_set);
    }
    gtk_icon_source_free (source);
}

static void
mixer_init_stock_icons ()
{
	GtkIconFactory *factory;
	
	if (icons_initialized)
		return;

	volume_icon_size = gtk_icon_size_register ("panel-menu", 
						   VOLUME_DEFAULT_ICON_SIZE,
						   VOLUME_DEFAULT_ICON_SIZE);

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	register_mixer_stock_icons (factory);

	g_object_unref (factory);

	icons_initialized = TRUE;
}

const BonoboUIVerb mixer_applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("RunMixer", mixer_start_gmix_cb),
	BONOBO_UI_UNSAFE_VERB ("Mute",     mixer_mute_cb),
	BONOBO_UI_UNSAFE_VERB ("Help",     mixer_help_cb),
	BONOBO_UI_UNSAFE_VERB ("About",    mixer_about_cb),
#ifdef OSS_API
	BONOBO_UI_UNSAFE_VERB ("Pref",    mixer_pref_cb),
#endif
        BONOBO_UI_VERB_END
};

static void
applet_style_event_cb (GtkWidget *w, GtkStyle *prev_style, MixerData *user_data)
{	
	mixer_load_volume_images (user_data);
	mixer_update_image (user_data);
}

mixer_applet_create (PanelApplet *applet)
{
	MixerData         *data;
	BonoboUIComponent *component;
	const gchar *device;
	gboolean retval;
#ifdef SUN_API
	gchar *ctl = NULL;
#endif

	data = g_new0 (MixerData, 1);
	data->applet = GTK_WIDGET (applet);
	panel_applet_add_preferences (applet, "/schemas/apps/mixer_applet/prefs", NULL);
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	get_channel (data);

#ifdef OSS_API
	/* /dev/sound/mixer for devfs */
	device = "/dev/mixer";
	retval = openMixer(device, data);
	if (!retval) {
		device = "/dev/sound/mixer";
		retval = openMixer (device, data);
	}
#endif
#ifdef SUN_API
	if (!(ctl = g_getenv("AUDIODEV")))
		ctl = "/dev/audio";
	device = g_strdup_printf("%sctl",ctl);
	retval = openMixer(device, data);
#endif
	mixer_init_stock_icons ();
	if (!retval) {
		device_present = FALSE;
	}

 	if (GTK_IS_ACCESSIBLE (gtk_widget_get_accessible(GTK_WIDGET(applet)))) {
		gail_loaded = TRUE;
	}

	/* Try to find some mixers - sdtaudiocontrol is on Solaris and is needed
	** because gnome-volume-meter doesn't work */
	if (run_mixer_cmd == NULL) 
		run_mixer_cmd = g_find_program_in_path ("gnome-volume-control");
	if (run_mixer_cmd == NULL) 
		run_mixer_cmd = g_find_program_in_path ("gmix");
	if (run_mixer_cmd == NULL) 
		run_mixer_cmd = g_find_program_in_path ("sdtaudiocontrol");
	
	data->device = g_strdup (device);
	data->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (data->frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (applet), data->frame);
	
	data->image = gtk_image_new ();
	gtk_container_add (GTK_CONTAINER (data->frame), data->image);

	mixer_load_volume_images (data);

        data->tooltips = gtk_tooltips_new ();                                   
        gtk_tooltips_set_tip (data->tooltips,
			      data->applet,
			      _(access_name),
			      NULL);
	if (gail_loaded) { 
		add_atk_namedesc(GTK_WIDGET(data->applet),
				 _(access_name),
				 _("The volume control lets you set the volume level for your desktop"));
	} 
	
	g_signal_connect (data->applet,
			  "button-press-event",
			  (GCallback) applet_button_release_event_cb,
			  data);
	
	g_signal_connect (data->applet,
			  "key-press-event",
			  (GCallback) applet_key_press_event_cb,
			  data);
	
	g_signal_connect (data->applet,
			  "scroll-event",
			  (GCallback) applet_scroll_event_cb,
			  data);

	g_signal_connect (data->applet,
			  "style-set",
			  (GCallback) applet_style_event_cb,
			  data);
	
	data->adj = GTK_ADJUSTMENT (
		gtk_adjustment_new (-50,
				    -VOLUME_MAX,
				    0.0, 
				    VOLUME_MAX/20,
				    VOLUME_MAX/10,
				    0.0));

	g_signal_connect (data->adj,
			  "value-changed",
			  (GCallback) mixer_value_changed_cb,
			  data);

	/* Get the initial volume. */
	get_prefs (data);
	
	if (!data->mute)
		data->vol = readMixer (data);

	/* Install timeout handler, that keeps the applet up-to-date if the
	 * volume is changed somewhere else.
	 */
	data->timeout = gtk_timeout_add (500,
					 (GSourceFunc) mixer_timeout_cb,
					 data);

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

	g_signal_connect (applet,
			  "change_background",
			  G_CALLBACK (applet_change_background_cb),
			  data);

	panel_applet_setup_menu_from_file (applet, 
					   NULL, /* opt_datadir */
					   "GNOME_MixerApplet.xml",
					   NULL,
					   mixer_applet_menu_verbs,
					   data);

	if (panel_applet_get_locked_down (applet)) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (applet);

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/Pref",
					      "hidden", "1",
					      NULL);
	}


	component = panel_applet_get_popup_component (applet);
	g_signal_connect (component,
			  "ui-event",
			  (GCallback) mixer_ui_component_event,
			  data);
	if (data->mute)
		bonobo_ui_component_set_prop (component,
						      "/commands/Mute",
						      "state",
						      "1",
						      NULL);
	else
		bonobo_ui_component_set_prop (component,
						      "/commands/Mute",
						      "state",
						      "0",
						      NULL);

	if (run_mixer_cmd == NULL)
		bonobo_ui_component_rm (component, "/popups/popup/RunMixer", NULL);
		
#ifndef OSS_API
	bonobo_ui_component_rm (component, "/popups/popup/Pref", NULL);
#endif

	applet_change_orient_cb (GTK_WIDGET (applet),
				 panel_applet_get_orient (applet),
				 data);
	applet_change_size_cb (GTK_WIDGET (applet),
			       panel_applet_get_size (applet),
			       data);

	mixer_update_slider (data);
	mixer_update_image (data);

	gtk_widget_show_all (GTK_WIDGET (applet));
}

static gboolean
mixer_applet_factory (PanelApplet *applet,
		      const gchar *iid,
		      gpointer     data)
{
	mixer_applet_create (applet);

	return TRUE;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MixerApplet_Factory",
			     PANEL_TYPE_APPLET,
			     "mixer_applet2",
			     "0",
			     mixer_applet_factory,
			     NULL)
 
/* Accessible name and description */ 
void 
add_atk_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc) 
{ 
	AtkObject *atk_widget;
	atk_widget = gtk_widget_get_accessible(widget);
	atk_object_set_name(atk_widget, name);
	atk_object_set_description(atk_widget, desc);
}                                                                               
