/* GNOME (analog) clock applet
 * (C) 1999 Free Software Foundation
 *
 * Author: Iñigo Serna <inigo@gaztelan.bi.ehu.es>,
 *	   with some code by Miguel de Icaza <miguel@kernel.org>
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

#include <config.h>

#include <math.h>
#include <time.h>
#include <applet-widget.h>


#define APPLET_NAME	"another_clock_applet"
#define CLOCK_XPM_FILE	"another_clock.xpm"
#define APPLET_WIDTH	41
#define APPLET_HEIGHT	41
#define CX		20
#define CY		20
#define NEEDLE_SIZE	12

struct clock_props_t {
   gboolean	secneedle;
   gchar	*bg, *hour, *min, *sec;
};

struct clock_applet_t{
   GtkWidget		*area;
   GtkWidget		*clock;
   GtkWidget		*pixmap;
   GdkGC		*gc[4];		/* bg, hour, min, sec */
   struct clock_props_t	props;
};


GtkWidget		*applet;
static GtkWidget 	*props_window = NULL;
struct clock_applet_t 	clk = {0};
struct clock_props_t	props_tmp;

static int applet_width = APPLET_WIDTH;
static int applet_height = APPLET_HEIGHT;
static int cx = CX;
static int cy = CY;
static int needle_size = NEEDLE_SIZE;



/**************
 * Prototypes *
 **************/
static gint update_clock (gpointer data);
static void set_colors (GtkWidget *widget);

/****************
 * setup_sizes *
 ****************/
static void
setup_sizes (int size)
{
    applet_width = (APPLET_WIDTH*size)/48;
    applet_height = (APPLET_HEIGHT*size)/48;
    cx = (CX*size)/48;
    cy = (CY*size)/48;
    needle_size = (NEEDLE_SIZE*size)/48;
}


/**************
 * Properties *
 **************/
static void properties_load (char *path)
{
    gnome_config_push_prefix (path);
    clk.props.secneedle	= gnome_config_get_bool_with_default ("another_clock/sec_needle=TRUE", NULL);
    clk.props.bg 	= gnome_config_get_string_with_default ("another_clock/background=#68228b", NULL);
    clk.props.hour 	= gnome_config_get_string_with_default ("another_clock/hourneedle=#ffffff", NULL);
    clk.props.min 	= gnome_config_get_string_with_default ("another_clock/minneedle=#ffffff", NULL);
    clk.props.sec 	= gnome_config_get_string_with_default ("another_clock/secneedle=#ff4500", NULL);
    gnome_config_pop_prefix();
}


static int properties_save (char *path)
{
    gnome_config_push_prefix (path);
    gnome_config_set_bool ("another_clock/sec_needle", clk.props.secneedle);
    gnome_config_set_string ("another_clock/background", clk.props.bg);
    gnome_config_set_string ("another_clock/hourneedle", clk.props.hour);
    gnome_config_set_string ("another_clock/minneedle", clk.props.min);
    gnome_config_set_string ("another_clock/secneedle", clk.props.sec);
    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();

    return FALSE;
}


static void props_ok (GtkWidget *wid, int page, gpointer *data)
{
    if(page != -1)
        return;

    g_free(clk.props.bg);
    clk.props.bg = g_strdup(props_tmp.bg);
    g_free(clk.props.hour);
    clk.props.hour = g_strdup(props_tmp.hour);
    g_free(clk.props.min);
    clk.props.min = g_strdup(props_tmp.min);
    g_free(clk.props.sec);
    clk.props.sec = g_strdup(props_tmp.sec);
    clk.props.secneedle = props_tmp.secneedle;

    applet_widget_sync_config (APPLET_WIDGET(applet));
    set_colors(clk.area);
    update_clock (NULL);
    return;
}


static void props_cancel (GtkWidget *widget, GtkWidget **win)
{
    g_free(props_tmp.bg);
    props_tmp.bg = NULL;
    g_free(props_tmp.hour);
    props_tmp.hour = NULL;
    g_free(props_tmp.min);
    props_tmp.min = NULL;
    g_free(props_tmp.sec);
    props_tmp.sec = NULL;

    *win = NULL;
    return;
}


static void bg_color_changed (GnomeColorPicker *cp)
{
    guint8 r, g, b;
		
    gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
    g_free(props_tmp.bg);
    props_tmp.bg = g_strdup_printf ("#%02x%02x%02x", r, g, b);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


static void hour_color_changed (GnomeColorPicker *cp)
{
    guint8 r, g, b;
		
    gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
    g_free(props_tmp.hour);
    props_tmp.hour = g_strdup_printf ("#%02x%02x%02x", r, g, b);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


static void min_color_changed (GnomeColorPicker *cp)
{
    guint8 r, g, b;
		
    gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
    g_free(props_tmp.min);
    props_tmp.min = g_strdup_printf ("#%02x%02x%02x", r, g, b);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


static void sec_color_changed (GnomeColorPicker *cp)
{
    guint8 r, g, b;
		
    gnome_color_picker_get_i8 (cp, &r, &g, &b, NULL);
    g_free(props_tmp.sec);
    props_tmp.sec = g_strdup_printf ("#%02x%02x%02x", r, g, b);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
}


static void sec_needle_changed (GtkWidget *widget, GtkWidget **sec)
{
    props_tmp.secneedle = props_tmp.secneedle ? FALSE : TRUE;
    gtk_widget_set_sensitive (GTK_WIDGET(sec), props_tmp.secneedle);
    gnome_property_box_changed (GNOME_PROPERTY_BOX(props_window));
    return;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { 
		"anotherclock_applet", NULL
	};
	help_entry.path = data;
	gnome_help_display (NULL, &help_entry);
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	help_cb (APPLET_WIDGET (applet), "index.html#ANOTHERCLOCK-PREFS");
}

/*************
 * Callbacks *
 *************/
static void cb_properties (AppletWidget *applet, gpointer data)
{
    GtkWidget *label;
    GtkWidget *page;
    GtkWidget *frame;
    GtkWidget *table;
    GtkWidget *sec_checkbox;
    GtkWidget *colorpicker;
    guint     r, g, b;

    if (props_window)
    {
       gdk_window_raise (props_window->window);
       return;
    }

    /* init temporal properties */
    props_tmp.secneedle = clk.props.secneedle;
    props_tmp.bg = g_strdup (clk.props.bg);
    props_tmp.hour = g_strdup (clk.props.hour);
    props_tmp.min = g_strdup (clk.props.min);
    props_tmp.sec = g_strdup (clk.props.sec);

    /* Window and frame for settings */
    props_window = gnome_property_box_new ();
    gtk_window_set_title (GTK_WINDOW(&GNOME_PROPERTY_BOX(props_window)->dialog.window),
                	  _("Clock Settings"));

    gtk_signal_connect (GTK_OBJECT (props_window), "apply",
                	GTK_SIGNAL_FUNC(props_ok), NULL);
    gtk_signal_connect (GTK_OBJECT (props_window), "destroy",
                        GTK_SIGNAL_FUNC(props_cancel), &props_window);
    gtk_signal_connect (GTK_OBJECT (props_window), "help",
			GTK_SIGNAL_FUNC(phelp_cb), NULL);

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
    sscanf( clk.props.bg, "#%02x%02x%02x", &r, &g, &b);
    gnome_color_picker_set_i8 (GNOME_COLOR_PICKER(colorpicker),
			        r, g, b, 255);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 1, 2, 0, 1);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(bg_color_changed), NULL);
    gtk_widget_set_sensitive (colorpicker, FALSE);
    gtk_widget_show (colorpicker);

    label = gtk_label_new (_("Hour needle color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 2, 3, 0, 1);
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    sscanf( clk.props.hour, "#%02x%02x%02x", &r, &g, &b);
    gnome_color_picker_set_i8 (GNOME_COLOR_PICKER(colorpicker),
			        r, g, b, 255);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 3, 4, 0, 1);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(hour_color_changed), NULL);
    gtk_widget_show (colorpicker);

    label = gtk_label_new (_("Minute needle color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 0, 1, 1, 2);
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    sscanf( clk.props.min, "#%02x%02x%02x", &r, &g, &b);
    gnome_color_picker_set_i8 (GNOME_COLOR_PICKER(colorpicker),
			        r, g, b, 255);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 1, 2, 1, 2);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(min_color_changed), NULL);
    gtk_widget_show (colorpicker);

    label = gtk_label_new (_("Second needle color"));
    gtk_table_attach_defaults (GTK_TABLE(table), label, 2, 3, 1, 2);
/*    gtk_widget_set_sensitive (label, clk.props.secneedle);*/
    gtk_widget_show (label);
    colorpicker = gnome_color_picker_new();
    sscanf( clk.props.sec, "#%02x%02x%02x", &r, &g, &b);
    gnome_color_picker_set_i8 (GNOME_COLOR_PICKER(colorpicker),
			        r, g, b, 255);
    gtk_table_attach_defaults (GTK_TABLE(table), colorpicker, 3, 4, 1, 2);
    gtk_widget_set_sensitive (colorpicker, clk.props.secneedle);
    gtk_signal_connect (GTK_OBJECT(colorpicker), "color_set",
			GTK_SIGNAL_FUNC(sec_color_changed), NULL);
    gtk_widget_show (colorpicker);
    

    /* second needle visible? */
    sec_checkbox = gtk_check_button_new_with_label (_("Show seconds needle"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(sec_checkbox),
				  clk.props.secneedle);
    gtk_signal_connect (GTK_OBJECT(sec_checkbox), "toggled",
			GTK_SIGNAL_FUNC(sec_needle_changed), colorpicker);
    gtk_box_pack_start (GTK_BOX(page), sec_checkbox, FALSE, FALSE, 0);
    gtk_widget_show (sec_checkbox);



    gtk_widget_show (props_window);

    return;
}


static void cb_about (AppletWidget *applet, gpointer data)
{
    static GtkWidget *about = NULL;
    static const gchar *authors[] = {
			    /* if your encoding allows it, use ntilde (U00F1)
			     * instead of the 'n' of 'Inigo' */
			    N_("Inigo Serna <inigo@gaztelan.bi.ehu.es>"),
			    NULL
    };

    if (about != NULL)
    {
        gdk_window_show(about->window);
        gdk_window_raise(about->window);
	return;
    }
#ifdef ENABLE_NLS
    {
	int i=0;
	
	while (authors[i] != NULL) { authors[i]=_(authors[i]); i++; }
    }
#endif

    about = gnome_about_new (_("Another Clock Applet"),
			     VERSION,
			     _("(C) 1999 the Free Software Foundation"),
                    	     authors,
                    	     _("An analog clock similar to that in CDE panel."),
                    	     NULL);

    gtk_signal_connect( GTK_OBJECT(about), "destroy",
		        GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
    gtk_widget_show (about);

    return;
}


static gint save_session (GtkWidget *widget, char *privcfgpath,
			  char *globcfgpath)
{
    properties_save (privcfgpath);
    return FALSE;
}

static void
change_pixel_size(GtkWidget *w, int size, gpointer data)
{
    char *fname;

    setup_sizes (size);


    gtk_widget_set_usize (clk.area, applet_width, applet_height);

    gtk_widget_destroy (clk.clock);
    gtk_widget_destroy (clk.pixmap);

    /* load clock pixmap  */
    fname = gnome_unconditional_pixmap_file (CLOCK_XPM_FILE);
    if (!fname)
	g_error ("Can't find another_clock applet pixmap");
    clk.clock = gnome_pixmap_new_from_file_at_size (fname,
						    applet_width,
						    applet_height);
    g_free (fname);
    gtk_widget_show (clk.clock);

    /* create a pixmap for clock */
    clk.pixmap = gnome_pixmap_new_from_gnome_pixmap (GNOME_PIXMAP(clk.clock));
    gtk_fixed_put (GTK_FIXED(clk.area), clk.pixmap, 0, 0);
    gtk_widget_show (clk.pixmap);
    gtk_widget_show (clk.area);

    gdk_flush();

    update_clock (NULL);
    return;
}


/**************************************************
 * Stuff to mantain background under clock pixmap *
 * Taken from gen_util/printer applet		  *
 **************************************************/
static void applet_set_default_back (GtkWidget *w)
{
    GtkStyle *ns;
	
    ns = gtk_style_new();
    gtk_style_ref (ns);
    gtk_widget_set_style (w, ns);
    gtk_style_unref (ns);
    gtk_widget_queue_draw (w);
}


static void applet_set_back_color (GtkWidget *w, GdkColor *color)
{
    GtkStyle *ns;
	
    ns = gtk_style_copy (w->style);
    gtk_style_ref (ns);
    ns->bg[GTK_STATE_NORMAL] = *color;
    ns->bg[GTK_STATE_NORMAL].pixel = 1; /* bogus */
					
    if (ns->bg_pixmap[GTK_STATE_NORMAL])
    {
        gdk_imlib_free_pixmap (ns->bg_pixmap[GTK_STATE_NORMAL]);
	ns->bg_pixmap[GTK_STATE_NORMAL] = NULL;
    }

    gtk_widget_set_style (w, ns);
    gtk_style_unref (ns);
    gtk_widget_queue_draw (w);
}


static void applet_set_back_pixmap (GtkWidget *w, gchar *pixmap)
{
    GdkImlibImage *im;
    GdkPixmap 	  *p;
    GtkStyle 	  *ns;

    if (!pixmap || strcmp(pixmap,"") ==0 )
    {
        ns = gtk_style_copy (w->style);
        gtk_style_ref (ns);
        p = ns->bg_pixmap[GTK_STATE_NORMAL];
        if (p)
            gdk_imlib_free_pixmap (p);
        ns->bg_pixmap[GTK_STATE_NORMAL] = NULL;
        gtk_widget_set_style (w, ns);
        gtk_style_unref (ns);
        return;
    }

    if (!g_file_exists(pixmap))
        return;
			
    im = gdk_imlib_load_image (pixmap);
    if (!im)
        return;

    gdk_imlib_render (im, im->rgb_width, im->rgb_height);
    p = gdk_imlib_move_image (im);
    ns = gtk_style_copy (w->style);
    gtk_style_ref (ns);
    if(ns->bg_pixmap[GTK_STATE_NORMAL])
        gdk_imlib_free_pixmap (ns->bg_pixmap[GTK_STATE_NORMAL]);
    ns->bg_pixmap[GTK_STATE_NORMAL] = p;
    gtk_widget_set_style (w, ns);

    gtk_style_unref (ns);
    gdk_imlib_destroy_image (im);
}
			

static void applet_back_change (GtkWidget *widget, PanelBackType type,
			        gchar *pixmap, GdkColor *color,
				gpointer data)
{
    GtkWidget *w = data;

    if (type == PANEL_BACK_PIXMAP)
        applet_set_back_pixmap (w, pixmap);
    else if (type == PANEL_BACK_COLOR)
        applet_set_back_color (w, color);
    else
	applet_set_default_back (w);
    return;
}


/**********
 * Colors *
 **********/
static void set_gc_color(GdkColorContext *cc, int n)
{
    GdkColor    *c;
    gint	z;
    gchar	*color;
    guint	r, g, b;
    
    switch (n)
    {
	case 0:	color = clk.props.bg; break;
	case 1:	color = clk.props.hour; break;
	case 2:	color = clk.props.min; break;
	case 3:	color = clk.props.sec; break;
	default:	color = clk.props.bg; break;
    }

    c = g_new (GdkColor, 1);
    sscanf( color, "#%02x%02x%02x", &r ,&g,&b);

    c->red = (gulong) r * 256;
    c->green = (gulong) g * 256;
    c->blue = (gulong) b * 256;
    c->pixel = (gulong) 0;
    z = 0;
    gdk_color_context_get_pixels (cc, &c->red, &c->green, &c->blue, 1,
	                          &c->pixel, &z);
    if (n == 0)
    {
       /* Here it would be code to change a color (background, not
          transparent) in a pixmap */
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


static void set_colors (GtkWidget *widget)
{
    GdkColorContext *cc;

    cc = gdk_color_context_new (gtk_widget_get_visual(widget),
                                gtk_widget_get_colormap(widget));
    if(!clk.gc[0]) clk.gc[0] = gdk_gc_new (widget->window);
    if(!clk.gc[1]) clk.gc[1] = gdk_gc_new (widget->window);
    if(!clk.gc[2]) clk.gc[2] = gdk_gc_new (widget->window);
    if(!clk.gc[3]) clk.gc[3] = gdk_gc_new (widget->window);
    set_gc_color (cc, 0);
    set_gc_color (cc, 1);
    set_gc_color (cc, 2);
    set_gc_color (cc, 3);
    gdk_color_context_free (cc);
}


/****************
 * update_clock *
 ****************/
static gint update_clock (gpointer data)
{
    time_t	curtime;
    struct tm 	*tm;
    double	ang;
    GdkGC       *gc;

    if (!GTK_WIDGET_REALIZED (clk.pixmap))
      return TRUE;

    /* Get hour, minute and second */    
    curtime = time (NULL);
    tm = localtime (&curtime);

    /* draw clock */
    gdk_draw_pixmap (GNOME_PIXMAP(clk.pixmap)->pixmap,
	             clk.gc[0],
                     GNOME_PIXMAP(clk.clock)->pixmap,
                     0, 0, 0, 0,
		     applet_width, applet_height);

    /* draw needles */
    ang = ((tm->tm_hour > 12) ? tm->tm_hour-12 : tm->tm_hour) * M_PI / 6;
    ang += tm->tm_min * M_PI / 360;
    gdk_draw_line (GNOME_PIXMAP(clk.pixmap)->pixmap, clk.gc[1], cx, cy,
		   (int) (cx + (needle_size-3) * sin(ang)),
		   (int) (cy - (needle_size-3) * cos(ang)));
    ang = tm->tm_min * M_PI / 30;
    gdk_draw_line (GNOME_PIXMAP(clk.pixmap)->pixmap, clk.gc[2], cx, cy,
		   (int) (cx + needle_size * sin(ang)),
		   (int) (cy - needle_size * cos(ang)));
    if (clk.props.secneedle)
    {
       ang = tm->tm_sec * M_PI / 30;
       gdk_draw_line (GNOME_PIXMAP(clk.pixmap)->pixmap, clk.gc[3], cx, cy,
		      (int) (cx + needle_size * sin(ang)),
		      (int) (cy - needle_size * cos(ang)));
    }

    /* draw clock */
    gc = gdk_gc_new(clk.pixmap->window);
    gdk_gc_set_clip_mask (gc, GNOME_PIXMAP(clk.pixmap)->mask);
    gdk_draw_pixmap (clk.pixmap->window,
		     gc,
                     GNOME_PIXMAP(clk.pixmap)->pixmap,
                     0, 0, 0, 0,
		     applet_width, applet_height);
    gdk_gc_destroy(gc);

    return TRUE;
}

/*****************
 * Main function *
 *****************/
int main (int argc, char *argv[])
{
    char *fname;

    /* Initialize the i18n stuff */
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    /* create the main window, and attach delete_event signal to terminating
       the application */
    applet_widget_init (APPLET_NAME, VERSION, argc, argv, NULL, 0, NULL);
    applet = applet_widget_new (APPLET_NAME);
    if (!applet)
       g_error ("Can't create another_clock applet!\n");
    gtk_widget_realize (applet);

    setup_sizes (applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet)));

    /* create fixed area for clock pixmap */
    clk.area = gtk_fixed_new();
    gtk_widget_set_usize (clk.area, applet_width, applet_height);

    /* load clock pixmap  */
    fname = gnome_unconditional_pixmap_file (CLOCK_XPM_FILE);
    if (!fname)
	g_error ("Can't find another_clock applet pixmap");
    clk.clock = gnome_pixmap_new_from_file_at_size (fname,
						    applet_width,
						    applet_height);
    g_free (fname);
    gtk_widget_show (clk.clock);

    /* create a pixmap for clock */
    clk.pixmap = gnome_pixmap_new_from_gnome_pixmap (GNOME_PIXMAP(clk.clock));
    gtk_fixed_put (GTK_FIXED(clk.area), clk.pixmap, 0, 0);
    gtk_widget_show (clk.pixmap);
    gtk_widget_show (clk.area);

    /* load background and needles' colors */
    properties_load (APPLET_WIDGET(applet)->privcfgpath);
    set_colors (applet);

    /* callback for updating the time */
    gtk_timeout_add (500, update_clock, &clk);
    update_clock (&clk);

    /* callbacks for session, background change, about, properties, etc. */
    gtk_signal_connect (GTK_OBJECT(applet), "save_session",
			GTK_SIGNAL_FUNC(save_session), NULL);
    gtk_signal_connect (GTK_OBJECT(applet), "back_change",
	                GTK_SIGNAL_FUNC(applet_back_change), clk.area);
    gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
		       GTK_SIGNAL_FUNC(change_pixel_size), NULL);

    /* show applet */
    applet_widget_add (APPLET_WIDGET(applet), clk.area);
    gtk_widget_show (applet);

    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "properties",
					   GNOME_STOCK_MENU_PROP,
					   _("Properties..."),
					   cb_properties,
					   NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(applet),
					   "help",
					   GNOME_STOCK_PIXMAP_HELP,
					   _("Help"), help_cb, "index.html");
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
