/***************************************************
July 5, 1998
Justin Maurer
justin@slashdot.org
http://slashdot.org/
***************************************************/

#include <config.h>
#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

GtkWidget *applet;
GtkWidget *Table;
GtkWidget *Headline;
GtkWidget *HeadlineEventBox;

FILE *fresh_file = NULL;

int headlinecounter = 0;

GtkWidget *FreshPixmap;
GdkImlibImage *FreshGif;

GtkWidget *create_fresh_widget(GtkWidget *window);

int main(int argc, char *argv[])
{
GtkWidget *fresh;

bindtextdomain(PACKAGE, GNOMELOCALEDIR);
textdomain(PACKAGE);

applet_widget_init_defaults("fresh_applet", NULL, argc, argv, 0, NULL,
                            argv[0]);

applet = applet_widget_new();
  if (!applet)
    g_error(_("Can't create applet!\n"));

gtk_widget_realize(applet);
fresh = create_fresh_widget(applet);
gtk_widget_show(fresh);

applet_widget_add(APPLET_WIDGET(applet), fresh);
gtk_widget_show(applet);

applet_widget_gtk_main();
return 0;
}
				
GtkWidget *create_fresh_widget(GtkWidget *window)
{
Table = gtk_table_new(1, 1, FALSE);
pixfname = g_strdup(gnome_unconditional_pixmap_file("freshsplash.gif"));
  if (g_file_exists(pixfname)) 
    {
      FreshGif = gdk_imlib_load_image(pixfname);
      gdk_imlib_render(FreshGif, FreshGif->rgb_width, FreshGif->rgb_height);
      FreshPixmap = gtk_pixmap_new(FreshGif->pixmap, FreshGif->shape_mask);
      gtk_widget_show(FreshPixmap);
      g_free(pixfname);
    } 

  else
    {
      g_warning("Unable to find %s", pixfname);
    }

gtk_timeout_add(7200000, get_current_headlines, NULL);
gtk_timeout_add(TICKTIME, set_current_headline, NULL);

gtk_table_attach_defaults(GTK_TABLE(Table), FreshPixmap, 0, 1, 0, 1);
return Table;
}

static int set_current_headline(gpointer data)
{
char buf[80];
char headline[12][80];

  if (firsttime == 0)
    {
      get_current_headlines(NULL);
      gtk_widget_destroy(SlashPixmap);

      Headline = gtk_text_new(NULL, NULL);
      gtk_text_set_editable(GTK_TEXT(Headline), FALSE);
      gtk_text_set_word_wrap(GTK_TEXT(Headline), TRUE);
      gtk_table_attach_defaults(GTK_TABLE(Table), Headline, 1, 2, 0, 1);

      gtk_widget_show(Headline);
      firsttime = 1;
  }

  if (headlinecounter == 12)
    {
      headlinecounter = 0;
    }

  if (headlinecounter == 12)
    {
      headlinecounter = 0;
    }
    
if ((slash_file = fopen("freshnews", "r")) == NULL)
  {
  fprintf(stderr, "Failed to open file \"%s\": %s\n", fresh_file,
          strerror(errno));
    }

headlinecounter = 0;
	    
while ((fgets(buf, sizeof(buf), fresh_file) != NULL) && (headlinecounter < 12))
  {
    strncpy(&headlinestring[headlinecounter], buf, 80);
    fgets(buf, sizeof(buf), fresh_file);
    fgets(buf, sizeof(buf), fresh_file);
    headlinecounter++;
  }    

if (headlinestring[headlinecounter][0] == '\0')
  headlinecounter = 0;
  gtk_text_freeze(GTK_TEXT(Headline));
  gtk_text_backward_delete(GTK_TEXT(Headline),
  gtk_text_get_length(GTK_TEXT(Headline)));
  gtk_text_insert(GTK_TEXT(Headline), NULL, NULL, NULL,
	          headlinestring[headlinecounter],
	          strlen(headlinestring[headlinecounter]));
  gtk_text_thaw(GTK_TEXT(Headline));
  headlinecounter++;

fclose(fresh_file);
return TRUE;
}

static int get_current_headlines(gpointer data)
{
  if ((fresh_file = fopen("freshnews", "w")) == NULL)
  {
    fprintf(stderr, "Failed to open file \"%s\": %s\n", fresh_file,
            strerror(errno));
  }

http_get_to_file("files.freshmeat.net", 80, "/file/recentnews.txt", fresh_file);
fclose(fresh_file);
return TRUE;
}
	    
