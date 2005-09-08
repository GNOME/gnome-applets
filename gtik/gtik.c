/*
 *	GNOME Stock Ticker 
 *	(C) 2000 Jayson Lorenzen, Jim Garrison, Rached Blili
 *
 *	based on:
 *	desire, and the old great slash applet.
 *	
 *
 *	Authors: Jayson Lorenzen (jayson_lorenzen@yahoo.com)
 *               Jim Garrison (garrison@users.sourceforge.net)
 *               Rached Blili (striker@Dread.net)
 *
 *	The GNOME Stock Ticker is a free, Internet based application. 
 *      These quotes are not guaranteed to be timely or accurate.
 *
 *	Do not use the GNOME Stock Ticker for making investment decisions, 
 *      it is for informational purposes only.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <panel-applet.h>
#include <panel-applet-gconf.h> 
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libgnomeui/gnome-help.h>
#include <libgnomevfs/gnome-vfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <locale.h>
#include <gdk/gdkx.h>
#include "customdrawingarea.h"

#define STOCK_QUOTE(sq) ((StockQuote *)(sq))

#define MAX_SYMBOL_LEN 1024

#define NEVER_SENSITIVE		"never_sensitive"

	enum {
		WHITE,
		RED,
		GREEN
	};
	
	typedef struct {
		char *price;
		int pricelen;
		int priceheight;
		char *change;
		int changelen;
		int changeheight;
		int color;
	} StockQuote;
	
	typedef struct
	{
		char *tik_syms;
		gboolean output;
		gboolean scroll;
		gint timeout;
		gint scroll_speed;
		gchar *dcolor;
		gchar *ucolor;
		gchar *bgcolor;
		gchar *fgcolor;
		gchar *font;
		gboolean buttons;
		gboolean use_default_theme;
		gint width;

	} gtik_properties;

	
	typedef struct 
	{
		GtkWidget *applet;
		GtkWidget *label;
		GnomeVFSAsyncHandle *vfshandle;

		GdkPixmap *pixmap;
		GtkWidget * drawing_area;
		GtkWidget * leftButton;
		GtkWidget * rightButton;

		int location;
		int MOVE;
		int delta;
	
		GArray *quotes;

		int setCounter;

		GdkGC *gc;
		GdkColor gdkUcolor,gdkDcolor, gdkBGcolor, gdkFGcolor;

		/* end of COLOR vars */

		char *configFileName;

		/*  properties vars */
 	 
		GtkWidget *tik_syms_entry;
		
		GtkWidget *fontDialog;

		GtkWidget *about_dialog;

		GtkWidget * pb;
		
		gtik_properties props; 
	
		gint timeout;
		gint drawTimeID, updateTimeID;
		/* For fonts */
		PangoFontDescription * my_font;
		gchar * new_font;
		PangoLayout *layout;
		GtkTooltips * tooltips;
		gint symbolfont;
		gint whichlabel;
	} StockData;
	
	void removeSpace(char *buffer); 
	int configured(StockData *stockdata);
	static void destroy_applet(GtkWidget *widget, gpointer data) ;
	char *getSymsFromClist(GtkWidget *clist) ;

	char *splitPrice(char *data);
	char *splitChange(StockData *stockdata, char *data, StockQuote *quote);

	/* FOR COLOR */

	static gint updateOutput(gpointer data) ;
	static void reSetOutputArray(StockData *stockdata) ;
	static void setOutputArray(StockData *stockdata, char *param1) ;
	void setup_colors(StockData *stockdata);
	int create_gc(StockData *stockdata) ;
	static void ucolor_set_cb (GtkColorButton *cb, gpointer data);
	static void dcolor_set_cb (GtkColorButton *cb, gpointer data);
	static void bgcolor_set_cb (GtkColorButton *cb, gpointer data);
	static void fgcolor_set_cb (GtkColorButton *cb, gpointer data);

	/* end of color funcs */

	static void font_cb (GtkFontButton *gfb, gpointer data);

	/* accessibility funcs and vars */
	static GtkWidget *access_drawing_area;
	static StockData *access_stock;

	extern void set_relation (GtkWidget *widget, GtkLabel *label);
	static void setup_refchild_nchildren(GtkWidget * vbox);
	static gint gtik_vbox_accessible_get_n_children(AtkObject *obj);
	static AtkObject* gtik_vbox_accessible_ref_child(AtkObject *obj, gint i);
	/* end accessibility funcs and vars */

	/*-----------------------------------------------------------------*/
	/* set sensitive and setup NEVER_SENSITIVE appropriately */
	static void hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
	{
		gtk_widget_set_sensitive (w, sensitivity);
		g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
				   GINT_TO_POINTER ( ! sensitivity));
	}


	/*-----------------------------------------------------------------*/
	/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
	static void soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
	{
		if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
			gtk_widget_set_sensitive (w, FALSE);
		else
			gtk_widget_set_sensitive (w, sensitivity);
	}


	/*-----------------------------------------------------------------*/
	static gboolean key_writable (PanelApplet *applet, const char *key)
	{
		gboolean writable;
		char *fullkey;
		static GConfClient *client = NULL;
		if (client == NULL)
			client = gconf_client_get_default ();

		fullkey = panel_applet_gconf_get_full_key (applet, key);

		writable = gconf_client_key_is_writable (client, fullkey, NULL);

		g_free (fullkey);

		return writable;
	}

	/*-----------------------------------------------------------------*/
	static void load_fonts(StockData *stockdata)
	{
		GArray *quotes = stockdata->quotes;
		PangoRectangle rect;
		PangoContext *context;
		int i;
		
		if (stockdata->my_font)
			pango_font_description_free (stockdata->my_font);
		stockdata->my_font = NULL;
		if (stockdata->props.font ) {
			stockdata->my_font = pango_font_description_from_string (stockdata->props.font);
		}
	
		if (stockdata->props.use_default_theme || !stockdata->my_font) {
			context = gtk_widget_get_pango_context (stockdata->applet);
			stockdata->my_font = pango_font_description_copy (
								pango_context_get_font_description (context));
		}
		pango_layout_set_font_description (stockdata->layout, stockdata->my_font);

		/* make sure the cached strings widths are updated */
		for (i=0; i< stockdata->setCounter; i++) {
			pango_layout_set_text (stockdata->layout, 
					       STOCK_QUOTE(quotes->data)[i].price, -1);
			pango_layout_get_pixel_extents (stockdata->layout, NULL, &rect);
			STOCK_QUOTE(quotes->data)[i].pricelen = rect.width;
			STOCK_QUOTE(quotes->data)[i].priceheight = rect.height;
			pango_layout_set_text (stockdata->layout, 
					       STOCK_QUOTE(quotes->data)[i].change, -1);
			pango_layout_get_pixel_extents (stockdata->layout, NULL, &rect);
			STOCK_QUOTE(quotes->data)[i].changelen = rect.width;
			STOCK_QUOTE(quotes->data)[i].changeheight = rect.height;
		}
	}

/*-----------------------------------------------------------------*/
static void xfer_callback (GnomeVFSAsyncHandle *handle, GnomeVFSXferProgressInfo *info, gpointer data)
{
	StockData *stockdata = data;

	if (info->phase == GNOME_VFS_XFER_PHASE_COMPLETED) {
		stockdata->vfshandle = NULL;
		if (!configured(stockdata)) {
			reSetOutputArray(stockdata);
			setOutputArray(stockdata,
				       _("Could not retrieve the stock data."));
		}
	}
}

/*-----------------------------------------------------------------*/
static gint updateOutput(gpointer data)
{
	StockData *stockdata = data;
	GList *sources, *dests;
	GnomeVFSURI *source_uri, *dest_uri;
	char *source_text_uri, *dest_text_uri;

	if (stockdata->vfshandle != NULL) {
		gnome_vfs_async_cancel (stockdata->vfshandle);
		stockdata->vfshandle = NULL;
	}

	if (stockdata->props.tik_syms == NULL) {
		setOutputArray (stockdata, _("No stock list"));
		return TRUE;
	}
        /*
         * Now using a CSV file to get quotes. Specification of CSV format
         * is in URL f=sl1c1.
         * Availables data columns are:
         * s: ticker symbol
         * l: last price
         * d: date 
         * t: time
         * c: change
         * o: open price
         * h: daily high
         * g: daily low
         * v: volume
         * The digit following some data specifiers change data formating (html/raw)
         */
	source_text_uri = g_strconcat("http://finance.yahoo.com/d/quotes.csv?s=",
				      stockdata->props.tik_syms,
				      "&f=sl1c1&e=.csv",
				      NULL);

	source_uri = gnome_vfs_uri_new(source_text_uri);
	sources = g_list_append(NULL, source_uri);
	g_free(source_text_uri);
	dest_text_uri = g_strconcat ("file://",stockdata->configFileName, NULL);

	dest_uri = gnome_vfs_uri_new(dest_text_uri);
	dests = g_list_append(NULL, dest_uri);
	g_free (dest_text_uri);
	
	reSetOutputArray(stockdata);
	setOutputArray(stockdata,  _("Updating..."));

	if (GNOME_VFS_OK !=
	    gnome_vfs_async_xfer(&stockdata->vfshandle, sources, dests,
				 GNOME_VFS_XFER_DEFAULT,
				 GNOME_VFS_XFER_ERROR_MODE_ABORT,
				 GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
				 GNOME_VFS_PRIORITY_DEFAULT,
				 (GnomeVFSAsyncXferProgressCallback) xfer_callback,
				 stockdata, NULL, NULL)) {
		GnomeVFSXferProgressInfo info;

		info.phase = GNOME_VFS_XFER_PHASE_COMPLETED;
		xfer_callback(NULL, &info, stockdata);
		stockdata->vfshandle = NULL;
	}

	g_list_free(sources);
	g_list_free(dests);

	gnome_vfs_uri_unref(source_uri);
	gnome_vfs_uri_unref(dest_uri);
	
	return TRUE;
}




	/*-----------------------------------------------------------------*/
	static void properties_load(StockData *stockdata) {
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		GError *error = NULL;
		
		stockdata->props.tik_syms = panel_applet_gconf_get_string (applet,
									   "tik_syms",
									   &error);
		if (error) {
			g_print ("%s \n", error->message);
			g_error_free (error);
			error = NULL;
		}
		if (!stockdata->props.tik_syms)
			stockdata->props.tik_syms = g_strdup ("^DJI+^IXIC+^GSPC");
		stockdata->props.output = panel_applet_gconf_get_bool (applet,
									  "output",
									  NULL);
		stockdata->props.scroll = panel_applet_gconf_get_bool (applet,
									  "scroll",
									  NULL);
		stockdata->props.timeout = panel_applet_gconf_get_int (applet,
								       "timeout",
								       NULL);
		stockdata->props.timeout = MAX (stockdata->props.timeout, 1);
		stockdata->props.bgcolor = panel_applet_gconf_get_string (applet,
									 "bgcolor",
									 NULL);
		if (!stockdata->props.bgcolor)
			stockdata->props.bgcolor = g_strdup ("#ffffff");
		
		
		stockdata->props.fgcolor = panel_applet_gconf_get_string (applet,
									 "fgcolor",
									 NULL);		 
		if (!stockdata->props.fgcolor)
			stockdata->props.fgcolor = g_strdup ("#000000");
			
		stockdata->props.dcolor = panel_applet_gconf_get_string (applet,
									 "dcolor",
									 NULL);
		if (!stockdata->props.dcolor)
			stockdata->props.dcolor = g_strdup ("#ff0000");
		
		stockdata->props.ucolor = panel_applet_gconf_get_string (applet,
									 "ucolor",
									 NULL);
		if (!stockdata->props.ucolor)
			stockdata->props.ucolor = g_strdup ("#00ff00");
			
		stockdata->props.use_default_theme = panel_applet_gconf_get_bool(applet,
									  "use_default_theme",
									  NULL);
		stockdata->props.font = panel_applet_gconf_get_string (applet,
								       "font",
								       NULL);
				
		stockdata->props.buttons = panel_applet_gconf_get_bool(applet,
									  "buttons",
									  NULL);
									  
		stockdata->props.scroll_speed = panel_applet_gconf_get_int (applet,
									    "scroll_speed",
									    NULL);
		stockdata->props.scroll_speed = MAX (stockdata->props.scroll_speed, 5);
		
		stockdata->props.width = panel_applet_gconf_get_int (applet,
							 	     "width",
								     NULL);
		stockdata->props.width = MAX (stockdata->props.width, 20);
									
	}

	/*-----------------------------------------------------------------*/
	static void properties_set (StockData *stockdata, gboolean update) {
		if (stockdata->props.buttons == TRUE) {
			gtk_widget_show(stockdata->leftButton);
			gtk_widget_show(stockdata->rightButton);
		}
		else {
			gtk_widget_hide(stockdata->leftButton);
			gtk_widget_hide(stockdata->rightButton);
		}

		setup_colors(stockdata);		
		load_fonts(stockdata);
		if (update)
			updateOutput(stockdata);
	}


	/*-----------------------------------------------------------------*/
	static char *parseQuote(char line[MAX_SYMBOL_LEN]) {
		
                char *symbol;
                char *price;
                char *change;
                double price_val; 
                double change_val; 
                double percent;
                static char result[512]="";

                /* Yahoo file comme in C format */
                setlocale(LC_NUMERIC, "C");
                
                symbol = strtok(line, ",");
                symbol++;
                symbol[strlen(symbol) -1] = 0;
                g_message ("symbol: %s", symbol); 
        
                price = strtok(NULL, ",");
                price_val = strtod(price, NULL); 

                change = strtok(NULL, ",");
                /* file in DOS format: remove \r\n */
                change[strlen(change) -1] = 0;
                change[strlen(change) -1] = 0;
                change_val = strtod(change, NULL); 
               
		/* GNOME Bug 143737 - prothonotar@tarnation.dyndns.org */ 
                percent = (change_val/(price_val-change_val))*1E+02; 

                /* Restore numeric format for displaying */
                setlocale(LC_NUMERIC, g_getenv ("LANG"));
                
                if (change_val == 0.0)
                        sprintf(result,"%s:%1.3f:%1.3f:%1.2f%%",
                                symbol,price_val,change_val,percent);
                else
                        sprintf(result,"%s:%1.3f:%1.3f:%+1.2f%%",
                                symbol,price_val,change_val,percent);
        
                return result;

	}



	/*-----------------------------------------------------------------*/
	int configured(StockData *stockdata) {
		int retVar;

		char  buffer[MAX_SYMBOL_LEN];
		static FILE *CONFIG;

		CONFIG = fopen((const char *)stockdata->configFileName,"r");

		retVar = 0;

		/* clear the output variable */
		reSetOutputArray(stockdata);

		if ( CONFIG ) {
			while ( !feof(CONFIG) ) {
				fgets(buffer, sizeof(buffer)-1, CONFIG);

				if (!feof(CONFIG)) {

				      setOutputArray(stockdata, parseQuote(buffer));
				      retVar = 1;
				}
				else {
				      retVar = (retVar > 0) ? retVar : 0;
				}
			}
			fclose(CONFIG);

		}
		else {
			retVar = 0;
		}

		return retVar;
	}


	/*-----------------------------------------------------------------*/
	static gint expose_event (GtkWidget *widget,GdkEventExpose *event, gpointer data) {
		StockData *stockdata = data;
		gdk_draw_pixmap(widget->window,
		widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		stockdata->pixmap,
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width,event->area.height);

		return FALSE;
	}



	/*-----------------------------------------------------------------*/
	static gint configure_event(GtkWidget *widget,GdkEventConfigure *event, 
				    gpointer data){
		StockData *stockdata = data;
		if(stockdata->pixmap) {
			gdk_pixmap_unref (stockdata->pixmap);
		}

		stockdata->pixmap = gdk_pixmap_new(widget->window,
		widget->allocation.width,
		widget->allocation.height,
		-1);

		return TRUE;
	}
	
	static gboolean is_visible (int start, int end, int width)
	{
		if (start >= 0 && start<= width)
			return TRUE;
		else if (end >= 0 && end <= width)
			return TRUE;
		else if (start <= 0 && end >= width)
			return TRUE;
		else	
			return FALSE;
	}

	/*-----------------------------------------------------------------*/
	static gint Repaint (gpointer data) {
		StockData *stockdata = data;
		GtkWidget* drawing_area = stockdata->drawing_area;
		GArray *quotes = stockdata->quotes;
		GdkGC *gc = stockdata->gc;
		GdkRectangle update_rect;
		PangoLayout *layout;
		int	comp;
		int start, end, width, max_height=0, y;

		/* FOR COLOR */
		int totalLoc;
		int totalLen;
		int i;

		totalLoc = 0;
		totalLen = 0;

		if (stockdata->props.use_default_theme)
			gdk_gc_set_foreground( gc, &stockdata->applet->style->base[GTK_WIDGET_STATE (stockdata->applet)] );
		else
			gdk_gc_set_foreground( gc, &stockdata->gdkBGcolor );
		
		gdk_draw_rectangle (stockdata->pixmap,
				    gc, TRUE, 0,0,
				    drawing_area->allocation.width,
				    drawing_area->allocation.height);
		
		layout = stockdata->layout;
		
		for (i=0; i< stockdata->setCounter; i++) {
			totalLen += STOCK_QUOTE(quotes->data)[i].pricelen + 10;
			max_height = MAX (max_height, STOCK_QUOTE(quotes->data)[i].priceheight);
			if (stockdata->props.output == FALSE) {
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					totalLen += STOCK_QUOTE(quotes->data)[i].changelen 
						+ 10;
					max_height = MAX (max_height, STOCK_QUOTE(quotes->data)[i].changeheight);
					
				}
			}
			
		}
		
		y = MAX((drawing_area->allocation.height - max_height)/2, 0);
		comp = 0 - totalLen;

		if (stockdata->MOVE == 1) { stockdata->MOVE = 0; } 
		else { stockdata->MOVE = 1; }

		if (stockdata->MOVE == 1) {

		  if (stockdata->props.scroll == TRUE) {
			if (stockdata->location > comp) {
				stockdata->location -= stockdata->delta;	
			}
			else {
				stockdata->location = drawing_area->allocation.width +
						      stockdata->location - comp;
			}

		  }
		  else {
                       if (stockdata->location < drawing_area->allocation.width) {
                                stockdata->location += stockdata->delta;    
                        }
                        else {
                                stockdata->location = comp + stockdata->location -
                                		      drawing_area->allocation.width;
                        }
		  }



		}

		for (i=0;i<stockdata->setCounter;i++) {
			
			
			/* COLOR */
			if (stockdata->props.use_default_theme)
				gdk_gc_set_foreground( gc, &stockdata->applet->style->fg[GTK_WIDGET_STATE (stockdata->applet)] );
			else {
				if (STOCK_QUOTE(quotes->data)[i].color == GREEN) {
					gdk_gc_set_foreground( gc, &stockdata->gdkUcolor );
				}
				else if (STOCK_QUOTE(quotes->data)[i].color == RED) {
					gdk_gc_set_foreground( gc, &stockdata->gdkDcolor );
				}
				else {
					gdk_gc_set_foreground( gc, &stockdata->gdkFGcolor );
				}
			}

			start = stockdata->location + totalLoc;
			end = stockdata->location + totalLoc +
			      STOCK_QUOTE(quotes->data)[i].pricelen + 10;
			width = drawing_area->allocation.width;
			if (is_visible (start, end, width)) {
				pango_layout_set_text (layout,
					       STOCK_QUOTE(quotes->data)[i].price,
					       -1);	
				gdk_draw_layout (stockdata->pixmap, gc,
					 start , y,
					 layout);
			}
			totalLoc += STOCK_QUOTE(quotes->data)[i].pricelen + 10;
			
			if (stockdata->props.output == FALSE) {
				start = stockdata->location + totalLoc;
				end = stockdata->location + totalLoc +
				      STOCK_QUOTE(quotes->data)[i].changelen + 10;
				if (is_visible (start, end, width)) {
					pango_layout_set_text (layout, 
						STOCK_QUOTE(quotes->data)[i].change, -1);
					gdk_draw_layout (stockdata->pixmap,
					     		gc, stockdata->location + totalLoc,
					     		y, layout);
				}
				totalLoc += STOCK_QUOTE(quotes->data)[i].changelen + 10;
			}
			
		}
		update_rect.x = 0;
		update_rect.y = 0;
		update_rect.width = drawing_area->allocation.width;
		update_rect.height = drawing_area->allocation.height;

		gtk_widget_queue_draw_area (drawing_area,
					    update_rect.x,
					    update_rect.y,
					    update_rect.width,
					    update_rect.height);

		return 1;
	}




	/*-----------------------------------------------------------------*/
	static void about_cb (BonoboUIComponent *uic,
			      StockData         *stockdata, 
			      const gchar       *verbname) {

		static const gchar *authors[] = {
			"Jayson Lorenzen <jayson_lorenzen@yahoo.com>",
			"Jim Garrison <garrison@users.sourceforge.net>",
			"Rached Blili <striker@dread.net>",
			NULL
		};
		
		const gchar *documenters[] = {
		        "Sun GNOME Documentation Team <gdocteam@sun.com>",
			NULL
		};

		gtk_show_about_dialog(NULL,
			"name",		_("Stock Ticker"),
			"version",	VERSION,
			"copyright",	"\xC2\xA9 2000 Jayson Lorenzen, "
					"Jim Garrison, Rached Blili",
			"comments",	_("This program connects to "
					"a popular site and downloads current "
					"stock quotes.\n\n"
					"The GNOME Stock Ticker is a free "
					"Internet-based application. It comes "
					"with ABSOLUTELY NO WARRANTY.\n\n"
					"Do not use the GNOME Stock Ticker for "
					"making investment decisions; it is "
					"for informational purposes only."),
			"authors",	authors,
			"documenters",	documenters,
			"translator-credits",	_("translator-credits"),
			"logo_icon_name",	"gnome-money",
			NULL);
	}

	static void help_cb (BonoboUIComponent *uic,
			     StockData         *stockdata, 
			     const char        *verbname) 
	{
		gnome_help_display_on_screen (
				"gtik2_applet2", NULL,
				gtk_widget_get_screen (stockdata->applet),
				NULL);

	/* FIXME: display error to the user */
	}

	/*-----------------------------------------------------------------*/
	static void refresh_cb(BonoboUIComponent *uic, gpointer data, 
			       const gchar *verbname) {
		StockData *stockdata = data;
		updateOutput(stockdata);
	}


	/*-----------------------------------------------------------------*/
	static void zipLeft(GtkWidget *widget, gpointer data) {
		StockData *stockdata = data;
		gboolean current;
		gint temp;

		current = stockdata->props.scroll;
		stockdata->props.scroll = TRUE;
		temp = stockdata->delta;
		stockdata->delta = 50;
		stockdata->MOVE = 0;
		Repaint(stockdata);
		stockdata->delta = temp;
		stockdata->props.scroll = current;
	}

	/*-----------------------------------------------------------------*/
	static void zipRight(GtkWidget *widget, gpointer data) {
		StockData *stockdata = data;
		gboolean current;
		gint temp;

		current = stockdata->props.scroll;
		stockdata->props.scroll = FALSE;
		temp = stockdata->delta;
		stockdata->delta = 50;
		stockdata->MOVE = 0;
		Repaint(stockdata);
		stockdata->delta = temp;
		stockdata->props.scroll = current;
	}

	/*-----------------------------------------------------------------*/
	static gboolean timeout_cb(GtkWidget *spin, GdkEventFocus *event, gpointer data ) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		gint timeout;
		
		timeout=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
		if (timeout < 1)
			return FALSE;
		
		if (timeout == stockdata->props.timeout)
			return FALSE;
			
		stockdata->props.timeout = timeout;
		panel_applet_gconf_set_int (applet, "timeout", 
					    stockdata->props.timeout, NULL);
		gtk_timeout_remove(stockdata->updateTimeID);
		stockdata->updateTimeID = gtk_timeout_add(stockdata->props.timeout * 60000,
				                          updateOutput, stockdata);
				                          
		return FALSE;
		
	}
	
	static void scroll_speed_changed (GtkOptionMenu *option, gpointer data)
	{
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		gint speed;
		
		speed = gtk_option_menu_get_history (option);
		
		stockdata->props.scroll_speed = 10 + 20 * (2-speed);
		panel_applet_gconf_set_int (applet, "scroll_speed", 
					    stockdata->props.scroll_speed, NULL);
		gtk_timeout_remove(stockdata->drawTimeID);
		stockdata->drawTimeID = gtk_timeout_add(stockdata->props.scroll_speed,
						        Repaint,stockdata);
		
	}
	
	static void width_changed (GtkSpinButton *spin, StockData *stockdata)
 	{
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		gint width;
		
		width=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
		if (width < 1)
			return;
			
		if (stockdata->props.width == width)
			return;
			
		stockdata->props.width = width;
		panel_applet_gconf_set_int (applet, "width", 
					    stockdata->props.width, NULL);
				    
		gtk_drawing_area_size(GTK_DRAWING_AREA (stockdata->drawing_area),
						stockdata->props.width,-1);
	
		return;
	}
	
	static void
	output_toggled (GtkToggleButton *button, gpointer data)
	{
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		if (gtk_toggle_button_get_active (button) == stockdata->props.output)
			return;
		stockdata->props.output = gtk_toggle_button_get_active (button);
		panel_applet_gconf_set_bool (applet,"output",
					     stockdata->props.output, NULL);
					     
	}
	
	static void
	rtl_toggled (GtkToggleButton *button, gpointer data)
	{
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		stockdata->props.scroll = !gtk_toggle_button_get_active (button);
		panel_applet_gconf_set_bool (applet,"scroll",
					     stockdata->props.scroll, NULL);
					     
	}
		
	static void
	scroll_toggled (GtkToggleButton *button, gpointer data)
	{
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		stockdata->props.buttons = gtk_toggle_button_get_active (button);
		panel_applet_gconf_set_bool (applet,"buttons",
					     stockdata->props.buttons, NULL);
		if (stockdata->props.buttons == TRUE) {
			gtk_widget_show(stockdata->leftButton);
			gtk_widget_show(stockdata->rightButton);
		}
		else {
			gtk_widget_hide(stockdata->leftButton);
			gtk_widget_hide(stockdata->rightButton);
		}
					     
	}	
	
	static void
	def_font_toggled (GtkToggleButton *button, gpointer data)
	{
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		GtkWidget *fonts_widget, *colors_widget;
		gboolean toggle;
		
		toggle = gtk_toggle_button_get_active (button);
		if (toggle != stockdata->props.use_default_theme)
			stockdata->props.use_default_theme = toggle;
		else
			return;
		panel_applet_gconf_set_bool (applet,"use_default_theme",
					     stockdata->props.use_default_theme, NULL);
					     
		fonts_widget = g_object_get_data (G_OBJECT (button), "fonts_hbox");
		colors_widget = g_object_get_data (G_OBJECT (button), "colors_hbox");
		soft_set_sensitive (GTK_WIDGET (fonts_widget), 
					             !stockdata->props.use_default_theme);
		soft_set_sensitive (GTK_WIDGET (colors_widget), 
					             !stockdata->props.use_default_theme);
		load_fonts(stockdata);
	}
	
	static void
	add_symbol (GtkEntry *entry, gpointer data)
	{
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		GtkWidget *list;
		GtkTreeModel *model;
		GtkTreeIter row;
		gchar *symbol;
		gchar *temp_symbol;
		gchar *full_symbol;
		gchar *tmp;
		gint symbol_len;
		gint i, j=0;
		gboolean flag = FALSE;
		
		symbol = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
		symbol_len = strlen (symbol);

		full_symbol = (char *) malloc ((symbol_len + 1) * sizeof (char));
		if (!symbol)
			return;

		if (strlen (symbol) < 1) {
			g_free (symbol);
			return;
		}
		
		for (i=0; i<symbol_len; i++) {
			if (symbol[i] == ' ' || symbol[i] == ',') {
				if (flag) {
					full_symbol[j]='+';
					j++;
					flag = FALSE;
				}
				else 
					continue;
			}
			else {
				full_symbol[j] = symbol[i];
				j++;
				flag = TRUE;
			}
		}

		full_symbol[j]= '\0';
		
		tmp = stockdata->props.tik_syms;
		if (tmp)
			stockdata->props.tik_syms = g_strconcat (tmp, "+", full_symbol, NULL);
		else
			stockdata->props.tik_syms = g_strdup (full_symbol);
		
		panel_applet_gconf_set_string (applet, "tik_syms", 
					       stockdata->props.tik_syms, NULL);
		
		if (tmp)			       
			g_free (tmp);
		
		list = g_object_get_data (G_OBJECT (entry), "list");
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));

		temp_symbol = strtok (full_symbol, "+");
		for (; temp_symbol;) {
			gtk_list_store_append (GTK_LIST_STORE (model), &row);
			gtk_list_store_set (GTK_LIST_STORE (model), &row, 
					    0, temp_symbol, -1);
			temp_symbol = strtok (NULL, "+");
		}
					    
		gtk_entry_set_text (entry, "");
		g_free (symbol);
		g_free (full_symbol);
		updateOutput(stockdata);
		
	}
	
	static void
	add_button_clicked (GtkButton *button, gpointer data)
	{
		StockData *stockdata = data;
		GtkEntry *entry = g_object_get_data (G_OBJECT (button), "entry");
		
		add_symbol (entry, stockdata);
		
	}
	
	static gboolean
	get_symbols (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
	 	     gpointer data)
	{
		StockData *stockdata = data;
		gchar *symbol;
		gchar *tmp;
		
		gtk_tree_model_get (model, iter, 0, &symbol, -1);
		if (!symbol)
			return FALSE;
		
		tmp = stockdata->props.tik_syms;
		/* don't add the "+" if the symbol is the first one */
		if (stockdata->props.tik_syms)
			stockdata->props.tik_syms = g_strconcat (tmp, "+", symbol, NULL);
		else
			stockdata->props.tik_syms = g_strdup(symbol);
		g_free (symbol);
		if (tmp)
			g_free (tmp);
			
		return FALSE;
		
	}
	
	static void
	remove_symbol (GtkButton *button, gpointer data)
	{
		StockData *stockdata = data;
		GtkWidget *list;
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		GtkTreeIter iter, next;
		GtkTreePath *path;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		/* FIXME: allow for multiple selection */
		
		list = g_object_get_data (G_OBJECT (button), "list");
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list));
		
		if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
			return;
		
		/* select the next or previous item */
		next = iter;
		if (gtk_tree_model_iter_next (model, &next) )
			gtk_tree_selection_select_iter (selection, &next);
		else {
			path = gtk_tree_model_get_path (model, &iter);
			gtk_tree_path_prev (path);
			gtk_tree_selection_select_path (selection, path);
			gtk_tree_path_free (path);
		}
					
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
				
		/* rebuild symbol table */
		if (stockdata->props.tik_syms)
			g_free (stockdata->props.tik_syms);
		stockdata->props.tik_syms = NULL;
		gtk_tree_model_foreach (model, get_symbols, stockdata);
		if (stockdata->props.tik_syms)
			panel_applet_gconf_set_string (applet, "tik_syms",
					               stockdata->props.tik_syms, NULL);
		else
			panel_applet_gconf_set_string (applet, "tik_syms",
					               "", NULL);
		updateOutput(stockdata);
		
	}
	
	static void
	selection_changed (GtkTreeSelection *selection, gpointer data) 
	{
		soft_set_sensitive (GTK_WIDGET (data), 
						    gtk_tree_selection_get_selected (selection, NULL, NULL));
						    
	}
	
	static void
	entry_changed (GtkEditable *editable, gpointer data)
	{
		gboolean str = TRUE;
		gchar *string;
		
		string = gtk_editable_get_chars (editable, 0, -1);
		if (!string || !g_utf8_strlen (string, -1))
			str = FALSE;
		soft_set_sensitive (GTK_WIDGET (data), str);
		if (string) g_free (string);
	}
		

	static void
	font_cb (GtkFontButton *gfb, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		const gchar *font_name = gtk_font_button_get_font_name (gfb);
		
		if (stockdata->props.font)
			g_free (stockdata->props.font);
		stockdata->props.font = g_strdup (font_name);
		load_fonts (stockdata);
		panel_applet_gconf_set_string (applet,"font",
					       stockdata->props.font, NULL);
	}

        static void populatelist(StockData *stockdata, GtkWidget *list) {
	
		GtkTreeModel *model;
		GtkTreeIter row;
		char **syms;
		int i;
		
		if (stockdata->props.tik_syms == NULL)
			return;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));

		syms = g_strsplit (stockdata->props.tik_syms, "+", 0);

		for (i = 0; syms[i]; i++)
		{
			gtk_list_store_append (GTK_LIST_STORE (model), &row);
			gtk_list_store_set (GTK_LIST_STORE (model), &row, 
					    0, syms[i], -1);
		}
		
		g_strfreev (syms);
	}


	static GtkWidget *symbolManager(StockData *stockdata) { 
		GtkWidget *mainhbox;
		GtkWidget *hbox;
		GtkWidget *list;
		GtkListStore *model;
		GtkTreeViewColumn *column;
		GtkTreeSelection *selection;
  		GtkCellRenderer *cell_renderer;
		GtkWidget *swindow;
		GtkWidget *label;
		GtkWidget *entry;
		static GtkWidget *button;

		model = gtk_list_store_new (1, G_TYPE_STRING);
		list = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
		/*gtk_widget_set_size_request (list, 70, -1);*/
		g_object_unref (G_OBJECT (model));
		
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list));
		/*gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);*/
  		
  		cell_renderer = gtk_cell_renderer_text_new ();
  		column = gtk_tree_view_column_new_with_attributes ("hello",
						    	   cell_renderer,
						     	   "text", 0,
						     	   NULL);
		gtk_tree_view_column_set_sort_column_id (column, 0);
					     	   
		gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
		
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);
  		
		stockdata->tik_syms_entry = list;
		
		swindow = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (swindow), GTK_SHADOW_IN);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swindow),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(swindow),list);

		populatelist(stockdata, list);

		mainhbox = gtk_vbox_new(FALSE, 6);
		gtk_container_set_border_width (GTK_CONTAINER (mainhbox), 12);
		
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (mainhbox), hbox, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("Current _stocks:"));
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), list);
		
		gtk_box_pack_start(GTK_BOX(mainhbox),swindow,TRUE,TRUE,0);
		
		hbox = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (mainhbox), hbox, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("Add _new symbol:"));
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
		entry = gtk_entry_new();
		g_object_set_data(G_OBJECT(entry),"list",(gpointer)list);
		gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);
		g_signal_connect (G_OBJECT (entry), "activate",
				  G_CALLBACK (add_symbol), stockdata);		
		set_relation (entry, GTK_LABEL (label));
		button = gtk_button_new_from_stock (GTK_STOCK_ADD);
		g_object_set_data (G_OBJECT (button), "entry", entry);
		gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
		soft_set_sensitive (button, FALSE);
		g_signal_connect (G_OBJECT (button), "clicked",
				  G_CALLBACK (add_button_clicked), stockdata);
		g_signal_connect (G_OBJECT (entry), "changed",
					  G_CALLBACK (entry_changed), button);
		button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
		g_object_set_data (G_OBJECT (button), "list", list);
		g_signal_connect (G_OBJECT (button), "clicked",
				  G_CALLBACK (remove_symbol), stockdata);
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
		soft_set_sensitive (button, FALSE);
		g_signal_connect (G_OBJECT (selection), "changed",
					   G_CALLBACK (selection_changed), button);

		gtk_widget_show_all(mainhbox);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "tik_syms"))
			hard_set_sensitive (mainhbox, FALSE);

		return(mainhbox);

	}
	
	
	static void
	phelp_cb (GtkDialog *dialog)
	{
  		GError *error = NULL;

  		gnome_help_display_on_screen (
			"gtik2_applet2", "gtik-settings",
			gtk_window_get_screen (GTK_WINDOW (dialog)),
			&error);

  		if (error) {
     			g_warning ("help error: %s\n", error->message);
     			g_error_free (error);
     			error = NULL;
  		}

		/* FIXME: display error to the user */
	}
	
	static void
	response_cb (GtkDialog *dialog, gint id, gpointer data)
	{
		StockData *stockdata = data;
			if(id == GTK_RESPONSE_HELP){
			phelp_cb (dialog);
			return;
		}

		gtk_widget_destroy (GTK_WIDGET (dialog));
		stockdata->pb = NULL;
		
	}
	
	static GtkWidget *
	create_hig_category (GtkWidget *main_box, gchar *title)
	{
		GtkWidget *vbox, *vbox2, *hbox;
		GtkWidget *label;
		gchar *tmp;
		
		vbox = gtk_vbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (main_box), vbox, FALSE, FALSE, 0);
	
		tmp = g_strdup_printf ("<b>%s</b>", title);
		label = gtk_label_new (NULL);
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_label_set_markup (GTK_LABEL (label), tmp);
		g_free (tmp);
		gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);
	
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	
		label = gtk_label_new ("    ");
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
		vbox2 = gtk_vbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

		return vbox2;
		
	}

	/*-----------------------------------------------------------------*/
	static void properties_cb (BonoboUIComponent *uic, gpointer data, 
				   const gchar *verbname) {
		StockData * stockdata = data;
		GtkWidget * notebook;
		GtkWidget * vbox, *behav_vbox, *appear_vbox;
		GtkWidget *color_vbox;
		GtkWidget *hbox, *hbox2, *hbox3, *font_hbox;
		GtkWidget * label, *spin, *check;
		GtkWidget *color;
		GtkWidget *font;
		GtkWidget *option, *menu, *menuitem;
		GtkSizeGroup *size, *size2;
		GdkColor gdkcolor;
		int ur, ug, ub;

		if (stockdata->pb) {
			gtk_window_set_screen (GTK_WINDOW (stockdata->pb),
					       gtk_widget_get_screen (stockdata->applet));
			gtk_window_present (GTK_WINDOW (stockdata->pb));
			return;
		}

		stockdata->pb = gtk_dialog_new_with_buttons (_("Stock Ticker Preferences"), 
							     NULL,
						  	     GTK_DIALOG_DESTROY_WITH_PARENT,
						             GTK_STOCK_CLOSE, 
						             GTK_RESPONSE_CLOSE,
						             GTK_STOCK_HELP, 
						             GTK_RESPONSE_HELP,
						  	     NULL);
		gtk_window_set_screen (GTK_WINDOW (stockdata->pb),
				       gtk_widget_get_screen (stockdata->applet));
		gtk_dialog_set_default_response (GTK_DIALOG (stockdata->pb), 
		                                 GTK_RESPONSE_CLOSE);
		gtk_dialog_set_has_separator (GTK_DIALOG (stockdata->pb), FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (stockdata->pb), 5);
		gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (stockdata->pb)->vbox),2);
		gtk_widget_show (GTK_DIALOG (stockdata->pb)->vbox);
		notebook = gtk_notebook_new ();
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (stockdata->pb)->vbox), notebook,
				    TRUE, TRUE, 0);
		gtk_widget_show (notebook);

		hbox = symbolManager(stockdata);
		label = gtk_label_new (_("Symbols"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);
		gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);
		gtk_widget_show_all (hbox);
		
		behav_vbox = gtk_vbox_new (FALSE, 18);
		gtk_container_set_border_width (GTK_CONTAINER (behav_vbox), 12);
		label = gtk_label_new (_("Behavior"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), behav_vbox, label);
		
		vbox = create_hig_category (behav_vbox, _("Update"));
		
		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("Stock update fre_quency:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		
		hbox3 = gtk_hbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (hbox2), hbox3, FALSE, FALSE, 0);
		spin = gtk_spin_button_new_with_range (1, 1000, 1);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), stockdata->props.timeout);
		g_signal_connect (G_OBJECT (spin), "focus_out_event",
					   G_CALLBACK (timeout_cb), stockdata);
		gtk_box_pack_start (GTK_BOX (hbox3), spin, FALSE, FALSE, 0);
		label = gtk_label_new (_("minutes"));
		gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "timeout"))
			hard_set_sensitive (hbox2, FALSE);
		
		vbox = create_hig_category (behav_vbox, _("Scrolling"));
		
		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("_Scroll speed:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		menu = gtk_menu_new ();
		menuitem = gtk_menu_item_new_with_label (_("Slow"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
		menuitem = gtk_menu_item_new_with_label (_("Medium"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
		menuitem = gtk_menu_item_new_with_label (_("Fast"));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);		
		option = gtk_option_menu_new ();
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), option);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (option), menu);
		gtk_box_pack_start (GTK_BOX (hbox2), option, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (option), "changed",
					  G_CALLBACK (scroll_speed_changed), stockdata);
		if (stockdata->props.scroll_speed <=10)
			gtk_option_menu_set_history (GTK_OPTION_MENU (option), 2);
		else if (stockdata->props.scroll_speed <=40)
			gtk_option_menu_set_history (GTK_OPTION_MENU (option), 1);
		else
			gtk_option_menu_set_history (GTK_OPTION_MENU (option), 0);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "scroll_speed"))
			hard_set_sensitive (hbox2, FALSE);
		
		hbox2 = gtk_hbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		check = gtk_check_button_new_with_mnemonic(_("_Enable scroll buttons"));
		gtk_box_pack_start (GTK_BOX (hbox2), check, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (check), "toggled",
				  	  G_CALLBACK (scroll_toggled), stockdata);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
							   stockdata->props.buttons);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "buttons"))
			hard_set_sensitive (hbox2, FALSE);
							   
		hbox2 = gtk_hbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		check = gtk_check_button_new_with_mnemonic(_("Scroll _left to right"));
		gtk_box_pack_start (GTK_BOX (hbox2), check, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (check), "toggled",
				           G_CALLBACK (rtl_toggled), stockdata);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
							   !stockdata->props.scroll);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "scroll"))
			hard_set_sensitive (hbox2, FALSE);
							   
		gtk_widget_show_all (behav_vbox);
							   
		appear_vbox = gtk_vbox_new (FALSE, 18);
		gtk_container_set_border_width (GTK_CONTAINER (appear_vbox), 12);
		label = gtk_label_new (_("Appearance"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), appear_vbox, label);
		gtk_widget_show_all(appear_vbox);
		gtk_widget_show_all (notebook);
		
		size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		
		vbox = create_hig_category (appear_vbox, _("Display"));
		gtk_widget_show_all (vbox);
		
		hbox2 = gtk_hbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		check = gtk_check_button_new_with_mnemonic (_("Displa_y only symbols and price"));
		gtk_box_pack_start (GTK_BOX (hbox2), check, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (check), "toggled",
				           G_CALLBACK (output_toggled), stockdata);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
							   stockdata->props.output);
		gtk_widget_show_all (hbox2);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "output"))
			hard_set_sensitive (hbox2, FALSE);

		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("_Width:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size, label);
		gtk_widget_show_all (hbox2);
		
		hbox3 = gtk_hbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (hbox2), hbox3, FALSE, FALSE, 0);
		spin = gtk_spin_button_new_with_range (20, 500, 10);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), spin);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), stockdata->props.width);
		g_signal_connect (G_OBJECT (spin), "value_changed",
					   G_CALLBACK (width_changed), stockdata);
		gtk_box_pack_start (GTK_BOX (hbox3), spin, FALSE, FALSE, 0);
		label = gtk_label_new (_("pixels"));
		gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);
		gtk_widget_show_all (hbox2);	
		
		g_object_unref (size);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "width"))
			hard_set_sensitive (hbox2, FALSE);
		
		vbox = create_hig_category (appear_vbox, _("Font and Colors"));
		size = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		size2= gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
				
		hbox2 = gtk_hbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		check = gtk_check_button_new_with_mnemonic (_("Use _default theme font and colors"));
		gtk_box_pack_start (GTK_BOX (hbox2), check, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (check), "toggled",
				           G_CALLBACK (def_font_toggled), stockdata);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check),
							   stockdata->props.use_default_theme);
		gtk_widget_show_all (hbox2);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "use_default_theme"))
			hard_set_sensitive (hbox2, FALSE);
					   
		hbox2 = gtk_hbox_new (FALSE, 12);
		font_hbox = hbox2;
		g_object_set_data (G_OBJECT (check), "fonts_hbox", hbox2);
		gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("_Font:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size, label);
		font = gtk_font_button_new ();
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), font);
		if (stockdata->props.font)
			gtk_font_button_set_font_name (GTK_FONT_BUTTON (font),
						 stockdata->props.font);
		gtk_box_pack_start (GTK_BOX (hbox2), font, FALSE, FALSE, 0);
		g_signal_connect (G_OBJECT (font), "font_set",
                		  G_CALLBACK (font_cb), stockdata);
                gtk_size_group_add_widget (size2, font);
		gtk_widget_show_all (hbox2);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "font"))
			hard_set_sensitive (hbox2, FALSE);
               	
		color_vbox = gtk_vbox_new (FALSE, 6);
		g_object_set_data (G_OBJECT (check), "colors_hbox", color_vbox);
		gtk_box_pack_start (GTK_BOX (vbox), color_vbox, TRUE, TRUE, 0);
				   
		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (color_vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("Stock _raised:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size, label);
		color = gtk_color_button_new ();
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), color);
		sscanf( stockdata->props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );	
		gdkcolor.red = ur << 8;
		gdkcolor.green = ug << 8;
		gdkcolor.blue = ub << 8;
		gtk_color_button_set_color (GTK_COLOR_BUTTON (color), &gdkcolor);
		gtk_box_pack_start (GTK_BOX (hbox2), color, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size2, color);
		g_signal_connect(G_OBJECT(color), "color_set",
				G_CALLBACK(ucolor_set_cb), stockdata);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "ucolor"))
			hard_set_sensitive (hbox2, FALSE);
				
		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (color_vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("Stock _lowered:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size, label);
		color = gtk_color_button_new ();
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), color);
		sscanf( stockdata->props.dcolor, "#%02x%02x%02x", &ur,&ug,&ub );	
		gdkcolor.red = ur << 8;
		gdkcolor.green = ug << 8;
		gdkcolor.blue = ub << 8;
		gtk_color_button_set_color (GTK_COLOR_BUTTON (color), &gdkcolor);
		gtk_box_pack_start (GTK_BOX (hbox2), color, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size2, color);
		g_signal_connect(G_OBJECT(color), "color_set",
				G_CALLBACK(dcolor_set_cb), stockdata);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "dcolor"))
			hard_set_sensitive (hbox2, FALSE);
				
		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (color_vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("Stock _unchanged:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size, label);
		color = gtk_color_button_new ();
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), color);
		sscanf( stockdata->props.fgcolor, "#%02x%02x%02x", &ur,&ug,&ub );	
		gdkcolor.red = ur << 8;
		gdkcolor.green = ug << 8;
		gdkcolor.blue = ub << 8;
		gtk_color_button_set_color (GTK_COLOR_BUTTON (color), &gdkcolor);
		gtk_box_pack_start (GTK_BOX (hbox2), color, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size2, color);
		g_signal_connect(G_OBJECT(color), "color_set",
				G_CALLBACK(fgcolor_set_cb), stockdata);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "fgcolor"))
			hard_set_sensitive (hbox2, FALSE);
				
		hbox2 = gtk_hbox_new (FALSE, 12);
		gtk_box_pack_start (GTK_BOX (color_vbox), hbox2, FALSE, FALSE, 0);
		label = gtk_label_new_with_mnemonic (_("_Background:"));
		gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size, label);
		color = gtk_color_button_new ();
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), color);
		sscanf( stockdata->props.bgcolor, "#%02x%02x%02x", &ur,&ug,&ub );	
		gdkcolor.red = ur << 8;
		gdkcolor.green = ug << 8;
		gdkcolor.blue = ub << 8;
		gtk_color_button_set_color (GTK_COLOR_BUTTON (color), &gdkcolor);
		gtk_box_pack_start (GTK_BOX (hbox2), color, FALSE, FALSE, 0);
		gtk_size_group_add_widget (size2, color);
		g_signal_connect(G_OBJECT(color), "color_set",
				G_CALLBACK(bgcolor_set_cb), stockdata);

		if ( ! key_writable (PANEL_APPLET (stockdata->applet), "bgcolor"))
			hard_set_sensitive (hbox2, FALSE);
				
		gtk_widget_show_all (vbox);
				
		g_object_unref (G_OBJECT (size));
		g_object_unref (G_OBJECT (size2));
	
		gtk_widget_show_all(stockdata->pb);
		
		soft_set_sensitive (font_hbox, !stockdata->props.use_default_theme);
		soft_set_sensitive (color_vbox, !stockdata->props.use_default_theme);
		
		g_signal_connect (G_OBJECT (stockdata->pb), "response",
				  G_CALLBACK (response_cb), stockdata);

	}

	/* This is a hack around the fact that gtk+ doesn't
 	* propogate button presses on button2/3.
 	*/
	static gboolean
	button_press_hack (GtkWidget      *widget,
                           GdkEventButton *event,
                           gpointer data)
	{
		StockData *stockdata = data;
		GtkWidget *applet = GTK_WIDGET (stockdata->applet);
    		if (event->button == 3 || event->button == 2) {
			gtk_propagate_event (applet, (GdkEvent *) event);

			return TRUE;
		}
		return FALSE;
    	}
    	
    	static void
    	applet_change_size (PanelApplet *applet, guint size, StockData *stockdata)
    	{
    	
    		gtk_drawing_area_size(GTK_DRAWING_AREA (stockdata->drawing_area),
				      stockdata->props.width,size-4);
    	
    	}
    	
    	static void
    	style_set_cb (GtkWidget *widget, GtkStyle *previous_style, gpointer data)
    	{
    		StockData *stockdata = data;
    		
    		if (stockdata->props.use_default_theme)
    			load_fonts (stockdata);
    	}

	static gboolean 
	key_press_cb (GtkWidget *widget, GdkEventKey *event, StockData *stockdata)
	{
		switch (event->keyval) {	
		case GDK_u:
			if (event->state == GDK_CONTROL_MASK)
				updateOutput(stockdata);
			return TRUE;
		default:
			break;
		}

		return FALSE;

	}
    
	static const BonoboUIVerb gtik_applet_menu_verbs [] = {
        	BONOBO_UI_UNSAFE_VERB ("Props", properties_cb),
        	BONOBO_UI_UNSAFE_VERB ("Refresh", refresh_cb),
        	BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
        	BONOBO_UI_UNSAFE_VERB ("About", about_cb),

        	BONOBO_UI_VERB_END
	};

	/*-----------------------------------------------------------------*/
	static gboolean gtik_applet_fill (PanelApplet *applet){
		StockData *stockdata;
		GtkWidget * vbox;
		GtkWidget * frame;
		GtkWidget *event;
		GtkWidget *left_arrow;
		GtkWidget *right_arrow;

		gnome_vfs_init();
		
		gtk_window_set_default_icon_name ("gnome-money");
		panel_applet_add_preferences (applet, "/schemas/apps/gtik/prefs", NULL);
		panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
		
		access_stock = stockdata = g_new0 (StockData, 1);
		stockdata->applet = GTK_WIDGET (applet);
		stockdata->timeout = 0;
		stockdata->delta = 1;
		stockdata->vfshandle = NULL;
		stockdata->configFileName = g_strconcat (g_getenv ("HOME"), 
						         "/.gtik.conf", NULL);

		stockdata->quotes = g_array_new(FALSE, FALSE, sizeof(StockQuote));
		properties_load(stockdata);	

		vbox = gtk_hbox_new (FALSE,0);
		stockdata->leftButton = gtk_button_new();
		stockdata->rightButton = gtk_button_new();
		left_arrow = gtk_arrow_new (GTK_ARROW_LEFT, GTK_SHADOW_OUT);
		right_arrow = gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
		gtk_container_add (GTK_CONTAINER (stockdata->leftButton),
				   left_arrow);
		gtk_container_add (GTK_CONTAINER (stockdata->rightButton),
				   right_arrow);
		gtk_widget_show_all (stockdata->leftButton);
		gtk_widget_show_all (stockdata->rightButton);				
		g_signal_connect (GTK_OBJECT (stockdata->leftButton),
					"clicked",
					GTK_SIGNAL_FUNC(zipLeft),stockdata);
		g_signal_connect (GTK_OBJECT (stockdata->rightButton),
					"clicked",
					GTK_SIGNAL_FUNC(zipRight),stockdata);
		g_signal_connect (G_OBJECT (stockdata->leftButton), 
				  "button_press_event",
				  G_CALLBACK (button_press_hack), stockdata);
		g_signal_connect (G_OBJECT (stockdata->rightButton), 
				  "button_press_event",
				  G_CALLBACK (button_press_hack), stockdata);
		stockdata->tooltips = gtk_tooltips_new ();
		gtk_tooltips_set_tip(stockdata->tooltips, stockdata->leftButton, _("Skip forward"), NULL);
		gtk_tooltips_set_tip(stockdata->tooltips, stockdata->rightButton, _("Skip backward"), NULL);
		
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		
		event = gtk_event_box_new ();
		gtk_widget_show (event);
		gtk_container_add (GTK_CONTAINER(frame), event);
		gtk_tooltips_set_tip(stockdata->tooltips, event, _("Stock Ticker\nGet continuously updated stock quotes"), NULL);

		access_drawing_area = stockdata->drawing_area = GTK_WIDGET (custom_drawing_area_new());
		g_object_add_weak_pointer (G_OBJECT (access_drawing_area), (gpointer *)&access_drawing_area);
		gtk_drawing_area_size(GTK_DRAWING_AREA (stockdata->drawing_area),
						stockdata->props.width,-1);

		gtk_widget_show(stockdata->drawing_area);

		stockdata->layout = gtk_widget_create_pango_layout (stockdata->drawing_area, "");

		gtk_container_add(GTK_CONTAINER(event),stockdata->drawing_area);
		
		gtk_widget_show(frame);

		gtk_box_pack_start(GTK_BOX(vbox),stockdata->leftButton,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX (vbox), frame,TRUE,TRUE,0);
		gtk_box_pack_start(GTK_BOX(vbox),stockdata->rightButton,FALSE,FALSE,0);

		gtk_widget_show(vbox);

		gtk_container_add (GTK_CONTAINER (applet), vbox);

		g_signal_connect(GTK_OBJECT(stockdata->drawing_area),"expose_event",
				 (GtkSignalFunc) expose_event, stockdata);

		g_signal_connect(GTK_OBJECT(stockdata->drawing_area),"configure_event",
				 (GtkSignalFunc) configure_event, stockdata);


		/* CLEAN UP WHEN REMOVED FROM THE PANEL */
		g_signal_connect(GTK_OBJECT(applet), "destroy",
				 GTK_SIGNAL_FUNC(destroy_applet), stockdata);

		g_signal_connect (G_OBJECT (applet), "change_size",
				  G_CALLBACK (applet_change_size), stockdata);
				  
		g_signal_connect (G_OBJECT (applet), "style_set",
				  G_CALLBACK (style_set_cb), stockdata);
					   
		g_signal_connect (applet, "key_press_event",
				  G_CALLBACK (key_press_cb), stockdata);

		gtk_widget_show (GTK_WIDGET (applet));
		
		gtk_widget_realize (stockdata->drawing_area);
		create_gc(stockdata);

		properties_set(stockdata,FALSE);
		
		panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                                                   DATADIR,
				                   "GNOME_GtikApplet.xml",
                                                   NULL,
				                   gtik_applet_menu_verbs,
				                   stockdata);

		if (panel_applet_get_locked_down (PANEL_APPLET (applet))) {
			BonoboUIComponent *popup_component;

			popup_component = panel_applet_get_popup_component (PANEL_APPLET (applet));

			bonobo_ui_component_set_prop (popup_component,
						      "/commands/Props",
						      "hidden", "1",
						      NULL);
		}

		/* KEEPING TIMER ID FOR CLEANUP IN DESTROY */
		stockdata->drawTimeID = gtk_timeout_add(stockdata->props.scroll_speed,
						        Repaint,stockdata);
		stockdata->updateTimeID = gtk_timeout_add(stockdata->props.timeout * 60000,
				                          updateOutput,stockdata);


		if (stockdata->props.buttons == TRUE) {
			gtk_widget_show(stockdata->leftButton);
			gtk_widget_show(stockdata->rightButton);
		}

		setup_refchild_nchildren(vbox);

		updateOutput(stockdata);

		return TRUE;
	}
	
	static gboolean
	gtik_applet_factory (PanelApplet *applet,
			     const gchar *iid,
			     gpointer     data)
	{
		gboolean retval = FALSE;
    
		if (!strcmp (iid, "OAFIID:GNOME_GtikApplet"))
			retval = gtik_applet_fill (applet); 
    
		return retval;
	}

	PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_GtikApplet_Factory",
				PANEL_TYPE_APPLET,
			     	"gtik2_applet2",
			     	"0",
			     	gtik_applet_factory,
			     	NULL)




/* JHACK */
	/*-----------------------------------------------------------------*/
	static void destroy_applet(GtkWidget *widget, gpointer data) {
		StockData *stockdata = data;

		g_object_unref (G_OBJECT (stockdata->layout));
		
		if (stockdata->drawTimeID > 0) { 
			gtk_timeout_remove(stockdata->drawTimeID); }
		if (stockdata->updateTimeID >0) { 
			gtk_timeout_remove(stockdata->updateTimeID); }
		/*gtk_widget_destroy(stockdata->drawing_area);*/
		
		if (stockdata->configFileName)
			g_free (stockdata->configFileName);
		if (stockdata->my_font)
			pango_font_description_free (stockdata->my_font);

		if (stockdata->about_dialog)
			 gtk_widget_destroy (stockdata->about_dialog); 
		
		if (stockdata->pb)
			gtk_widget_destroy (stockdata->pb);
	}




	
/*HERE*/
	/*-----------------------------------------------------------------*/
	static void reSetOutputArray(StockData *stockdata) {
		GArray *quotes = stockdata->quotes;
		while (quotes->len) {
			int i = quotes->len - 1;
			g_free(STOCK_QUOTE(quotes->data)[i].price);
			g_free(STOCK_QUOTE(quotes->data)[i].change);
			g_array_remove_index(quotes, i);
		}

		stockdata->setCounter = 0;
	}	


	/*-----------------------------------------------------------------*/
	char *splitPrice(char *data) {
		char buff[MAX_SYMBOL_LEN]="";
		static char buff2[MAX_SYMBOL_LEN]="";
		char *var1, *var2;

		strcpy(buff,data);
		var1 = strtok(buff,":");
		var2 = strtok(NULL,":");

		if (var2)
			sprintf(buff2,"  %s %s",var1,var2);
		else 				/* not connected, no data */
			sprintf(buff2,"  %s",var1);

		return(buff2);
	}

	/*-----------------------------------------------------------------*/
	char *splitChange(StockData *stockdata, char *data, StockQuote *quote) {
		char *buff, *buff2;
		char *var1, *var2, *var3, *var4;

		buff = g_strdup (data);
		var1 = strtok(buff,":");
		var2 = strtok(NULL,":");
		var3 = strtok(NULL,":");
		var4 = strtok(NULL,"");

		if (!var3 || !var4)
		{
			g_free (buff);
			return NULL;
		}

		if (var4[0] == '+') { 
			quote->color = GREEN;
			buff2 = g_strdup_printf (_("+%s (%s)"), var3, &var4[1]);
		}
		else if (var4[0] == '-') {
			quote->color = RED;
			buff2 = g_strdup_printf (_("%s (%s)"), var3, &var4[1]);
		}
		else {
			buff2 = g_strdup(_("(No Change)"));
			quote->color = WHITE;
		}
		g_free (buff);

		return(buff2);
	}

	/*-----------------------------------------------------------------*/
	static void setOutputArray(StockData *stockdata, char *param1) {
		StockQuote quote;
		char *price;
		char *change;
		PangoRectangle rect;

		price = splitPrice(param1);		
		change = splitChange(stockdata, param1, &quote);

		quote.price = g_strdup(price);
		pango_layout_set_text (stockdata->layout, price, -1);
		pango_layout_get_pixel_extents (stockdata->layout, NULL, &rect);
		quote.pricelen = rect.width;
		quote.priceheight = rect.height;
		if (change) {
			quote.change = g_strdup(change);
			pango_layout_set_text (stockdata->layout, change, -1);
			pango_layout_get_pixel_extents (stockdata->layout, NULL, &rect);
			quote.changelen = rect.width;
			quote.changeheight = rect.height;
		}
		else {
			quote.change = g_strdup("");
			quote.changelen = 0;
			quote.changeheight = 0;
		}
		g_free (change);
		 

#if 0
		g_message("Param1: %s\nPrice: %s\nChange: %s\nColor: %d\n\n", param1, price, change, quote.color);
#endif

		g_array_append_val(stockdata->quotes, quote);

		stockdata->setCounter++;
	}



	/*-----------------------------------------------------------------*/

	void setup_colors(StockData *stockdata) {
		GdkColormap *colormap;

		colormap = gtk_widget_get_colormap(stockdata->drawing_area);
		
		gdk_color_parse(stockdata->props.ucolor, &stockdata->gdkUcolor);
		gdk_color_alloc(colormap, &stockdata->gdkUcolor);

		gdk_color_parse(stockdata->props.dcolor, &stockdata->gdkDcolor);
		gdk_color_alloc(colormap, &stockdata->gdkDcolor);
		
		gdk_color_parse(stockdata->props.bgcolor, &stockdata->gdkBGcolor);
		gdk_color_alloc(colormap, &stockdata->gdkBGcolor);
		
		gdk_color_parse(stockdata->props.fgcolor, &stockdata->gdkFGcolor);
		gdk_color_alloc(colormap, &stockdata->gdkFGcolor);


	}

	
	int create_gc(StockData *stockdata) {
		stockdata->gc = gdk_gc_new( stockdata->drawing_area->window );
		gdk_gc_copy(stockdata->gc, stockdata->drawing_area->style->white_gc );
		return 0;
	}

	static void
	ucolor_set_cb (GtkColorButton *cb, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		GdkColor color;
		
		if (stockdata->props.ucolor)
			g_free (stockdata->props.ucolor);
		
		gtk_color_button_get_color (cb, &color);
		red = color.red >> 8;
		gr = color.green >> 8;
		bl = color.blue >> 8;
		stockdata->props.ucolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "ucolor", 
					       stockdata->props.ucolor, NULL);
		
		setup_colors(stockdata);
	}



	static void
	dcolor_set_cb (GtkColorButton *cb, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		GdkColor color;
		
		if (stockdata->props.dcolor)
			g_free (stockdata->props.dcolor);
		
		gtk_color_button_get_color (cb, &color);
		red = color.red >> 8;
		gr = color.green >> 8;
		bl = color.blue >> 8;
		stockdata->props.dcolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "dcolor", 
					       stockdata->props.dcolor, NULL);
		
		setup_colors(stockdata);
	}
	
	static void
	bgcolor_set_cb (GtkColorButton *cb, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		GdkColor color;
		
		if (stockdata->props.bgcolor)
			g_free (stockdata->props.bgcolor);
		
		gtk_color_button_get_color (cb, &color);
		red = color.red >> 8;
		gr = color.green >> 8;
		bl = color.blue >> 8;
		stockdata->props.bgcolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "bgcolor", 
					       stockdata->props.bgcolor, NULL);
		
		setup_colors(stockdata);
	}

	static void
	fgcolor_set_cb (GtkColorButton *cb, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		GdkColor color;
		
		if (stockdata->props.fgcolor)
			g_free (stockdata->props.fgcolor);
		
		gtk_color_button_get_color (cb, &color);
		red = color.red >> 8;
		gr = color.green >> 8;
		bl = color.blue >> 8;
		stockdata->props.fgcolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "fgcolor", 
					       stockdata->props.fgcolor, NULL);
		
		setup_colors(stockdata);
	}

	/*------------------------------------------------------------*/
	void removeSpace(char *buffer) {

		int i;
		int j;
		int len;
		char *buff;

		buff = g_new (char, strlen(buffer)+1);
		
		j = 0;
		if(strlen(buffer) > 1) {
			for (i = 0;i<strlen(buffer);i++){
				if ( (unsigned char) buffer[i] > 32 ) {
					strcpy(buff+j,buffer+i);
					j++;
				}
			}				


			j = 0;

			for (i = 0;i<strlen(buff);i++) {
				if ( (unsigned char) buff[i] > 32 ) {
					j = i;
					break;
				}
			}
			strcpy(buffer,buff+j);
			len = strlen(buffer);
			for (i = len - 1; i >= 0; i--) {
				if ( (unsigned char) buff[i] > 32 ) {
					buffer[i+1] = 0;
					break;
				}
			}
			memset(buff,0,sizeof(buff));
		}

		g_free (buff);
	}

	static void setup_refchild_nchildren(GtkWidget * vbox){
		AtkObject *atk_vbox;
		AtkObjectClass *atk_vbox_class;

		atk_vbox = gtk_widget_get_accessible(vbox);
		atk_vbox_class = ATK_OBJECT_GET_CLASS(atk_vbox);
		atk_vbox_class->get_n_children = gtik_vbox_accessible_get_n_children;
		atk_vbox_class->ref_child = gtik_vbox_accessible_ref_child;
	}

	static gint gtik_vbox_accessible_get_n_children(AtkObject *obj){
		return 1;
	}

	static AtkObject* gtik_vbox_accessible_ref_child(AtkObject *obj, gint i){
		if (i == 0 && access_drawing_area)
			return g_object_ref (gtk_widget_get_accessible(access_drawing_area));
		else
			return NULL;
		
	}
