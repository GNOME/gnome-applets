/*****************************************************
The table is too big on purpose. I will later implement the ability to have 
the topical icon show up.

Had trouble making headlines respond to mouse clicks - could do it, but 
i don't know how do it and make it stil respond to right-mouse clicks. all 
i know is that it involves setting up an event box, attaching a signal handler 
to it, and playing around with a GdkEvent (something about eventbox->button3 
needing to return false.

i'd be grateful if someone could hack it in. should be trivial for anyone who 
knows anything about GdkEvents.

Also, any way to make the text less bland.
Any way to center it? Make it bigger?

June 29, 1998
Justin Maurer
mike911@clark.net
http://slashdot.org/
***************************************************/
#undef DEBUG

#include <config.h>
#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

GtkWidget *applet;
GtkWidget *Table;
GtkWidget *Headline;
GtkWidget *Author;
GtkWidget *HeadlineEventBox;

int firsttime = 0;
int topiccounter = 0;
int headlinecounter = 0;
int authorcounter = 0;

FILE *slash_file = NULL;

GtkWidget *SlashPixmap;
GdkImlibImage *SlashGif;

GtkWidget *create_slash_widget(GtkWidget *window);
static int slashapp_refresh(gpointer data);
static int set_current_headline(gpointer data);
static int getarticle(gpointer data);
static int get_current_headlines(gpointer data);
void aboutwindow(void);

int main(int argc, char *argv[])
{
GtkWidget *slash;

bindtextdomain(PACKAGE, GNOMELOCALEDIR);
textdomain(PACKAGE);

applet_widget_init_defaults("slash_applet", NULL, argc, argv, 0, NULL, 
			    argv[0]);

applet = applet_widget_new();
if (!applet)
    g_error(_("Can't create applet!\n"));
gtk_widget_realize(applet);

slash = create_slash_widget(applet);
gtk_widget_show(slash);

applet_widget_add(APPLET_WIDGET(applet), slash);
gtk_widget_show(applet);

applet_widget_gtk_main();
return 0;
}

GtkWidget *create_slash_widget(GtkWidget *window)
{
char *slashappdir;
char *pixfname;

Table = gtk_table_new(2, 2, FALSE);
pixfname = g_strdup(gnome_unconditional_pixmap_file("slashsplash.gif"));
if (g_file_exists(pixfname)) {
  SlashGif = gdk_imlib_load_image(pixfname);
  gdk_imlib_render(SlashGif, SlashGif->rgb_width, SlashGif->rgb_height);
  SlashPixmap = gtk_pixmap_new(SlashGif->pixmap, SlashGif->shape_mask);
  gtk_widget_show(SlashPixmap);
  g_free(pixfname);
} else {
  g_warning("Unable to find %s", pixfname);
}
gtk_timeout_add(1800000, get_current_headlines, NULL);
gtk_timeout_add(3000, set_current_headline, NULL);

gtk_table_attach_defaults(GTK_TABLE(Table), SlashPixmap, 0, 2, 0, 2);
return Table;
}

static int set_current_headline(gpointer data)
{
char buf[80]; /* Is that ok? */
char headlinestring[12][80];
char authorstring[12][8];
char currentheadline[80];

if (firsttime == 0)
  {
get_current_headlines(NULL);
gtk_widget_destroy(SlashPixmap);

Headline = gtk_text_new(NULL, NULL);
Author = gtk_text_new(NULL, NULL);

gtk_text_set_editable(GTK_TEXT(Headline), FALSE);
gtk_text_set_editable(GTK_TEXT(Author), FALSE);

gtk_text_set_word_wrap(GTK_TEXT(Headline), TRUE);
gtk_text_set_word_wrap(GTK_TEXT(Author), TRUE);

gtk_table_attach_defaults(GTK_TABLE(Table), Headline, 1, 2, 0, 1);
gtk_table_attach_defaults(GTK_TABLE(Table), Author, 1, 2, 1, 2);

gtk_widget_show(Author);
gtk_widget_show(Headline);
firsttime = 1;
  }


if (topiccounter == 12)
{
    topiccounter = 0;
}

if (headlinecounter == 12)
{
    headlinecounter = 0;
}

if (authorcounter == 12)
{
    authorcounter = 0;
}

if ((slash_file = fopen("slashnews", "r")) == NULL)
  {
fprintf(stderr, "Failed to open file \"%s\": %s\n", slash_file,
        strerror(errno));
  }
topiccounter = 0;
while ((fgets(buf, sizeof(buf), slash_file) != NULL) && (topiccounter < 12))
{
  if (strcmp(buf, "%%\n") == 0) 
  {
    if (fgets(buf, sizeof(buf), slash_file) != NULL)
    {
      strncpy(&headlinestring[topiccounter], buf, 80);
      g_print("%d long: %s", strlen(headlinestring[topiccounter]), headlinestring[topiccounter]);
      fgets(buf, sizeof(buf), slash_file);
      strncpy(&tauthorstring[topiccounter], buf, 8);
      topiccounter++;
    }
  }
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

if (authorstring[headlinecounter][0] == '\0')
     authorcounter = 0;
gtk_text_freeze(GTK_TEXT(Author));
gtk_text_backward_delete(GTK_TEXT(Author), 
			 gtk_text_get_length(GTK_TEXT(Author)));
gtk_text_insert(GTK_TEXT(Author), NULL, NULL, NULL, 
		authorstring[headlinecounter], 
		strlen(headlinestring[headlinecounter]));
gtk_text_thaw(GTK_TEXT(Author));
headlinecounter++;

fclose(slash_file);
return TRUE;
}

static int get_current_headlines(gpointer data)
{
if ((slash_file = fopen("slashnews", "w")) == NULL)
  {
fprintf(stderr, "Failed to open file \"%s\": %s\n", slash_file,
        strerror(errno));
  }
http_get_to_file("slashdot.org", 80, "/ultramode.txt", slash_file);
fclose(slash_file);
return TRUE;
}
