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
static digits[6] =		/* more digits stuff */
{0, 0, 0, 0, 0, 0};
int blink = 0;			/* to blink, or not to blink */
char **xpms[] =			/* array of xpms. thanks miguel */
{t0_xpm, t1_xpm, t2_xpm, t3_xpm, t4_xpm, t5_xpm, t6_xpm, t7_xpm, t8_xpm, t9_xpm};
char wtitle[] = "jbc!";		/* wtitle is the window titlebar text */
char buf[128];


/*
 * GtkWidget is the storage type for widgets 
 */
GtkWidget *window, *box, *event_box, *menu, *menu_about, *menu_quit, *root_menu;
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

gint jbc_about() {
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *button;

    dialog = gtk_dialog_new();

    label = gtk_label_new("Jon's BCD Clock\n(C) 1998 Jon Anhold\n<jon@snoopy.net>");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, TRUE, 10);
    gtk_widget_show(label);

    button = gtk_button_new_with_label (("Ok"));
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->action_area), button, TRUE, TRUE, 10);
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC (gtk_widget_destroy), GTK_OBJECT(dialog));
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button);
    gtk_widget_show (button);

    gtk_widget_show(dialog);

}

gint jbc_menu() {
	menu = gtk_menu_new();
	menu_about = gtk_menu_item_new_with_label("About..");
	gtk_menu_append(GTK_MENU(menu), menu_about);
	gtk_signal_connect_object(GTK_OBJECT(menu_about), "activate",
		GTK_SIGNAL_FUNC(jbc_about), (gpointer) g_strdup("About.."));
	menu_quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_append(GTK_MENU(menu), menu_quit);
	gtk_signal_connect_object(GTK_OBJECT(menu_quit), "activate",
		GTK_SIGNAL_FUNC(gtk_exit), (gpointer) g_strdup("quit"));
	gtk_widget_show(menu_about);
	gtk_widget_show(menu_quit);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, NULL, 20);
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
				

 
  /* ??? */
/*  style = gtk_widget_get_style (applet); */

  /* create GdkPixmaps from the included xpms */
  for (i = 0; i < 10; ++i)
    {
      pw[i] = gnome_pixmap_new_from_xpm_d ((gchar **)xpms[i]);
    }

  /* not worth a for loop here */
  pwc[0] = gnome_pixmap_new_from_xpm_d ((gchar **)tcolon_xpm);
  pwc[1] = gnome_pixmap_new_from_xpm_d ((gchar **)tcolon_xpm);
  pwbc[0] = gnome_pixmap_new_from_xpm_d ((gchar **)tbcolon_xpm);
  pwbc[1] = gnome_pixmap_new_from_xpm_d ((gchar **)tbcolon_xpm);

  /* show the pixmap widgets */
  for (i = 0; i < 10; ++i)
    {
      gtk_widget_show (pw[i]);
    }

  /* again.. not enough for for's */
  gtk_widget_show (pwc[0]);
  gtk_widget_show (pwc[1]);
  gtk_widget_show (pwbc[0]);
  gtk_widget_show (pwbc[1]);

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
  gtk_timeout_add (900, do_clock, NULL);

  /* call gtk_main that sits and waits for events */

  applet_widget_gtk_main();

  return(0);  
}
/* end */
