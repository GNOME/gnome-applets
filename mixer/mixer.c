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

#include "vslider.h"
#include "hslider.h"

#ifdef HAVE_LINUX_SOUNDCARD_H
#include <linux/soundcard.h>
#elif HAVE_MACHINE_SOUNDCARD_H
#include <machine/soundcard.h>
#elif HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#else
#error No soundcard defenition!
#endif /* SOUNDCARD_H */
 
#include "lamp-small.xpm"
#include "lamp-small-red.xpm"

typedef void (*MixerUpdateFunc) (GtkWidget *, gint);

typedef struct {
	/* gint mastervol; */
	gint mute;
	gint timeout;
	MixerUpdateFunc update_func;
	PanelOrientType orient;
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
		vol = -(-100 - GTK_ADJUSTMENT(data)->value);
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

	if ( other_adj->value != -100 - adj->value)
	gtk_adjustment_set_value(other_adj, -100 - adj->value);
	
}

static int
mixer_timeout_callback(gpointer data)
{
	gint mvol;

	mvol = readMixer();
	(*md->update_func) (mixerw, mvol);

	return 1;
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
	if (md->orient == ORIENT_UP || md->orient == ORIENT_DOWN) 
	       	cur_slider=mx->vslider;
	else 
		cur_slider=mx->hslider;

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
	gtk_adjustment_set_value(mx->hadj, (gfloat)(-100 + mvol));
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
	GtkWidget *vbox, *hbox, *vbutton, *hbutton, *vscale, *hscale; 
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
	hbox=gtk_hbox_new(FALSE,0);

	vscale = vslider_new();
	hscale = hslider_new();
	
        hadj=(GtkAdjustment *) gtk_adjustment_new (-50, -100.0, 0.0, 
					50.0, 25.0, 0.0);
        vadj=(GtkAdjustment *) gtk_adjustment_new (-50, -100.0, 0.0, 
					50.0, 25.0, 0.0);

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

	gtk_widget_set_usize(GTK_WIDGET(vscale), 20, 36);
	gtk_widget_set_usize(GTK_WIDGET(hscale), 36, 20);

	gtk_widget_set_usize(GTK_WIDGET(vbutton),20 , 12);
	gtk_widget_set_usize(GTK_WIDGET(hbutton),12 , 20);

	
	/* pack the widgets */
	gtk_box_pack_start(GTK_BOX(vbox),vscale, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),vbutton, TRUE, TRUE, 0); 

	gtk_box_pack_start(GTK_BOX(hbox),hbutton, TRUE, TRUE, 0); 
	gtk_box_pack_start(GTK_BOX(hbox),hscale, TRUE, TRUE, 0);

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

	applet_widget_set_tooltip(APPLET_WIDGET(applet),  "Main Volume and Mute");

	

	/* show the widgets */
	gtk_widget_show (hbutton);
	gtk_widget_show (vbutton);

	gtk_widget_show(hlightwid);
	gtk_widget_show(vlightwid);

	gtk_widget_show (vscale);
	gtk_widget_show (hscale);

	if (md->orient == ORIENT_UP || md->orient == ORIENT_DOWN)
		gtk_widget_show(vbox);
	else
		gtk_widget_show(hbox);
		
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

	mx->startcolor[0] = 30000;  /* R */
	mx->startcolor[1] = 0;	    /* G */
	mx->startcolor[2] = 0;      /* B */
	
	mx->endcolor[0] = 0;
	mx->endcolor[1] = 0;
	mx->endcolor[2] = 30000;

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
	start_gmix_cb()
{
	gnome_execute_shell(NULL, "gmix");
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
	
	/* ask the panel for its current orientation. */
	md->orient = applet_widget_get_panel_orient(APPLET_WIDGET(applet));

	/* do the nitty-gritty GTK-related stuff */
	create_computer_mixer_widget(&mixer, &md->update_func);

	/* Install timeout handler */

	md->timeout = gtk_timeout_add(50, mixer_timeout_callback, mixer);
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
	MixerWidget* mx;
	
	mvol = readMixer();

	(*md->update_func) (mixerw, mvol);

	mx = gtk_object_get_user_data(GTK_OBJECT(w));	

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

	md->orient = o;

	/* without this, the mixer sometimes draws itself incorrectly
	   if you drag the panel around the screen a lot. */
	while(gtk_events_pending())
		gtk_main_iteration();
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
	applet_widget_init("mixer_applet", VERSION, argc, argv,
				    NULL, 0, NULL);

	applet = applet_widget_new("mixer_applet");
	if (!applet)
		g_error("Can't create applet!\n");

	mixerw = create_mixer_widget();
	gtk_widget_show(mixerw);
	applet_widget_add(APPLET_WIDGET(applet), mixerw);

	gtk_object_set_user_data(GTK_OBJECT(applet),
				gtk_object_get_user_data(GTK_OBJECT(mixerw)));

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

        applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                                "run_gmix",
                                                GNOME_STOCK_MENU_VOLUME,
                                                _("Run gmix..."),
                                                start_gmix_cb, NULL);

	gtk_widget_show(applet);

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
