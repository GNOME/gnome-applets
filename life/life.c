/* 
 * The game of life
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: The man in the box
 */

#include <config.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <panel-applet.h>
#include <gnome.h>
#include <gdk/gdk.h>

#define LIFE_CYCLE 600
#define MAX_SIZE 80

int board[MAX_SIZE][MAX_SIZE];
guchar rgb_buffer[MAX_SIZE*MAX_SIZE*3];
GtkWidget *darea = NULL;
int size = 45;

double rsin = 0.1;
double gsin = 0.4;
double bsin = 1.0;

static int bouncex = 10;
static int bouncey = 10;
static int bouncexsp = 2;
static int bounceysp = 1;

static void
life_draw(void)
{
	int i,j;
	GdkGC *gc;
	int r,g,b;
	
	if(!darea ||
	   !GTK_WIDGET_REALIZED(darea) ||
	   !GTK_WIDGET_DRAWABLE(darea) ||
	   size<=0)
		return;
	
	r = 255*((sin(rsin)+1)/2);
	g = 255*((sin(gsin)+1)/2);
	b = 255*((sin(bsin)+1)/2);
	rsin+=((rand()>>6)%5 + 1)*0.01;
	gsin+=((rand()>>6)%5 + 1)*0.01;
	bsin+=((rand()>>6)%5 + 1)*0.01;
	
	bouncex+=bouncexsp;
	if(bouncex>size) {
		bouncex=size;
		bouncexsp=-((rand()>>6)%3+1);
	} else if(bouncex<0) {
		bouncex=0;
		bouncexsp=((rand()>>6)%3+1);
	}
	bouncey+=bounceysp;
	if(bouncey>size) {
		bouncey=size;
		bounceysp=-((rand()>>6)%3+1);
	} else if(bouncey<0) {
		bouncey=0;
		bounceysp=((rand()>>6)%3+1);
	}
	
	gc = gdk_gc_new(darea->window);

	for(j=0;j<size;j++) {
		guchar *p = rgb_buffer + j*MAX_SIZE*3;
		for(i=0;i<size;i++) {
			double distance =
				abs(sqrt((i-bouncex)*(i-bouncex)+
					 (j-bouncey)*(j-bouncey)))/5.0;
			double mult = 1.0;
			int val;
			if(distance < 0.1)
				distance = 0.1;
			mult += -log(distance)+2.4;
			if(mult<1.0) mult = 1.0;
			else if(mult>200.0) mult = 200.0;

			if(board[i][j]) {
				*(p++) = 255-r;
				val = (255-g)*mult;
				if(val>255) val = 255;
				*(p++) = val;
				*(p++) = (255-b)/mult;
			} else {
				val = r*mult;
				if(val>255) val = 255;
				*(p++) = val;
				*(p++) = g/mult;
				*(p++) = b;
			}
		}
	}
	gdk_draw_rgb_image(darea->window,gc,
			   0,0, size, size,
			   GDK_RGB_DITHER_NORMAL,
			   rgb_buffer, MAX_SIZE*3);
	
	gdk_gc_destroy(gc);
}

static int
life_expose(void)
{
	life_draw();
	return FALSE;
}

#define IS_THERE(i,j,ofi,ofj) \
(((board[(size+i+ofi)%size][(size+j+ofj)%size])&1)?1:0)

static int
cycle(gpointer data)
{
	int i,j;
	for(i=0;i<size;i++)
		for(j=0;j<size;j++) {
			int n = 0;
			n += IS_THERE(i,j,-1,-1);
			n += IS_THERE(i,j,0,-1);
			n += IS_THERE(i,j,1,-1);
			n += IS_THERE(i,j,-1,1);
			n += IS_THERE(i,j,0,1);
			n += IS_THERE(i,j,1,1);
			n += IS_THERE(i,j,1,0);
			n += IS_THERE(i,j,-1,0);
			/*clear everything but the first bit*/
			board[i][j] &= 1;
			if((board[i][j] && (n==3 || n==2)) ||
			   (!board[i][j] && (n == 3)))
				board[i][j] |= 2;
		}
	/*shift in the new generation*/
	for(i=0;i<size;i++)
		for(j=0;j<size;j++)
			board[i][j] >>= 1;
	life_draw();
	return TRUE;
}

static void
randomize (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	int i,j;
	/*randomize the entire board*/
	for(i=0;i<77;i++)
		for(j=0;j<77;j++)
			board[i][j]=((rand()>>6)&1);
	life_draw();
	return;
}


static GtkWidget *
create_life (void)
{
	GtkWidget *frame;
	
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	darea = gtk_drawing_area_new();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
	gtk_drawing_area_size(GTK_DRAWING_AREA(darea), size,size);

	gtk_container_add (GTK_CONTAINER (frame), darea);
	gtk_widget_show (darea);
	gtk_widget_show (frame);

	gtk_signal_connect_after(GTK_OBJECT(darea), "realize",
				 GTK_SIGNAL_FUNC(life_draw), NULL);
	gtk_signal_connect(GTK_OBJECT(darea), "expose_event",
			   GTK_SIGNAL_FUNC(life_expose), NULL);

	return frame;
}

static void
about (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	static const char *authors[] = { "The man in the box", NULL };
	static GtkWidget *about_box = NULL;

	if (about_box != NULL)
	{
		gdk_window_show(about_box->window);
		gdk_window_raise(about_box->window);
		return;
	}
	about_box = gnome_about_new (_("The Game of Life"),
				     VERSION,
				     _("Copyright (C) The Free Software Foundation"),
				     _("A complete waste of perfectly good CPU cycles."),
				     authors,
				     NULL,
				     NULL,
				     NULL);

	gtk_signal_connect( GTK_OBJECT(about_box), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box );
	gtk_widget_show(about_box);
	return;
}

static void
applet_change_size_cb(PanelApplet *w, gint sz, gpointer data)
{
	size = sz - 4;
	if(size>MAX_SIZE) size=MAX_SIZE;
	gtk_drawing_area_size(GTK_DRAWING_AREA(darea), size,size);
	gtk_widget_set_usize(GTK_WIDGET(darea), size,size);
	life_draw();
	return;
}

static void
help_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
#ifdef FIXME
    GnomeHelpMenuEntry help_entry = { "life_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
#endif
}

static const BonoboUIVerb life_applet_menu_verbs [] = {
        BONOBO_UI_VERB ("Randomize", randomize),
        BONOBO_UI_VERB ("Help", help_cb),
        BONOBO_UI_VERB ("About", about),

        BONOBO_UI_VERB_END
};

static const char life_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Item 1\" verb=\"Randomize\" _label=\"Randomize\"/>\n"
	"   <menuitem name=\"Item 2\" verb=\"Help\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"Item 3\" verb=\"About\" _label=\"About\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";

static gboolean
life_applet_fill (PanelApplet *applet)
{
	GtkWidget *life;
	
	/*do some randomizing*/
	srand(time(NULL));
	rsin += ((rand()>>6)%255)/100.0;
	gsin += ((rand()>>6)%255)/100.0;
	bsin += ((rand()>>6)%255)/100.0;

	randomize(NULL,NULL,NULL);
	
	size = panel_applet_get_size(applet) - 2;
	if(size>MAX_SIZE) size=MAX_SIZE;
	
	g_signal_connect (G_OBJECT (applet), "change_size",
			  G_CALLBACK (applet_change_size_cb), NULL);
	
	life = create_life ();
	gtk_container_add (GTK_CONTAINER (applet), life);
	gtk_widget_show (GTK_WIDGET (applet));
	
	gtk_timeout_add(LIFE_CYCLE,cycle,NULL);
	
	panel_applet_setup_menu (applet,
				 life_applet_menu_xml,
				 life_applet_menu_verbs,
				 life);
	
	return TRUE;
}

static gboolean
life_applet_factory (PanelApplet *applet,
			const gchar *iid,
			gpointer     data)
{
	gboolean retval = FALSE;
    
	if (!strcmp (iid, "OAFIID:GNOME_LifeApplet"))
		retval = life_applet_fill (applet); 
    
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_LifeApplet_Factory",
			     "Life Applet",
			     "0",
			     life_applet_factory,
			     NULL)

