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
#include <libgnomeui/gnome-about.h>
#include <libgnomeui/gnome-window-icon.h>
#include <panel-applet.h>
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

#define IS_PANEL_HORIZONTAL(o) (o == PANEL_APPLET_ORIENT_UP || o == PANEL_APPLET_ORIENT_DOWN)

typedef struct {
	PanelAppletOrient  orientation;

	gint               timeout;
	gboolean           mute;
	gint               vol;
	gint               vol_before_popup;

	GtkAdjustment     *adj;

	GtkWidget         *applet;
	GtkWidget         *frame;
	GtkWidget         *image;
	
	/* The popup window and scale. */
	GtkWidget         *popup;
	GtkWidget         *scale;

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

void add_atk_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc);

static gint mixerfd = -1;
static gchar *run_mixer_cmd = NULL;

static const gchar *access_name = N_("Volume Control");     
static const gchar *access_name_mute = N_("Volume Control (muted)");
gboolean gail_loaded = FALSE;  

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
static gboolean
openMixer(const gchar *device_name ) 
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
	res = ioctl(mixerfd, MIXER_READ(SOUND_MIXER_DEVMASK), &devmask);
	if (res != 0) {
		char *s = g_strdup_printf(_("Querying available channels of mixer device %s failed\n"), device_name);
		gnome_error_dialog(s);
		g_free(s);
		return TRUE;
	} else if (devmask & SOUND_MASK_VOLUME) {
		mixerchannel = SOUND_MIXER_VOLUME;
	} else if (devmask & SOUND_MASK_PCM) {
		g_message(_("warning: mixer has no volume channel - using PCM instead.\n"));
		mixerchannel = SOUND_MIXER_PCM;
	} else {
		char *s = g_strdup_printf(_("Mixer device %s has neither volume nor PCM channels.\n"), device_name);
		gnome_error_dialog(s);
		g_free(s);
		return TRUE;
	}
#endif	
 	return TRUE;	
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
		
		gtk_tooltips_set_tip (data->tooltips,
				      data->applet,
				      _(access_name),
			              NULL); 
		if (gail_loaded) {
			add_atk_namedesc (data->applet, _(access_name), NULL);
		}
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


	scaled = gdk_pixbuf_scale_simple (pixbuf, size, size, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf (GTK_IMAGE (data->image), scaled);
	g_object_unref (scaled);
	gtk_widget_set_size_request (GTK_WIDGET (data->frame), 
					     MAX (11, size), MAX (11, size));
	g_object_unref (pixbuf);
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

	if (data->popup != NULL) {
		mixer_popup_hide (data, FALSE);
		return TRUE;
	}

	return FALSE;
}

static gboolean
applet_button_release_event_cb (GtkWidget *widget, GdkEventButton *event, MixerData *data)
{
	if (event->button == 1) {
		if (data->popup == NULL) {
			mixer_popup_show (data);
			return TRUE;
		}
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

	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		/* Apply. */
		if (data->popup != NULL)
			mixer_popup_hide (data, FALSE);
		else
			mixer_popup_show (data);
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
	gint            x, y;
	gint            width, height;
	GdkGrabStatus   pointer, keyboard;

	data->popup = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_screen (GTK_WINDOW (data->popup),
			       gtk_widget_get_screen (data->applet));

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
		gtk_widget_set_size_request (data->scale, -1, 100);			
	} else {
		data->scale = gtk_hscale_new (data->adj);
		gtk_widget_set_size_request (data->scale, 100, -1);
		gtk_range_set_inverted (GTK_RANGE (data->scale), TRUE);
	}
	
	if (gail_loaded) {
		add_atk_namedesc (data->scale, 
				_("Volume Controller"),
				_("Use Up/Down arrow keys to change volume"));
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
						 GTK_BUTTONS_CLOSE,
						 _("There was an error executing '%s' : %s"),
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

	egg_screen_help_display (
		gtk_widget_get_screen (data->applet),
		"mixer_applet2", NULL, &error);

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
			setMixer (data->vol);
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

#ifdef OSS_API
	/* /dev/sound/mixer for devfs */
	device = "/dev/mixer";
	retval = openMixer(device);
	if (!retval) {
		device = "/dev/sound/mixer";
		retval = openMixer (device);
	}
#endif
#ifdef SUN_API
	if (!(ctl = g_getenv("AUDIODEV")))
		ctl = "/dev/audio";
	device = g_strdup_printf("%sctl",ctl);
	retval = openMixer(device);
#endif
	mixer_init_stock_icons ();
	if (!retval) {
		GtkWidget *dialog;
		dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
						 ("Couldn't open mixer device %s\n"),
						 device, NULL);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (applet)));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

 	if (GTK_IS_ACCESSIBLE (gtk_widget_get_accessible(GTK_WIDGET(applet)))) {
		gail_loaded = TRUE;
	}

	data = g_new0 (MixerData, 1);

	/* Try to find some mixers - sdtaudiocontrol is on Solaris and is needed
	** because gnome-volume-meter doesn't work */
	if (run_mixer_cmd == NULL) 
		run_mixer_cmd = g_find_program_in_path ("gnome-volume-control");
	if (run_mixer_cmd == NULL) 
		run_mixer_cmd = g_find_program_in_path ("gmix");
	if (run_mixer_cmd == NULL) 
		run_mixer_cmd = g_find_program_in_path ("sdtaudiocontrol");
	
	data->frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (data->frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (applet), data->frame);
	
	data->image = gtk_image_new ();
	gtk_container_add (GTK_CONTAINER (data->frame), data->image);

	data->applet = GTK_WIDGET (applet);
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
			  "button-release-event",
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
	data->vol = readMixer ();

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

	component = panel_applet_get_popup_component (applet);
	g_signal_connect (component,
			  "ui-event",
			  (GCallback) mixer_ui_component_event,
			  data);

	if (run_mixer_cmd == NULL)
		bonobo_ui_component_rm (component, "/popups/popup/RunMixer", NULL);

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
