#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"
#include "bussign.h"

static const bussign_properties_t sg_properties_defaults =
{
  "www1.netscape.com",
  "/fishcam/livefishcamsmall.cgi?livesigncamsmall",
  80,
  ".bussign_image"
};

static bussign_properties_t sg_properties =
{
  NULL,
  NULL,
  0,
  NULL
};

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

static void
properties_window(AppletWidget *a_applet, gpointer a_data);

void
load_properties(char *a_path, bussign_properties_t *a_properties);

void
save_properties(char *a_path, bussign_properties_t *a_properties);

static void
bussign_apply_properties(GnomePropertyBox *a_property_box, gint a_page, gpointer a_data);

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

  load_properties(APPLET_WIDGET(l_applet)->cfgpath, &sg_properties);

  gtk_widget_realize(l_applet);

  /* set up the bussign widget */
  l_bussign = create_bussign_widget(l_applet);
  gtk_widget_show(l_bussign);

  /* add it */
  applet_widget_add(APPLET_WIDGET(l_applet), l_bussign);
  gtk_widget_show(l_applet);

  /* attach the about window */
  applet_widget_register_callback(APPLET_WIDGET(l_applet),
				  "about",
				  _("About..."),
				  about_window,
				  NULL);

  /* attach the properties button */
  /*
  applet_widget_register_callback(APPLET_WIDGET(l_applet),
				  "properties",
				  _("Properties..."),
				  properties_window,
				  NULL);
  */

  /* attach a refresh button */
  applet_widget_register_callback(APPLET_WIDGET(l_applet),
				  "refresh",
				  _("Refresh Image"),
				  bussign_refresh_widget_dummy,
				  NULL);

  /* do it. */
  applet_widget_gtk_main();
  return 0;
}

static void
bussign_apply_changes(GnomePropertyBox *a_property_box, gint a_page, gpointer a_data)
{

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
  
  if ((l_file = fopen(IMAGE_FILENAME, "w+")) == NULL)
    {
      fprintf(stderr, "Failed to open file \"%s\": %s\n", IMAGE_FILENAME, strerror(errno));
      l_return = -1;
      goto ec;
    }
  if (http_get_to_file("www1.netscape.com", 80, "/fishcam/livefishcamsmall.cgi?livesigncamsmall",
		       l_file) < 1)
    {
      l_return = -1;
      goto ec;
    }
  fclose(l_file);
  /*
    l_return = system("wget -q http://www1.netscape.com/fishcam/livefishcamsmall.cgi?livesigncamsmall");
  */
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

void
load_properties(char *a_path, bussign_properties_t *a_properties)
{
  gnome_config_push_prefix(a_path);
  sg_properties.resource = gnome_config_get_string("resource=/fishcam/livefishcamsmall.cgi?livesigncamsmal");
  sg_properties.host     = gnome_config_get_string("host=www1.netscape.com");
  sg_properties.port     = gnome_config_get_int("port=80");
  sg_properties.file     = gnome_config_get_string("file=.bussign_image");
  gnome_config_pop_prefix();
}

void
save_properties(char *a_path, bussign_properties_t *a_properties)
{
  gnome_config_push_prefix(a_path);
  gnome_config_set_string("resource", sg_properties.resource);
  gnome_config_set_string("host", sg_properties.host);
  gnome_config_set_int("port", sg_properties.port);
  gnome_config_set_string("file", sg_properties.file);
  gnome_config_pop_prefix();
  gnome_config_sync();
  gnome_config_drop_all();
}

static void
properties_window(AppletWidget *a_applet, gpointer a_data)
{
  GtkWidget *l_property_box = NULL;
  GtkWidget *l_label = NULL;
  GtkWidget *l_vbox = NULL;
  GtkWidget *l_hbox = NULL;
  GtkWidget *l_entry = NULL;
  char       l_port[20] = "";

  l_property_box = gnome_property_box_new();
  gtk_window_set_title(GTK_WINDOW(l_property_box), "Bussign Settings");
  l_vbox = gtk_vbox_new(GNOME_PAD, FALSE);

  /* begin host entry */
  l_hbox = gtk_hbox_new(GNOME_PAD, FALSE);
  gtk_container_border_width(GTK_CONTAINER(l_hbox), GNOME_PAD);
  l_label = gtk_label_new("Host:");
  l_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(l_entry), sg_properties.host);
  gtk_signal_connect_object(GTK_OBJECT(l_entry), "changed",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(l_property_box));
  gtk_box_pack_start(GTK_BOX(l_hbox), l_label, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_hbox), l_entry, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_vbox), l_hbox, FALSE, FALSE, GNOME_PAD);

  /* begin the resource entry */
  l_hbox = gtk_hbox_new(GNOME_PAD, FALSE);
  gtk_container_border_width(GTK_CONTAINER(l_hbox), GNOME_PAD);
  l_label = gtk_label_new("Resource:");
  l_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(l_entry), sg_properties.resource);
  gtk_signal_connect_object(GTK_OBJECT(l_entry), "changed",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(l_property_box));
  gtk_box_pack_start(GTK_BOX(l_hbox), l_label, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_hbox), l_entry, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_vbox), l_hbox, FALSE, FALSE, GNOME_PAD);

  /* begin the port entry */
  l_hbox = gtk_hbox_new(GNOME_PAD, FALSE);
  gtk_container_border_width(GTK_CONTAINER(l_hbox), GNOME_PAD);
  l_label = gtk_label_new("Port:");
  l_entry = gtk_entry_new();
  sprintf(l_port, "%d", sg_properties.port);
  gtk_entry_set_text(GTK_ENTRY(l_entry), l_port);
  gtk_signal_connect_object(GTK_OBJECT(l_entry), "changed",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(l_property_box));
  gtk_box_pack_start(GTK_BOX(l_hbox), l_label, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_hbox), l_entry, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_vbox), l_hbox, FALSE, FALSE, GNOME_PAD);

  /* begin the filename entry */
  l_hbox = gtk_hbox_new(GNOME_PAD, FALSE);
  gtk_container_border_width(GTK_CONTAINER(l_hbox), GNOME_PAD);
  l_label = gtk_label_new("File:");
  l_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(l_entry), sg_properties.file);
  gtk_signal_connect_object(GTK_OBJECT(l_entry), "changed",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(l_property_box));
  gtk_box_pack_start(GTK_BOX(l_hbox), l_label, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_hbox), l_entry, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(l_vbox), l_hbox, FALSE, FALSE, GNOME_PAD);

  gnome_property_box_append_page(GNOME_PROPERTY_BOX(l_property_box), l_vbox,
				 gtk_label_new("Properties"));
  gtk_signal_connect(GTK_OBJECT(l_property_box), "apply",
		     GTK_SIGNAL_FUNC(bussign_apply_changes),
		     l_entry);
  
  gtk_widget_show_all(l_property_box);

}
