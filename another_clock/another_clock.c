/* GNOME (analog) clock applet
 * (C) 1999 Iñigo Serna
 *
 * Author: Iñigo Serna
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 
 * 02139, USA.
 *
 */


#include <math.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>


#define APPLET_NAME	"another_clock_applet"
#define CLOCK_XPM_FILE	"another_clock.xpm"
#define APPLET_WIDTH	41
#define APPLET_HEIGHT	41
#define CX		20
#define CY		20
#define NEEDLE_SIZE	12



struct clock_props_t {
   gboolean	sec;
   gushort	red[4], blue[4], green[4];	/* bg, hour, min, sec */
};

struct clock_applet_t{
   GtkWidget		*area;
   GdkPixmap		*xpm;
   GdkPixmap		*clock_xpm;
   GdkBitmap		*clock_mask;
   GdkGC		*gc[4];		/* bg, hour, min, sec */
   struct clock_props_t	props;
};


GtkWidget		*applet;
static GtkWidget 	*props_window = NULL;
struct clock_applet_t 	clk;
struct clock_props_t	props_tmp;


/**************
 * Prototypes *
 **************/
static gint update_clock (gpointer data);
void set_colors (GtkWidget *widget);



/**************
 * Properties *
 **************/
void properties_load (char *path)
{
    gnome_config_push_prefix (path);
    clk.props.sec	= gnome_config_get_bool_with_default ("another_clock/sec_needle=TRUE", NULL);
    clk.props.red[0]	= gnome_config_get_int_with_default ("another_clock/background_red=37888", NULL);
    clk.props.green[0]	= gnome_config_get_int_with_default ("another_clock/background_green=0", NULL);
    clk.props.blue[0]	= gnome_config_get_int_with_default ("another_clock/background_blue=54016", NULL);
    clk.props.red[1]	= gnome_config_get_int_with_default ("another_clock/hourneedle_red=65535", NULL);
    clk.props.green[1]	= gnome_config_get_int_with_default ("another_clock/hourneedle_green=65535", NULL);
    clk.props.blue[1]	= gnome_config_get_int_with_default ("another_clock/hourneedle_blue=65535", NULL);
    clk.props.red[2]	= gnome_config_get_int_with_default ("another_clock/minneedle_red=65535", NULL);
    clk.props.green[2]	= gnome_config_get_int_with_default ("another_clock/minneedle_green=65535", NULL);
    clk.props.blue[2]	= gnome_config_get_int_with_default ("another_clock/minneedle_blue=65535", NULL);
    clk.props.red[3]	= gnome_config_get_int_with_default ("another_clock/secneedle_red=65280", NULL);
    clk.props.green[3]	= gnome_config_get_int_with_default ("another_clock/secneedle_green=25344", NULL);
    clk.props.blue[3]	= gnome_config_get_int_with_default ("another_clock/secneedle_blue=18176", NULL);
    gnome_config_pop_prefix();
}


int properties_save (char *path)
{
    gnome_config_push_prefix (path);
    gnome_config_set_bool ("another_clock/sec_needle", clk.props.sec);
    gnome_config_set_int ("another_clock/background_red", clk.props.red[0]);
    gnome_config_set_int ("another_clock/background_green", clk.props.green[0]);
    gnome_config_set_int ("another_clock/background_blue", clk.props.blue[0]);
    gnome_config_set_int ("another_clock/hourneedle_red", clk.props.red[1]);
    gnome_config_set_int ("another_clock/hourneedle_green", clk.props.green[1]);
    gnome_config_set_int ("another_clock/hourneedle_blue", clk.props.blue[1]);
    gnome_config_set_int ("another_clock/minneedle_red", clk.props.red[2]);
    gnome_config_set_int ("another_clock/minneedle_green", clk.props.green[2]);
    gnome_config_set_int ("another_clock/minneedle_blue", clk.props.blue[2]);
    gnome_config_set_int ("another_clock/secneedle_red", clk.props.red[3]);
    gnome_config_set_int ("another_clock/secneedle_green", clk.props.green[3]);
    gnome_config_set_int ("another_clock/secneedle_blue", clk.props.blue[3]);
    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();

    return FALSE;
}


void props_ok (GtkWidget *wid, int page, gpointer *data)
{
    int i;
    
    clk.props.sec = props_tmp.sec;
    for (i = 0; i < 4; i++)
    {
	clk.props.red[i] = props_tmp.red[i];
	clk.props.green[i] = props_tmp.green[i];
	clk.props.blue[i] = props_tmp.blue[i];
    } 
    applet_widget_sync_config (APPLET_WIDGET(applet));
    set_colors(clk.area);
    update_clock (NULL);
}


void props_cancel (GtkWidget *widget, GtkWidget **win)
{
    *win = NULL;
}


void bg_color_changed (GnomeColorPicker *cp)
{
    gushort	a;
    
    gnome_color_picker_get_i16 (cp,
				&props_tmp.red[0], &props_tmp.green[0],
				&props_tmp.blue[0], &a);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


void hour_color_changed (GnomeColorPicker *cp)
{
    gushort	a;

    gnome_color_picker_get_i16 (cp,
				&props_tmp.red[1], &props_tmp.green[1],
				&props_tmp.blue[1], &a);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


void min_color_changed (GnomeColorPicker *cp)
{
    gushort	a;

    gnome_color_picker_get_i16 (cp,
				&props_tmp.red[2], &props_tmp.green[2],
				&props_tmp.blue[2], &a);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


void sec_color_changed (GnomeColorPicker *cp)
{
    gushort	a;

    gnome_color_picker_get_i16 (cp,
				&props_tmp.red[3], &props_tmp.green[3],
				&props_tmp.blue[3], &a);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


void sec_needle_changed (GtkWidget *widget, GtkWidget **sec)
{
    props_tmp.sec = props_tmp.sec ? FALSE : TRUE;
    gtk_widget_set_sensitive (GTK_WIDGET(sec), props_tmp.sec);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


/*************
 * Callbacks *
 *************/
void cb_properties (AppletWidget *applet, gpointer data)
{
    static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
    GtkWidget *label;
    GtkWidget *page;
    GtkWidget *frame;
    GtkWidget *table;
    GtkWidget *sec_checkbox;
    GtkWidget *colorpicker;
    int	      i;

    /* init temporal properties */
    props_tmp.sec = clk.props.sec;
    for (i = 0; i < 4; i++)
    {
	props_tmp.red[i] = clk.props.red[i];
	props_tmp.green[i] = clk.props.green[i];
	props_tmp.blue[i] = clk.props.blue[i];
    } 


    help_entry.name = gnome_app_id;

    if (props_window)
    {
       gdk_window_raise (props_window->window);
       return;
    }
																			 
    /* Window and frame for settings */
    props_window = gnome_property_box_new ();
    gtk_window_set_title (GTK_WINDOW(&GNOME_PROPERTY_BOX(props_window)->dialog.window),
                	  _("Clock Settings"));

    gtk_signal_connect (GTK_OBJECT (props_window), "apply",
                	GTK_SIGNAL_FUNC(props_ok), NULL);
    gtk_signal_connect (GTK_OBJECT (props_window), "destroy",
                        GTK_SIGNAL_FUNC(props_cancel), &props_window);
    gtk_signal_connect (GTK_OBJECT (props_window), "help",
                        GTK_SIGNAL_FUNC(gnome_help_pbox_display),
			&help_entry);

    label = gtk_label_new (_("General"));
    gtk_widget_show (label);
    page = gtk_vbox_new (FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER(page), GNOME_PAD);
    gtk_widget_show (page);
    gnome_property_box_append_page (GNOME_PROPERTY_BOX(props_window),
				    page, label);


    /* frame for colors */
    frame = gtk_frame_new (_("Colors"));
    gtk_box_pack_start (GTK_BOX(page), frame, FALSE, FALSE, 0);
    gtk_widget_show (frame);
    table = gtk_table_new (4, 2, FALSE);
    gtk_container_set_border_width (GTK_CONTAINER(table), GNOME_PAD_SMALL);
    gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
    gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
    gtk_container_add (GTK_CONTAINER(frame), table);
    gtk_widget_show (table);

    label = gtk_label_new (_("Clock color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 0, 1);
gtk_widget_set_sensitive (label, FALSE);
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(colorpicker),
			        props_tmp.red[0], props_tmp.green[0],
			        props_tmp.blue[0], 65535);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 1, 2, 0, 1);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(bg_color_changed), NULL);
gtk_widget_set_sensitive (colorpicker, FALSE);
    gtk_widget_show (colorpicker);

    label = gtk_label_new (_("Hour needle color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 2, 3, 0, 1);
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(colorpicker),
			        props_tmp.red[1], props_tmp.green[1],
			        props_tmp.blue[1], 65535);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 3, 4, 0, 1);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(hour_color_changed), NULL);
    gtk_widget_show (colorpicker);

    label = gtk_label_new (_("Minute needle color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 1, 2);
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(colorpicker),
			        props_tmp.red[2], props_tmp.green[2],
			        props_tmp.blue[2], 65535);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 1, 2, 1, 2);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(min_color_changed), NULL);
    gtk_widget_show (colorpicker);

    label = gtk_label_new (_("Second needle color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 2, 3, 1, 2);
/*    gtk_widget_set_sensitive (label, props_tmp.sec);*/
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(colorpicker),
			        props_tmp.red[3], props_tmp.green[3],
			        props_tmp.blue[3], 65535);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 3, 4, 1, 2);
    gtk_widget_set_sensitive (colorpicker, props_tmp.sec);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(sec_color_changed), NULL);
    gtk_widget_show (colorpicker);
    

    /* second needle visible? */
    sec_checkbox = gtk_check_button_new_with_label (_("Show seconds needle"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(sec_checkbox),
				  clk.props.sec);
    gtk_signal_connect (GTK_OBJECT(sec_checkbox), "toggled",
			GTK_SIGNAL_FUNC(sec_needle_changed), colorpicker);
    gtk_box_pack_start (GTK_BOX(page), sec_checkbox, FALSE, FALSE, 0);
    gtk_widget_show (sec_checkbox);



    gtk_widget_show (props_window);

    return;
}


void cb_about (AppletWidget *applet, gpointer data)
{
    GtkWidget *about;
    static const gchar *authors[] = {
			    "Iñigo Serna <inigo@gaztelan.bi.ehu.es>",
			    NULL
    };

    about = gnome_about_new (_("Another Clock Applet"), "0.1",
			     "(C) 1999 the Free Software Foundation",
                    	     authors,
                    	     _("An analog clock similar to that in CDE panel."),
                    	     NULL);

    gtk_widget_show (about);

    return;
}


static gint save_session (GtkWidget *widget, char *privcfgpath,
			  char *globcfgpath)
{
    properties_save (privcfgpath);
    return FALSE;
}


/**********
 * Colors *
 **********/
void set_gc_color(GdkColorContext *cc, int n)
{
    GdkColor        *c;
    gint	    z;

    c = g_new (GdkColor, 1);
    c->red = clk.props.red[n];
    c->green = clk.props.green[n];
    c->blue = clk.props.blue[n];
    c->pixel = (gulong) 0;
    z = 0;
    gdk_color_context_get_pixels (cc, &c->red, &c->green, &c->blue, 1,
	                          &c->pixel, &z);
    if (n == 0)
    {
       /* Next doesn't work! Why????????? */
/*       gdk_window_set_background (clk.clock_xpm, c);*/
    }
    else
    {
       gdk_gc_set_foreground (clk.gc[n], c);
       gdk_gc_set_line_attributes (clk.gc[n],
				   (n == 1) ? 2 : 1,
				   GDK_LINE_SOLID,
	                	   GDK_CAP_ROUND,
				   GDK_JOIN_ROUND);
    }
    g_free (c);
}


void set_colors (GtkWidget *widget)
{
    GdkColorContext *cc;

    cc = gdk_color_context_new (gtk_widget_get_visual(widget),
                                gtk_widget_get_colormap(widget));
    clk.gc[0] = gdk_gc_new (widget->window);
    clk.gc[1] = gdk_gc_new (widget->window);
    clk.gc[2] = gdk_gc_new (widget->window);
    clk.gc[3] = gdk_gc_new (widget->window);
    set_gc_color (cc, 0);
    set_gc_color (cc, 1);
    set_gc_color (cc, 2);
    set_gc_color (cc, 3);
    g_free (cc);
}


/****************
 * update_clock *
 ****************/
static gint update_clock (gpointer data)
{
    time_t	curtime;
    struct tm 	*tm;
    double	ang;

    GdkRectangle r;

    /* Get hour, minute and second */    
    curtime = time (NULL);
    tm = localtime (&curtime);

    /* draw clock */
    gdk_draw_pixmap (clk.xpm,
	             clk.gc[0],
                     clk.clock_xpm,
                     0, 0, 0, 0,
		     APPLET_WIDTH, APPLET_HEIGHT);

    /* draw needles */
    ang = ((tm->tm_hour > 12) ? tm->tm_hour-12 : tm->tm_hour) * M_PI / 6;
    ang += tm->tm_min * M_PI / 360;
    gdk_draw_line (clk.xpm, clk.gc[1], CX, CY,
		   (int) (CX + (NEEDLE_SIZE-3) * sin(ang)),
		   (int) (CY - (NEEDLE_SIZE-3) * cos(ang)));
    ang = tm->tm_min * M_PI / 30;
    gdk_draw_line (clk.xpm, clk.gc[2], CX, CY,
		   (int) (CX + NEEDLE_SIZE * sin(ang)),
		   (int) (CY - NEEDLE_SIZE * cos(ang)));
    if (clk.props.sec)
    {
       ang = tm->tm_sec * M_PI / 30;
       gdk_draw_line (clk.xpm, clk.gc[3], CX, CY,
		      (int) (CX + NEEDLE_SIZE * sin(ang)),
		      (int) (CY - NEEDLE_SIZE * cos(ang)));
    }

    r.x = 0;
    r.y = 0;
    r.width = APPLET_WIDTH;
    r.height = APPLET_HEIGHT;

    gdk_window_set_back_pixmap (clk.area->window, clk.xpm, FALSE);
    gdk_window_clear (clk.area->window);
    gtk_widget_draw (clk.area, &r);

    return TRUE;
}



/*****************
 * Main function *
 *****************/
int main (int argc, char *argv[])
{
    char	*tmp, *fname;

    /* Initialize the i18n stuff */
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    /* create the main window, and attach delete_event signal to terminating
       the application */
    applet_widget_init (APPLET_NAME, VERSION, argc, argv, NULL, 0, NULL);
    applet = applet_widget_new (APPLET_NAME);
    if (!applet)
       g_error ("Can't create another_clock applet");

    /* create drawing area for clock pixmap */
    clk.area = gtk_drawing_area_new();
    gtk_drawing_area_size (GTK_DRAWING_AREA(clk.area),
			   APPLET_WIDTH, APPLET_HEIGHT);
    gtk_widget_show (clk.area);
    gtk_widget_set_usize (clk.area, APPLET_WIDTH, APPLET_HEIGHT);
    applet_widget_add (APPLET_WIDGET(applet), clk.area);
    gtk_widget_realize (clk.area);

    /* load clock pixmap  */
    fname = gnome_unconditional_pixmap_file (CLOCK_XPM_FILE);
    if (!fname)
	g_error ("Can't find another_clock applet pixmap");
    clk.clock_xpm = gdk_pixmap_create_from_xpm (clk.area->window,
						&clk.clock_mask,
                                                &clk.area->style->bg[GTK_STATE_NORMAL],
                                                fname);
    
    /* create a pixmap for clock */
    clk.xpm = gdk_pixmap_new (clk.area->window, APPLET_WIDTH, APPLET_HEIGHT, -1);

    /* load background and needles' colors */
    properties_load (APPLET_WIDGET(applet)->privcfgpath);
    set_colors (clk.area);

    /* show applet */
    gtk_widget_show(applet);

    /* callback for updating the time */
    gtk_timeout_add (100, update_clock, &clock);
    update_clock(&clock);

    /* callbacks for session, about, properties, etc. */
    gtk_signal_connect (GTK_OBJECT(applet), "save_session",
			GTK_SIGNAL_FUNC(save_session), NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Properties..."),
					   cb_properties,
					   NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "about",
					   GNOME_STOCK_MENU_ABOUT,
					   _("About..."),
					   cb_about,
					   NULL);

    /* applet main loop */
    applet_widget_gtk_main ();
          
    return 0;
}
