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

#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h> 
#include <gconf/gconf-client.h>
#include <gtk/gtk.h>
#include <time.h>

#include <libgnomevfs/gnome-vfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <gdk/gdkx.h>
#include "customdrawingarea.h"

#define STOCK_QUOTE(sq) ((StockQuote *)(sq))

	enum {
		WHITE,
		RED,
		GREEN
	};
	
	typedef struct {
		char *price;
		char *change;
		int color;
	} StockQuote;
	
	typedef struct
	{
		char *tik_syms;
		gboolean output;
		gboolean scroll;
		gint timeout;
		gchar *dcolor;
		gchar *ucolor;
		gchar *bgcolor;
		gchar *font;
		gchar *font2;
		gboolean buttons;

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
		GdkColor gdkUcolor,gdkDcolor, gdkBGcolor;

		/* end of COLOR vars */

		char *configFileName;

		/*  properties vars */
 	 
		GtkWidget *tik_syms_entry;
		GtkWidget *proxy_url_entry;
		GtkWidget *proxy_port_entry;
		GtkWidget *proxy_user_entry;
		GtkWidget *proxy_passwd_entry;
		GtkWidget *proxy_auth_button;

		GtkWidget *fontDialog;

		GtkWidget * pb;
		
		gtik_properties props; 
	
		gint timeout;
		gint drawTimeID, updateTimeID;
		/* For fonts */
		PangoFontDescription * my_font;
		gchar * new_font;
		PangoFontDescription * extra_font;
		PangoFontDescription * small_font;
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
	void ucolor_set_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
			   guint a, gpointer data) ;
	void dcolor_set_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
			   guint a, gpointer data) ;
	void bgcolor_set_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
			   guint a, gpointer data) ;

	/* end of color funcs */

	

        void font_cb(GnomeFontPicker *gfp, const gchar *font_name, gpointer data) ;
        gint OkClicked( GtkWidget *widget, void *data ) ;
        gint QuitFontDialog( GtkWidget *widget, void *data ) ;
	/* end font funcs and vars */

	/* accessibility funcs and vars */
	GtkWidget *access_drawing_area;
	StockData *access_stock;

	static void setup_refchild_nchildren(GtkWidget * vbox);
	static gint gtik_vbox_accessible_get_n_children(AtkObject *obj);
	static AtkObject* gtik_vbox_accessible_ref_child(AtkObject *obj, gint i);
	/* end accessibility funcs and vars */

	/*-----------------------------------------------------------------*/
	static void load_fonts(StockData *stockdata)
	{
		if (stockdata->my_font)
			pango_font_description_free (stockdata->my_font);
		stockdata->my_font = pango_font_description_from_string (stockdata->props.font);	

		if (!stockdata->extra_font) {
			stockdata->extra_font = pango_font_description_from_string ("Fixed 14");
		}
		
		if (stockdata->small_font)
			pango_font_description_free (stockdata->small_font);
		stockdata->small_font = pango_font_description_from_string (stockdata->props.font2);

		if ( !stockdata->extra_font  ){
			
			stockdata->extra_font = pango_font_description_from_string ("fixed 12");
			stockdata->symbolfont = 0;
		}
		
		if (!stockdata->my_font) {
			stockdata->my_font = pango_font_description_from_string ("fixed 12");
		}
		if (!stockdata->small_font) {
			stockdata->small_font = pango_font_description_from_string ("fixed 12");
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
				       _("No data available or properties not set"));
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
	GnomeVFSAsyncHandle *vfshandle;

	if (stockdata->vfshandle != NULL)
		return FALSE;
		
	source_text_uri = g_strconcat("http://finance.yahoo.com/q?s=",
				      stockdata->props.tik_syms,
				      "&d=v2",
				      NULL);

	source_uri = gnome_vfs_uri_new(source_text_uri);
	sources = g_list_append(NULL, source_uri);
	g_free(source_text_uri);
	dest_text_uri = g_strconcat ("file://",stockdata->configFileName, NULL);

	dest_uri = gnome_vfs_uri_new(dest_text_uri);
	dests = g_list_append(NULL, dest_uri);
	g_free (dest_text_uri);

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
	
	return FALSE;
}




	/*-----------------------------------------------------------------*/
	static void properties_load(StockData *stockdata) {
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		stockdata->props.tik_syms = panel_applet_gconf_get_string (applet,
									   "tik_syms",
									   NULL);
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
		stockdata->props.font = panel_applet_gconf_get_string (applet,
								       "font",
								       NULL);
		if (!stockdata->props.font)
			stockdata->props.font = g_strdup ("fixed 12");
		stockdata->props.font2 = panel_applet_gconf_get_string (applet,
									"font2",
									NULL);
		if (!stockdata->props.font2)
			stockdata->props.font2 = g_strdup ("fixed 12");
		
		stockdata->props.buttons = panel_applet_gconf_get_bool(applet,
									  "buttons",
									  NULL);
									
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
	static char *extractText(const char *line) {

		int  i=0;
		int  j=0;
		static char Text[256]="";

		if (line == NULL) {
			Text[0] = '\0';
			return Text;
		}

		while (i < (strlen(line) -1)) {
			if (line[i] != '>')
				i++;
			else {
				i++;
				while (line[i] != '<') {
					Text[j] = line[i];
					i++;j++;
				}
			}
		}
		Text[j] = '\0';
		i = 0;
		while (i < (strlen(Text)) ) {
			if (Text[i] < 32)
				Text[i] = '\0';
			i++;
		}
		return(Text);
				
	}

	/*-----------------------------------------------------------------*/
	static char *parseQuote(FILE *CONFIG, char line[512]) {
		
		char symbol[512];
		char buff[512];
		char price[16];
		char change[16];
		char percent[16];
		static char result[512]="";
		int  linenum=0;
		int AllOneLine=0;
		int flag=0;
		char *section = NULL;
		char *ptr;

		if (strlen(line) > 64) AllOneLine=1;

		if (AllOneLine) {
			strcpy(buff,line);
			while (!flag) {
				if ((ptr=strstr(buff,"</td>"))!=NULL) {
					ptr[0] = '|';
				}	
				else flag=1;
			}
			section = strtok(buff,"|");
		}
		/* Get the stock symbol */
		if (!AllOneLine) strcpy(symbol,extractText(line));
		else strcpy(symbol,extractText(section));
		linenum++;

		/* Skip the time... */
		if (!AllOneLine) fgets(line,255,CONFIG);
		else section = strtok(NULL,"|");
		linenum++;

		while (linenum < 8 ) {
			if (!AllOneLine) {
				fgets(line,255,CONFIG);
			
				if (strstr(line,
			  	 "<td align=center nowrap colspan=2>")) {
					strcpy(change,"");
					strcpy(percent,"");
					linenum=100;
				}
			}
			else {
				section = strtok(NULL,"|");
				if (strstr(section,
			  	 "<td align=center nowrap colspan=2>")) {
					strcpy(change,"");
					strcpy(percent,"");
					linenum=100;
				}
			}

			if (linenum == 2) { 
				if (!AllOneLine) 
					strcpy(price,extractText(line));
				else
					strcpy(price,extractText(section));
			}
			else if (linenum == 3) {
				if (!AllOneLine)
					strcpy(change,extractText(line));
				else
					strcpy(change,extractText(section));
			}
			else if (linenum == 4) {
				if (!AllOneLine) 
					strcpy(percent,extractText(line));
				else
					strcpy(percent,extractText(section));
			}
			linenum++;
		}
		sprintf(result,"%s:%s:%s:%s",
				symbol,price,change,percent);			
		return(result);

	}



	/*-----------------------------------------------------------------*/
	int configured(StockData *stockdata) {
		int retVar;

		char  buffer[512];
		static FILE *CONFIG;

		CONFIG = fopen((const char *)stockdata->configFileName,"r");

		retVar = 0;

		/* clear the output variable */
		reSetOutputArray(stockdata);

		if ( CONFIG ) {
			while ( !feof(CONFIG) ) {
				fgets(buffer, sizeof(buffer)-1, CONFIG);

				if (strstr(buffer,
				    "<td nowrap align=left><font face=arial size=-1><a href=\"/q\?s=")) {

				      setOutputArray(stockdata, parseQuote(CONFIG,buffer));
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

	/*-----------------------------------------------------------------*/
	static gint Repaint (gpointer data) {
		StockData *stockdata = data;
		GtkWidget* drawing_area = stockdata->drawing_area;
		GArray *quotes = stockdata->quotes;
		PangoFontDescription *my_font = stockdata->my_font;
		PangoFontDescription *small_font = stockdata->small_font;
		PangoFontDescription *extra_font = stockdata->extra_font;
		GdkGC *gc = stockdata->gc;
		GdkGC *bg;
		GdkRectangle update_rect;
		PangoLayout *layout;
		PangoRectangle logical_rect;
		int	comp;

		/* FOR COLOR */
		char *tmpSym;
		int totalLoc;
		int totalLen;
		int i;

		totalLoc = 0;
		totalLen = 0;
	
		bg = gdk_gc_new (stockdata->pixmap);
		gdk_gc_set_foreground( bg, &stockdata->gdkBGcolor );
		gdk_draw_rectangle (stockdata->pixmap,
				    bg, TRUE, 0,0,
				    drawing_area->allocation.width,
				    drawing_area->allocation.height);
		g_object_unref (bg);

		layout = stockdata->layout;
		for(i=0;i<stockdata->setCounter;i++) {
			pango_layout_set_font_description (layout, my_font);
			pango_layout_set_text (layout,
					       STOCK_QUOTE(quotes->data)[i].price,
					       -1);			
			pango_layout_get_pixel_extents (layout, NULL,
							&logical_rect);
			totalLen += logical_rect.width + 10;
			if (stockdata->props.output == FALSE) {
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					pango_layout_set_font_description (layout, extra_font);
					if (g_utf8_validate (&STOCK_QUOTE(quotes->data)[i].change[0], 1, NULL)) {
						pango_layout_set_text (layout, &STOCK_QUOTE(quotes->data)[i].change[0], 1);
						pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
						totalLen += logical_rect.width + 10;
					}
				}
				pango_layout_set_font_description (layout,
								   small_font);
				pango_layout_set_text (layout,
						       &STOCK_QUOTE(quotes->data)[i].change[1],
						       -1);		
				pango_layout_get_pixel_extents (layout, NULL,
								&logical_rect);
				totalLen += logical_rect.width +10;
			}
		}

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
			if (STOCK_QUOTE(quotes->data)[i].color == GREEN) {
				gdk_gc_set_foreground( gc, &stockdata->gdkUcolor );
			}
			else if (STOCK_QUOTE(quotes->data)[i].color == RED) {
				gdk_gc_set_foreground( gc, &stockdata->gdkDcolor );
			}
			else {
				gdk_gc_copy( gc, drawing_area->style->black_gc );
			}

			tmpSym = STOCK_QUOTE(quotes->data)[i].price;
			pango_layout_set_font_description (layout, my_font);
			pango_layout_set_text (layout,
					       STOCK_QUOTE(quotes->data)[i].price,
					       -1);	
			gdk_draw_layout (stockdata->pixmap, gc,
					 stockdata->location + totalLoc , 3,
					 layout);
			pango_layout_get_pixel_extents (layout, NULL,
							&logical_rect);
			totalLoc += logical_rect.width + 10;


			if (stockdata->props.output == FALSE) {
				tmpSym = STOCK_QUOTE(quotes->data)[i].change;
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					pango_layout_set_font_description (layout, extra_font);
					if (g_utf8_validate (&STOCK_QUOTE(quotes->data)[i].change[0], 1, NULL)) {
						pango_layout_set_text (layout, &STOCK_QUOTE(quotes->data)[i].change[0], 1);
						gdk_draw_layout (stockdata->pixmap,
					     		gc, stockdata->location + totalLoc,
					     		3, layout);
						pango_layout_get_pixel_extents (layout, NULL, &logical_rect);
						totalLoc += logical_rect.width + 5;
					}
				}
				pango_layout_set_font_description (layout,
								   small_font);	
				pango_layout_set_text (layout,
						       &STOCK_QUOTE(quotes->data)[i].change[1],
						       -1);		
				gdk_draw_layout (stockdata->pixmap,
				     gc, stockdata->location + totalLoc,
				     3, layout);
				pango_layout_get_pixel_extents (layout, NULL,
								&logical_rect);
				totalLoc += logical_rect.width + 10;
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
	static void about_cb (BonoboUIComponent *uic, gpointer data, 
			      const gchar *verbname) {
		GtkWidget *about;
		GdkPixbuf *pixbuf;
		GError    *error = NULL;
		gchar     *file;
		static const gchar *authors[] = {
			"Jayson Lorenzen <jayson_lorenzen@yahoo.com>",
			"Jim Garrison <garrison@users.sourceforge.net>",
			"Rached Blili <striker@dread.net>",
			NULL
		};
		
		const gchar *documenters[] = {
			NULL
		};

		const gchar *translator_credits = _("translator_credits");
		
		
		file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
			"gnome-money.png", FALSE, NULL);
		
		pixbuf = gdk_pixbuf_new_from_file (file, &error);
		
		g_free (file);
		
		if (error) {
			g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
			
			g_error_free (error);
		}
		
		about = gnome_about_new (_("Stock Ticker"), VERSION,
		"(C) 2000 Jayson Lorenzen, Jim Garrison, Rached Blili",
		_("This program connects to "
		"a popular site and downloads current stock quotes.  "
		"The GNOME Stock Ticker is a free Internet-based application.  "
		"It comes with ABSOLUTELY NO WARRANTY.  "
		"Do not use the GNOME Stock Ticker for making investment decisions; it is for "
		"informational purposes only."),
		authors,
		documenters,
		strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
		pixbuf);
		
		if (pixbuf)
			gdk_pixbuf_unref (pixbuf);
		
		gnome_window_icon_set_from_file (GTK_WINDOW (about),
			GNOME_ICONDIR"/gnome-money.png");
		
		gtk_widget_show (about);

		return;
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
		gint i;

		current = stockdata->props.scroll;
		stockdata->props.scroll = TRUE;
		stockdata->delta = 150;
		stockdata->MOVE = 0;
		/*for (i=0;i<151;i++) {
			Repaint(stockdata);
		}*/
		Repaint(stockdata);
		stockdata->delta = 1;
		stockdata->props.scroll = current;
	}

	/*-----------------------------------------------------------------*/
	static void zipRight(GtkWidget *widget, gpointer data) {
		StockData *stockdata = data;
		gboolean current;
		gint i;

		current = stockdata->props.scroll;
		stockdata->props.scroll = FALSE;
		stockdata->delta = 150;
		stockdata->MOVE = 0;
		/*
		for (i=0;i<151;i++) {
			Repaint(stockdata);
		}*/
		Repaint(stockdata);
		stockdata->delta = 1;
		stockdata->props.scroll = current;
	}

	/*-----------------------------------------------------------------*/
	static void changed_cb(GnomePropertyBox * pb, gpointer data) {
		gnome_property_box_changed(GNOME_PROPERTY_BOX(pb));
	}

	/*-----------------------------------------------------------------*/

	static void selected_cb(GtkCList *clist,gint row, gint column, 
				GdkEventButton *event, gpointer data) {
		GtkWidget *button;

		button = GTK_WIDGET(data);
		gtk_widget_set_sensitive(button,TRUE);
	}

	/*-----------------------------------------------------------------*/
	static void timeout_cb(GtkSpinButton *spin, gpointer data ) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		gint timeout;
		
		timeout=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
		if (timeout < 1)
			return;
		stockdata->props.timeout = timeout;
		panel_applet_gconf_set_int (applet, "timeout", 
					    stockdata->props.timeout, NULL);
		gtk_timeout_remove(stockdata->updateTimeID);
		stockdata->updateTimeID = gtk_timeout_add(stockdata->props.timeout * 60000,
				                         (gpointer)updateOutput, stockdata);
		
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
	proxy_toggled (GtkToggleButton *button, gpointer data)
	{
		StockData *stockdata = data;
    		GConfClient *client = gconf_client_get_default ();
    		gboolean toggled;	
	
    		toggled = gtk_toggle_button_get_active(button);
    		gtk_widget_set_sensitive(stockdata->proxy_url_entry, toggled);
    		gtk_widget_set_sensitive(stockdata->proxy_user_entry, toggled);
    		gtk_widget_set_sensitive(stockdata->proxy_port_entry, toggled);
    		gtk_widget_set_sensitive(stockdata->proxy_passwd_entry, toggled);
    		gtk_widget_set_sensitive(stockdata->proxy_auth_button, toggled);

    		gconf_client_set_bool (client, "/system/gnome-vfs/use-http-proxy", 
    			   toggled, NULL);		
	}	

	static gboolean
	proxy_port_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
	{
    		GConfClient *client = gconf_client_get_default ();
    		gchar *text;
    		gint port;

    		text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
    		if (!text) 
    			return FALSE;
    		port = atoi (text);
    		g_free (text);
    
    		gconf_client_set_int (client, "/system/gnome-vfs/http-proxy-port", 
    			  port, NULL);
    		
    		return FALSE;

	}

	static void
	proxy_auth_toggled (GtkToggleButton *button, gpointer data)
	{
    		GConfClient *client = gconf_client_get_default ();
    		gboolean toggled;
	
    		toggled = gtk_toggle_button_get_active(button);
    		gconf_client_set_bool (client, "/system/gnome-vfs/use-http-proxy-authorization", 
    			   toggled, NULL);
	}

	static gboolean
	proxy_url_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
	{
    		GConfClient *client = gconf_client_get_default ();
    		gchar *text;

    		text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
    		if (!text) 
    			return FALSE;
    
    		gconf_client_set_string (client, "/system/gnome-vfs/http-proxy-host", 
    			     text, NULL);
    		g_free (text);
    		
    		return FALSE;	
	}

	static gboolean
	proxy_user_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
	{
    		GConfClient *client = gconf_client_get_default ();
    		gchar *text;
   
    		text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
    		if (!text)
   			return FALSE;

    		gconf_client_set_string (client, "/system/gnome-vfs/http-proxy-authorization-user", 
    			     text, NULL); 
    		g_free (text);
    		
    		return FALSE;

	}
	
	static gboolean
	proxy_password_changed (GtkWidget *entry, GdkEventFocus *event, gpointer data)
	{
    		GConfClient *client = gconf_client_get_default ();
    		gchar *text;
   
    		text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
    
    		if (!text)
   			return FALSE;

    		gconf_client_set_string (client, "/system/gnome-vfs/http-proxy-authorization-password", 
    			     text, NULL);
    		g_free (text);
    		
    		return FALSE;
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
		gchar *tmp;
		
		symbol = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
		if (!symbol)
			return;
		
		g_strstrip (symbol);
		
		if (strlen (symbol) < 1) {
			g_free (symbol);
			return;
		}
		
		tmp = stockdata->props.tik_syms;
		if (tmp)
			stockdata->props.tik_syms = g_strconcat (tmp, "+", symbol, NULL);
		else
			stockdata->props.tik_syms = g_strdup (symbol);
		
		panel_applet_gconf_set_string (applet, "tik_syms", 
					       stockdata->props.tik_syms, NULL);
		
		if (tmp)			       
			g_free (tmp);
		
		list = g_object_get_data (G_OBJECT (entry), "list");
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));
		gtk_list_store_append (GTK_LIST_STORE (model), &row);
		gtk_list_store_set (GTK_LIST_STORE (model), &row, 
					    0, symbol, -1);
					    
		gtk_entry_set_text (entry, "");
		g_free (symbol);
		
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
			return;
		
		tmp = stockdata->props.tik_syms;
		/* don't add the "+" if the symbol is the first one */
		if (stockdata->props.tik_syms)
			stockdata->props.tik_syms = g_strconcat (tmp, "+", symbol, NULL);
		else
			stockdata->props.tik_syms = g_strdup(symbol);
		g_free (symbol);
		if (tmp)
			g_free (tmp);
		
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
		
	}

	void font_cb(GnomeFontPicker *gfp, const gchar *font_name, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		if (!font_name)
			return; 
		
		if (stockdata->props.font)
			g_free (stockdata->props.font);
		stockdata->props.font = g_strdup (font_name);
		load_fonts (stockdata);
		panel_applet_gconf_set_string (applet,"font",
					       stockdata->props.font, NULL);

	}

        /*-----------------------------------------------------------------*/
	static void font2_cb(GnomeFontPicker *gfp, const gchar *font_name, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		
		if (!font_name)
			return;
			
		stockdata->props.font2 = g_strdup (font_name);
		load_fonts (stockdata);
		panel_applet_gconf_set_string (applet,"font2",
					       stockdata->props.font2, NULL);

	}

        static void populatelist(StockData *stockdata, GtkWidget *list) {
	
		gchar *symbol;
		gchar *syms;
		gchar *temp;
		GtkTreeModel *model;
		GtkTreeIter row;
		
		if (!stockdata->props.tik_syms)
			return;
			
		syms = g_strdup(stockdata->props.tik_syms);

		if ((temp=strtok(syms,"+")))
			symbol = g_strdup(temp);
		
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));
		while (symbol) {
			gtk_list_store_append (GTK_LIST_STORE (model), &row);
			gtk_list_store_set (GTK_LIST_STORE (model), &row, 
					    0, symbol, -1);
			g_free (symbol);
			if ((temp=strtok(NULL,"+"))) 
				symbol = g_strdup(temp);
			else
				symbol=NULL;
		}
		
		if ((temp=strtok(NULL,""))) {
			symbol = g_strdup(temp);
			gtk_list_store_append (GTK_LIST_STORE (model), &row);
			gtk_list_store_set (GTK_LIST_STORE (model), &row, 
					    0, symbol, -1);
			g_free (symbol);
		}
		
		g_free (syms);
		
	}


	static GtkWidget *symbolManager(StockData *stockdata) { 
		GtkWidget *vbox;
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
		gtk_widget_set_size_request (list, 70, -1);
		g_object_unref (G_OBJECT (model));
		
		/*selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  		gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);*/
  		
  		cell_renderer = gtk_cell_renderer_text_new ();
  		column = gtk_tree_view_column_new_with_attributes ("hello",
						    	   cell_renderer,
						     	   "text", 0,
						     	   NULL);
		gtk_tree_view_column_set_sort_column_id (column, 0);
					     	   
		gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
		
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), FALSE);
  		
		stockdata->tik_syms_entry = list;
		
		/*gtk_signal_connect_object(GTK_OBJECT(clist),"drag_drop",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));*/

		swindow = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swindow),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(swindow),list);

		populatelist(stockdata, list);

		hbox = gtk_hbox_new(FALSE,5);
		vbox = gtk_vbox_new(FALSE,5);

		label = gtk_label_new_with_mnemonic(_("_New Symbol:"));
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

		entry = gtk_entry_new();
		g_object_set_data(G_OBJECT(entry),"list",(gpointer)list);
		gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);
		g_signal_connect (G_OBJECT (entry), "activate",
				  G_CALLBACK (add_symbol), stockdata);
		
		set_relation(entry, GTK_LABEL(label));
		
		button = gtk_button_new_with_mnemonic(_("_Add"));
		g_object_set_data (G_OBJECT (button), "entry", entry);
		gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
		g_signal_connect (G_OBJECT (button), "clicked",
				  G_CALLBACK (add_button_clicked), stockdata);
		
		
		gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

		hbox = gtk_hbox_new(FALSE,5);

		button = gtk_button_new_with_mnemonic(_("_Remove Selected"));
		g_object_set_data (G_OBJECT (button), "list", list);
		g_signal_connect (G_OBJECT (button), "clicked",
				  G_CALLBACK (remove_symbol), stockdata);
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);

		gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
		mainhbox = gtk_hbox_new(FALSE,5);
		gtk_box_pack_start(GTK_BOX(mainhbox),swindow,TRUE,TRUE,0);
		gtk_box_pack_start(GTK_BOX(mainhbox),vbox,FALSE,FALSE,0);


		gtk_widget_show_all(mainhbox);
		return(mainhbox);

	}
	
	static GtkWidget *
	create_proxy_box (StockData *stockdata)
	{
		GtkWidget *vbox;
		GtkWidget *hbox, *button, *label;
		GConfClient *client = gconf_client_get_default ();
		gboolean use_proxy, use_proxy_auth;
    		gint proxy_port;
    		gchar *string;
    		gchar *proxy_url, *proxy_user, *proxy_psswd;
		
		use_proxy = 
    			gconf_client_get_bool (client, 
    					       "/system/gnome-vfs/use-http-proxy", 
    					       NULL);
    		use_proxy_auth = gconf_client_get_bool (client, 
    			"/system/gnome-vfs/use-http-proxy-authorization", 
    			NULL);
    		proxy_url = gconf_client_get_string (client, 
    			"/system/gnome-vfs/http-proxy-host", NULL);
    		proxy_user = gconf_client_get_string (client, 
    		 	"/system/gnome-vfs/http-proxy-authorization-user", NULL);
    		proxy_psswd = gconf_client_get_string (client, 
    		 	"/system/gnome-vfs/http-proxy-authorization-password", NULL);
    		proxy_port = gconf_client_get_int (client, 
    		 	"/system/gnome-vfs/http-proxy-port", NULL);
		
		vbox = gtk_vbox_new (FALSE, 4);
		gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
		
		button = gtk_check_button_new_with_mnemonic (_("_Use HTTP proxy"));
    		gtk_box_pack_start (GTK_BOX (vbox), button, 
    				    FALSE, FALSE, 0);
    		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), use_proxy);
   		g_signal_connect (G_OBJECT (button), "toggled",
    		       		  G_CALLBACK (proxy_toggled), stockdata);

    		hbox = gtk_hbox_new (FALSE, 2);
    		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    		label = gtk_label_new_with_mnemonic (_("_Location :"));
    		gtk_box_pack_start (GTK_BOX (hbox), label, 
    			    FALSE, FALSE, 0);
    
    		stockdata->proxy_url_entry = gtk_entry_new ();
    		gtk_widget_show (stockdata->proxy_url_entry);
    		gtk_box_pack_start (GTK_BOX (hbox), stockdata->proxy_url_entry, 
    			    FALSE, FALSE, 0);
    		g_signal_connect (G_OBJECT (stockdata->proxy_url_entry), "focus_out_event",
    		          G_CALLBACK (proxy_url_changed), stockdata);

    		set_relation (stockdata->proxy_url_entry, GTK_LABEL (label));

    		hbox = gtk_hbox_new (FALSE, 2);
    		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    		label = gtk_label_new_with_mnemonic (_("P_ort :"));
    		gtk_widget_show (label);
    		gtk_box_pack_start (GTK_BOX (hbox), label, 
    			    FALSE, FALSE, 0);
    			
    		stockdata->proxy_port_entry = gtk_entry_new ();
    		gtk_box_pack_start (GTK_BOX (hbox), stockdata->proxy_port_entry, 
    				    FALSE, FALSE, 0);
    		g_signal_connect (G_OBJECT (stockdata->proxy_port_entry), "focus_out_event",
    		          G_CALLBACK (proxy_port_changed), stockdata);
    		set_relation (stockdata->proxy_port_entry, GTK_LABEL (label));
    		
    		stockdata->proxy_auth_button = gtk_check_button_new_with_mnemonic (_("Pro_xy requires a username and password"));
    		gtk_box_pack_start (GTK_BOX (vbox), stockdata->proxy_auth_button, 
    				    FALSE, FALSE, 0);
    		gtk_toggle_button_set_active 
    			(GTK_TOGGLE_BUTTON (stockdata->proxy_auth_button), use_proxy_auth);
        	g_signal_connect (G_OBJECT (stockdata->proxy_auth_button), "toggled",
    		       G_CALLBACK (proxy_auth_toggled), stockdata);
    		       
    		hbox = gtk_hbox_new (FALSE, 2);
    		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    		label = gtk_label_new_with_mnemonic (_("Us_ername:"));
    		gtk_box_pack_start (GTK_BOX (hbox), label, 
    			    FALSE, FALSE, 0);
    
    		stockdata->proxy_user_entry = gtk_entry_new ();
    		gtk_box_pack_start (GTK_BOX (hbox), stockdata->proxy_user_entry, 
    			    FALSE, FALSE, 0);
    		g_signal_connect (G_OBJECT (stockdata->proxy_user_entry), "focus_out_event",
    		          G_CALLBACK (proxy_user_changed), stockdata);

    		set_relation (stockdata->proxy_user_entry, GTK_LABEL (label));

    		hbox = gtk_hbox_new (FALSE, 2);
    		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
    		label = gtk_label_new_with_mnemonic (_("Pass_word:"));
    		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    		stockdata->proxy_passwd_entry = gtk_entry_new ();
    		gtk_entry_set_visibility (GTK_ENTRY (stockdata->proxy_passwd_entry), FALSE);
    		gtk_box_pack_start (GTK_BOX (hbox), stockdata->proxy_passwd_entry, 
    			    FALSE, FALSE, 0);
    		g_signal_connect (G_OBJECT (stockdata->proxy_passwd_entry), 
    			          "focus_out_event",
    		                  G_CALLBACK (proxy_password_changed), stockdata);

    		set_relation (stockdata->proxy_passwd_entry, GTK_LABEL (label));
    		 	
    		gtk_entry_set_text(GTK_ENTRY(stockdata->proxy_url_entry), 
    		       proxy_url ? 
    		       proxy_url : "");
    		gtk_entry_set_text(GTK_ENTRY(stockdata->proxy_passwd_entry), 
    		       proxy_psswd ? 
    		       proxy_psswd : "");
    		gtk_entry_set_text(GTK_ENTRY(stockdata->proxy_user_entry), 
    		       proxy_user ? 
    		       proxy_user : "");
    		string = g_strdup_printf ("%d", proxy_port);
    		gtk_entry_set_text(GTK_ENTRY(stockdata->proxy_port_entry), string);
    		g_free (string); 
    		
    		gtk_widget_set_sensitive (stockdata->proxy_auth_button, use_proxy);
    		gtk_widget_set_sensitive (stockdata->proxy_url_entry, use_proxy);
    		gtk_widget_set_sensitive (stockdata->proxy_port_entry, use_proxy);
    		gtk_widget_set_sensitive (stockdata->proxy_passwd_entry, use_proxy);
    		gtk_widget_set_sensitive (stockdata->proxy_user_entry, use_proxy);
    		
    		g_free (proxy_url);
    		g_free (proxy_psswd);
    		g_free (proxy_user);
    			
		gtk_widget_show_all (vbox);
		return vbox;
	} 
	
	static void
	response_cb (GtkDialog *dialog, gint id, gpointer data)
	{
		StockData *stockdata = data;
		gtk_widget_destroy (GTK_WIDGET (dialog));
		stockdata->pb = NULL;
		
	}

	/*-----------------------------------------------------------------*/
	static void properties_cb (BonoboUIComponent *uic, gpointer data, 
				   const gchar *verbname) {
		StockData * stockdata = data;
		GtkWidget * notebook;
		GtkWidget * vbox;
		GtkWidget * vbox2;
		GtkWidget * vbox3, * vbox4;
		GtkWidget * hbox3;
		GtkWidget *hbox;
		GtkWidget * label;
		GtkWidget *panela, *panel1 ,*panel2;
		GtkWidget *label1, *label5;
		GtkWidget *timeout_label,*timeout_c;
		GtkObject *timeout_a;
		GtkWidget *upColor, *downColor, *upLabel, *downLabel;
		GtkWidget *bgColor, *bgLabel;
		GtkWidget *check, *check2, *check4, *fontButton;
		GtkWidget *font_picker;

		int ur,ug,ub, dr,dg,db; 
		
		if (stockdata->pb) {
			gtk_window_present (GTK_WINDOW (stockdata->pb));
			return;
		}

		stockdata->pb = gtk_dialog_new_with_buttons (_("Stock Ticker Preferences"), 
							     NULL,
						  	     GTK_DIALOG_DESTROY_WITH_PARENT,
						             GTK_STOCK_CLOSE, 
						             GTK_RESPONSE_CLOSE,
						  	     NULL);
		
		notebook = gtk_notebook_new ();
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (stockdata->pb)->vbox), notebook,
				    TRUE, TRUE, 0);
				    
		vbox = gtk_vbox_new(GNOME_PAD, FALSE);
		vbox2 = gtk_vbox_new(GNOME_PAD, FALSE);

		panela = gtk_hbox_new(FALSE, 5); /* Color selection */
		panel1 = gtk_hbox_new(FALSE, 5); /* Symbol list */
		panel2 = gtk_hbox_new(FALSE, 5); /* Update timer */

		gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD);
		gtk_container_set_border_width(GTK_CONTAINER(vbox2), GNOME_PAD);

		timeout_label = gtk_label_new_with_mnemonic(_("Update Fre_quency in minutes:"));
		timeout_a = gtk_adjustment_new( stockdata->props.timeout, 1, 128, 
					       1, 8, 8 );
		timeout_c  = gtk_spin_button_new( GTK_ADJUSTMENT(timeout_a), 1, 0 );
		gtk_widget_set_usize(timeout_c,60,-1);

		set_relation(timeout_c, GTK_LABEL(timeout_label));

		gtk_box_pack_start_defaults( GTK_BOX(panel2), timeout_label );
		gtk_box_pack_start_defaults( GTK_BOX(panel2), timeout_c );
		gtk_box_pack_start(GTK_BOX(vbox), panel2, FALSE,
				    FALSE, GNOME_PAD);
				    
		g_signal_connect (G_OBJECT (timeout_c), "value_changed",
				  G_CALLBACK (timeout_cb), stockdata);
		gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(timeout_c),
						   GTK_UPDATE_ALWAYS );
						   
		label1 = gtk_label_new(_("Enter symbols delimited with \"+\" in the box below."));

		stockdata->tik_syms_entry = gtk_entry_new_with_max_length(60);

		gtk_entry_set_text(GTK_ENTRY(stockdata->tik_syms_entry), 
			stockdata->props.tik_syms ? stockdata->props.tik_syms : "");
		gtk_signal_connect_object(GTK_OBJECT(stockdata->tik_syms_entry), 
					  "changed",GTK_SIGNAL_FUNC(changed_cb),
					  GTK_OBJECT(stockdata->pb));

		check = gtk_check_button_new_with_mnemonic(_("Displa_y only symbols and price"));
		g_signal_connect (G_OBJECT (check), "toggled",
				  G_CALLBACK (output_toggled), stockdata);
		check2 = gtk_check_button_new_with_mnemonic(_("Scroll _left to right"));
		g_signal_connect (G_OBJECT (check2), "toggled",
				  G_CALLBACK (rtl_toggled), stockdata);
		check4 = gtk_check_button_new_with_mnemonic(_("_Enable scroll buttons"));
		g_signal_connect (G_OBJECT (check4), "toggled",
				  G_CALLBACK (scroll_toggled), stockdata);


		gtk_box_pack_start(GTK_BOX(vbox2), check, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox2), check2, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), check4, FALSE, FALSE, GNOME_PAD);


		/* Set the checkbox according to current prefs */
		if (stockdata->props.output == TRUE)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
							TRUE);
		if (stockdata->props.scroll == FALSE)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check2),
							TRUE);
		if (stockdata->props.buttons == TRUE)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check4),
							TRUE);
		
		/* COLOR */
		upLabel = gtk_label_new_with_mnemonic(_("+ C_olor"));
		upColor = gnome_color_picker_new();
		
		set_relation(upColor, GTK_LABEL(upLabel));
	
		sscanf( stockdata->props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
	
		gnome_color_picker_set_i8(GNOME_COLOR_PICKER (upColor), 
					  ur, ug, ub, 255);
		
		gtk_signal_connect(GTK_OBJECT(upColor), "color_set",
				GTK_SIGNAL_FUNC(ucolor_set_cb), stockdata);

		vbox3 = gtk_vbox_new(FALSE, 5); 
		hbox3 = gtk_hbox_new(FALSE, 5);
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), upLabel );
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), upColor );
		gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);

		downLabel = gtk_label_new_with_mnemonic(_("- Colo_r"));
		downColor = gnome_color_picker_new();
		
		set_relation(downColor, GTK_LABEL(downLabel));

		sscanf( stockdata->props.dcolor, "#%02x%02x%02x", &dr,&dg,&db );

		gnome_color_picker_set_i8(GNOME_COLOR_PICKER (downColor), 
					  dr, dg, db, 255);

		gtk_signal_connect(GTK_OBJECT(downColor), "color_set",
				GTK_SIGNAL_FUNC(dcolor_set_cb), stockdata);

		hbox3 = gtk_hbox_new(FALSE, 5);
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), downLabel );
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), downColor );
		gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
		
		bgLabel = gtk_label_new_with_mnemonic(_("Back_ground Color"));
		bgColor = gnome_color_picker_new();
		
		set_relation(bgColor, GTK_LABEL(bgLabel));

		sscanf( stockdata->props.bgcolor, "#%02x%02x%02x", &dr,&dg,&db );

		gnome_color_picker_set_i8(GNOME_COLOR_PICKER (bgColor), 
					  dr, dg, db, 255);

		gtk_signal_connect(GTK_OBJECT(bgColor), "color_set",
				GTK_SIGNAL_FUNC(bgcolor_set_cb), stockdata);

		hbox3 = gtk_hbox_new(FALSE, 5);
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), bgLabel );
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), bgColor );
		gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
		gtk_box_pack_start_defaults(GTK_BOX(panela),vbox3);

                /* For FONTS */
		vbox3 = gtk_vbox_new(FALSE, 5); 
		hbox3 = gtk_hbox_new(FALSE, 5);
		label5 = gtk_label_new_with_mnemonic(_("Stock Sy_mbol:"));

		font_picker = gnome_font_picker_new ();
		gnome_font_picker_set_font_name (GNOME_FONT_PICKER (font_picker),
						 stockdata->props.font);
		gtk_box_pack_start_defaults(GTK_BOX(hbox3),label5);
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),font_picker);
                gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
                g_signal_connect (G_OBJECT (font_picker), "font_set",
                		  G_CALLBACK (font_cb), stockdata);

		set_relation(font_picker, GTK_LABEL(label5));
                                
		hbox3 = gtk_hbox_new(FALSE, 5);
		label5 = gtk_label_new_with_mnemonic(_("Stock C_hange:"));
                font_picker = gnome_font_picker_new ();
                gnome_font_picker_set_font_name (GNOME_FONT_PICKER (font_picker),
						 stockdata->props.font2);
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),label5);
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),font_picker);
                gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
                g_signal_connect (G_OBJECT(font_picker),"font_set",
                                  G_CALLBACK(font2_cb),stockdata);
                
		set_relation(font_picker, GTK_LABEL(label5));

		gtk_box_pack_start_defaults(GTK_BOX(panela),vbox3);



		gtk_box_pack_start(GTK_BOX(panel1), label1, FALSE, 
				   FALSE, GNOME_PAD);

		gtk_box_pack_start(GTK_BOX(vbox2), panela, FALSE,
				    FALSE, GNOME_PAD);

		hbox = symbolManager(stockdata);

		label = gtk_label_new_with_mnemonic (_("_Symbols"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);
		label = gtk_label_new_with_mnemonic (_("_Behavior"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
		label = gtk_label_new_with_mnemonic (_("_Appearance"));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox2, label);
		label = gtk_label_new_with_mnemonic (_("_Proxy"));
		vbox4 = create_proxy_box (stockdata);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox4, label);

		gtk_widget_show_all(stockdata->pb);
		
		g_signal_connect (G_OBJECT (stockdata->pb), "response",
				  G_CALLBACK (response_cb), stockdata);

	}

	static const BonoboUIVerb gtik_applet_menu_verbs [] = {
        	BONOBO_UI_VERB ("Props", properties_cb),
        	BONOBO_UI_VERB ("Refresh", refresh_cb),
        	BONOBO_UI_VERB ("About", about_cb),

        	BONOBO_UI_VERB_END
	};

	/*-----------------------------------------------------------------*/
	static gboolean gtik_applet_fill (PanelApplet *applet){
		StockData *stockdata;
		GtkWidget * vbox;
		GtkWidget * frame;

		gnome_vfs_init();
		
		panel_applet_add_preferences (applet, "/schemas/apps/gtik/prefs", NULL);
		
		access_stock = stockdata = g_new0 (StockData, 1);
		stockdata->applet = GTK_WIDGET (applet);
		stockdata->timeout = 0;
		stockdata->delta = 1;
		stockdata->vfshandle = NULL;
		stockdata->configFileName = g_strconcat (g_getenv ("HOME"), 
						         "/.gtik.conf", NULL);

		stockdata->quotes = g_array_new(FALSE, FALSE, sizeof(StockQuote));	

		vbox = gtk_hbox_new (FALSE,0);
		stockdata->leftButton = gtk_button_new_with_label("<<");
		stockdata->rightButton = gtk_button_new_with_label(">>");
		gtk_signal_connect (GTK_OBJECT (stockdata->leftButton),
					"clicked",
					GTK_SIGNAL_FUNC(zipLeft),stockdata);
		gtk_signal_connect (GTK_OBJECT (stockdata->rightButton),
					"clicked",
					GTK_SIGNAL_FUNC(zipRight),stockdata);

		stockdata->tooltips = gtk_tooltips_new ();
		gtk_tooltips_set_tip(stockdata->tooltips, stockdata->leftButton, _("Skip forward"), NULL);
		gtk_tooltips_set_tip(stockdata->tooltips, stockdata->rightButton, _("Skip backword"), NULL);
		gtk_tooltips_set_tip(stockdata->tooltips, stockdata->applet, _("Stock Ticker\nGet continuously updated stock quotes"), NULL);


		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

		access_drawing_area = stockdata->drawing_area = GTK_WIDGET (custom_drawing_area_new());
		gtk_drawing_area_size(GTK_DRAWING_AREA (stockdata->drawing_area),200,20);

		gtk_widget_show(stockdata->drawing_area);

		stockdata->layout = gtk_widget_create_pango_layout (stockdata->drawing_area, "");

		gtk_container_add(GTK_CONTAINER(frame),stockdata->drawing_area);
		
		gtk_widget_show(frame);

		gtk_box_pack_start(GTK_BOX(vbox),stockdata->leftButton,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX (vbox), frame,TRUE,TRUE,0);
		gtk_box_pack_start(GTK_BOX(vbox),stockdata->rightButton,FALSE,FALSE,0);

		gtk_widget_show(vbox);

		gtk_container_add (GTK_CONTAINER (applet), vbox);

		gtk_signal_connect(GTK_OBJECT(stockdata->drawing_area),"expose_event",
		(GtkSignalFunc) expose_event, stockdata);

		gtk_signal_connect(GTK_OBJECT(stockdata->drawing_area),"configure_event",
		(GtkSignalFunc) configure_event, stockdata);


		/* CLEAN UP WHEN REMOVED FROM THE PANEL */
		gtk_signal_connect(GTK_OBJECT(applet), "destroy",
			GTK_SIGNAL_FUNC(destroy_applet), stockdata);



		gtk_widget_show (GTK_WIDGET (applet));
		
		create_gc(stockdata);

		properties_load(stockdata);
		properties_set(stockdata,FALSE);
		
		panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                                                   NULL,
				                   "GNOME_GtikApplet.xml",
                                                   NULL,
				                   gtik_applet_menu_verbs,
				                   stockdata);

		/* KEEPING TIMER ID FOR CLEANUP IN DESTROY */
		stockdata->drawTimeID = gtk_timeout_add(40,Repaint,stockdata);
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
    
		setup_factory();
    
		if (!strcmp (iid, "OAFIID:GNOME_GtikApplet"))
			retval = gtik_applet_fill (applet); 
    
		return retval;
	}

	PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_GtikApplet_Factory",
				PANEL_TYPE_APPLET,
			     	"Gtik applet",
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
		if (stockdata->extra_font)
			pango_font_description_free (stockdata->extra_font);
		if (stockdata->small_font)
			pango_font_description_free (stockdata->small_font);
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
		char buff[128]="";
		static char buff2[128]="";
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
	char *splitChange(StockData *stockdata,char *data, StockQuote *quote) {
		char buff[128]="";
		static char buff2[128]="";
		char *var1, *var2, *var3, *var4;

		strcpy(buff,data);
		var1 = strtok(buff,":");
		var2 = strtok(NULL,":");
		var3 = strtok(NULL,":");
		var4 = strtok(NULL,"");

		if (!var3 || !var4)
			return data;

		if (var3[0] == '+') { 
#if 0
			if (stockdata->symbolfont)
				var3[0] = 221;
#endif
			var4[0] = '(';
			quote->color = GREEN;
		}
		else if (var3[0] == '-') {
#if 0
			if (stockdata->symbolfont)
				var3[0] = 223;	
#endif
			var4[0] = '(';
			quote->color = RED;
		}
		else {
			var3 = g_strdup(_("(No"));
			var4 = g_strdup(_("Change "));
			quote->color = WHITE;
		}

		sprintf(buff2,"%s %s)",var3,var4);
		return(buff2);
	}

	/*-----------------------------------------------------------------*/
	static void setOutputArray(StockData *stockdata, char *param1) {
		StockQuote quote;
		char *price;
		char *change;

		price = splitPrice(param1);
		change = splitChange(stockdata, param1, &quote);

		quote.price = g_strdup(price);
		quote.change = g_strdup(change);

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

	}

	
	int create_gc(StockData *stockdata) {
		stockdata->gc = gdk_gc_new( stockdata->drawing_area->window );
		gdk_gc_copy(stockdata->gc, stockdata->drawing_area->style->white_gc );
		return 0;
	}

	void ucolor_set_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
			   guint a, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		
		if (stockdata->props.ucolor)
			g_free (stockdata->props.ucolor);
		
		gnome_color_picker_get_i8 (GNOME_COLOR_PICKER(cp), &red, &gr, &bl, NULL);
		stockdata->props.ucolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "ucolor", 
					       stockdata->props.ucolor, NULL);
		
		setup_colors(stockdata);
	}



	void dcolor_set_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
			   guint a, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		
		if (stockdata->props.dcolor)
			g_free (stockdata->props.dcolor);
		gnome_color_picker_get_i8 (GNOME_COLOR_PICKER(cp), &red, &gr, &bl, NULL);
		stockdata->props.dcolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "dcolor", 
					       stockdata->props.dcolor, NULL);
		
		setup_colors(stockdata);
	}
	
	void bgcolor_set_cb(GnomeColorPicker *cp, guint r, guint g, guint b,
			   guint a, gpointer data) {
		StockData *stockdata = data;
		PanelApplet *applet = PANEL_APPLET (stockdata->applet);
		guint8 red, gr, bl;
		
		if (stockdata->props.bgcolor)
			g_free (stockdata->props.bgcolor);
		gnome_color_picker_get_i8 (GNOME_COLOR_PICKER(cp), &red, &gr, &bl, NULL);
		stockdata->props.bgcolor = g_strdup_printf("#%06X", (red << 16) + (gr << 8) + bl);
		panel_applet_gconf_set_string (applet, "bgcolor", 
					       stockdata->props.bgcolor, NULL);
		
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
		return (gtk_widget_get_accessible(access_drawing_area));
	}

	gchar* gtik_get_text(void) {
		GArray *quotes = access_stock->quotes;
		gchar **strs = NULL;
		gchar *buff;
		gint i;

		strs = g_new0 (gchar*, access_stock->setCounter + 1);
		g_return_if_fail (strs != NULL);

		for(i=0;i<access_stock->setCounter;i++) {
			if (access_stock->props.output == FALSE) {
				strs[i] = g_strdup_printf ("%s  %s",
					STOCK_QUOTE (quotes->data)[i].price,
					STOCK_QUOTE (quotes->data)[i].change);
			}
			else {
				strs[i] = g_strdup_printf ("%s",
					STOCK_QUOTE (quotes->data)[i].price);
			}
		}
		buff = g_strjoinv ("  ", strs);

		g_strfreev (strs);

		return buff;
	}
