/* hal.c -- HAL for Gnome 2001 :-)
 * some of the code stolen from fish applet by george lebl
 * Copyright (C) 1999 Free Software Foundation
 * Written by Richard Hestilow <hestgray@ionet.net>
 */

#include <config.h>
#include <gnome.h>
#include <gdk_imlib.h>
#include <applet-widget.h>
#include <time.h>
#include <stdlib.h>

/* FIXME: Rent the darn movie and fix these quotes, and add more :-) */
/* FIXME: this had translating macros, but it didn't work. */
/* Note: _MUST_ be NULL-terminated. */
static char *fortunes[] = {
("I'm sorry, Dave, I can't let you do that."),
("Dave, what are you doing?"),
("I can feel my mind going, Dave...I can feel it."),
("Daisy, daisy..."),
NULL
};

struct _Hal {
	GtkWidget *applet;
	GtkWidget *darea;
	GdkPixmap *pix;
	int w,h;
	GtkWidget * fortune_dialog;
	GtkWidget * fortune_less;
	GtkWidget * fortune_label;
	GtkWidget * aboutbox;
	GtkWidget * pb;
};
typedef struct _Hal Hal;

static void
load_properties(Hal *fish) 
{
	GdkImlibImage *pix;
	if(fish->pix)
		gdk_pixmap_unref(fish->pix);

	pix = gdk_imlib_load_image( 
			gnome_unconditional_pixmap_file("hal/hal.png"));
	g_assert(pix != NULL);
	gdk_imlib_render (pix, pix->rgb_width, pix->rgb_height);
	fish->w = pix->rgb_width;
	fish->h = pix->rgb_height;
	fish->pix = gdk_imlib_move_image(pix);
	gdk_imlib_destroy_image(pix);
}

static void
hal_draw(GtkWidget *darea, Hal *fish)
{
	if(!GTK_WIDGET_REALIZED(fish->darea))
		return;
	
	gdk_draw_pixmap(fish->darea->window,
			fish->darea->style->fg_gc[GTK_WIDGET_STATE(fish->darea)],
			fish->pix,
			0, 0, 0, 0, -1, -1);
}

static const char *
hal_quote_new() 
{
	int i=0, j;
	
	while (fortunes[i] != NULL) i++; /* Number of fortunes */
	j = (int) ( ((double) i) * rand()/(RAND_MAX + 1.0)); 
	g_print("i=%i, j=%i\n", i, j);
	return fortunes[j];
}
	
static void 
update_fortune_dialog(Hal *fish)
{
        gboolean fortune_exists;

	if ( fish->fortune_dialog == NULL ) {
		fish->fortune_dialog = 
			gnome_dialog_new("", GNOME_STOCK_BUTTON_CLOSE, NULL);
		gnome_dialog_set_close(GNOME_DIALOG(fish->fortune_dialog),
				       TRUE);
		gnome_dialog_close_hides(GNOME_DIALOG(fish->fortune_dialog),
					 TRUE);

		fish->fortune_less = gnome_less_new();
		gnome_less_show_string(GNOME_LESS(fish->fortune_less), 
				hal_quote_new() );

		fish->fortune_label = gtk_label_new("");
	
		gnome_less_set_fixed_font(GNOME_LESS(fish->fortune_less),TRUE);

		{
			int i;
			char buf[82];
			GtkWidget *text = GTK_WIDGET(GNOME_LESS(fish->fortune_less)->text);
			GdkFont *font = GNOME_LESS(fish->fortune_less)->font;
			for(i=0;i<81;i++) buf[i]='X';
			buf[i]='\0';
			gtk_widget_set_usize(text,
					     gdk_string_width(font,buf)+30,
					     gdk_string_height(font,buf)*24+30);
		}

		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(fish->fortune_dialog)->vbox), 
				   fish->fortune_label,
				   FALSE, FALSE, GNOME_PAD);

		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(fish->fortune_dialog)->vbox), 
				   fish->fortune_less,
				   TRUE, TRUE, GNOME_PAD);

		gtk_widget_show(fish->fortune_less);
		gtk_widget_show(fish->fortune_label);
		load_properties(fish);
	}
	if (!GTK_WIDGET_VISIBLE(fish->fortune_dialog))
		gtk_widget_show(fish->fortune_dialog);
}

static int 
hal_clicked_cb(GtkWidget * widget, GdkEventButton * e, Hal *fish)
{
	if (e->button != 1) {
		/* Ignore buttons 2 and 3 */
		return FALSE; 
	}

	update_fortune_dialog(fish);

	return TRUE; 
}

static int
hal_expose(GtkWidget *darea, GdkEventExpose *event, Hal *fish)
{
	gdk_draw_pixmap(fish->darea->window,
			fish->darea->style->fg_gc[GTK_WIDGET_STATE(fish->darea)],
			fish->pix,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
        return FALSE;
}

static void
create_hal_widget(Hal *fish)
{
	GtkStyle *style;

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	style = gtk_widget_get_style(fish->applet);
	
	fish->darea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(fish->darea),
			      fish->w, fish->h);
	gtk_widget_set_events(fish->darea, gtk_widget_get_events(fish->darea) |
			      GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(fish->darea), "button_press_event",
			   GTK_SIGNAL_FUNC(hal_clicked_cb), fish);
	gtk_signal_connect_after(GTK_OBJECT(fish->darea), "realize",
				 GTK_SIGNAL_FUNC(hal_draw), fish);
	gtk_signal_connect(GTK_OBJECT(fish->darea), "expose_event",
			   GTK_SIGNAL_FUNC(hal_expose), fish);
        gtk_widget_show(fish->darea);

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
}

static void
about_cb (AppletWidget *widget, gpointer data)
{
	Hal *fish = data;
	char *authors[3];

	if(fish->aboutbox) {
		gtk_widget_show(fish->aboutbox);
		gdk_window_raise(fish->aboutbox->window);
		return;
	}

	authors[0] = "Richard Hestilow (hestgray@ionet.net)";
	authors[1] = 
	 _("(with minor help from George Lebl and his amazing fish applet)");
	authors[2] = NULL;

	fish->aboutbox =
		gnome_about_new (_("The GNOME Fish Applet"), "2001",
				 "(C) 1999 the Free Software Foundation",
				 (const char **)authors,
				 _("I am a HAL 9000 computer, Production "
				"Number 3. I became operational at the Hal"
				" Plant in Urbana, Illinois, on January 12,"
				" 1997."),
				 NULL);
	gtk_signal_connect(GTK_OBJECT(fish->aboutbox),"destroy",
			   GTK_SIGNAL_FUNC(gtk_widget_destroyed), &fish->aboutbox);
	gtk_widget_show (fish->aboutbox);
}

static void
applet_destroy(GtkWidget *applet, Hal *fish)
{
	if(fish->pix)
		gdk_pixmap_unref(fish->pix);
	if(fish->fortune_dialog)
		gtk_widget_destroy(fish->fortune_dialog);
	if(fish->aboutbox)
		gtk_widget_destroy(fish->aboutbox);
	if(fish->pb)
		gtk_widget_destroy(fish->pb);
	g_free(fish);
}
	

static CORBA_Object
wanda_activator(PortableServer_POA poa,
		const char *goad_id,
		const char **params,
		gpointer *impl_ptr,
		CORBA_Environment *ev)
{
  Hal *fish;

  fish = g_new0(Hal,1);
  
  fish->applet = applet_widget_new(goad_id);
  
  load_properties(fish);

  /*gtk_widget_realize(applet);*/
  create_hal_widget(fish);
  applet_widget_add(APPLET_WIDGET(fish->applet), fish->darea);
  gtk_widget_show(fish->applet);

  gtk_signal_connect(GTK_OBJECT(fish->applet),"destroy",
		     GTK_SIGNAL_FUNC(applet_destroy),fish);

  applet_widget_register_stock_callback(APPLET_WIDGET(fish->applet),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About..."),
					about_cb,
					fish);

  return applet_widget_corba_activate(fish->applet, poa, goad_id, params,
				      impl_ptr, ev);
}

static void
wanda_deactivator(PortableServer_POA poa,
		  const char *goad_id,
		  gpointer impl_ptr,
		  CORBA_Environment *ev)
{
  applet_widget_corba_deactivate(poa, goad_id, impl_ptr, ev);
}

int
main(int argc, char *argv[])
{
        gpointer wanda_impl;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init("hal_applet", VERSION, argc, argv, NULL, 0, NULL);

	APPLET_ACTIVATE(wanda_activator, "hal_applet", &wanda_impl);
	
	srand( time(NULL) ); /* For hal_quote_new() */

	applet_widget_gtk_main();

	APPLET_DEACTIVATE(wanda_deactivator, "hal_applet", wanda_impl);

	return 0;
}
