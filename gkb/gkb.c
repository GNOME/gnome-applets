#include <config.h>
#include <string.h>
#include <gnome.h>
#include <gdk_imlib.h>
#include <applet-widget.h>

typedef struct _gkb_properties gkb_properties;

struct _gkb_properties {
	char *command;
	char *image[2];
        char *dmap[2];
	int  curpix;
};

static gkb_properties defaults = {
	"xmodmap",
	{ NULL, NULL },
        { "us", "hu" },
	0
};

void do_that_command(void);
void about_cb (AppletWidget *, gpointer);

static gkb_properties properties = { NULL };

static GtkWidget *applet;
static GtkWidget *darea;
static GdkImlibImage *pix[2]={ NULL, NULL };
static int timeout_id = 0;

static GtkWidget * fortune_dialog = NULL;
static GtkWidget * fortune_label;



static void
load_properties(char *privcfgpath)
{
	char buf[256];
	gnome_config_push_prefix(privcfgpath);
	g_free(properties.command);
	g_snprintf(buf,256,"gkb/name=%s",defaults.command);
	properties.command = gnome_config_get_string(buf);
	g_free(properties.image[0]);
	g_snprintf(buf,256,"gkb/image0=%s",defaults.image[0]);
	properties.image[0] = gnome_config_get_string(buf);
	g_free(properties.image[1]);
	g_snprintf(buf,256,"gkb/image1=%s",defaults.image[1]);
	properties.image[1] = gnome_config_get_string(buf);
	g_free(properties.dmap[0]);
	g_snprintf(buf,256,"gkb/map0=%s",defaults.dmap[0]);
	properties.dmap[0] = gnome_config_get_string(buf);
	g_free(properties.dmap[1]);
	g_snprintf(buf,256,"gkb/map1=%s",defaults.dmap[1]);
	properties.dmap[1] = gnome_config_get_string(buf);
	gnome_config_pop_prefix();

	if(pix[0])
		gdk_imlib_destroy_image(pix[0]);
	if(pix[1])
		gdk_imlib_destroy_image(pix[1]);
	
	pix[0] = gdk_imlib_load_image(properties.image[0]);
	pix[1] = gdk_imlib_load_image(properties.image[1]);
	gdk_imlib_render (pix[0], pix[0]->rgb_width, pix[0]->rgb_height);
	gdk_imlib_render (pix[1], pix[1]->rgb_width, pix[1]->rgb_height);
}

static void
gkb_draw(GtkWidget *darea)
{
	if(!GTK_WIDGET_REALIZED(darea))
		return;
	gdk_draw_pixmap(darea->window,
			darea->style->fg_gc[GTK_WIDGET_STATE(darea)],
			pix[properties.curpix]->pixmap,
			0, 0,
			0, 0,
			-1, -1);
}

/* Prop changes */

void
do_that_command(void)
{
  char comm[100];
  int len;

/*  comm = g_malloc(len=(
   strlen(properties.command)+           
   (strcmp(properties.command,"xmodmap")?11:
             strlen(gnome_datadir_file(g_copy_strings ("xmodmap/",
             "xmodmap.", properties.dmap[properties.curpix], NULL)) )+7) ) );
 */
  g_snprintf(comm, len, "%s %s%c", properties.command,
      (strcmp(properties.command,"xmodmap")?properties.dmap[properties.curpix]:
      gnome_datadir_file(g_copy_strings ("xmodmap/",
                            "xmodmap.", properties.dmap[properties.curpix], NULL)) ),0);
  system(comm);
}

static int 
gkb_cb(GtkWidget * widget, GdkEventButton * e, 
		gpointer data)
{
	if (e->button != 1) {
		/* Ignore buttons 2 and 3 */
		return FALSE; 
	}
	properties.curpix++;
	if(properties.curpix>=2) properties.curpix=0;
	gkb_draw(GTK_WIDGET(widget));

	do_that_command();

	return FALSE;
}

static int
gkb_expose(GtkWidget *darea, GdkEventExpose *event)
{
	gdk_draw_pixmap(darea->window,
			darea->style->fg_gc[GTK_WIDGET_STATE(darea)],
			pix[properties.curpix]->pixmap,
			event->area.x, event->area.y,
			event->area.x, event->area.y,
			event->area.width, event->area.height);
        return FALSE;
}

static GtkWidget *
create_gkb_widget(GtkWidget *window)
{
	GtkWidget *frame;
	GtkStyle *style;

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	style = gtk_widget_get_style(window);
	
	darea = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(darea),
			      pix[properties.curpix]->rgb_width,
			      pix[properties.curpix]->rgb_height);
	gtk_widget_set_events(darea, gtk_widget_get_events(darea) |
			      GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(darea), "button_press_event",
			   GTK_SIGNAL_FUNC(gkb_cb), NULL);
	gtk_signal_connect_after(GTK_OBJECT(darea), "realize",
			   GTK_SIGNAL_FUNC(gkb_draw), NULL);
	gtk_signal_connect(GTK_OBJECT(darea), "expose_event",
			   GTK_SIGNAL_FUNC(gkb_expose), NULL);
        gtk_widget_show(darea);

        properties.curpix = 0;

        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
        gtk_container_add(GTK_CONTAINER(frame),darea);

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
        return frame;
}

void
about_cb (AppletWidget *widget, gpointer data)
{
        GtkWidget *about;

        static const char *authors[] = { "Szabolcs Ban (Shooby) <bansz@szif.hu>", NULL };

        about = gnome_about_new (_("The GNOME KB Applet"), _("0.99"),
                        _("(C) 1998 LSC - Linux Supporting Center"),
                        (const char **)authors,
                        _("This applet used to switch between "   
                          "keyboard maps. No more. It uses "
                          "setxkbmap, or xmodmap. "
                          "The main site of this app moved "
                          "tempolary to URL http://lsc.kva.hu/gkb."
                          "Mail me your flag, please (60x40 size),"
                          "I will put it to CVS."
                          "So long, and thanks for all the fish."
                          ),
                        _("gkb.xpm"));
        gtk_widget_show (about);

        return;
}

static int
applet_save_session(GtkWidget *w,
		    const char *privcfgpath,
		    const char *globcfgpath)
{
	gnome_config_push_prefix(privcfgpath);
	gnome_config_set_string("gkb/command",properties.command);
	gnome_config_set_string("gkb/image0",properties.image[0]);
	gnome_config_set_string("gkb/image1",properties.image[1]);
	gnome_config_set_string("gkb/map0",properties.dmap[0]);
	gnome_config_set_string("gkb/map1",properties.dmap[1]);
	gnome_config_pop_prefix();

	gnome_config_sync();
	gnome_config_drop_all();

	return FALSE;
}

static CORBA_Object
gkb_activator(PortableServer_POA poa,
		const char *goad_id,
		const char **params,
		gpointer *impl_ptr,
		CORBA_Environment *ev)
{
  GtkWidget *gkb, *applet;
  
  applet = applet_widget_new(goad_id);

  load_properties(APPLET_WIDGET(applet)->privcfgpath);

  /*gtk_widget_realize(applet);*/
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


/*  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties..."),
					properties_dialog,
					NULL);
*/
  return applet_widget_corba_activate(applet, poa, goad_id, params,
				      impl_ptr, ev);
}

static void
gkb_deactivator(PortableServer_POA poa,
		  const char *goad_id,
		  gpointer impl_ptr,
		  CORBA_Environment *ev)
{
  applet_widget_corba_deactivate(poa, goad_id, impl_ptr, ev);
}

#if 1 || defined(SHLIB_APPLETS)
static const char *repo_id[]={"IDL:GNOME/Applet:1.0", NULL};
static GnomePluginObject applets_list[] = {
  {repo_id, "gkb_applet", NULL, "The GNOME keyboard switcher",
   &gkb_activator, &gkb_deactivator},
  {NULL}
};

GnomePlugin GNOME_Plugin_info = {
  applets_list, NULL
};
#else
int
main(int argc, char *argv[])
{
        gpointer gkb_impl;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init("gkb_applet", VERSION, argc, argv, NULL, 0, NULL);

	APPLET_ACTIVATE(gkb_activator, "gkb_applet", &gkb_impl);
	
	applet_widget_gtk_main();

	APPLET_DEACTIVATE(gkb_deactivator, "gkb_applet", gkb_impl);

	return 0;
}
#endif
