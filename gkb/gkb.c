/* gkb.c - The gkb keyboard map changer (kbmap frontend) for GNOME panel
 * Written by Szabolcs Ban <bansz@szif.hu>
 * Thanx for help to Neko <neko@kornel.szif.hu>
 */

#include <config.h>
#include <string.h>
#include <gnome.h>
#include <gdk_imlib.h>
#include "applet-lib.h"
#include "applet-widget.h"
#include "properties.h"

#define MAXITEM 2

gkb_properties pr;

gint curpix = 0;

GtkWidget *pixmap;

static GtkWidget *applet;

static gint 
gkb_clicked_cb(GtkWidget * widget, GdkEventButton * e, 
		gpointer data)
{
    gint len;
    
    gchar * comm;

    if (e->button != 1) { 
      return FALSE; 
    }

    curpix++; 
    if(curpix>=MAXITEM) curpix=0;

    gnome_pixmap_load_file (GNOME_PIXMAP (pixmap), pr.dfile[curpix]);

    comm = g_malloc(len=(strlen(pr.dmap[curpix])+(strcmp(pr.dcmd,"xmodmap")?11:
     strlen(gnome_datadir_file(g_copy_strings ("xmodmap/", "xmodmap.", pr.dmap[curpix], NULL)) )+8) ) );
    g_snprintf(comm, len, "%s %s", pr.dcmd ,
     (strcmp(pr.dcmd,"xmodmap")?pr.dmap[curpix]:
      gnome_datadir_file(g_copy_strings ("xmodmap/", "xmodmap.", pr.dmap[curpix], NULL)) ) );
                                                                           
    system(comm);
    free(comm);

    return FALSE;
}

static GtkWidget *
create_gkb_widget(GtkWidget *window)
{
	GtkWidget *frame;
	GtkWidget *event_box;
	GtkStyle *style;
        
        gint len;

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	style = gtk_widget_get_style(window);

        pixmap = gnome_pixmap_new_from_file_at_size ( pr.dfile[curpix], 60, 40);

        gtk_widget_show(pixmap);

	event_box = gtk_event_box_new();
	gtk_widget_show(event_box);

	gtk_widget_set_events(event_box, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(event_box), "button_press_event",
			   GTK_SIGNAL_FUNC(gkb_clicked_cb), NULL);
        curpix = 0;

        frame = gtk_frame_new(NULL);

        gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
        gtk_container_add(GTK_CONTAINER(event_box),pixmap);
        gtk_container_add(GTK_CONTAINER(frame),event_box);

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
        return frame;
}

void
about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;

        static const char *authors[] = { "Szabolcs Ban (Shooby) <bansz@szif.hu>", NULL };
	
	about = gnome_about_new (_("The GNOME KB Applet"), _("0.31"),
			_("(C) 1998 LSC - Linux Supporting Center"),
			authors,
			_("This applet used to switch between "
			  "keyboard maps. No more. It uses "
			  "setxkbmap, or xmodmap. "
			  "The main site of this app moved "
			  "tempolary to URL http://lsc.kva.hu/gkb."
			  "Mail me your flag, please (60x40 size),"
			  "I will put it to CVS."
			  ),
			_("gkb.xpm"));
	gtk_widget_show (about);

	return;
}

static gint
applet_save_session(GtkWidget *widget, char *privcfgpath,
        char *globcfgpath, gpointer data)

{
	save_properties(privcfgpath,&pr);
	return FALSE;
}

int
main(int argc, char *argv[])
{
	GtkWidget *gkb;
        gchar * comm;
        gint len;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init_defaults("gkb_applet", VERSION,
				    argc, argv, NULL, 0, NULL);

	applet = applet_widget_new("gkb_applet");

	if (!applet)
		g_error(_("Can't create applet!\n"));

        /* Load and save.. it gives the base config */

	load_properties(APPLET_WIDGET(applet)->privcfgpath,&pr);
	save_properties(APPLET_WIDGET(applet)->privcfgpath,&pr);

        /* first: set the keymap */

        curpix = 0;
        comm = g_malloc(len=(strlen(pr.dmap[curpix])+(strcmp(pr.dcmd,"xmodmap")?11:
         strlen(gnome_datadir_file(g_copy_strings ("xmodmap/", "xmodmap.", pr.dmap[curpix], NULL)) )+8) ) );
        g_snprintf(comm, len, "%s %s", pr.dcmd ,
         (strcmp(pr.dcmd,"xmodmap")?pr.dmap[curpix]:
          gnome_datadir_file(g_copy_strings ("xmodmap/", "xmodmap.", pr.dmap[curpix], NULL)) ) );

        system(comm); 
        free(comm);
        
	gtk_widget_realize(applet);

	gkb = create_gkb_widget(applet);

	gtk_widget_show(gkb);

	applet_widget_add(APPLET_WIDGET(applet), gkb);

	gtk_widget_show(applet);

	gtk_signal_connect(GTK_OBJECT(applet),"save_session",
			   GTK_SIGNAL_FUNC(applet_save_session),
			   NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb,
					      NULL);
        applet_widget_register_stock_callback(APPLET_WIDGET(applet),
	                                      "properties",
	                                    GNOME_STOCK_MENU_PROP,     
                                            _("Properties..."),
		                           properties,
	                                  NULL);
																													      
	applet_widget_gtk_main();

	return 0;
}
