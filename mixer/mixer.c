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
 *
 */

/*
 * Contact Dave Larson <davlarso@acm.org> for questions of
 * bugs concerning the Solaris audio api code.
 *
 */	

#include <math.h>
#include <stdio.h>
#include <config.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>

#include "vslider.h"
#include "hslider.h"

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

#include "lamp-small.xpm"
#include "lamp-small-red.xpm"

typedef void (*MixerUpdateFunc) (GtkWidget *, gint);

typedef struct {
	/* gint mastervol; */
	gint mute;
	gint timeout;
	MixerUpdateFunc update_func;
	PanelOrientType orient;
	int size;
	gint last_vol;
} MixerData;

typedef struct {
	GtkWidget *base;

	GtkWidget *vbox;
	GtkWidget *vslider;
	GtkWidget *vmutebutton;
	GtkWidget *vlightwid;

	GtkWidget *hbox;
	GtkWidget *hslider;
	GtkWidget *hmutebutton;
	GtkWidget *hlightwid;

	GtkAdjustment *hadj;
	GtkAdjustment *vadj;

	guint16 startcolor[3];
	guint16 endcolor[3];
} MixerWidget;

GtkWidget *applet = NULL;
GtkWidget *mixerw = NULL;

MixerData *md = NULL;

gint mixerfd = -1;

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
free_data(GtkWidget * widget, gpointer data)
{
	g_free(data);
	return;
	widget = NULL;
}

static void 
mute_cb (GtkWidget *widget, gpointer   data)
{
	static gint old_vol=-1;
	MixerData *p = data;
	MixerWidget *mx;

	mx = gtk_object_get_user_data(GTK_OBJECT(widget));

	if (GTK_TOGGLE_BUTTON (widget)->active ) {
		if (p->mute ==0) {
			/* turn off the timeout */
			gtk_widget_set_sensitive(mx->hslider, FALSE);
			gtk_widget_set_sensitive(mx->vslider, FALSE);
			p->mute = 1;
			old_vol = readMixer();
			setMixer(0);
			gnome_pixmap_load_xpm_d(GNOME_PIXMAP(mx->hlightwid),
				       		lamp_small_red_xpm);
			gnome_pixmap_load_xpm_d(GNOME_PIXMAP(mx->vlightwid),
				       		lamp_small_red_xpm);
		}
	} else {
		if (p->mute ==1) {
			p->mute = 0;
			if (old_vol >= 0)
				setMixer(old_vol);
			gtk_widget_set_sensitive(mx->hslider, TRUE);
			gtk_widget_set_sensitive(mx->vslider, TRUE);
			gnome_pixmap_load_xpm_d(GNOME_PIXMAP(mx->hlightwid),
				       		lamp_small_xpm);
			gnome_pixmap_load_xpm_d(GNOME_PIXMAP(mx->vlightwid),
				       		lamp_small_xpm);
		}
	}
/* printf("In mute_cb: mute button state is %d\n",p->mute); */
/*	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), p->mute); */
}

static void
adj_cb(GtkWidget *widget, gpointer   data)
{
	gint vol;
	MixerWidget *mx;
	mx = gtk_object_get_user_data(GTK_OBJECT(widget));
	if (GTK_ADJUSTMENT(widget) == mx->vadj)
		vol = -GTK_ADJUSTMENT(data)->value;
	else
		vol = -(-VOLUME_MAX - GTK_ADJUSTMENT(data)->value);
	/*	printf("In adj_cb: value is %d\n", vol); */
	setMixer(vol);
}

static void
set_other_button_cb(GtkWidget *widget, GtkWidget* other_button)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(other_button),
		       		GTK_TOGGLE_BUTTON(widget)->active);
}

static void
set_other_slider_cb(GtkAdjustment *adj, GtkAdjustment* other_adj)
{

	/* without the funny formula, the hslider would turn up the 
	   volume to the left and down to the right.  */

	if ( other_adj->value != -VOLUME_MAX - adj->value)
	gtk_adjustment_set_value(other_adj, -VOLUME_MAX - adj->value);
	
}

static int
mixer_timeout_callback(gpointer data)
{
	gint mvol;

	mvol = readMixer();
	if (mvol != md->last_vol) {
		(*md->update_func) (mixerw, mvol);
		md->last_vol = mvol;
	}
	return 1;
	data = NULL;
}

static void
mixer_set_color(MixerWidget* mx)
{
	GdkColormap* colormap;
	GdkColor bgColor;
	guint16 baseColor[3];
	gfloat inc_values[3];
	gfloat  mod_value;
	GtkWidget* cur_slider;
	gint win_height, win_width;

	/* operate on the slider that's currently showing */
	if (md->orient == ORIENT_UP || md->orient == ORIENT_DOWN) {
		if(md->size < PIXEL_SIZE_STANDARD)
			cur_slider=mx->hslider;
		else
			cur_slider=mx->vslider;
	} else  {
		if(md->size < PIXEL_SIZE_STANDARD)
			cur_slider=mx->vslider;
		else
			cur_slider=mx->hslider;
	}

	/* don't do anything if the slider isn't mapped to the screen */
	if (GTK_RANGE(cur_slider)->trough){
	
		colormap = gdk_window_get_colormap(GTK_RANGE(cur_slider)->
								trough);

		/* mod_value becomes a float between 0 and 1 inclusive. */
		mod_value = fabs( mx->hadj->value/
				(mx->hadj->upper - mx->hadj->lower));

		baseColor[0] = mx->endcolor[0];		
		baseColor[1] = mx->endcolor[1];		
		baseColor[2] = mx->endcolor[2];

		inc_values[0] =  (gfloat)mx->startcolor[0] - mx->endcolor[0];	
		inc_values[1] =  (gfloat)mx->startcolor[1] - mx->endcolor[1];	
		inc_values[2] =  (gfloat)mx->startcolor[2] - mx->endcolor[2];	

		/* interpolate a colour between startcolor and endcolor */	
		bgColor.red   = baseColor[0] + (mod_value * inc_values[0]);
		bgColor.green = baseColor[1] + (mod_value * inc_values[1]);
		bgColor.blue  = baseColor[2] + (mod_value * inc_values[2]);

		/* apply the new colour */
		gdk_color_alloc(colormap, &bgColor);
		gdk_window_set_background(GTK_RANGE(cur_slider)->trough,
								&bgColor);
		gdk_window_get_size(GTK_RANGE(cur_slider)->trough, &win_width,
				       				&win_height);
		gdk_window_clear_area(GTK_RANGE(cur_slider)->trough, 2, 2,
						win_width -4,win_height -4);
	}
}

	
static void
mixer_update_func(GtkWidget *mixer, gint mvol)
{
	MixerWidget *mx;

	mx = gtk_object_get_user_data(GTK_OBJECT(mixer));

	/* get out of mute mode if the volume is turned up by another program. */
	if ((md->mute ==1) && (mvol !=0))
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mx->hmutebutton),0);

	/* change the sliders to reflect the new value. */
	/*   ignore if we're in mute mode. */
	if (!md->mute) {
	gtk_adjustment_set_value(mx->vadj, (gfloat)(-mvol));
	gtk_adjustment_set_value(mx->hadj, (gfloat)(-VOLUME_MAX + mvol));
	}
	mixer_set_color(mx);
	

	/* need code to read current mixer ? Or not since timeout cb does it */
	/* how do I move the scale value once its realized?? */
/*	gtk_scale_set_draw_value(GTK_SCALE(mx->slider), mvol); */
}

static void
create_computer_mixer_widget(GtkWidget ** mixer, 
			     MixerUpdateFunc * update_func)
{
	MixerWidget* mx;
	GtkWidget *vbox, *align, *hbox, *vbutton, *hbutton, *vscale, *hscale; 
	GtkWidget *vlightwid, *hlightwid, *base;
	GtkAdjustment *hadj, *vadj;
	/* widget hierarchy:  
	   
	                        applet_widget
		                      |
			        ----vbox------ 
	                        |            |
                               vbox          hbox (only one of these boxes 
                                |            |       is shown at a given time)
                toggle_button AND slider     .
	  	   |                         .
	        gnome_pixmap                 .
	  */ 

	base = gtk_vbox_new(FALSE, 0);	
	
	hbutton = gtk_toggle_button_new();
	vbutton = gtk_toggle_button_new();
	
	vbox=gtk_vbox_new(FALSE,0);	
	align=gtk_alignment_new(0.5,0.0, 1.0, 1.0);
	hbox=gtk_hbox_new(FALSE,0);

	vscale = vslider_new();
	hscale = hslider_new();
	
        hadj=(GtkAdjustment *) gtk_adjustment_new (-50, -VOLUME_MAX, 0.0, 
					VOLUME_MAX/100, VOLUME_MAX/10, 0.0);
        vadj=(GtkAdjustment *) gtk_adjustment_new (-50, -VOLUME_MAX, 0.0, 
					VOLUME_MAX/100, VOLUME_MAX/10, 0.0);

	hlightwid = gnome_pixmap_new_from_xpm_d(lamp_small_xpm);
	vlightwid = gnome_pixmap_new_from_xpm_d(lamp_small_xpm);

	gtk_range_set_adjustment(GTK_RANGE(vscale), vadj);
	gtk_range_set_adjustment(GTK_RANGE(hscale), hadj);

	gtk_range_set_update_policy (GTK_RANGE(vscale), GTK_UPDATE_CONTINUOUS);
       	gtk_range_set_update_policy (GTK_RANGE(hscale), GTK_UPDATE_CONTINUOUS);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(hbutton), 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(vbutton), 0);


	/* cosmetic settings */
	GTK_WIDGET_UNSET_FLAGS(vscale, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(vscale, GTK_CAN_FOCUS);

	GTK_WIDGET_UNSET_FLAGS(hscale, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(hscale, GTK_CAN_FOCUS);

	GTK_WIDGET_UNSET_FLAGS(hbutton, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(hbutton, GTK_CAN_FOCUS);

	GTK_WIDGET_UNSET_FLAGS(vbutton, GTK_CAN_DEFAULT);
	GTK_WIDGET_UNSET_FLAGS(vbutton, GTK_CAN_FOCUS);
	
        gtk_scale_set_draw_value(GTK_SCALE(vscale), FALSE);
        gtk_scale_set_draw_value(GTK_SCALE(hscale), FALSE);

	gtk_widget_set_usize(GTK_WIDGET(vscale), 20, 32);
	gtk_widget_set_usize(GTK_WIDGET(vbutton),20, 15);

	gtk_widget_set_usize(GTK_WIDGET(hscale), 32, 20);
	gtk_widget_set_usize(GTK_WIDGET(hbutton),15, 20);

	
	/* pack the widgets */
	gtk_container_add(GTK_CONTAINER(align), vscale);
	gtk_box_pack_start(GTK_BOX(vbox),align, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),vbutton, FALSE, FALSE, 0); 

	gtk_box_pack_start(GTK_BOX(hbox),hbutton, FALSE, FALSE, 0); 
	gtk_box_pack_start(GTK_BOX(hbox),hscale, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(base), vbox, TRUE, TRUE,0);
	gtk_box_pack_start(GTK_BOX(base), hbox, TRUE, TRUE,0);

	gtk_container_add(GTK_CONTAINER(hbutton),hlightwid);
	gtk_container_add(GTK_CONTAINER(vbutton),vlightwid);

	/* keep the hor. and vert. sliders in sync. */
	gtk_signal_connect(GTK_OBJECT(hadj), "value_changed",
			  (GtkSignalFunc)set_other_slider_cb, (gpointer)vadj);
	gtk_signal_connect(GTK_OBJECT(vadj), "value_changed",
			  (GtkSignalFunc)set_other_slider_cb, (gpointer)hadj);

	/* write slider events to the mixer. */
	gtk_signal_connect(GTK_OBJECT(vadj), "value_changed",
			   (GtkSignalFunc)adj_cb, (gpointer)vadj);
        gtk_signal_connect(GTK_OBJECT(hadj), "value_changed",
			   (GtkSignalFunc)adj_cb, (gpointer)hadj);

	/* keep mute buttons in sync */	
	gtk_signal_connect(GTK_OBJECT(vbutton), "toggled",
		  (GtkSignalFunc)set_other_button_cb, (gpointer)hbutton);
	gtk_signal_connect(GTK_OBJECT(hbutton), "toggled",
		  (GtkSignalFunc)set_other_button_cb, (gpointer)vbutton);

	/* apply mute button events */
	gtk_signal_connect(GTK_OBJECT(vbutton), "toggled",
		   (GtkSignalFunc)mute_cb, (gpointer)md);
        gtk_signal_connect(GTK_OBJECT(hbutton), "toggled",
		   (GtkSignalFunc)mute_cb, (gpointer)md);

	applet_widget_set_tooltip(APPLET_WIDGET(applet), _("Main Volume and Mute"));

	

	/* show the widgets */
	gtk_widget_show (hbutton);
	gtk_widget_show (vbutton);

	gtk_widget_show(hlightwid);
	gtk_widget_show(vlightwid);

	gtk_widget_show (vscale);
	gtk_widget_show (hscale);
	gtk_widget_show (align);

	if (md->orient == ORIENT_UP || md->orient == ORIENT_DOWN) {
		if(md->size < PIXEL_SIZE_STANDARD)
			gtk_widget_show(hbox);
		else
			gtk_widget_show(vbox);
	} else  {
		if(md->size < PIXEL_SIZE_STANDARD)
			gtk_widget_show(vbox);
		else
			gtk_widget_show(hbox);
	}
		
	/* signal handlers can use gtk_object_get_user_data to get a
   	pointer to mx and access any of the mixer's widgets. */	   
	mx = g_new(MixerWidget, 1);
	mx->base = base;
	mx->vbox = vbox;
	mx->hbox = hbox;
	mx->vslider = vscale;
	mx->hslider = hscale;
	mx->vmutebutton = vbutton;
	mx->hmutebutton = hbutton;
	mx->vlightwid = vlightwid;
	mx->hlightwid = hlightwid;
	mx->hadj = hadj;
        mx->vadj = vadj;	

	/* set the slider background colors */

	mx->startcolor[0] = 45000;  /* R */
	mx->startcolor[1] = 0;	    /* G */
	mx->startcolor[2] = 0;      /* B */
	
	mx->endcolor[0] = 0;
	mx->endcolor[1] = 0;
	mx->endcolor[2] = 45000;

	gtk_object_set_user_data(GTK_OBJECT(base), mx);
	gtk_object_set_user_data(GTK_OBJECT(hbutton), mx);
	gtk_object_set_user_data(GTK_OBJECT(vbutton), mx);
	gtk_object_set_user_data(GTK_OBJECT(hadj), mx);
	gtk_object_set_user_data(GTK_OBJECT(vadj), mx);

	gtk_signal_connect(GTK_OBJECT(base), "destroy",
			   (GtkSignalFunc) free_data, mx);

	*mixer = base;
	*update_func = mixer_update_func;

}

static void
start_gmix_cb (AppletWidget *applet, gpointer data)
{
	gnome_execute_shell(NULL, "gmix");
}

static void
destroy_mixer(GtkWidget * widget, void *data)
{
	gtk_timeout_remove(md->timeout);
	g_free(md);
	return;
	widget = NULL;
	data = NULL;
}

static GtkWidget *
create_mixer_widget(void)
{
	GtkWidget *mixer;
	gint      curmvol;

	md = g_new(MixerData, 1);
	
	/* ask the panel for its current orientation and size. */
	md->orient = applet_widget_get_panel_orient(APPLET_WIDGET(applet));
	md->size = applet_widget_get_panel_pixel_size(APPLET_WIDGET(applet));

	/* do the nitty-gritty GTK-related stuff */
	create_computer_mixer_widget(&mixer, &md->update_func);

	/* Install timeout handler */

	md->timeout = gtk_timeout_add(500, mixer_timeout_callback, mixer);
	md->mute   = 0;

	gtk_signal_connect(GTK_OBJECT(mixer), "destroy",
			   (GtkSignalFunc) destroy_mixer,
			   NULL);
	/* Call the mixer's update function so that it paints its first state */

	/* add code to read currect status of mixer */
	curmvol = readMixer(); /* replace with proper read */
	(*md->update_func) (mixer, curmvol);
	md->last_vol = curmvol;
	return mixer;
}

static void
mixer_about (AppletWidget *applet, gpointer data)
{
        static GtkWidget   *about     = NULL;
        static const gchar *authors[] =
        {
		"Michael Fulbright <msf@redhat.com>",
                NULL
        };

        if (about != NULL)
        {
                gdk_window_show(about->window);
                gdk_window_raise(about->window);
                return;
        }
        
        about = gnome_about_new (_("Mixer Applet"), VERSION,
                                 _("(c) 1998 The Free Software Foundation"),
                                 authors,
                                 _("The mixer applet gives you instant access to setting the master volume level on your soundcard device"),
                                 NULL);
        gtk_signal_connect (GTK_OBJECT(about), "destroy",
                            GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
        gtk_widget_show (about);
}

/*these are commands sent over corba: */
static void
applet_change_orient(GtkWidget *w, PanelOrientType o)
{
	gint mvol;
	MixerWidget* mx;
	
	mvol = readMixer();

	(*md->update_func) (mixerw, mvol);

	mx = gtk_object_get_user_data(GTK_OBJECT(w));	

	if(md->size < PIXEL_SIZE_STANDARD) {
		/* hide one box */
		if((ORIENT_UP == md->orient) || (ORIENT_DOWN == md->orient))
			gtk_widget_hide(mx->hbox);
		else
			gtk_widget_hide(mx->vbox);

		/* and show the other */
		if(ORIENT_UP == o || ORIENT_DOWN == o)
			gtk_widget_show(mx->hbox);
		else
			gtk_widget_show(mx->vbox);
	} else {
		/* hide one box */
		if((ORIENT_UP == md->orient) || (ORIENT_DOWN == md->orient))
			gtk_widget_hide(mx->vbox);
		else
			gtk_widget_hide(mx->hbox);

		/* and show the other */
		if(ORIENT_UP == o || ORIENT_DOWN == o)
			gtk_widget_show(mx->vbox);
		else
			gtk_widget_show(mx->hbox);
	}

	md->orient = o;

	/* without this, the mixer sometimes draws itself incorrectly
	   if you drag the panel around the screen a lot. */
	while(gtk_events_pending())
		gtk_main_iteration();
}
static void
applet_change_pixel_size(GtkWidget *w, int size)
{
	gint mvol;
	MixerWidget* mx;
	
	mvol = readMixer();

	(*md->update_func) (mixerw, mvol);

	mx = gtk_object_get_user_data(GTK_OBJECT(w));	
	
	md->size = size;

	if (md->orient == ORIENT_UP || md->orient == ORIENT_DOWN) {
		if(md->size < PIXEL_SIZE_STANDARD) {
			gtk_widget_hide(mx->vbox);
			gtk_widget_show(mx->hbox);
		} else {
			gtk_widget_hide(mx->hbox);
			gtk_widget_show(mx->vbox);
		}
	} else  {
		if(md->size < PIXEL_SIZE_STANDARD) {
			gtk_widget_hide(mx->hbox);
			gtk_widget_show(mx->vbox);
		} else {
			gtk_widget_hide(mx->vbox);
			gtk_widget_show(mx->hbox);
		}
	}

	/* without this, the mixer sometimes draws itself incorrectly
	   if you drag the panel around the screen a lot. */
	while(gtk_events_pending())
		gtk_main_iteration();
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "mixer_applet", "index.html"};
	gnome_help_display(NULL, &help_entry);
}

int
main(int argc, char **argv)
{
	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	applet_widget_init("mixer_applet", VERSION, argc, argv,
				    NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mixer.png");

#ifdef OSS_API
	openMixer("/dev/mixer");
#endif
#ifdef SUN_API
	openMixer("/dev/audioctl");
#endif

	applet = applet_widget_new("mixer_applet");
	if (!applet)
		g_error(("Can't create applet!\n"));

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
			   GTK_SIGNAL_FUNC(applet_change_pixel_size),
			   NULL);

	mixerw = create_mixer_widget();
	gtk_widget_show(mixerw);
	applet_widget_add(APPLET_WIDGET(applet), mixerw);

	gtk_object_set_user_data(GTK_OBJECT(applet),
				gtk_object_get_user_data(GTK_OBJECT(mixerw)));

        applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "run_gmix",
					       GNOME_STOCK_MENU_VOLUME,
					       _("Run Audio Mixer..."),
					       start_gmix_cb,
					       NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"), help_cb, NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       mixer_about,
					       NULL);

	gtk_widget_show(applet);

	applet_widget_gtk_main();

	return 0;
}
