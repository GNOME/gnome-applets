#include <gnome.h>
#include <ghttp.h>
#include "applet-lib.h"
#include "applet-widget.h"
#include "bussign.h"

/* globals give me the willies. */

static GdkImlibImage *sg_bus = NULL;
static GtkWidget     *sg_pixmap = NULL;

/* some prototypes */

static GtkWidget *
create_bussign_widget(GtkWidget *a_parent);

static int
refresh_imagefile(void);

static void
bussign_refresh_widget_dummy(AppletWidget *a_applet, gpointer a_data);

static int
bussign_refresh(gpointer data);

static gint
destroy_applet(GtkWidget *widget, gpointer data);

static void
about_window(AppletWidget *a_widget, gpointer a_data);

int main(int argc, char **argv)
{
  GtkWidget *l_bussign = NULL;
  GtkWidget *l_applet = NULL;
  
  /* set up the usual stuff */
  applet_widget_init_defaults("bussign_applet", NULL, argc, argv, 0, NULL,
			      argv[0]);
  l_applet = applet_widget_new();
  if (!l_applet)
    g_error("Can't create applet!\n");

  gtk_widget_realize(l_applet);

  /* set up the bussign widget */
  l_bussign = create_bussign_widget(l_applet);
  gtk_widget_show(l_bussign);

  /* add it */
  applet_widget_add(APPLET_WIDGET(l_applet), l_bussign);
  gtk_widget_show(l_applet);

  /* attach the about window */
  applet_widget_register_stock_callback(APPLET_WIDGET(l_applet),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About..."),
					about_window,
					NULL);

  /* attach a refresh button */
  applet_widget_register_stock_callback(APPLET_WIDGET(l_applet),
					"refresh",
					GNOME_STOCK_MENU_REFRESH,
					_("Refresh Image"),
					bussign_refresh_widget_dummy,
					NULL);

  /* do it. */
  applet_widget_gtk_main();
  return 0;
}

static void
bussign_refresh_widget_dummy(AppletWidget *a_applet, gpointer a_data)
{
  bussign_refresh(NULL);
}

static GtkWidget *
create_bussign_widget(GtkWidget *a_parent)
{
  GtkWidget              *l_frame = NULL;
  GtkStyle               *l_style = NULL;

  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  l_style = gtk_widget_get_style(a_parent);
  
  /* refresh the image */
  if (refresh_imagefile() < 0)
    {
      fprintf(stderr, "Failed to refresh image: %s\n", strerror(errno));
      exit(1);
    }
    
  /* load the file */
  sg_bus = gdk_imlib_load_image(IMAGE_FILENAME);
  /* render it */
  gdk_imlib_render(sg_bus, sg_bus->rgb_width, sg_bus->rgb_height);
  /* get the pixmap */
  sg_pixmap = gtk_pixmap_new(sg_bus->pixmap, sg_bus->shape_mask);
  /* show it */
  gtk_widget_show(sg_pixmap);

  /* set up the timeout to refresh */
  gtk_timeout_add(20000, bussign_refresh, NULL);
  
  /* set the frame up */
  l_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(l_frame), GTK_SHADOW_IN);
  /* add the pixmap to the frame */
  gtk_container_add(GTK_CONTAINER(l_frame), sg_pixmap);
  gtk_widget_pop_colormap();
  gtk_widget_pop_visual();
  return l_frame;
}

static int
refresh_imagefile(void)
{
  int   l_return = 0;
  FILE *l_file = NULL;
  ghttp_request *l_req = NULL;
  
  l_req = ghttp_request_new();
  if (ghttp_set_uri(l_req, "http://www1.netscape.com/fishcam/livefishcamsmall.cgi?livesigncamsmall") < 0)
    {
      l_return = -1;
      goto ec;
    }
  if (ghttp_prepare(l_req) < 0)
    {
      l_return = -1;
      goto ec;
    }
  ghttp_set_header(l_req, http_hdr_Connection, "close");
  if (ghttp_process(l_req) < 0)
    {
      l_return = -1;
      goto ec;
    }
  l_file = fopen(IMAGE_FILENAME, "w+");
  if (l_file == NULL)
    {
      l_return = -1;
      goto ec;
    }
  if (fwrite(ghttp_get_body(l_req),
	     ghttp_get_body_len(l_req), 
	     1,
	     l_file) == 0)
    {
      l_return = -1;
      goto ec;
    }
  fclose(l_file);
  ghttp_request_destroy(l_req);
 ec:
  return l_return;
}

static int
bussign_refresh(gpointer data)
{
  if (refresh_imagefile() < 0)
    {
      fprintf(stderr, "Failed to refresh image: %s\n", strerror(errno));
      goto ec;
    }
  /* kill the image and flush it */
  if (sg_bus)
    {
      gdk_imlib_kill_image(sg_bus);
      gdk_imlib_changed_image(sg_bus);
    }
  /* reload the image */
  if ((sg_bus = gdk_imlib_load_image(IMAGE_FILENAME)) != NULL)
    {
      /* render it */
      gdk_imlib_render(sg_bus, sg_bus->rgb_width, sg_bus->rgb_height);
      /* set the pixmap */
      gtk_pixmap_set(GTK_PIXMAP(sg_pixmap), sg_bus->pixmap, sg_bus->shape_mask);
      /* redraw that sucker. */
      gtk_widget_queue_draw(sg_pixmap);
    }
 ec:
  return TRUE;
}

static void
about_window(AppletWidget *a_widget, gpointer a_data)
{
  GtkWidget *l_about = NULL;
  gchar *l_author[2];
  
  l_author[0] = "Christopher Blizzard";
  l_author[1] = NULL;

  l_about = gnome_about_new ( _("The Bus Sign Applet"), "1.0",
			      _("(c) 1998 the Free Software Foundation"),
			      l_author,
			      _("This applet is a total waste of time. "
				"Get back to work!\n\n"
				"To fill in the sign please see:\n\n"
				"http://people.netscape.com/mtoy/sign/index.html"),
			      NULL);
  gtk_widget_show(l_about);
  return;
}
