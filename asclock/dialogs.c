#include <gdk/gdk.h>
#include <gnome.h>
#include <applet-widget.h>
#include <stdio.h>
#include <math.h>
#include "asclock.h"

/* prototypes */
void location_selected(GtkWidget *list, gint row, gint column, GdkEventButton *event, gpointer data);


GtkWidget *pic = NULL;

void about_dialog(AppletWidget *applet, gpointer data)
{
  GtkWidget *about;
  static const gchar *authors[] = {
    "Beat Christen (beat@longstreet.ch)",
    NULL
  };

  about = gnome_about_new (_("Afterstep Clock Applet"), "1.2",
                        "(C) 1998 the Free Software Foundation",
                        authors,
                        _("Who said NeXT is dead?"),
                        NULL);

        gtk_widget_show (about);

        return;
}

static void
properties_timezone_render(GtkWidget *parent, float lat, float lon)
{
  GdkPixmap *pmap;
  GdkBitmap * mask;
  GdkGC *gc = NULL;
  char *fname;
  float yloc;
  char cmd[1024];

  fname = tempnam(NULL, "asclock_globe");

  snprintf(cmd, 1024, 
          "xearth -ppm -night 10 -size '320 320' -mag 0.95 -pos 'fixed 0 %.4f' -nostars > %s",
          lon, fname);

  system(cmd);

  gdk_imlib_load_file_to_pixmap(fname, &pmap, &mask);

  gc= gdk_gc_new(pmap);

  yloc = 160.0 - (sin((lat/180.0)*3.1415926))*160*0.95;
  
  gdk_draw_rectangle(pmap, parent->style->black_gc, 1, 158, fabs(yloc)-2, 4, 4);
  gdk_draw_rectangle(pmap, parent->style->white_gc, 1, 159, fabs(yloc)-1, 2, 2);

  if(pic) 
  {
    gtk_pixmap_set(GTK_PIXMAP(pic), pmap, mask);
    gtk_widget_draw_default(pic);
  }
  else
    pic = gtk_pixmap_new(pmap, mask);
  
  unlink(fname);
  free(fname);
}

void location_selected(GtkWidget *list, gint row, gint column, GdkEventButton *event, gpointer data)
{
  location *my;
  gchar *line;
  int lon, lat;
  gtk_clist_get_text(GTK_CLIST(list), row, 0, &line); 

  my = (location *) gtk_clist_get_row_data(GTK_CLIST(list), row);

  properties_timezone_render(list, my->lat, my->lon);

}

GtkWidget *pwin = NULL;

void properties_dialog(AppletWidget *applet, gpointer data)
{
  gchar *titles[2] = { "Continent/City" , NULL};

  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *list;
  GtkWidget *sw;
  
  if(pwin) 
  {
    gdk_window_raise(pwin->window);
    return;
  }

  pwin = gnome_property_box_new();
  
  gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(pwin)->dialog.window),
                "AfterStep Clock Settings");

  frame =  gtk_vbox_new(5, TRUE);

  label = gtk_label_new(_("Timezone"));

  properties_timezone_render(frame, 0, 0);

  hbox = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(hbox), pic, FALSE, FALSE, 5);

  gtk_widget_show(pic);


  list = gtk_clist_new_with_titles(1, titles);

  gtk_clist_set_column_width(GTK_CLIST(list), 0, 200);
  gtk_widget_set_usize(list, 260, 320);

#ifdef GTK_HAVE_FEATURES_1_1_4
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (sw), list);
  gtk_box_pack_start (GTK_BOX(hbox), sw, TRUE, TRUE, 5);
  gtk_widget_show (sw);
#else
  gtk_clist_set_policy (GTK_CLIST (list), GTK_POLICY_ALWAYS,
			GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX(hbox), list, TRUE, TRUE, 5);
#endif
  gtk_widget_show(list);
 
  gtk_box_pack_start(GTK_BOX(frame), hbox, FALSE, FALSE, 5);

  gtk_widget_show(hbox);
  
  gtk_widget_show(frame);
  gnome_property_box_append_page( GNOME_PROPERTY_BOX(pwin),frame ,label);
  
  gtk_signal_connect( GTK_OBJECT(list), "select_row", GTK_SIGNAL_FUNC(location_selected), NULL);
/*
  gtk_signal_connect( GTK_OBJECT(pwin),"apply", GTK_SIGNAL_FUNC(property_apply_cb), NULL );
  gtk_signal_connect( GTK_OBJECT(pwin),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), NULL );
*/
  gtk_widget_show_all(pwin);

  enum_timezones(list);
} 

