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

#include <gnome.h>
#include <applet-widget.h>
 
#include <gtk/gtk.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libgnomevfs/gnome-vfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <gdk/gdkx.h>

#define STOCK_QUOTE(sq) ((StockQuote *)(sq))

	GtkWidget *applet;
	GtkWidget *label;


	static GdkPixmap *pixmap = NULL;
	GtkWidget * drawing_area;
	GtkWidget * leftButton;
	GtkWidget * rightButton;

	int location;
	int MOVE;

	typedef struct {
		char *price;
		char *change;
		int color;
	} StockQuote;
	GArray *quotes;

	enum {
		WHITE,
		RED,
		GREEN
	};

	static const int max_rgb_str_len = 7;
	static const int max_rgb_str_size = 8;

	int setCounter;

	GdkGC *gc;
	GdkColor gdkUcolor,gdkDcolor;

	/* end of COLOR vars */


	char *configFileName;


	/*  properties vars */
 	 
	GtkWidget *tik_syms_entry;

	GtkWidget *fontDialog = NULL;

	GtkWidget * pb;
	char buttons[16]="blank";
	char arrows[16]="blank";
	char scroll[16]="blank";
	char poutput[16]="blank";

	typedef struct
	{
		char *tik_syms;
		char *output;
		char *scroll;
		char *arrows;
		gint timeout;
		gchar dcolor[8];
		gchar ucolor[8];
		gchar *font;
		gchar *font2;
		gchar *buttons;

	} gtik_properties;

	gtik_properties props = {"cald+rhat+corl","default","right2left",
			"arrows",5,"#ff0000","#00ff00","fixed",
			"-*-clean-medium-r-normal-*-*-100-*-*-c-*-iso8859-1",
			"yes"};

	/* end prop vars */


	gint timeout = 0;
	gint drawTimeID, updateTimeID;

	void removeSpace(char *buffer); 
	int configured(void);
	void timeout_cb( GtkWidget *widget, GtkWidget *spin );
	void properties_save(char *path) ;
	static void destroy_applet(GtkWidget *widget, gpointer data) ;
	char *getSymsFromClist(GtkWidget *clist) ;

	char *splitPrice(char *data);
	char *splitChange(char *data);

	/* FOR COLOR */

	static void updateOutput(void) ;
	static void reSetOutputArray(void) ;
	static void setOutputArray(char *param1) ;
	void setup_colors(void);
	int create_gc(void) ;
	void ucolor_set_cb(GnomeColorPicker *cp) ;
	void dcolor_set_cb(GnomeColorPicker *cp) ;

	/* end of color funcs */

	/* For fonts */
	GdkFont * my_font;
	gchar * new_font = NULL;
	GdkFont * extra_font;
	GdkFont * small_font;
	static gint symbolfont = 1;
	static gint whichlabel = 1;

        gint font_cb( GtkWidget *widget, void *data ) ;
        gint OkClicked( GtkWidget *widget, void *data ) ;
        gint QuitFontDialog( GtkWidget *widget, void *data ) ;
	/* end font funcs and vars */

	/*-----------------------------------------------------------------*/
	static gint applet_save_session(GtkWidget *widget, char *privcfgpath, 
				char *globcfgpath) {
		properties_save(privcfgpath);
		return FALSE;
	}


	/*-----------------------------------------------------------------*/
	static void load_fonts()
	{
		if (new_font != NULL) {
			if (whichlabel == 1)
				my_font = gdk_font_load(new_font);
			else
				small_font = gdk_font_load(new_font);
		}
		if (!my_font)
			my_font = gdk_font_load (props.font);

		if (!extra_font)
			extra_font = gdk_font_load ("-*-symbol-medium-r-normal-*-*-140-*-*-p-*-adobe-fontspecific");
		if (!small_font)
			small_font = gdk_font_load (props.font2);


		/* If fonts do not load */
		if (!my_font)
			g_error("Could not load fonts!");

		if ( !extra_font || (strcmp(props.arrows,"noArrows")) == 0 ) {
			
			if (extra_font)
				gdk_font_unref(extra_font);
			extra_font = gdk_font_load("fixed");
			symbolfont = 0;
		}
		else {
			if (extra_font)
				gdk_font_unref(extra_font);
			extra_font = gdk_font_load ("-*-symbol-medium-r-normal-*-*-140-*-*-p-*-adobe-fontspecific");
			symbolfont = 1;

		}
		if (!small_font) {
			if (small_font)
				gdk_font_unref(small_font);
			small_font = gdk_font_load("fixed");
		}
	}

/*-----------------------------------------------------------------*/
static void xfer_callback (GnomeVFSAsyncHandle *handle, GnomeVFSXferProgressInfo *info, gpointer data)
{
	if (info->phase == GNOME_VFS_XFER_PHASE_COMPLETED) {
		if (!configured()) {
			reSetOutputArray();
			fprintf(stderr, "No data!\n");
			setOutputArray(_("No data available or properties not set"));
		}
	}
}

/*-----------------------------------------------------------------*/
static void updateOutput(void)
{
	GList *sources, *dests;
	GnomeVFSURI *source_uri, *dest_uri;
	char *source_text_uri;
	GnomeVFSAsyncHandle *vfshandle;

	source_text_uri = g_strconcat("http://finance.yahoo.com/q?s=",
				      props.tik_syms,
				      "&d=v2",
				      NULL);
	source_uri = gnome_vfs_uri_new(source_text_uri);
	sources = g_list_append(NULL, source_uri);
	g_free(source_text_uri);

	dest_uri = gnome_vfs_uri_new(configFileName);
	dests = g_list_append(NULL, dest_uri);

	if (GNOME_VFS_OK !=
	    gnome_vfs_async_xfer(&vfshandle, sources, dests,
				 GNOME_VFS_XFER_DEFAULT,
				 GNOME_VFS_XFER_ERROR_MODE_ABORT,
				 GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
				 (GnomeVFSAsyncXferProgressCallback) xfer_callback,
				 NULL, NULL, NULL)) {
		GnomeVFSXferProgressInfo info;
		info.phase = GNOME_VFS_XFER_PHASE_COMPLETED;
		xfer_callback(NULL, &info, NULL);
	}	

	g_list_free(sources);
	g_list_free(dests);

	gnome_vfs_uri_unref(source_uri);
	gnome_vfs_uri_unref(dest_uri);
}




	/*-----------------------------------------------------------------*/
	static void properties_load( char *path) {


		gnome_config_push_prefix (path);
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
	}



	/*-----------------------------------------------------------------*/
	void properties_save(char *path) {

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
	}	


	/*-----------------------------------------------------------------*/
	static void properties_set (gboolean update) {
		if (!strcmp(props.buttons,"yes")) {
			gtk_widget_show(leftButton);
			gtk_widget_show(rightButton);
		}
		else {
			gtk_widget_hide(leftButton);
			gtk_widget_hide(rightButton);
		}

		setup_colors();		
		load_fonts();
		if (update)
			updateOutput();
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
	int configured() {
		int retVar;

		char  buffer[512];
		static FILE *CONFIG;

		CONFIG = fopen((const char *)configFileName,"r");

		retVar = 0;

		/* clear the output variable */
		reSetOutputArray();

		if ( CONFIG ) {
			while ( !feof(CONFIG) ) {
				fgets(buffer, sizeof(buffer)-1, CONFIG);

				if (strstr(buffer,
				    "<td nowrap align=left><a href=\"/q\?s=")) {

				      setOutputArray(parseQuote(CONFIG,buffer));
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
	static gint expose_event (GtkWidget *widget,GdkEventExpose *event) {

		gdk_draw_pixmap(widget->window,
		widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		pixmap,
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width,event->area.height);

		return FALSE;
	}



	/*-----------------------------------------------------------------*/
	static gint configure_event(GtkWidget *widget,GdkEventConfigure *event){

		if(pixmap) {
			gdk_pixmap_unref (pixmap);
		}

		pixmap = gdk_pixmap_new(widget->window,
		widget->allocation.width,
		widget->allocation.height,
		-1);

		return TRUE;
	}






	/*-----------------------------------------------------------------*/
	static gint Repaint (gpointer data) {
		GtkWidget* drawing_area = (GtkWidget *) data;
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
		gdk_draw_rectangle (pixmap,
		drawing_area->style->black_gc,
		TRUE,
		0,0,
		drawing_area->allocation.width,
		drawing_area->allocation.height);


		for(i=0;i<setCounter;i++) {
			totalLen += (gdk_string_width(my_font,
					STOCK_QUOTE(quotes->data)[i].price) + 10);
			if (!strcmp(props.output,"default")) {
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					totalLen += (gdk_text_width(extra_font,
								    STOCK_QUOTE(quotes->data)[i].change,1) + 10);
				}
				totalLen += (gdk_string_width(small_font,
						&STOCK_QUOTE(quotes->data)[i].change[1]) +10 );
			}
		}

		comp = 0 - totalLen;

		if (MOVE == 1) { MOVE = 0; } else { MOVE = 1; }

		if (MOVE == 1) {


		  if (!strcmp(props.scroll,"right2left")) {
			if (location > comp) {
				location--;
			}
			else {
				location = drawing_area->allocation.width;
			}

		  }
		  else {
                       if (location < drawing_area->allocation.width) {
                                location ++;
                        }
                        else {
                                location = comp;
                        }
		  }



		}

		for (i=0;i<setCounter;i++) {

			/* COLOR */
			if (STOCK_QUOTE(quotes->data)[i].color == GREEN) {
				gdk_gc_set_foreground( gc, &gdkUcolor );
			}
			else if (STOCK_QUOTE(quotes->data)->color == RED) {
				gdk_gc_set_foreground( gc, &gdkDcolor );
			}
			else {
				gdk_gc_copy( gc, drawing_area->style->white_gc );
			}

			tmpSym = STOCK_QUOTE(quotes->data)[i].price;
			gdk_draw_string (pixmap,my_font,
			gc,
			location + totalLoc ,14,STOCK_QUOTE(quotes->data)[i].price);
			totalLoc += (gdk_string_width(my_font,tmpSym) + 10); 


			if (!strcmp(props.output,"default")) {
				tmpSym = STOCK_QUOTE(quotes->data)[i].change;
				if (*(STOCK_QUOTE(quotes->data)[i].change)) {
					gdk_draw_text (pixmap,extra_font,
					     gc, location + totalLoc,
					     14,STOCK_QUOTE(quotes->data)[i].change,1);
					totalLoc += (gdk_text_width(extra_font,
							STOCK_QUOTE(quotes->data)[i].change,1) + 5);
				}
				gdk_draw_string (pixmap,small_font,
				     gc, location + totalLoc,
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
	static void about_cb (AppletWidget *widget, gpointer data) {
		GtkWidget *about;
		static const gchar *authors[] = {
			"Jayson Lorenzen <jayson_lorenzen@yahoo.com>",
			"Jim Garrison <garrison@users.sourceforge.net>",
			"Rached Blili <striker@dread.net>"
		};

		about = gnome_about_new (_("The GNOME Stock Ticker"), VERSION,
		"(C) 2000 Jayson Lorenzen, Jim Garrison, Rached Blili",
		authors,
		_("This program connects to "
		"a popular site and downloads current stock quotes.  "
		"The GNOME Stock Ticker is a free Internet-based application.  "
		"It comes with ABSOLUTELY NO WARRANTY.  "
		"Do not use the GNOME Stock Ticker for making investment decisions; it is for "
		"informational purposes only."),
		NULL);
		gtk_widget_show (about);

		return;
	}



	/*-----------------------------------------------------------------*/
	static void refresh_cb(AppletWidget *widget, gpointer data) {
		updateOutput();
	}


	/*-----------------------------------------------------------------*/
	static void zipLeft(GtkWidget *widget, gpointer data) {
		gchar *current;
		gint i;

		current = g_strdup(props.scroll);
		props.scroll = g_strdup("right2left");
		for (i=0;i<151;i++) {
			Repaint((gpointer)drawing_area);
		}
		props.scroll = g_strdup(current);
		g_free(current);
	}

	/*-----------------------------------------------------------------*/
	static void zipRight(GtkWidget *widget, gpointer data) {
		gchar *current;
		gint i;

		current = g_strdup(props.scroll);
		props.scroll = g_strdup("left2right");
		for (i=0;i<151;i++) {
			Repaint((gpointer)drawing_area);
		}
		props.scroll = g_strdup(current);
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
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(poutput,"nochange");
		else
			strcpy(poutput,"default");
			
	}

	/*-----------------------------------------------------------------*/
	static void toggle_scroll_cb(GtkWidget *widget, gpointer data) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(scroll,"left2right");
		else
			strcpy(scroll,"right2left");
			
	}


	/*-----------------------------------------------------------------*/
	static void toggle_arrows_cb(GtkWidget *widget, gpointer data) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(arrows,"arrows");
		else
			strcpy(arrows,"noArrows");
			
	}

	/*-----------------------------------------------------------------*/
	static void toggle_buttons_cb(GtkWidget *widget, gpointer data) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
			strcpy(buttons,"yes");
		else
			strcpy(buttons,"no");
	}


	/*-----------------------------------------------------------------*/
	void timeout_cb( GtkWidget *widget, GtkWidget *spin ) {
		timeout=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
		gnome_property_box_changed(GNOME_PROPERTY_BOX(pb));
	}




	/*-----------------------------------------------------------------*/
	static void apply_cb( GtkWidget *widget, void *data ) {
#if 0
		char *tmpText;

		tmpText = gtk_entry_get_text(GTK_ENTRY(tik_syms_entry));
		props.tik_syms = g_strdup(tmpText);
#endif
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



#if 0
	/*-----------------------------------------------------------------*/
	static gint destroy_cb( GtkWidget *widget, void *data ) {
		pb = NULL;
		return FALSE;
	}
#endif

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

        /*-----------------------------------------------------------------*/

	gint font_cb(GtkWidget *widget, gpointer data) {
		whichlabel = 1;
		font_selector(widget,data);
		return FALSE;
	}

        /*-----------------------------------------------------------------*/
	static gint font2_cb(GtkWidget *widget, gpointer data) {
		whichlabel = 2;
		font_selector(widget,data);
		return FALSE;
	}

        /*-----------------------------------------------------------------*/
        gint OkClicked( GtkWidget *widget, void *fontDialog ) {
                gchar *newFont = NULL;


                GtkFontSelectionDialog *fsd = 
			GTK_FONT_SELECTION_DIALOG(fontDialog);

                newFont = gtk_font_selection_dialog_get_font_name(fsd);
                new_font = g_strdup(newFont);
                gtk_widget_destroy(fontDialog);
		return FALSE;
        }

        /*-----------------------------------------------------------------*/
        gint QuitFontDialog( GtkWidget *widget, void *data ) {
		fontDialog = NULL;
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

	static void populateClist(GtkWidget *clist) {
	
		gchar *symbol[1];
		gchar *syms;
		gchar *temp;

		syms = g_strdup(props.tik_syms);

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

	static GtkWidget *symbolManager() { 
		GtkWidget *vbox;
		GtkWidget *mainhbox;
		GtkWidget *hbox;
		GtkWidget *clist;
		GtkWidget *swindow;
		GtkWidget *label;
		GtkWidget *entry;
		static GtkWidget *button;

		clist = gtk_clist_new(1);
		tik_syms_entry = clist;
		gtk_clist_set_selection_mode(GTK_CLIST(clist),
						GTK_SELECTION_EXTENDED);
		gtk_clist_column_titles_passive(GTK_CLIST(clist));
		gtk_widget_set_usize(clist,100,-1);
		gtk_clist_set_column_width(GTK_CLIST(clist),0,60);
		gtk_clist_set_reorderable(GTK_CLIST(clist),TRUE);
		gtk_signal_connect_object(GTK_OBJECT(clist),"drag_drop",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		swindow = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swindow),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(swindow),clist);

		populateClist(clist);

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
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));
		
		button = gtk_button_new_with_label(_("Add"));
		gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
		gtk_signal_connect(GTK_OBJECT(button),"clicked",
					GTK_SIGNAL_FUNC(addToClist), entry);
		gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

		hbox = gtk_hbox_new(FALSE,5);

		button = gtk_button_new_with_label(_("Remove Selected"));
		gtk_signal_connect(GTK_OBJECT(clist),"select-row",
				GTK_SIGNAL_FUNC(selected_cb),(gpointer)button);
		gtk_object_set_data(GTK_OBJECT(button),"clist",(gpointer)clist);
		gtk_signal_connect(GTK_OBJECT(button),"clicked",
					GTK_SIGNAL_FUNC(remFromClist),NULL);
		gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));
		gtk_widget_set_sensitive(button,FALSE);
		gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);

		gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
		mainhbox = gtk_hbox_new(FALSE,5);
		gtk_box_pack_start(GTK_BOX(mainhbox),swindow,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(mainhbox),vbox,FALSE,FALSE,0);


		gtk_widget_show_all(mainhbox);
		return(mainhbox);

	}
	
	/*-----------------------------------------------------------------*/
	static void properties_cb (AppletWidget *widget, gpointer data) {
		GtkWidget * vbox;
		GtkWidget * vbox2;
		GtkWidget * vbox3;
		GtkWidget * hbox3;
		GtkWidget *hbox;
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

		pb = gnome_property_box_new();

		gtk_window_set_title(GTK_WINDOW(pb), 
			_("Gnome Stock Ticker Properties"));

		vbox = gtk_vbox_new(GNOME_PAD, FALSE);
		vbox2 = gtk_vbox_new(GNOME_PAD, FALSE);

		panela = gtk_hbox_new(FALSE, 5); /* Color selection */
		panel1 = gtk_hbox_new(FALSE, 5); /* Symbol list */
		panel2 = gtk_hbox_new(FALSE, 5); /* Update timer */

		gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD);
		gtk_container_set_border_width(GTK_CONTAINER(vbox2), GNOME_PAD);

		timeout_label = gtk_label_new(_("Update Frequency in minutes:"));
		timeout_a = gtk_adjustment_new( timeout, 0.5, 128, 1, 8, 8 );
		timeout_c  = gtk_spin_button_new( GTK_ADJUSTMENT(timeout_a), 1, 0 );
		gtk_widget_set_usize(timeout_c,60,-1);

		gtk_box_pack_start_defaults( GTK_BOX(panel2), timeout_label );
		gtk_box_pack_start_defaults( GTK_BOX(panel2), timeout_c );
		gtk_box_pack_start(GTK_BOX(vbox), panel2, FALSE,
				    FALSE, GNOME_PAD);
	
		gtk_signal_connect_object(GTK_OBJECT(timeout_c), "changed",GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		gtk_signal_connect( GTK_OBJECT(timeout_a),"value_changed",
		GTK_SIGNAL_FUNC(timeout_cb), timeout_c );
		gtk_signal_connect( GTK_OBJECT(timeout_c),"changed",
		GTK_SIGNAL_FUNC(timeout_cb), timeout_c );
		gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(timeout_c),
		GTK_UPDATE_ALWAYS );

		label1 = gtk_label_new(_("Enter symbols delimited with \"+\" in the box below."));

		tik_syms_entry = gtk_entry_new_with_max_length(60);

		gtk_entry_set_text(GTK_ENTRY(tik_syms_entry), props.tik_syms ? props.tik_syms : "");
		gtk_signal_connect_object(GTK_OBJECT(tik_syms_entry), "changed",GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		check = gtk_check_button_new_with_label(_("Display only symbols and price"));
		check2 = gtk_check_button_new_with_label(_("Scroll left to right"));
		check3 = gtk_check_button_new_with_label(_("Display arrows instead of -/+"));
		check4 = gtk_check_button_new_with_label(_("Enable scroll buttons"));


		gtk_box_pack_start(GTK_BOX(vbox2), check, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox2), check2, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), check3, FALSE, FALSE, GNOME_PAD);
		gtk_box_pack_start(GTK_BOX(vbox), check4, FALSE, FALSE, GNOME_PAD);


		/* Set the checkbox according to current prefs */
		if (strcmp(props.output,"default")!=0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
							TRUE);
		if (strcmp(props.scroll,"right2left")!=0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check2),
							TRUE);
		if (strcmp(props.buttons,"yes") == 0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check4),
							TRUE);
		if (strcmp(props.arrows,"arrows") ==0)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check3),
							TRUE);

		gtk_signal_connect_object(GTK_OBJECT(check),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));
		gtk_signal_connect_object(GTK_OBJECT(check2),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));
		gtk_signal_connect_object(GTK_OBJECT(check3),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));
		gtk_signal_connect_object(GTK_OBJECT(check4),"toggled",
				GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

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
	
		sscanf( props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
	
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

		sscanf( props.dcolor, "#%02x%02x%02x", &dr,&dg,&db );

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
                                GTK_SIGNAL_FUNC(font_cb),GTK_OBJECT(pb));
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		hbox3 = gtk_hbox_new(FALSE, 5);
		label5 = gtk_label_new(_("Stock Change:"));
                fontButton = gtk_button_new_with_label(_("Select Font"));
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),label5);
                gtk_box_pack_start_defaults(GTK_BOX(hbox3),fontButton);
                gtk_box_pack_start_defaults(GTK_BOX(vbox3),hbox3);
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(font2_cb),GTK_OBJECT(pb));
                gtk_signal_connect_object(GTK_OBJECT(fontButton),"clicked",
                                GTK_SIGNAL_FUNC(changed_cb),GTK_OBJECT(pb));

		gtk_box_pack_start_defaults(GTK_BOX(panela),vbox3);



		gtk_box_pack_start(GTK_BOX(panel1), label1, FALSE, 
				   FALSE, GNOME_PAD);

		gtk_box_pack_start(GTK_BOX(vbox2), panela, FALSE,
				    FALSE, GNOME_PAD);

		hbox = symbolManager();

		gnome_property_box_append_page(GNOME_PROPERTY_BOX(pb), hbox,
		gtk_label_new(_("Symbols")));
		gnome_property_box_append_page(GNOME_PROPERTY_BOX(pb), vbox,
		gtk_label_new(_("Behavior")));
		gnome_property_box_append_page(GNOME_PROPERTY_BOX(pb), vbox2,
		gtk_label_new(_("Appearance")));

#if 0
		gtk_signal_connect_object(GTK_OBJECT(tik_syms_entry), 
				   "changed",GTK_SIGNAL_FUNC(changed_cb),
				   GTK_OBJECT(pb));
#endif

		gtk_signal_connect(GTK_OBJECT(pb), "apply",
				    GTK_SIGNAL_FUNC(apply_cb), NULL);

		gtk_widget_show_all(pb);
	}



	/*-----------------------------------------------------------------*/
	int main(int argc, char **argv) {
		GtkWidget * vbox;
		GtkWidget * frame;

		configFileName = g_strconcat (g_getenv ("HOME"), "/.gtik.conf", NULL);

		/* Initialize the i18n stuff */
		bindtextdomain (PACKAGE, GNOMELOCALEDIR);
		textdomain (PACKAGE);


		/* intialize, this will basically set up the applet, corba and
		call gnome_init */
		applet_widget_init(GTIK_APPLET_NAME, VERSION, argc, argv,
				    NULL, 0, NULL);
		gnome_vfs_init();

		/* create a new applet_widget */
		applet = applet_widget_new(GTIK_APPLET_NAME);
		/* in the rare case that the communication with the panel
		failed, error out */
		if (!applet)
			g_error("Can't create applet!\n");

		quotes = g_array_new(FALSE, FALSE, sizeof(StockQuote));	

		vbox = gtk_hbox_new (FALSE,0);
		leftButton = gtk_button_new_with_label("<<");
		rightButton = gtk_button_new_with_label(">>");
		gtk_signal_connect (GTK_OBJECT (leftButton),
					"clicked",
					GTK_SIGNAL_FUNC(zipLeft),NULL);
		gtk_signal_connect (GTK_OBJECT (rightButton),
					"clicked",
					GTK_SIGNAL_FUNC(zipRight),NULL);


		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);


		drawing_area = gtk_drawing_area_new();
		gtk_drawing_area_size(GTK_DRAWING_AREA (drawing_area),200,20);

		gtk_widget_show(drawing_area);
		gtk_container_add(GTK_CONTAINER(frame),drawing_area);
		gtk_widget_show(frame);

		gtk_box_pack_start(GTK_BOX(vbox),leftButton,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX (vbox), frame,TRUE,TRUE,0);
		gtk_box_pack_start(GTK_BOX(vbox),rightButton,FALSE,FALSE,0);

		gtk_widget_show(vbox);

		applet_widget_add (APPLET_WIDGET (applet), vbox);

		gtk_signal_connect(GTK_OBJECT(drawing_area),"expose_event",
		(GtkSignalFunc) expose_event, NULL);

		gtk_signal_connect(GTK_OBJECT(drawing_area),"configure_event",
		(GtkSignalFunc) configure_event, NULL);


		/* CLEAN UP WHEN REMOVED FROM THE PANEL */
		gtk_signal_connect(GTK_OBJECT(applet), "destroy",
			GTK_SIGNAL_FUNC(destroy_applet), NULL);



		gtk_widget_show (applet);
		create_gc();

		/* add an item to the applet menu */
		applet_widget_register_stock_callback(APPLET_WIDGET(applet),
			"refresh",
			GNOME_STOCK_MENU_REFRESH,
			_("Refresh"),
			refresh_cb,
			NULL);



		/* add an item to the applet menu */
		applet_widget_register_stock_callback(APPLET_WIDGET(applet),
			"properties",
			GNOME_STOCK_MENU_PROP,
			_("Properties..."),
			properties_cb,
			NULL);


		/* add an item to the applet menu */
		applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "about",
                                              GNOME_STOCK_MENU_ABOUT,
                                              _("About..."),
                                              about_cb,
                                              NULL);





		properties_load(APPLET_WIDGET(applet)->privcfgpath);
		properties_set(FALSE);

		gtk_signal_connect(GTK_OBJECT(applet),"save_session",
		GTK_SIGNAL_FUNC(applet_save_session), NULL);


		/* KEEPING TIMER ID FOR CLEANUP IN DESTROY */
		drawTimeID   = gtk_timeout_add(2,Repaint,drawing_area);
		updateTimeID = gtk_timeout_add(props.timeout * 60000,
				   (gpointer)updateOutput,"NULL");


		if (!strcmp(props.buttons,"yes")) {
			gtk_widget_show(leftButton);
			gtk_widget_show(rightButton);
		}

		updateOutput();

		/* special corba main loop */
		applet_widget_gtk_main ();

		return 0;
	}



/* JHACK */
	/*-----------------------------------------------------------------*/
	static void destroy_applet(GtkWidget *widget, gpointer data) {
	
		if (drawTimeID > 0) { gtk_timeout_remove(drawTimeID); }
		if (updateTimeID >0) { gtk_timeout_remove(updateTimeID); }
		gtk_widget_destroy(drawing_area);

	}




	
/*HERE*/
	/*-----------------------------------------------------------------*/
	static void reSetOutputArray() {

		while (quotes->len) {
			int i = quotes->len - 1;
			g_free(STOCK_QUOTE(quotes->data)[i].price);
			g_free(STOCK_QUOTE(quotes->data)[i].change);
			g_array_remove_index(quotes, i);
		}

		setCounter = 0;
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
	char *splitChange(char *data) {
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
			if (symbolfont)
				var3[0] = 221;
			var4[0] = '(';
		}
		else if (var3[0] == '-') {
			if (symbolfont)
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
	static void setOutputArray(char *param1) {
		StockQuote quote;
		char *price;
		char *change;

		price = splitPrice(param1);
		change = splitChange(param1);

		quote.price = g_strdup(price);
		quote.change = g_strdup(change);

		if (strstr(change,"-"))
			quote.color = RED;
		else if (strstr(change,"+"))
			quote.color = GREEN;
		else
			quote.color = WHITE;

#if 0
		g_message("Param1: %s\nPrice: %s\nChange: %s\nColor: %d\n\n", param1, price, change, quote.color);
#endif

		g_array_append_val(quotes, quote);

		setCounter++;
	}



	/*-----------------------------------------------------------------*/

	void setup_colors(void) {
		GdkColormap *colormap;

		colormap = gtk_widget_get_colormap(drawing_area);

		gdk_color_parse(props.ucolor, &gdkUcolor);
		gdk_color_alloc(colormap, &gdkUcolor);

		gdk_color_parse(props.dcolor, &gdkDcolor);
		gdk_color_alloc(colormap, &gdkDcolor);
	}

	
	int create_gc(void) {
		gc = gdk_gc_new( drawing_area->window );
		gdk_gc_copy( gc, drawing_area->style->white_gc );
		return 0;
	}







	void ucolor_set_cb(GnomeColorPicker *cp) {
		guint8 r,g,b;
		gnome_color_picker_get_i8(cp,
					&r,
					&g,
					&b,
					NULL);

		g_snprintf( props.ucolor, max_rgb_str_size,
		"#%02x%02x%02x", r, g, b );
		gnome_property_box_changed(GNOME_PROPERTY_BOX(pb));
	}



	void dcolor_set_cb(GnomeColorPicker *cp) {
		guint8 r,g,b;
		gnome_color_picker_get_i8(cp,
					&r,
					&g,
					&b,
					NULL);
		g_snprintf( props.dcolor, max_rgb_str_size,
		"#%02x%02x%02x", r, g, b );
		gnome_property_box_changed(GNOME_PROPERTY_BOX(pb));
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



