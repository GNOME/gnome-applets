/* Fifteen sliding pieces game
 *
 * The game of life
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: The man in the box
 */

#include <config.h>
#include <applet-widget.h>

#define APPLET_SIZE 45

#define LIFE_CYCLE 300

int board[APPLET_SIZE][APPLET_SIZE];
GtkWidget *darea = NULL;

static void
life_draw(void)
{
	int i,j;
	GdkGC *gc;
	if(!darea ||
	   !GTK_WIDGET_REALIZED(darea) ||
	   !GTK_WIDGET_DRAWABLE(darea))
		return;
	
	gc = gdk_gc_new(darea->window);

	for(i=0;i<APPLET_SIZE;i++)
		for(j=0;j<APPLET_SIZE;j++) {
			if(board[i][j])
				gdk_gc_set_foreground(gc,&darea->style->black);
			else
				gdk_gc_set_foreground(gc,&darea->style->white);
			gdk_draw_point(darea->window,gc,i,j);
		}
	       
	gdk_gc_destroy(gc);
}

static int
life_expose(void)
{
	life_draw();
	return FALSE;
}

#define IS_THERE(i,j,ofi,ofj) \
(((board[(APPLET_SIZE+i+ofi)%APPLET_SIZE][(APPLET_SIZE+j+ofj)%APPLET_SIZE])&1)?1:0)

static int
cycle(gpointer data)
{
	int i,j;
	for(i=0;i<APPLET_SIZE;i++)
		for(j=0;j<APPLET_SIZE;j++) {
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
	for(i=0;i<APPLET_SIZE;i++)
		for(j=0;j<APPLET_SIZE;j++)
			board[i][j] >>= 1;
	life_draw();
	return TRUE;
}

static void
randomize (AppletWidget *applet, gpointer data)
{
	int i,j;
	for(i=0;i<APPLET_SIZE;i++)
		for(j=0;j<APPLET_SIZE;j++)
			board[i][j]=(rand()&1);
	life_draw();
}


static GtkWidget *
create_life (void)
{
	GtkWidget *frame;
	
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	darea = gtk_drawing_area_new();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();

	gtk_drawing_area_size(GTK_DRAWING_AREA(darea),
			      APPLET_SIZE,APPLET_SIZE);

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
about (AppletWidget *applet, gpointer data)
{
	static const char *authors[] = { "The man in the box", NULL };
	GtkWidget *about_box;

	about_box = gnome_about_new (_("The Game of Life"),
				     _("1.0"),
				     _("Copyright (C) The Free Software Foundation"),
				     authors,
				     _("A complete waste of perfectly good CPU cycles."),
				     NULL);

	gtk_widget_show(about_box);
}

int
main (int argc, char **argv)
{
	GtkWidget *applet;
	GtkWidget *life;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("life_applet", VERSION, argc,
			    argv, NULL, 0, NULL);

	applet = applet_widget_new ("life_applet");
	if (!applet)
		g_error (_("Can't create life applet!"));
	
	randomize(NULL,NULL);

	life = create_life ();
	applet_widget_add (APPLET_WIDGET (applet), life);
	gtk_widget_show (life);
	
	gtk_widget_show (applet);

	applet_widget_register_callback (APPLET_WIDGET (applet),
					 "randomize",
					 _("Randomize"),
					 randomize,
					 life);

	applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       about,
					       NULL);

	gtk_timeout_add(LIFE_CYCLE,cycle,NULL);

	applet_widget_gtk_main ();

	return 0;
}
