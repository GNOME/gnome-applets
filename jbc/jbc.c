/********************************************************
 * jbc.c - Jon's Binary Clock                           *
 *                                                      *
 * Jon Anhold (jon@snoopy.net) 04.07.98                 *
 *                                                      *
 * Clock engine by Roger Gulbranson (rlg@iagnet.net).   *
 *                                                      *
 * Displays a binary clock using gtk+ 0.99.9            *
 *                                                      *
 * Now with much niftier pixmaps & blinking cursors     *
 *                                                      *
 * Please let me know what you think! This is my first  *
 * C project, as well as my first X11/GTK+ project.     *
 * If there's something you see that can be done better,*
 * I would really like to know. Drop me a note!         *
 ********************************************************/

#include "jbc.h"
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

/* initialize variables */
struct tm atime;		/* this is the time structure */
time_t thetime;  		/* variable to grab the time */
int d[6];        		/* array of digits */
int i;           		/* generic counter for the for's */
static int digits[6] = {0, 0, 0, 0, 0, 0};
int blink = 0;			/* to blink, or not to blink */
int timeout_id;
char **xpms[] =			/* array of xpms. thanks miguel */
{t0_xpm, t1_xpm, t2_xpm, t3_xpm, t4_xpm, t5_xpm, t6_xpm, t7_xpm, t8_xpm, t9_xpm};
char wtitle[] = "jbc!";		/* wtitle is the window titlebar text */
char buf[128];


/*
 * GtkWidget is the storage type for widgets 
 */
GtkWidget *window, *box, *event_box;
GnomePixmap *pw[10], *pwc[2], *pwbc[2];
/*
 * GdkPixmap is the storage type for pixmaps
 */ 
GdkPixmap *p[10], *pc[2], *pbc[2];
/*
 * GtkBitmaps is used for the pixmap masks
 */
GdkBitmap *mask, *mask2;
/*
 * ??
 */
GtkStyle *style;

static void about_cb(AppletWidget *widget, gpointer data)
{
		GtkWidget *about;
		const gchar *authors[8];
		gchar version[32];
		
		sprintf(version,_("%d.%d.%d"),APPLET_VERSION_MAJ,
			APPLET_VERSION_MIN, APPLET_VERSION_REV);
			
		authors[0] = _("Jon Anhold <jon@snoopy.net>");
		authors[1] = _("Roger Gulbranson <rlg@qual.net>");
		authors[2] = NULL;
		
		about = gnome_about_new( _("Jon's Binary Clock"), version,
				_("(C) 1998"),
				authors,
				_("A binary clock for your panel\nhttp://snoopy.net/~jon/jbc/\n"),
				NULL);
		gtk_widget_show (about);
}

static void
destroy_clock(GtkWidget * widget, void *data)
{
	   GtkWidget *applet;
           gtk_timeout_remove(timeout_id);
           g_free(GTK_OBJECT(applet));
}


/* do nifty clock stuff */
gint do_clock() {

      /* get unix time */
      thetime = time (NULL);
      /* 
       * localtime converts unix time into something usable,
       * broken down into the tm structure
       */
      localtime_r (&thetime, &atime);

      /* parse time into individual digits */
      d[0] = atime.tm_hour / 10;
      d[1] = atime.tm_hour % 10;
      d[2] = atime.tm_min / 10;
      d[3] = atime.tm_min % 10;
      d[4] = atime.tm_sec / 10;
      d[5] = atime.tm_sec % 10;
      /* routine that does the nifty blinking colons */
      if (d[5] != digits[5]) {
      if (blink == 0) /* if no blink, display colons */
	{
	/*
	  gnome_pixmap_load_xpm_d (pwc[0], tcolon_xpm);
	  gnome_pixmap_load_xpm_d (pwc[1], tcolon_xpm);
	  blink = 0;
	*/
	}
      else /* otherwise display the blank spacer pixmaps */
	{
	/*
	  gnome_pixmap_load_xpm_d (pwc[0], tbcolon_xpm);
	  gnome_pixmap_load_xpm_d (pwc[1], tbcolon_xpm);
	  blink = 0;
	*/
	}
      }
      /* big routine to figure out what pixmaps to display for each digit */
      /* whee */
      for (i = 0; i < 6; ++i)
	{
	  /*
	   * this if statement makes sure we only update
	   * digits that have changed
	   */
	  if (d[i] != digits[i])
	    {
	      /* this function changes the pixmap that the widget is managing */
               gnome_pixmap_load_xpm_d (pw[i], xpms[d[i]]);
	      /* gtk_pixmap_set ((GtkPixmap *)pw[i], p[d[i]], mask); */
	      /* set this so it doesn't update unless it has to */
	      digits[i] = d[i];
	    }
	}
      return 1;
}


/* main */
int
main (int argc, char *argv[])
{

 	GtkWidget *applet;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE); 	
	applet_widget_init_defaults("jbc_applet", NULL, argc, argv, 0,
					NULL, argv[0]);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(GTK_WIDGET(window));
	applet = applet_widget_new();
	if (!applet) 
		g_error("Can't create applet!\n");					
				
        applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                                      "about",
                                                      GNOME_STOCK_MENU_ABOUT,
                                                      _("About..."),
                                                      about_cb, applet);
   
                gtk_signal_connect(GTK_OBJECT(applet), "destroy",
                              (GtkSignalFunc) destroy_clock,
                              applet);
   

  /* ??? */
/*  style = gtk_widget_get_style (applet); */

  /* create GdkPixmaps from the included xpms */
  for (i = 0; i < 10; ++i)
    {
      (GtkPixmap **)pw[i] = gnome_pixmap_new_from_xpm_d ((gchar **)xpms[i]);
    }

  /* not worth a for loop here */
  
  (GtkPixmap **)pwc[0] = gnome_pixmap_new_from_xpm_d ((gchar **)tcolon_xpm);
  (GtkPixmap **)pwc[1] = gnome_pixmap_new_from_xpm_d ((gchar **)tcolon_xpm);
  (GtkPixmap **)pwbc[0] = gnome_pixmap_new_from_xpm_d ((gchar **)tbcolon_xpm);
  (GtkPixmap **)pwbc[1] = gnome_pixmap_new_from_xpm_d ((gchar **)tbcolon_xpm);

  /* show the pixmap widgets */
  for (i = 0; i < 10; ++i)
    {
      gtk_widget_show (GTK_WIDGET(pw[i]));
    }

  /* again.. not enough for for's */
  gtk_widget_show (GTK_WIDGET(pwc[0]));
  gtk_widget_show (GTK_WIDGET(pwc[1]));
  gtk_widget_show (GTK_WIDGET(pwbc[0]));
  gtk_widget_show (GTK_WIDGET(pwbc[1]));

  /* create the gtk_hbox to pile these pixmap widgets into */
  box = gtk_hbox_new (FALSE, 0);


  /* pile the widgets into the box 
   *
   * can't for loop here, because of the colons. 
   *
   */
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pw[0]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pw[1]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pwc[0]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pw[2]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pw[3]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pwc[1]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pw[4]);
  gtk_container_add (GTK_CONTAINER (box), (GtkWidget *)pw[5]);
  applet_widget_add (APPLET_WIDGET (applet), box);

  /* show the window widget and the box widget */
  gtk_widget_show (applet);
  gtk_widget_show (box);


  /* this tells gtk_main() to call do_clock() every 900 milliseconds */
  (gint **)timeout_id = gtk_timeout_add (900, do_clock, NULL);

  /* call gtk_main that sits and waits for events */

  applet_widget_gtk_main();

  return(0);  
}
/* end */
