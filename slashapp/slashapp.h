#include <config.h>
#include <gnome.h>
#include <applet-widget.h>
#include <gnome-xml/parser.h>

/* mkdir(2) */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef SLASHAPP_H
#define SLASHAPP_H

#define APPLET_VERSION_MAJ 0
#define APPLET_VERSION_MIN 3
#define APPLET_VERSION_REV 2

#define UPDATE_DELAY 70

typedef struct _AppData AppData;
typedef struct _InfoData InfoData;

/* some standard RDF sites, FIXME: add custom one some time */
enum {
	RDFSITE_SLASHDOT,
	RDFSITE_GNOTICES
};

struct _InfoData
{
        gchar *text;            /* any length, but be reasonable */
        gint length;
        gchar *icon_path;       /* location of icon */
        GtkWidget *icon;        /* maximum 24 x 24 please (or be reasonable)*/
        gint icon_w;
        gint icon_h;
        gint offset;            /* offset of text from left, set to line text up even with no icon */
        gint center;            /* Center text, only has effect if text fits on one line */
        gint shown;
        gint show_count;        /* when 0 show forever, other wise times to show (1 or +) */
        gint end_delay;         /* time to delay after displaying last character of text
                                   this number is calculated from tenths of a second */

        /* click callback stuff */

        void (*click_func)(gpointer data, InfoData *id, AppData *ad);
        gpointer data;
        void (*free_func)(gpointer data);

	/* from tick-a-stat */
	void(*pre_func)(gpointer data, InfoData *id, AppData *ad);
	void(*end_func)(gpointer data, InfoData *id, AppData *ad);
};


struct _AppData
{

	gchar *privcfgpath; /* gross hack */
	
	GtkWidget *applet;
	GtkWidget *frame;
	GtkWidget *draw_area;
	gint height;
	gint width;

	gint scroll_delay;
	gint scroll_speed;
	gint scroll_height;
	gint smooth_scroll;
	gint smooth_type;

	GtkWidget *display_w;
	GtkWidget *disp_buf_w;
	GtkWidget *background_w;
	GdkPixmap *display;
	GdkPixmap *disp_buf;
	GdkPixmap *background;

	GdkColor text_color;

	GList *text;		/* GList of InfoData structs */
	gint text_lines;

	gint current_line;
	gint current_line_pos;

	gint scroll_count;
	gint scroll_pos;
	gint new_line;
	gint x_pos;
	gint text_pos;

	gint text_height;
	gint text_y_line;

	gint display_timeout_id;

	GList *click_list;		/* GList of clickable target on display */

	/* properties stuff */

	GtkWidget *propwindow;

	int rdf_site;
	int p_rdf_site;

	gint p_smooth_scroll;
	gint p_smooth_type;
	gint p_scroll_delay;
	gint p_scroll_speed;

	/* per app stuff (when writing an actual applet based on this, put new vars here) */

	gint headline_timeout_id;
	gint startup_timeout_id;

	gchar *slashapp_dir;

	gint show_images;
	gint show_info;
	gint show_department;
	gint p_show_images;
	gint p_show_info;
	gint p_show_department;

	gint new_browser_window;
	gint p_new_browser_window;

	gint article_delay;
	gint p_article_delay;

	GtkWidget *article_window;
	GtkWidget *article_list;

	gchar *host;
	gchar *proxy_host;
	gchar *p_proxy_host;
	GtkWidget *proxy_host_widget;
	gchar *proxy_port;
	gchar *p_proxy_port;
	GtkWidget *proxy_port_widget;
	gint proxy_enabled;
	gint p_proxy_enabled;
	gchar *resource;
	gint port;

	/* from tick-a-stat */

	PanelOrientType orient;
	gint sizehint;
	gint width_hint;

	gint win_height;
	gint win_width;
	gint follow_hint_height;
	gint follow_hint_width;
	gint user_width;
	
	gint user_height;

	InfoData *info_next;
	InfoData *info_current;

	gint free_current;

	GList *info_list;
	gint y_pos;
};

typedef struct _ClickData ClickData;
struct _ClickData
{
	InfoData *line_id;
	gint x;
	gint y;
	gint w;
	gint h;
	gpointer data;

	/* from tick-a-stat */
	gint free_id;
	void (*click_func)(gpointer data, InfoData *id, AppData *ad);
	
};

	/* display.c */
InfoData *add_info_line(AppData *ad, const gchar *text, const gchar *icon_path, gint offset, gint center,
                   gint show_count, gint delay);
InfoData *add_info_line_with_pixmap(AppData *ad, const gchar *text, GtkWidget *icon, gint offset, gint center,
		   gint show_count, gint delay);
void remove_info_line(AppData *ad, InfoData *id);
void remove_all_lines(AppData *ad);
void set_info_click_signal(InfoData *id, void (*click_func)(gpointer data, InfoData *id, AppData *ad),
		gpointer data, void (*free_func)(gpointer data));
void set_mouse_cursor (AppData *ad, gint icon);
void init_app_display(AppData *ad);

	/* properties.c */
void property_load(gchar *path, AppData *ad);
void property_save(gchar *path, AppData *ad);
void property_show(AppletWidget *applet, gpointer data);

/* slashapp.c */
void resized_app_display(AppData *ad, gint force);
AppData *create_new_app(GtkWidget *applet);
gchar *check_for_dir(char *d);
void destroy_applet(GtkWidget *widget, gpointer data);
gint applet_save_session(GtkWidget *widget, gchar *privcfgpath, 
	       		 gchar *globcfgpath, gpointer data);	
void about_cb(AppletWidget *widget, gpointer data);
void show_article_window(AppletWidget *applet, gpointer data);
void populate_article_window(AppData *ad);
void refresh_cb(AppletWidget *widget, gpointer data);
int startup_delay_cb(gpointer data);
int get_current_headlines(gpointer data);
void parse_headlines(gpointer data);
int http_get_to_file(gchar *host, gint port, gchar *resource, FILE *file, gpointer data); 
void tree_walk(xmlNodePtr root, gpointer data);
void destroy_article_window(GtkWidget *widget, gpointer data);
void article_button_cb(GtkWidget *widget, gpointer data);
const char *layer_find(xmlNodePtr node, const char *match, const char *fail);
void click_headline_cb (gpointer data, InfoData *id, AppData *ad);

#endif /* SLASHAPP_H */
