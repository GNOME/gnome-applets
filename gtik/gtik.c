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
 
#include <gtk/gtk.h>
#include <time.h>

#include <libgnomevfs/gnome-vfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <gdk/gdkx.h>

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
		char *output;
		char *scroll;
		char *arrows;
		gint timeout;
		gchar *dcolor;
		gchar *ucolor;
		gchar *font;
		gchar *font2;
		gchar *buttons;

	} gtik_properties;

	
	typedef struct 
	{
		GtkWidget *applet;
		GtkWidget *label;

		GdkPixmap *pixmap;
		GtkWidget * drawing_area;
		GtkWidget * leftButton;
		GtkWidget * rightButton;

		int location;
		int MOVE;
	
		GArray *quotes;

		int max_rgb_str_len;
		int max_rgb_str_size;

		int setCounter;

		GdkGC *gc;
		GdkColor gdkUcolor,gdkDcolor;

		/* end of COLOR vars */

		char *configFileName;

		/*  properties vars */
 	 
		GtkWidget *tik_syms_entry;

		GtkWidget *fontDialog;

		GtkWidget * pb;
		gchar *buttons;
		gchar *arrows;
		gchar *scroll;
		gchar *poutput;
	
		gtik_properties props; 
	
		gint timeout;
		gint drawTimeID, updateTimeID;
		/* For fonts */
		GdkFont * my_font;
		gchar * new_font;
		GdkFont * extra_font;
		GdkFont * small_font;
		gint symbolfont;
		gint whichlabel;
	} StockData;
	
	void removeSpace(char *buffer); 
	int configured(StockData *stockdata);
	void timeout_cb( GtkWidget *widget, GtkWidget *spin );
	void properties_save(char *path) ;
	static void destroy_applet(GtkWidget *widget, gpointer data) ;
	char *getSymsFromClist(GtkWidget *clist) ;

	char *splitPrice(char *data);
	char *splitChange(StockData *stockdata, char *data);

	/* FOR COLOR */

	static gint updateOutput(gpointer data) ;
	static void reSetOutputArray(StockData *stockdata) ;
	static void setOutputArray(StockData *stockdata, char *param1) ;
	void setup_colors(StockData *stockdata);
	int create_gc(StockData *stockdata) ;
	void ucolor_set_cb(GnomeColorPicker *cp, gpointer data) ;
	void dcolor_set_cb(GnomeColorPicker *cp, gpointer data) ;

	/* end of color funcs */

	

        gint font_cb( GtkWidget *widget, void *data ) ;
        gint OkClicked( GtkWidget *widget, void *data ) ;
        gint QuitFontDialog( GtkWidget *widget, void *data ) ;
	/* end font funcs and vars */

	/*-----------------------------------------------------------------*/
	static void load_fonts(StockData *stockdata)
	{
		
		if (stockdata->new_font != NULL) {
			if (stockdata->whichlabel == 1)
				stockdata->my_font = gdk_fontset_load(stockdata->new_font);
			else
				stockdata->small_font = gdk_fontset_load(stockdata->new_font);
		}
g_print ("new font \n");
		if (!stockdata->my_font)
			stockdata->my_font = gdk_fontset_load (stockdata->props.font);

		if (!stockdata->extra_font)
			stockdata->extra_font = gdk_fontset_load ("fixed");

		if (!stockdata->small_font)
			stockdata->small_font = gdk_fontset_load (stockdata->props.font2);


		/* If fonts do not load */
		if (!stockdata->my_font)
			g_error("Could not load fonts!");

		if ( !stockdata->extra_font || (strcmp(stockdata->props.arrows,"noArrows")) == 0 ) {
			
			if (stockdata->extra_font)
				gdk_font_unref(stockdata->extra_font);
			stockdata->extra_font = gdk_fontset_load("fixed");
			stockdata->symbolfont = 0;
		}
		else {
			if (stockdata->extra_font)
				gdk_font_unref(stockdata->extra_font);
			stockdata->extra_font = gdk_fontset_load ("fixed");
			stockdata->symbolfont = 1;

		}
		if (!stockdata->small_font) {
			if (stockdata->small_font)
				gdk_font_unref(stockdata->small_font);
			stockdata->small_font = gdk_fontset_load("fixed");
		}

	}

/*-----------------------------------------------------------------*/
static void xfer_callback (GnomeVFSAsyncHandle *handle, GnomeVFSXferProgressInfo *info, gpointer data)
{
	StockData *stockdata = data;
g_print ("%d \n", info->phase);
	if (info->phase == GNOME_VFS_XFER_PHASE_COMPLETED) {
g_print ("completed \n");
		if (!configured(stockdata)) {
			reSetOutputArray(stockdata);
			fprintf(stderr, "No data!\n");
			setOutputArray(stockdata,
				       _("No data available or properties not set"));
		}
	}
}

/*-----------------------------------------------------------------*/
static gint updateOutput(gpointer data)
{
	StockData *stockdata = data;
	GList *sources = NULL, *dests = NULL;
	GnomeVFSURI *source_uri, *dest_uri;
	char *source_text_uri;
	GnomeVFSAsyncHandle *vfshandle;

	source_text_uri = g_strconcat("http://finance.yahoo.com/q?s=",
				      stockdata->props.tik_syms,
				      "&d=v2",
				      NULL);
g_print ("%s \n", source_text_uri);
	source_uri = gnome_vfs_uri_new(source_text_uri);
	sources = g_list_append(sources, source_uri);
	g_free(source_text_uri);
g_print ("%s \n", stockdata->configFileName);
	dest_uri = gnome_vfs_uri_new(stockdata->configFileName);
	dests = g_list_append(dests, dest_uri);
#if 1
	if (GNOME_VFS_OK !=
	    gnome_vfs_async_xfer(&vfshandle, sources, dests,
				 GNOME_VFS_XFER_DEFAULT,
				 GNOME_VFS_XFER_ERROR_MODE_ABORT,
				 GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
				 GNOME_VFS_PRIORITY_DEFAULT,
				 (GnomeVFSAsyncXferProgressCallback) xfer_callback,
				 stockdata, 
				 NULL, NULL)) {
		GnomeVFSXferProgressInfo info;
g_print ("hello \n");
		info.phase = GNOME_VFS_XFER_PHASE_COMPLETED;
		xfer_callback(NULL, &info, stockdata);
	}
#else
	gnome_vfs_xfer_uri (source_uri, dest_uri, GNOME_VFS_XFER_DEFAULT,
			    GNOME_VFS_XFER_ERROR_MODE_ABORT,
			    GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
			    NULL,
			    NULL);
	configured (stockdata);
#endif
	g_list_free(sources);
	g_list_free(dests);

	gnome_vfs_uri_unref(source_uri);
	gnome_vfs_uri_unref(dest_uri);
	
	return FALSE;
}




	/*-----------------------------------------------------------------*/
	static void properties_load(StockData *stockdata) {
		stockdata->props.tik_syms = g_strdup ("cald+rhat+corl");
		stockdata->props.output = g_strdup ("default");
		stockdata->props.scroll = g_strdup ("right2left");
		stockdata->props.arrows = g_strdup ("arrows");
		stockdata->props.timeout = 5;
		stockdata->props.dcolor = g_strdup ("#ff0000");
		stockdata->props.ucolor = g_strdup ("#00ff00");
		stockdata->props.font = g_strdup ("fixed");
		stockdata->props.font2 = g_strdup ("-*-clean-medium-r-normal-*-*-100-*-*-c-*-iso8859-1");
		stockdata->props.buttons = g_strdup ("yes");
#ifdef FIXME	
		
		gnome_config_push_prefix ("/");
		if( gnome_config_get_string ("gtik/tik_syms") != NULL ) 
		   props.tik_syms = gnome_config_get_string("gtik/tik_syms");
		

		if (gnome_config_get_int("gtik/timeout") > 0)
			props.timeout = gnome_config_get_int("gtik/timeout");
		timeout = props.timeout;

		if ( gnome_config_get_string ("gtik/output") != NULL ) 
			props.output = gnome_config_get_string("gtik/output");
		
		if ( gnome_config_get_string ("gtik/font") != NULL ) 
			props.font = gnome_config_get_string("gtik/font");

		if ( gnome_config_get_string ("gtik/font2") != NULL ) 
			props.font2 = gnome_config_get_string("gtik/font2");

		if ( gnome_config_get_string ("gtik/scroll") != NULL ) 
			props.scroll = gnome_config_get_string("gtik/scroll");

		if ( gnome_config_get_string ("gtik/arrows") != NULL ) 
			props.arrows = gnome_config_get_string("gtik/arrows");
		
		if ( gnome_config_get_string ("gtik/ucolor") != NULL ) 
		   strcpy(props.ucolor, gnome_config_get_string("gtik/ucolor"));
		
		if ( gnome_config_get_string ("gtik/dcolor") != NULL ) 
		   strcpy(props.dcolor, gnome_config_get_string("gtik/dcolor"));
		
		if ( gnome_config_get_string ("gtik/buttons") != NULL ) 
			props.buttons = gnome_config_get_string("gtik/buttons");
		
		gnome_config_pop_prefix ();
#endif
	}



	/*-----------------------------------------------------------------*/
	void properties_save(char *path) {
#ifdef FIXME
		gnome_config_push_prefix (path);
		gnome_config_set_string( "gtik/tik_syms", props.tik_syms );
		gnome_config_set_string( "gtik/output", props.output );
		gnome_config_set_string( "gtik/scroll", props.scroll );
		gnome_config_set_string( "gtik/arrows", props.arrows );
		gnome_config_set_string( "gtik/ucolor", props.ucolor );
		gnome_config_set_string( "gtik/dcolor", props.dcolor );
		gnome_config_set_string( "gtik/font", props.font );
		gnome_config_set_string( "gtik/font2", props.font2 );
		gnome_config_set_string( "gtik/buttons", props.buttons );

		gnome_config_set_int("gtik/timeout",props.timeout);

		gnome_config_pop_prefix ();
		gnome_config_sync();
		gnome_config_drop_all();
#endif
	}	


	/*-----------------------------------------------------------------*/
	static void properties_set (StockData *stockdata, gboolean update) {
		if (!strcmp(stockdata->props.buttons,"yes")) {
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
g_print ("parse \n");
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
g_print ("configured \n");
		retVar = 0;

		/* clear the output variable */
		reSetOutputArray(stockdata);

		if ( CONFIG ) {
			while ( !feof(CONFIG) ) {
				fgets(buffer, sizeof(buffer)-1, CONFIG);

				if (strstr(buffer,
				    "<td nowrap align=left><a href=\"/q\?s=")) {

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
		GdkFont *my_font = stockdata->my_font;
		GdkFont *small_font = stockdata->small_font;
		GdkFont *extra_font = stockdata->extra_font;
		GdkGC *gc = stockdata->gc;
		GdkRectangle update_rect;
		int	comp;

		/* FOR COLOR */
		char *tmpSym;
		int totalLoc;
		int totalLen;
		int i;


		totalLoc = 0;
		totalLen = 0;

		/* clear the pixmap */
		gdk_draw_rectangle (stockdata->pixmap,
		drawing_area->style->black_gc,
		TRUE,
		0,0,
		drawing_area->allocation.width,
		drawing_area->allocation.height);


		for(i=0;i<stockdata->setCounter;i++) {
			totalLen += (gdk_string_width(my_font,
				STOCK_QUOTE(quotes->data)[i].price) + 10);
			if (!strcmp(stockdata->props.output,"default")) {
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					totalLen += (gdk_text_width(extra_font,
								    STOCK_QUOTE(quotes->data)[i].change,1) + 10);
				}
				totalLen += (gdk_string_width(small_font,
						&STOCK_QUOTE(quotes->data)[i].change[1]) +10 );
			}
		}

		comp = 0 - totalLen;

		if (stockdata->MOVE == 1) { stockdata->MOVE = 0; } 
		else { stockdata->MOVE = 1; }

		if (stockdata->MOVE == 1) {


		  if (!strcmp(stockdata->props.scroll,"right2left")) {
			if (stockdata->location > comp) {
				stockdata->location--;
			}
			else {
				stockdata->location = drawing_area->allocation.width;
			}

		  }
		  else {
                       if (stockdata->location < drawing_area->allocation.width) {
                                stockdata->location ++;
                        }
                        else {
                                stockdata->location = comp;
                        }
		  }



		}

		for (i=0;i<stockdata->setCounter;i++) {

			/* COLOR */
			if (STOCK_QUOTE(quotes->data)[i].color == GREEN) {
				gdk_gc_set_foreground( gc, &stockdata->gdkUcolor );
			}
			else if (STOCK_QUOTE(quotes->data)->color == RED) {
				gdk_gc_set_foreground( gc, &stockdata->gdkDcolor );
			}
			else {
				gdk_gc_copy( gc, drawing_area->style->white_gc );
			}

			tmpSym = STOCK_QUOTE(quotes->data)[i].price;
			gdk_draw_string (stockdata->pixmap,my_font, gc,
			stockdata->location + totalLoc ,14,
			STOCK_QUOTE(quotes->data)[i].price);

			totalLoc += (gdk_string_width(my_font,tmpSym) + 10); 


			if (!strcmp(stockdata->props.output,"default")) {
				tmpSym = STOCK_QUOTE(quotes->data)[i].change;
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					gdk_draw_text (stockdata->pixmap,extra_font,
					     gc, stockdata->location + totalLoc,
					     14,STOCK_QUOTE(quotes->data)[i].change,1);
					totalLoc += (gdk_text_width(extra_font,
							STOCK_QUOTE(quotes->data)[i].change,1) + 5);
				}
				gdk_draw_string (stockdata->pixmap,small_font,
				     gc, stockdata->location + totalLoc,
				     14, &STOCK_QUOTE(quotes->data)[i].change[1]);
				totalLoc += (gdk_string_width(small_font,
						tmpSym) + 10); 
			}
			
		}
		update_rect.x = 0;
		update_rect.y = 0;
		update_rect.width = drawing_area->allocation.width;
		update_rect.height = drawing_area->allocation.height;

		gtk_widget_draw(drawing_area,&update_rect);
		return 1;
	}




	/*-----------------------------------------------------------------*/
	static void about_cb (BonoboUIComponent *uic, gpointer data, 
			      const gchar *verbname) {
		GtkWidget *about;
		static const gchar *authors[] = {
			"Jayson Lorenzen <jayson_lorenzen@yahoo.com>",
			"Jim Garrison <garrison@users.sourceforge.net>",
			"Rached Blili <striker@dread.net>"
		};

		about = gnome_about_new (_("The GNOME Stock Ticker"), VERSION,
		"(C) 2000 Jayson Lorenzen, Jim Garrison, Rached Blili",
		_("This program connects to "
		"a popular site and downloads current stock quotes.  "
		"The GNOME Stock Ticker is a free Internet-based application.  "
		"It comes with ABSOLUTELY NO WARRANTY.  "
		"Do not use the GNOME Stock Ticker for making investment decisions; it is for "
		"informational purposes only."),
		authors,
		NULL,
		NULL,
		NULL);
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
		gchar *current;
		gint i;

		current = g_strdup(stockdata->props.scroll);
		stockdata->props.scroll = g_strdup("right2left");
		for (i=0;i<151;i++) {
			Repaint(stockdata);
		}
		stockdata->props.scroll = g_strdup(current);
		g_free(current);
	}

	/*-----------------------------------------------------------------*/
	static void zipRight(GtkWidget *widget, gpointer data) {
		StockData *stockdata = data;
		gchar *current;
		gint i;

		current = g_strdup(stockdata->props.scroll);
		stockdata->props.scroll = g_strdup("left2right");
		for (i=0;i<151;i++) {
			Repaint(stockdata);
		}
		stockdata->props.scroll = g_strdup(current);
		g_free(current);
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
	static void toggle_output_cb(GtkWidget *widget, gpointer data) {
#ifdef FIXME
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(poutput,"nochange");
		else
			strcpy(poutput,"default");
#endif
			
	}

	/*-----------------------------------------------------------------*/
	static void toggle_scroll_cb(GtkWidget *widget, gpointer data) {
#ifdef FIXME
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(scroll,"left2right");
		else
			strcpy(scroll,"right2left");
#endif
			
	}


	/*-----------------------------------------------------------------*/
	static void toggle_arrows_cb(GtkWidget *widget, gpointer data) {
#ifdef FIXME
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(arrows,"arrows");
		else
			strcpy(arrows,"noArrows");
#endif			
	}

	/*-----------------------------------------------------------------*/
	static void toggle_buttons_cb(GtkWidget *widget, gpointer data) {
#ifdef FIXME
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(buttons,"yes");
		else
			strcpy(buttons,"no");
#endif
	}


	/*-----------------------------------------------------------------*/
	void timeout_cb( GtkWidget *widget, GtkWidget *spin ) {
#ifdef FIXME
		timeout=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

		gnome_property_box_changed(GNOME_PROPERTY_BOX(pb));
#endif
	}



#ifdef FIXME
	/*-----------------------------------------------------------------*/
	static void apply_cb( GtkWidget *widget, void *data ) {

		char *tmpText;

		tmpText = gtk_entry_get_text(GTK_ENTRY(tik_syms_entry));
		props.tik_syms = g_strdup(tmpText);

		props.tik_syms = getSymsFromClist(tik_syms_entry);
		if  (props.timeout) {	
			props.timeout = timeout > 0 ? timeout : props.timeout;
		}

		if (new_font != NULL) {
			if (whichlabel == 1) 
				props.font = g_strdup(new_font);
			else
				props.font2 = g_strdup(new_font);
		}
		if (strcmp(buttons,"blank") !=0) {
			props.buttons = g_strdup(buttons);
		}
		if (strcmp(arrows,"blank") !=0) {
			props.arrows = g_strdup(arrows);
		}
		if (strcmp(scroll,"blank") !=0) {
			props.scroll = g_strdup(scroll);
		}
		if (strcmp(poutput,"blank") !=0) {
			props.output = g_strdup(poutput);
		}


		if (updateTimeID > 0)
			gtk_timeout_remove(updateTimeID);
		updateTimeID = gtk_timeout_add(props.timeout * 60000,
				   (gpointer)updateOutput,"NULL");

		properties_save(APPLET_WIDGET(applet)->privcfgpath);

		properties_set(TRUE);

	}


	/*-----------------------------------------------------------------*/
        static gint font_selector( GtkWidget *widget, void *data ) {

		if (!fontDialog) {
			fontDialog = gtk_font_selection_dialog_new("Font Selector");
			gtk_signal_connect (GTK_OBJECT (GTK_FONT_SELECTION_DIALOG(fontDialog)->ok_button),
					    "clicked",
					    GTK_SIGNAL_FUNC(OkClicked),fontDialog);
            		gtk_signal_connect_object(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(fontDialog)->cancel_button),
						  "clicked",
						  GTK_SIGNAL_FUNC(gtk_widget_destroy),
						  GTK_OBJECT(fontDialog));
            		gtk_signal_connect (GTK_OBJECT (fontDialog), "destroy",
					    GTK_SIGNAL_FUNC(QuitFontDialog),
					    fontDialog);
			
            		gtk_widget_show(fontDialog);
		} else
			gdk_window_raise(fontDialog->window);
		
		return FALSE;
	}
#endif	
        /*-----------------------------------------------------------------*/

	gint font_cb(GtkWidget *widget, gpointer data) {
#ifdef FIXME
		whichlabel = 1;
		font_selector(widget,data);
#endif
		return FALSE;
	}

        /*-----------------------------------------------------------------*/
	static gint font2_cb(GtkWidget *widget, gpointer data) {
#ifdef FIXME
		whichlabel = 2;
		font_selector(widget,data);
#endif
		return FALSE;
	}

        /*-----------------------------------------------------------------*/
        gint OkClicked( GtkWidget *widget, void *fontDialog ) {
#ifdef FIXME
                gchar *newFont = NULL;


                GtkFontSelectionDialog *fsd = 
			GTK_FONT_SELECTION_DIALOG(fontDialog);

                newFont = gtk_font_selection_dialog_get_font_name(fsd);
                new_font = g_strdup(newFont);
                gtk_widget_destroy(fontDialog);
#endif
		return FALSE;
        }

        /*-----------------------------------------------------------------*/
        gint QuitFontDialog( GtkWidget *widget, void *data ) {
        
		/*fontDialog = NULL;*/
	
		return FALSE;
        }

	/*-----------------------------------------------------------------*/

	char *getSymsFromClist(GtkWidget *clist) {
		GList *selection;
		char *symbol;
		char *ret;
		GString *symlist;
		gint row;

		gtk_clist_freeze(GTK_CLIST(clist));
		gtk_clist_select_all(GTK_CLIST(clist));

		symlist = g_string_new (NULL);

		selection = GTK_CLIST(clist)->selection;

		while (selection) {
			row=GPOINTER_TO_INT(selection->data);
			selection=selection->next;
			gtk_clist_get_text(GTK_CLIST(clist),row,0,&symbol);

			symbol = g_strdup (symbol);

			/* XXX: couldn't this be just g_strstrip ????
			 * -George */
			removeSpace(symbol);

			if (strcmp(symlist->str,"") != 0) {
				g_string_append (symlist, "+");
			}

			g_string_append (symlist, symbol);

			g_free (symbol);
		}
		gtk_clist_thaw(GTK_CLIST(clist));

		ret = symlist->str;
		g_string_free (symlist, FALSE);
		return ret;
	}

	static void populateClist(StockData *stockdata, GtkWidget *clist) {
	
		gchar *symbol[1];
		gchar *syms;
		gchar *temp;

		syms = g_strdup(stockdata->props.tik_syms);

		if ((temp=strtok(syms,"+")))
			symbol[0] = g_strdup(temp);

		while (symbol[0]) {
			gtk_clist_append(GTK_CLIST(clist),symbol);
			if ((temp=strtok(NULL,"+")))
				symbol[0] = g_strdup(temp);
			else
				symbol[0]=NULL;
		}

		if ((temp=strtok(NULL,""))) {
			symbol[0] = g_strdup(temp);
			gtk_clist_append(GTK_CLIST(clist),symbol);
		}
	}

	/*-----------------------------------------------------------------*/

	/* Thanks to Mike Oliphant for inspiration. */

	static void addToClist(GtkWidget *widget_unused, gpointer entry) {
		char *symbols, *symbol_start, *symbol_end;
		gint row = -1;
		GtkWidget *clist;

		clist = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(entry),
							"clist"));

		symbols = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
		symbol_start = symbol_end = symbols;

		while (*symbol_end) {
			if (*symbol_end == ' ') {
				*symbol_end = '\0';
				if (symbol_start != symbol_end)
					row = gtk_clist_append(GTK_CLIST(clist),&symbol_start);
				symbol_start = symbol_end + 1;
			}
			symbol_end++;
		}
		if (symbol_start != symbol_end)
			row = gtk_clist_append(GTK_CLIST(clist),&symbol_start);

		gtk_entry_set_text(GTK_ENTRY(entry),"");
		if (row != -1)
			gtk_clist_moveto(GTK_CLIST(clist),row,0,0,0);
		g_free(symbols);
	}

	static void remFromClist(GtkWidget *widget, gpointer data) {
		GtkCList *clist;

		clist = GTK_CLIST(gtk_object_get_data(GTK_OBJECT(widget),
						      "clist"));
		while (GPOINTER_TO_INT(clist->selection))
			gtk_clist_remove(GTK_CLIST(clist), GPOINTER_TO_INT(clist->selection->data));
	}

	static GtkWidget *symbolManager(StockData *stockdata) { 
		GtkWidget *vbox;
		GtkWidget *mainhbox;
		GtkWidget *hbox;
		GtkWidget *clist;
		GtkWidget *swindow;
		GtkWidget *label;
		GtkWidget *entry;
		static GtkWidget *button;

		clist = gtk_clist_new(1);
		stockdata->tik_syms_entry = clist;
		gtk_clist_set_selection_mode(GTK_CLIST(clist),
						GTK_SELECTION_EXTENDED);
		gtk_clist_column_titles_passive(GTK_CLIST(clist));
		gtk_widget_set_usize(clist,100,-1);
		gtk_clist_set_column_width(GTK_CLIST(clist),0,60);
		gtk_clist_set_reorderable(GTK_CLIST(clist),TRUE);
		gtk_signal_connect_object(GTK_OBJECT(clist),"drag_drop",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));

		swindow = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swindow),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(swindow),clist);

		populateClist(stockdata, clist);

		hbox = gtk_hbox_new(FALSE,5);
		vbox = gtk_vbox_new(FALSE,5);

		label = gtk_label_new(_("New Symbol:"));
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

		entry = gtk_entry_new();
		gtk_object_set_data(GTK_OBJECT(entry),"clist",(gpointer)clist);
		gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);
		gtk_signal_connect(GTK_OBJECT(entry),"activate",
					GTK_SIGNAL_FUNC(addToClist), entry);
		gtk_signal_connect_object(GTK_OBJECT(entry),"activate",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));
		
		button = gtk_button_new_with_label(_("Add"));
		gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
		gtk_signal_connect(GTK_OBJECT(button),"clicked",
					GTK_SIGNAL_FUNC(addToClist), entry);
		gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));

		gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

		hbox = gtk_hbox_new(FALSE,5);

		button = gtk_button_new_with_label(_("Remove Selected"));
		gtk_signal_connect(GTK_OBJECT(clist),"select-row",
				GTK_SIGNAL_FUNC(selected_cb),(gpointer)button);
		gtk_object_set_data(GTK_OBJECT(button),"clist",(gpointer)clist);
		gtk_signal_connect(GTK_OBJECT(button),"clicked",
					GTK_SIGNAL_FUNC(remFromClist),NULL);
		gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));
		gtk_widget_set_sensitive(button,FALSE);
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);

		gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
		mainhbox = gtk_hbox_new(FALSE,5);
		gtk_box_pack_start(GTK_BOX(mainhbox),swindow,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(mainhbox),vbox,FALSE,FALSE,0);


		gtk_widget_show_all(mainhbox);
		return(mainhbox);

	}
	
	static void
	response_cb (GtkDialog *dialog, gint id, gpointer data)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));
		
	}

	/*-----------------------------------------------------------------*/
	static void properties_cb (BonoboUIComponent *uic, gpointer data, 
				   const gchar *verbname) {
		StockData * stockdata = data;
		GtkWidget * notebook;
		GtkWidget * vbox;
		GtkWidget * vbox2;
		GtkWidget * vbox3;
		GtkWidget * hbox3;
		GtkWidget *hbox;
		GtkWidget * label;
#if 0
		GtkWidget *urlcheck, *launchcheck;
#endif

		GtkWidget *panela, *panel1 ,*panel2;
		GtkWidget *label1, *label5;


		GtkWidget *timeout_label,*timeout_c;
		GtkObject *timeout_a;

		GtkWidget *upColor, *downColor, *upLabel, *downLabel;
		GtkWidget *check, *check2, *check3, *check4, *fontButton;

		int ur,ug,ub, dr,dg,db; 

		stockdata->pb = gtk_dialog_new_with_buttons (_("Stock Ticker Properties"), 
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

		timeout_label = gtk_label_new(_("Update Frequency in minutes:"));
		timeout_a = gtk_adjustment_new( stockdata->timeout, 0.5, 128, 1, 8, 8 );
		timeout_c  = gtk_spin_button_new( GTK_ADJUSTMENT(timeout_a), 1, 0 );
		gtk_widget_set_usize(timeout_c,60,-1);

		gtk_box_pack_start_defaults( GTK_BOX(panel2), timeout_label );
		gtk_box_pack_start_defaults( GTK_BOX(panel2), timeout_c );
		gtk_box_pack_start(GTK_BOX(vbox), panel2, FALSE,
				    FALSE, GNOME_PAD);
#ifdef FIXME	
		gtk_signal_connect_object(GTK_OBJECT(timeout_c), "changed",GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		gtk_signal_connect( GTK_OBJECT(timeout_a),"value_changed",
		GTK_SIGNAL_FUNC(timeout_cb), timeout_c );
		gtk_signal_connect( GTK_OBJECT(timeout_c),"changed",
		GTK_SIGNAL_FUNC(timeout_cb), timeout_c );
		gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(timeout_c),
		GTK_UPDATE_ALWAYS );
#endif
		label1 = gtk_label_new(_("Enter symbols delimited with \"+\" in the box below."));

		stockdata->tik_syms_entry = gtk_entry_new_with_max_length(60);

		gtk_entry_set_text(GTK_ENTRY(stockdata->tik_syms_entry), 
			stockdata->props.tik_syms ? stockdata->props.tik_syms : "");
		gtk_signal_connect_object(GTK_OBJECT(stockdata->tik_syms_entry), 
					  "changed",GTK_SIGNAL_FUNC(changed_cb),
					  GTK_OBJECT(stockdata->pb));

		check = gtk_check_button_new_with_label(_("Display only symbols and price"));
		check2 = gtk_check_button_new_with_label(_("Scroll left to right"));
		check3 = gtk_check_button_new_with_label(_("Display arrows instead of -/+"));
		check4 = gtk_check_button_new_with_label(_("Enable scroll buttons"));


		gtk_box_pack_start(GTK_BOX(vbox2), check, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox2), check2, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), check3, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), check4, FALSE, FALSE, GNOME_PAD);


		/* Set the checkbox according to current prefs */
		if (strcmp(stockdata->props.output,"default")!=0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
							TRUE);
		if (strcmp(stockdata->props.scroll,"right2left")!=0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check2),
							TRUE);
		if (strcmp(stockdata->props.buttons,"yes") == 0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check4),
							TRUE);
		if (strcmp(stockdata->props.arrows,"arrows") ==0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check3),
							TRUE);

		gtk_signal_connect_object(GTK_OBJECT(check),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));
		gtk_signal_connect_object(GTK_OBJECT(check2),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));
		gtk_signal_connect_object(GTK_OBJECT(check3),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));
		gtk_signal_connect_object(GTK_OBJECT(check4),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));

#if 1
		gtk_signal_connect(GTK_OBJECT(check4),"toggled",
				GTK_SIGNAL_FUNC(toggle_buttons_cb),NULL);
		gtk_signal_connect(GTK_OBJECT(check3),"toggled",
				GTK_SIGNAL_FUNC(toggle_arrows_cb),NULL);
		gtk_signal_connect(GTK_OBJECT(check),"toggled",
				GTK_SIGNAL_FUNC(toggle_output_cb),NULL);
		gtk_signal_connect(GTK_OBJECT(check2),"toggled",
				GTK_SIGNAL_FUNC(toggle_scroll_cb),NULL);
#endif

		/* COLOR */
		upLabel = gtk_label_new(_("+ Color"));
		upColor = gnome_color_picker_new();
	
		sscanf( stockdata->props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
	
		gnome_color_picker_set_i8(GNOME_COLOR_PICKER (upColor), 
					  ur, ug, ub, 255);
		
		gtk_signal_connect(GTK_OBJECT(upColor), "color_set",
				GTK_SIGNAL_FUNC(ucolor_set_cb), NULL);

		vbox3 = gtk_vbox_new(FALSE, 5); 
		hbox3 = gtk_hbox_new(FALSE, 5);
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), upLabel );
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), upColor );
		gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);

		downLabel = gtk_label_new(_("- Color"));
		downColor = gnome_color_picker_new();

		sscanf( stockdata->props.dcolor, "#%02x%02x%02x", &dr,&dg,&db );

		gnome_color_picker_set_i8(GNOME_COLOR_PICKER (downColor), 
					  dr, dg, db, 255);

		gtk_signal_connect(GTK_OBJECT(downColor), "color_set",
				GTK_SIGNAL_FUNC(dcolor_set_cb), NULL);

		hbox3 = gtk_hbox_new(FALSE, 5);
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), downLabel );
		gtk_box_pack_start_defaults( GTK_BOX(hbox3), downColor );
		gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
		gtk_box_pack_start_defaults(GTK_BOX(panela),vbox3);

                /* For FONTS */
		vbox3 = gtk_vbox_new(FALSE, 5); 
		hbox3 = gtk_hbox_new(FALSE, 5);
		label5 = gtk_label_new(_("Stock Symbol:"));
                fontButton = gtk_button_new_with_label(_("Select Font"));
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),label5);
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),fontButton);
                gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(font_cb),GTK_OBJECT(stockdata->pb));
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));

		hbox3 = gtk_hbox_new(FALSE, 5);
		label5 = gtk_label_new(_("Stock Change:"));
                fontButton = gtk_button_new_with_label(_("Select Font"));
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),label5);
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),fontButton);
                gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(font2_cb),GTK_OBJECT(stockdata->pb));
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(stockdata->pb));

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
		

#if 0
		gtk_signal_connect_object(GTK_OBJECT(tik_syms_entry), 
				   "changed",GTK_SIGNAL_FUNC(changed_cb),
				   GTK_OBJECT(pb));
#endif

		gtk_widget_show_all(stockdata->pb);
		
		g_signal_connect (G_OBJECT (stockdata->pb), "response",
				  G_CALLBACK (response_cb), NULL);

	}

	static const BonoboUIVerb gtik_applet_menu_verbs [] = {
        	BONOBO_UI_VERB ("Props", properties_cb),
        	BONOBO_UI_VERB ("Refresh", refresh_cb),
        	BONOBO_UI_VERB ("About", about_cb),

        	BONOBO_UI_VERB_END
	};

	static const char gtik_applet_menu_xml [] =
		"<popup name=\"button3\">\n"
		"   <menuitem name=\"Item 1\" verb=\"Props\" _label=\"Properties\"\n"
		"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
		"   <menuitem name=\"Item 2\" verb=\"Refresh\" _label=\"Refresh\"/>\n"
		"   <menuitem name=\"Item 3\" verb=\"About\" _label=\"About\"\n"
		"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
		"</popup>\n";


	/*-----------------------------------------------------------------*/
	static gboolean gtik_applet_fill (PanelApplet *applet){
		StockData *stockdata;
		GtkWidget * vbox;
		GtkWidget * frame;

		gnome_vfs_init();
		
		stockdata = g_new0 (StockData, 1);
		stockdata->max_rgb_str_len = 7;
		stockdata->max_rgb_str_size = 8;
		stockdata->buttons = g_strdup ("blank");
		stockdata->arrows = g_strdup ("blank");
		stockdata->scroll = g_strdup ("blank");
		stockdata->poutput = g_strdup ("blank");
		stockdata->timeout = 0;
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


		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);


		stockdata->drawing_area = gtk_drawing_area_new();
		gtk_drawing_area_size(GTK_DRAWING_AREA (stockdata->drawing_area),200,20);

		gtk_widget_show(stockdata->drawing_area);
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
		
		panel_applet_setup_menu (PANEL_APPLET (applet),
				         gtik_applet_menu_xml,
				         gtik_applet_menu_verbs,
				         stockdata);

		/* KEEPING TIMER ID FOR CLEANUP IN DESTROY */
		stockdata->drawTimeID = gtk_timeout_add(100,Repaint,stockdata);
		stockdata->updateTimeID = gtk_timeout_add(stockdata->props.timeout * 60000,
				                          updateOutput,stockdata);


		if (!strcmp(stockdata->props.buttons,"yes")) {
			gtk_widget_show(stockdata->leftButton);
			gtk_widget_show(stockdata->rightButton);
		}

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
			     	"Gtik applet",
			     	"0",
			     	gtik_applet_factory,
			     	NULL)




/* JHACK */
	/*-----------------------------------------------------------------*/
	static void destroy_applet(GtkWidget *widget, gpointer data) {
		StockData *stockdata = data;
		if (stockdata->drawTimeID > 0) { 
			gtk_timeout_remove(stockdata->drawTimeID); }
		if (stockdata->updateTimeID >0) { 
			gtk_timeout_remove(stockdata->updateTimeID); }
		gtk_widget_destroy(stockdata->drawing_area);

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
	char *splitChange(StockData *stockdata,char *data) {
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
			if (stockdata->symbolfont)
				var3[0] = 221;
			var4[0] = '(';
		}
		else if (var3[0] == '-') {
			if (stockdata->symbolfont)
				var3[0] = 223;	
			var4[0] = '(';
		}
		else {
			var3 = g_strdup(_("(No"));
			var4 = g_strdup(_("Change "));
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
		change = splitChange(stockdata, param1);

		quote.price = g_strdup(price);
		quote.change = g_strdup(change);

		if (strstr(change,"-"))
			quote.color = RED;
		else if (strstr(change,"+"))
			quote.color = GREEN;
		else
			quote.color = WHITE;

#if 1
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

	}

	
	int create_gc(StockData *stockdata) {
		stockdata->gc = gdk_gc_new( stockdata->drawing_area->window );
		gdk_gc_copy(stockdata->gc, stockdata->drawing_area->style->white_gc );
		return 0;
	}







	void ucolor_set_cb(GnomeColorPicker *cp, gpointer data) {
		StockData *stockdata = data;
		guint8 r,g,b;
		gnome_color_picker_get_i8(cp,
					&r,
					&g,
					&b,
					NULL);

		g_snprintf( stockdata->props.ucolor, stockdata->max_rgb_str_size,
		"#%02x%02x%02x", r, g, b );
		gnome_property_box_changed(GNOME_PROPERTY_BOX(stockdata->pb));
	}



	void dcolor_set_cb(GnomeColorPicker *cp, gpointer data) {
		StockData *stockdata = data;
		guint8 r,g,b;
		gnome_color_picker_get_i8(cp,
					&r,
					&g,
					&b,
					NULL);
		g_snprintf( stockdata->props.dcolor, stockdata->max_rgb_str_size,
		"#%02x%02x%02x", r, g, b );
		gnome_property_box_changed(GNOME_PROPERTY_BOX(stockdata->pb));
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



