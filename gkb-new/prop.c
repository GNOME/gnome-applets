/* File: prop.c
 * Purpose: GNOME Keyboard switcher property box
 *
 * Copyright (C) 1999 Free Software Foundation
 * Author: Szabolcs BAN <shooby@gnome.hu>, 1998-2000
 *
 * Some of functions came from Helixcode's keyboard grabbing sections.
 * Other functions, ideas stolen from other applets, for example
 * from the fish applet, Wanda.
 *
 * Thanks for aid of George Lebl <jirka@5z.com> and solidarity
 * Balazs Nagy <js@lsc.hu>, Charles Levert <charles@comm.polymtl.ca>
 * and Emese Kovacs <emese@gnome.hu> for her docs and ideas.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <X11/keysym.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include "gkb.h"

typedef struct _PropWg PropWg;
struct _PropWg
{
  GdkPixmap *pix;

  char *name;
  char *command;
  char *flag;

  GtkWidget *diff_ch;
  GtkWidget *propbox;
  GtkWidget *notebook;
  GtkWidget *label1;
  GtkWidget *iconentry;
  GtkWidget *keymapname;
  GtkWidget *commandinput;
  GtkWidget *iconpathinput;
  GtkWidget *scrolledwin, *scrolledwinl;
  GtkWidget *hidebox, *hfa, *hfn;
  GtkWidget *frame21, *frame22, *label25, *entry21;
  GtkWidget *vbox1, *hbox1, *vbox2, *hbox2, *hbox3, *hboxmap;
  GtkWidget *frame1, *frame2, *frame3, *frame4, *frame6;
  GtkWidget *vbox21, *hbox21;
  GtkWidget *list, *tree, *iconentry21;
  GtkWidget *newkeymap, *delkeymap;
};

typedef struct _GKBpreset GKBpreset;
struct _GKBpreset
{
  gchar *name, *lang, *country; /* Names for items */
  gchar *command; /* Full switching command */
  gchar *codepage; /* Codepage, information only */
  gchar *flag; /* Flag filename */
  gchar *label; /* Label for label mode */
  gchar *arch; /* Sun, IBM, i386, or DEC */
  gchar *type; /* 101, 102, 105, microsoft, or any type */
};

typedef struct _CountryData CountryData;
struct _CountryData
{
  GtkWidget * widget;
  GList * keymaps;
};

typedef struct _LangData LangData;
struct _LangData
{
  GtkWidget * widget;
  GHashTable * countries;
};

typedef struct _KeymapData KeymapData;
struct _KeymapData
{
 GtkWidget * widget;
 GKBpreset * preset;
};

static void prophelp_cb (AppletWidget * widget, gpointer data);
static void addhelp_cb (AppletWidget * widget, gpointer data);
static void edithelp_cb (AppletWidget * widget);
gint addwadd_cb (GtkWidget * addbutton, GtkWidget * ctree);
gint wdestroy_cb (GtkWidget *closebutton, GtkWidget *window);
static void list_select_cb ();
static void apply_edited_cb (GtkWidget * button, gint pos);
void list_show (gint pos);

/* Phew.. Globals... */
static gchar *prefixdir;
GKBpreset * a_keymap;

static GList *
find_presets ()
{
  DIR *dir;
  struct dirent *actfile;
  GList *diritems = NULL;

  /* TODO: user's local presets */

  dir = opendir (gnome_unconditional_datadir_file ("gnome/gkb/"));

  prefixdir = g_strdup (gnome_unconditional_datadir_file ("gnome/gkb/"));

  if (dir == NULL)
    return NULL;

  while ((actfile = readdir (dir)) != NULL)
    {
      if (!strstr (actfile->d_name, ".keyprop"))
	continue;

      if (actfile->d_name[0] != '.')
	{
	  diritems = g_list_insert_sorted (diritems,
					   g_strdup (actfile->d_name),
					   (GCompareFunc) strcmp);
	}
    }
  return diritems;
}

static GList *
gkb_preset_load (GList * list)
{
  GKBpreset * val;
  GList * retlist, * templist;
  gchar * prefix;
  gchar * tname, * tcodepage;
  gchar * ttype, * tarch, * tcommand;
  gchar * set, * filename;
  gint i, knum = 1;

  templist = list;
  retlist = NULL;

  for(templist =list; templist != NULL ;templist = g_list_next (templist))
    {

      filename = templist->data;

      g_assert (filename != NULL);

      prefix =
	g_strconcat ("=", prefixdir, "/", filename, "=/Keymap Entry/", NULL);

      gnome_config_push_prefix (prefix);
      g_free (prefix);

      knum = gnome_config_get_int ("Countries");

      tname = gnome_config_get_translated_string ("Name");
      tcodepage = gnome_config_get_string ("Codepage");
      ttype = gnome_config_get_string ("Type");
      tarch = gnome_config_get_string ("Arch");
      tcommand = gnome_config_get_string ("Command");

      gnome_config_pop_prefix ();

      for (i = 0; i < knum; i++)
	{
	  val = g_new0 (GKBpreset, 1);


	  set = g_strdup_printf ("=/Country %d/", i);
	  prefix = g_strconcat ("=", prefixdir, "/", filename, set, NULL);
	  gnome_config_push_prefix (prefix);
	  g_free (prefix);
	  g_free (set);

	  val->name = g_strdup (tname);
	  val->codepage = g_strdup (tcodepage);
	  val->type = g_strdup (ttype);
	  val->arch = g_strdup (tarch);
	  val->command = g_strdup (tcommand);

          val->flag = gnome_config_get_string ("Flag");
          val->label = gnome_config_get_string ("Label");

	  val->lang = gnome_config_get_translated_string ("Language");
	  val->country = gnome_config_get_translated_string ("Country");

	  retlist = g_list_append(retlist,val);

	  gnome_config_pop_prefix ();
	}

      g_free (tname);
      g_free (tcodepage);
      g_free (ttype);
      g_free (tarch);
      g_free (tcommand);

    }
  return retlist;
}

static void
switch_normal_cb (GnomePropertyBox * pb)
{
 gkb->tempsmall = 0;
 gkb->tempsize =
    applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
changed_cb (GnomePropertyBox * pb)
{
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

/* --- keygrab --- */

static gboolean
string_empty (const char *string)  
{
        if (string == NULL ||
            string[0] == '\0')
		return TRUE;
	else
            	return FALSE;
}


gboolean
convert_string_to_keysym_state(const char *string,
			       guint *keysym,
			       guint *state)
{
	char *s, *p;

	g_return_val_if_fail (keysym != NULL, FALSE);
	g_return_val_if_fail (state != NULL, FALSE);
	
	*state = 0;
	*keysym = 0;

	if(string_empty (string) ||
	   strcmp (string, "Disabled") == 0 ||
	   strcmp (string, _("Disabled")) == 0)
		return FALSE;

	s = g_strdup (string);

	gdk_error_trap_push ();

	p = strtok (s, "-");
	while (p != NULL) {
		if(strcmp(p, "Control")==0) {
			*state |= GDK_CONTROL_MASK;
		} else if(strcmp(p, "Lock")==0) {
			*state |= GDK_LOCK_MASK;
		} else if(strcmp(p, "Shift")==0) {
			*state |= GDK_SHIFT_MASK;
		} else if(strcmp(p, "Mod1")==0) {
			*state |= GDK_MOD1_MASK;
		} else if(strcmp(p, "Mod2")==0) {
			*state |= GDK_MOD2_MASK;
		} else if(strcmp(p, "Mod3")==0) {
			*state |= GDK_MOD3_MASK;
		} else if(strcmp(p, "Mod4")==0) {
			*state |= GDK_MOD4_MASK;
		} else if(strcmp(p, "Mod5")==0) {
			*state |= GDK_MOD5_MASK;
		} else {
			*keysym = gdk_keyval_from_name(p);
			if(*keysym == 0) {
				gdk_flush();
				gdk_error_trap_pop();
				g_free(s);
				return FALSE;
			}
		} 
		p = strtok(NULL, "-");
	}

	gdk_flush ();
	gdk_error_trap_pop ();

	g_free (s);

	if (*keysym == 0)
		return FALSE;

	return TRUE;
}

char *
convert_keysym_state_to_string(guint keysym,
						 guint state)
{
	GString *gs;
	char *sep = "";
	char *key;

	if(keysym == 0)
		return g_strdup(_("Disabled"));

	gdk_error_trap_push();
	key = gdk_keyval_name(keysym);
	gdk_flush();
	gdk_error_trap_pop();
	if(!key) return NULL;

	gs = g_string_new(NULL);

	if(state & GDK_CONTROL_MASK) {
		/*g_string_append(gs, sep);*/
		g_string_append(gs, "Control");
		sep = "-";
	}
	if(state & GDK_LOCK_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Lock");
		sep = "-";
	}
	if(state & GDK_SHIFT_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Shift");
		sep = "-";
	}
	if(state & GDK_MOD1_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Mod1");
		sep = "-";
	}
	if(state & GDK_MOD2_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Mod2");
		sep = "-";
	}
	if(state & GDK_MOD3_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Mod3");
		sep = "-";
	}
	if(state & GDK_MOD4_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Mod4");
		sep = "-";
	}
	if(state & GDK_MOD5_MASK) {
		g_string_append(gs, sep);
		g_string_append(gs, "Mod5");
		sep = "-";
	}

	g_string_append(gs, sep);
	g_string_append(gs, key);

	{
		char *ret = gs->str;
		g_string_free(gs, FALSE);
		return ret;
	}
}


static GtkWidget *grab_dialog;

static GdkFilterReturn
grab_key_filter(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	XEvent *xevent = (XEvent *)gdk_xevent;
	GtkEntry *entry;
	char *key;

	if (xevent->type != KeyPress && xevent->type != KeyRelease)
	        puts("EXIT X");
	if (event->type != GDK_KEY_PRESS && event->type != GDK_KEY_RELEASE)
		puts("EXIT GDK");

	if (xevent->type != KeyPress && xevent->type != KeyRelease)
	/*if (event->type != GDK_KEY_PRESS && event->type != GDK_KEY_RELEASE)*/
		return GDK_FILTER_CONTINUE;
	
        entry = GTK_ENTRY (data);

	/* note: GDK has already translated the keycode to a keysym for us */
	g_message ("keycode: %d\tstate: %d",
                   event->key.keyval, event->key.state);

	key = convert_keysym_state_to_string (event->key.keyval,
	                                      event->key.state);
	gtk_entry_set_text (GTK_ENTRY(entry), key?key:"");
	g_free(key);

	gdk_keyboard_ungrab (GDK_CURRENT_TIME);
	gtk_widget_destroy (grab_dialog);
	gdk_window_remove_filter (GDK_ROOT_PARENT(),
	                          grab_key_filter, data);

	return GDK_FILTER_REMOVE;
}

static void
grab_button_pressed (GtkButton *button, gpointer data)
{
 	GtkWidget *frame;
 	GtkWidget *box;  
	GtkWidget *label;
	grab_dialog = gtk_window_new (GTK_WINDOW_POPUP);

	gdk_keyboard_grab (GDK_ROOT_PARENT(), TRUE, GDK_CURRENT_TIME);   
	gdk_window_add_filter (GDK_ROOT_PARENT(), grab_key_filter, data);

	gtk_window_set_policy (GTK_WINDOW (grab_dialog), FALSE, FALSE, TRUE);  
	gtk_window_set_position (GTK_WINDOW (grab_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (grab_dialog), TRUE);

	frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (grab_dialog), frame);

	box = gtk_hbox_new (0, 0);
	gtk_container_set_border_width (GTK_CONTAINER (box), 20);
	gtk_container_add (GTK_CONTAINER (frame), box);

	label = gtk_label_new (_("Press a key..."));
	gtk_container_add (GTK_CONTAINER (box), label);
	
	gtk_widget_show_all (grab_dialog);
	return;
}

/* --- list --- */

static void
list_init()
{
 GList * maps;
 Prop * data;
 GKBpreset * tdata;
 
 maps = gkb->maps;

 if (gkb->tempmaps)
  g_list_free (gkb->tempmaps);
 gkb->tempmaps = NULL;

 while (maps){

  data = maps->data;

  tdata = g_new0 (GKBpreset,1);

  tdata->name = g_strdup (data->name);
  tdata->command = g_strdup (data->name);
  tdata->flag = g_strdup (data->flag);
  tdata->country = g_strdup (data->country);
  tdata->command = g_strdup (data->command);
  tdata->lang = g_strdup (data->lang);
  tdata->label = g_strdup (data->label);

  gkb->tempmaps = g_list_append (gkb->tempmaps, tdata);

  maps = g_list_next (maps);
 } 
 list_show(0);
}

static void
switch_small_cb (GnomePropertyBox * pb)
{
 gkb->tempsmall = 1;
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
apply_cb (GtkWidget * pb, gint page)
{
  GList * list;
  GKBpreset * tdata;
  Prop * data;

  if (page != -1)
    return;

  for(list = gkb->maps; list != NULL; list = list->next) {
	  PropWg *p = list->data;
	  if(p) {
		  g_free(p->name);
		  g_free(p->command);
		  g_free(p->flag);
		  g_free(p);
	  }
  }
  
  g_list_free(gkb->maps);
  gkb->maps = NULL;

  for(list = gkb->tempmaps; list != NULL; list = list->next) 
   {

    tdata = list->data;   
    data = g_new0 (Prop,1);
    data->name = g_strdup (tdata->name);
    data->flag = g_strdup (tdata->flag);
    data->command = g_strdup (tdata->command);
    data->type = g_strdup (tdata->type);
    data->arch = g_strdup (tdata->arch);
    data->codepage = g_strdup (tdata->codepage);
    data->lang = g_strdup (tdata->lang);
    data->label = g_strdup (tdata->label);
    data->country = g_strdup (tdata->country);

    gkb->maps = g_list_append(gkb->maps, data);

    g_free(tdata->name);
    g_free(tdata->command);
    g_free(tdata->flag);
    g_free(tdata->lang);
    g_free(tdata->label);
    g_free(tdata->codepage);
    g_free(tdata->country);
    g_free(tdata->type);
    g_free(tdata->arch);
    g_free(tdata);

  }

  if (gkb->tempmaps)
   g_list_free (gkb->tempmaps);

  gkb->n = g_list_length(gkb->maps);

  gkb->cur = 0;

  gkb->dact = g_list_nth_data (gkb->maps, 0);

  gkb->small = gkb->tempsmall;
  gkb->size = gkb->tempsize;

/*  entry1 = gtk_object_get_data (GTK_OBJECT(gkb->propbox), "entry1");
  gkb->key = g_strdup (gtk_entry_get_text(GTK_ENTRY(entry1)));
  convert_string_to_keysym_state(gkb->key,
                                &gkb->keysym,
                                &gkb->state);
                                                                                                               
*/

  gkb_update (gkb, FALSE);
  gkb_update (gkb, TRUE);

  applet_widget_sync_config(APPLET_WIDGET(gkb->applet));
}

static void
treeitems_create(GtkWidget *tree)
{
	GList * sets = NULL;
	GList * retval = NULL;
        GtkWidget *sitem, *titem, *subitem, *subtree, *subtree2;

	GHashTable * langs = g_hash_table_new(g_str_hash, g_str_equal);
	LangData * ldata;
	CountryData * cdata;

	/* TODO: Error checking... */
	sets = gkb_preset_load(find_presets());
	retval = sets;
        
        while ((sets = g_list_next(sets)) != NULL)
         {
	  GKBpreset * item; 

	  item = sets->data;

	  if ((ldata = g_hash_table_lookup (langs,item->lang)) != NULL)
	   {
	    /* There is lang */
	    if ((cdata = g_hash_table_lookup (ldata->countries,item->country)) != NULL)
	     {
	      /* There is country */
             sitem = gtk_tree_item_new_with_label (item->name);
             gtk_tree_append (GTK_TREE(cdata->widget), sitem);
             gtk_widget_show(sitem);

             cdata->keymaps = g_list_append(cdata->keymaps,item);

             gtk_object_set_data (GTK_OBJECT(sitem),"item",item);

	     }
	     else
	     {
	      /* There is no country */

	      subitem = gtk_tree_item_new_with_label (item->country);
	      gtk_tree_append (GTK_TREE(ldata->widget), subitem);
	      gtk_widget_show (subitem);

              subtree = gtk_tree_new();
              gtk_tree_set_selection_mode (GTK_TREE(subtree),
                                 GTK_SELECTION_SINGLE);
              gtk_tree_set_view_mode (GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
              gtk_tree_item_set_subtree (GTK_TREE_ITEM(subitem), subtree);
              subitem = gtk_tree_item_new_with_label (item->name);
              gtk_tree_append (GTK_TREE(subtree), subitem);
	      gtk_widget_show (subitem);

	      cdata = g_new0 (CountryData,1);

	      cdata->widget = subtree;
	      cdata->keymaps = NULL;

              cdata->keymaps = g_list_append (cdata->keymaps,item);
              gtk_object_set_data (GTK_OBJECT(subitem),"item",item);

	      g_hash_table_insert (ldata->countries, item->country, cdata);
	     }
	   }
	  else
	   {
	    /* There is no lang */

	    titem = gtk_tree_item_new_with_label (item->lang);
	    gtk_tree_append (GTK_TREE(tree), titem);
	    gtk_widget_show (titem);

	    subtree = gtk_tree_new();
	    gtk_tree_set_selection_mode (GTK_TREE(subtree),
                                 GTK_SELECTION_SINGLE);
	    gtk_tree_set_view_mode (GTK_TREE(subtree), GTK_TREE_VIEW_ITEM);
	    gtk_tree_item_set_subtree (GTK_TREE_ITEM(titem), subtree);
	    subitem = gtk_tree_item_new_with_label (item->country);
	    gtk_tree_append (GTK_TREE(subtree), subitem);
	    gtk_widget_show (subitem);

	    subtree2 = gtk_tree_new();
	    gtk_tree_set_selection_mode (GTK_TREE(subtree2),
                                 GTK_SELECTION_SINGLE);
	    gtk_tree_set_view_mode (GTK_TREE(subtree2), GTK_TREE_VIEW_ITEM);
	    gtk_tree_item_set_subtree (GTK_TREE_ITEM(subitem), subtree2);
            sitem = gtk_tree_item_new_with_label (item->name);
            gtk_tree_append (GTK_TREE(subtree2), sitem);
            gtk_widget_show (sitem);

	    ldata = g_new0 (LangData, 1);
            cdata = g_new0 (CountryData,1);

            cdata->widget = subtree2;
            cdata->keymaps = NULL;
	    
	    ldata->widget = subtree;

	    ldata->countries = g_hash_table_new (g_str_hash, g_str_equal);

            cdata->keymaps = g_list_append(cdata->keymaps,item);
            gtk_object_set_data (GTK_OBJECT(sitem),"item",item);

	    g_hash_table_insert (ldata->countries, item->country, cdata);

            g_hash_table_insert (langs, item->lang, ldata);
	   }
	  
	}
}
static void
ok_edited_cb (GtkWidget * button, gint pos)
{
 apply_edited_cb (button, pos);
 gtk_widget_destroy(gkb->mapedit);
 gkb->mapedit=NULL;
}

static void
apply_edited_cb (GtkWidget * button, gint pos)
{
 GtkWidget * nameentry, * labelentry;
 GtkWidget * langentry, * countryentry;
 GtkWidget * iconentry, * commandentry;
 GtkWidget * codepageentry, * archentry;
 GtkWidget * typeentry;

 GKBpreset * data = g_list_nth_data (gkb->tempmaps, pos);
 
 g_free(data->name);
 g_free(data->label);
 g_free(data->country);
 g_free(data->flag);
 g_free(data->lang);
 g_free(data->type);
 g_free(data->command);
 g_free(data->codepage);
 g_free(data->arch);

 nameentry = gtk_object_get_data ( GTK_OBJECT(gkb->mapedit), "entry41");
 data->name = g_strdup (gtk_entry_get_text(GTK_ENTRY(nameentry)));

 labelentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry42");
 data->label = g_strdup (gtk_entry_get_text(GTK_ENTRY(labelentry)));

 langentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry43");
 data->lang = g_strdup (gtk_entry_get_text(GTK_ENTRY(langentry)));

 countryentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry26");
 data->country = g_strdup (gtk_entry_get_text(GTK_ENTRY(countryentry)));

 iconentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "iconentry5");
 data->flag = g_strdup( g_basename (
     gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (iconentry))));

 printf ("Applied flag: %s\n", data->flag); fflush (stdout);

 archentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry37");
 data->arch = g_strdup (gtk_entry_get_text(GTK_ENTRY(archentry)));

 typeentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry38");
 data->type = g_strdup (gtk_entry_get_text(GTK_ENTRY(typeentry)));

 codepageentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry39");
 data->codepage = g_strdup (gtk_entry_get_text(GTK_ENTRY(codepageentry)));

 commandentry = gtk_object_get_data (GTK_OBJECT(gkb->mapedit), "entry40");
 data->command = g_strdup (gtk_entry_get_text(GTK_ENTRY(commandentry)));

 list_show(pos);

}

static void
mapedit_cb ()
{
  GList *combo28_items = NULL;
  GList *combo34_items = NULL;
  GList *combo35_items = NULL;
  GList *combo36_items = NULL;
  GList *combo37_items = NULL;
  GtkWidget *button4, *button5, *button6, *button7;
  GtkWidget *combo28, *combo34, *combo35, *combo36;
  GtkWidget *combo37;
  GtkWidget *entry26, *entry37, *entry38, *entry39;
  GtkWidget *entry40, *entry41, *entry42, *entry43;
  GtkWidget *hbox1;
  GtkWidget *hbuttonbox1;
  GtkWidget *hseparator1;
  GtkWidget *iconentry5;
  GtkWidget *label46, *label51, *label52, *label53;
  GtkWidget *label54, *label55, *label56, *label57;
  GtkWidget *label58, *label59;
  GtkWidget *table6, *table7;
  GtkWidget *vbox2, *vbox3;
  GtkWidget *vseparator1;
  GList * mlist;
  gint pos;
  GKBpreset * data;

  mlist = GTK_LIST(gkb->list1)->selection;

  if (mlist){

  pos = gtk_list_child_position (GTK_LIST(gkb->list1), GTK_WIDGET(mlist->data));

  data = g_list_nth_data (gkb->tempmaps, pos);

  if (gkb->mapedit)
    {
      gtk_widget_destroy (gkb->mapedit);
    }
    
  gkb->mapedit = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW (gkb->mapedit), TRUE);
  gtk_object_set_data (GTK_OBJECT (gkb->mapedit), "mapedit", gkb->mapedit);
  gtk_window_set_title (GTK_WINDOW (gkb->mapedit), _("Edit keymap"));

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (gkb->mapedit), vbox2);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "hbox1", hbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox1, TRUE, TRUE, 0);

  table7 = gtk_table_new (5, 2, FALSE);
  gtk_widget_ref (table7);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "table7", table7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table7);
  gtk_box_pack_start (GTK_BOX (hbox1), table7, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table7), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table7), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table7), 5);

  label55 = gtk_label_new (_("Name"));
  gtk_widget_ref (label55);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label55", label55,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label55);
  gtk_table_attach (GTK_TABLE (table7), label55, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label55), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label55), 1, 0.5);

  label56 = gtk_label_new (_("Label"));
  gtk_widget_ref (label56);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label56", label56,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label56);
  gtk_table_attach (GTK_TABLE (table7), label56, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label56), 1, 0.5);

  label57 = gtk_label_new (_("Language"));
  gtk_widget_ref (label57);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label57", label57,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label57);
  gtk_table_attach (GTK_TABLE (table7), label57, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label57), 1, 0.5);

  label58 = gtk_label_new (_("Country"));
  gtk_widget_ref (label58);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label58", label58,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label58);
  gtk_table_attach (GTK_TABLE (table7), label58, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label58), 1, 0.5);

  label59 = gtk_label_new (_("Flag\nPixmap"));
  gtk_widget_ref (label59);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label59", label59,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label59);
  gtk_table_attach (GTK_TABLE (table7), label59, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label59), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label59), 1, 0.5);

  entry41 = gtk_entry_new ();
  gtk_widget_ref (entry41);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry41", entry41,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text(GTK_ENTRY(entry41), strdup(data->name));

  gtk_widget_show (entry41);
  gtk_table_attach (GTK_TABLE (table7), entry41, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  entry42 = gtk_entry_new ();
  gtk_widget_ref (entry42);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry42", entry42,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text(GTK_ENTRY(entry42), strdup(data->label));
  gtk_widget_show (entry42);
  gtk_table_attach (GTK_TABLE (table7), entry42, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  combo37 = gtk_combo_new ();
  gtk_widget_ref (combo37);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "combo37", combo37,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo37);
  gtk_table_attach (GTK_TABLE (table7), combo37, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo37_items = g_list_append (combo37_items, _("Armenian"));
  combo37_items = g_list_append (combo37_items, _("Basque"));
  combo37_items = g_list_append (combo37_items, _("Bulgarian"));
  combo37_items = g_list_append (combo37_items, _("Dutch"));
  combo37_items = g_list_append (combo37_items, _("English"));
  combo37_items = g_list_append (combo37_items, _("French"));
  combo37_items = g_list_append (combo37_items, _("Georgian"));
  combo37_items = g_list_append (combo37_items, _("German"));
  combo37_items = g_list_append (combo37_items, _("Hungarian"));
  combo37_items = g_list_append (combo37_items, _("Norwegian"));
  combo37_items = g_list_append (combo37_items, _("Polish"));
  combo37_items = g_list_append (combo37_items, _("Portuglese"));
  combo37_items = g_list_append (combo37_items, _("Portuguese"));
  combo37_items = g_list_append (combo37_items, _("Russian"));
  combo37_items = g_list_append (combo37_items, _("Slovak"));
  combo37_items = g_list_append (combo37_items, _("Slovenian"));
  combo37_items = g_list_append (combo37_items, _("Swedish"));
  combo37_items = g_list_append (combo37_items, _("Thai"));
  combo37_items = g_list_append (combo37_items, _("Turkish"));
  combo37_items = g_list_append (combo37_items, _("Wallon"));
  combo37_items = g_list_append (combo37_items, _("Yugoslavian"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo37), combo37_items);
  g_list_free (combo37_items);

  entry43 = GTK_COMBO (combo37)->entry;
  gtk_widget_ref (entry43);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry43", entry43,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry43);
  gtk_entry_set_text (GTK_ENTRY (entry43), data->lang);

  combo28 = gtk_combo_new ();
  gtk_widget_ref (combo28);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "combo28", combo28,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo28);
  gtk_table_attach (GTK_TABLE (table7), combo28, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo28_items = g_list_append (combo28_items, _("Armenia"));
  combo28_items = g_list_append (combo28_items, _("Australia"));
  combo28_items = g_list_append (combo28_items, _("Austria"));
  combo28_items = g_list_append (combo28_items, _("Belgium"));
  combo28_items = g_list_append (combo28_items, _("Brazil"));
  combo28_items = g_list_append (combo28_items, _("Bulgaria"));
  combo28_items = g_list_append (combo28_items, _("Canada"));
  combo28_items = g_list_append (combo28_items, _("Caribic"));
  combo28_items = g_list_append (combo28_items, _("France"));
  combo28_items = g_list_append (combo28_items, _("Georgia"));
  combo28_items = g_list_append (combo28_items, _("Germany"));
  combo28_items = g_list_append (combo28_items, _("Great Britain"));
  combo28_items = g_list_append (combo28_items, _("Hungary"));
  combo28_items = g_list_append (combo28_items, _("Jamaica"));
  combo28_items = g_list_append (combo28_items, _("New Zealand"));
  combo28_items = g_list_append (combo28_items, _("Norway"));
  combo28_items = g_list_append (combo28_items, _("Poland"));
  combo28_items = g_list_append (combo28_items, _("Portugal"));
  combo28_items = g_list_append (combo28_items, _("Russia"));
  combo28_items = g_list_append (combo28_items, _("Slovak Republic"));
  combo28_items = g_list_append (combo28_items, _("Slovenia"));
  combo28_items = g_list_append (combo28_items, _("South Africa"));
  combo28_items = g_list_append (combo28_items, _("Spain"));
  combo28_items = g_list_append (combo28_items, _("Sweden"));
  combo28_items = g_list_append (combo28_items, _("Switzerland"));
  combo28_items = g_list_append (combo28_items, _("Thailand"));
  combo28_items = g_list_append (combo28_items, _("Turkey"));
  combo28_items = g_list_append (combo28_items, _("United States"));
  combo28_items = g_list_append (combo28_items, _("Yugoslavia"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo28), combo28_items);
  g_list_free (combo28_items);

  entry26 = GTK_COMBO (combo28)->entry;
  gtk_widget_ref (entry26);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry26", entry26,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry26);
  gtk_entry_set_text (GTK_ENTRY (entry26), data->country);

  iconentry5 = gnome_icon_entry_new (NULL, NULL);
  gtk_widget_ref (iconentry5);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "iconentry5", iconentry5,
                            (GtkDestroyNotify) gtk_widget_unref);

  gnome_icon_entry_set_pixmap_subdir (GNOME_ICON_ENTRY(iconentry5),"gkb");

  gtk_widget_show (iconentry5);
  gtk_table_attach (GTK_TABLE (table7), iconentry5, 1, 2, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY (iconentry5),
  		    data->flag);

  vseparator1 = gtk_vseparator_new ();
  gtk_widget_ref (vseparator1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "vseparator1", vseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vseparator1);
  gtk_box_pack_start (GTK_BOX (hbox1), vseparator1, TRUE, TRUE, 0);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox3);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "vbox3", vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox3, TRUE, TRUE, 0);

  table6 = gtk_table_new (4, 2, FALSE);
  gtk_widget_ref (table6);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "table6", table6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table6);
  gtk_box_pack_start (GTK_BOX (vbox3), table6, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table6), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 8);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 5);

  label51 = gtk_label_new (_("Architecture"));
  gtk_widget_ref (label51);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label51", label51,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label51);
  gtk_table_attach (GTK_TABLE (table6), label51, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label51), 1, 0.5);

  label52 = gtk_label_new (_("Type"));
  gtk_widget_ref (label52);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label52", label52,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label52);
  gtk_table_attach (GTK_TABLE (table6), label52, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label52), 1, 0.5);

  label53 = gtk_label_new (_("Codepage"));
  gtk_widget_ref (label53);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label53", label53,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label53);
  gtk_table_attach (GTK_TABLE (table6), label53, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label53), 1, 0.5);

  label54 = gtk_label_new (_("Command"));
  gtk_widget_ref (label54);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label54", label54,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label54);
  gtk_table_attach (GTK_TABLE (table6), label54, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label54), 1, 0.5);

  combo34 = gtk_combo_new ();
  gtk_widget_ref (combo34);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "combo34", combo34,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo34);
  gtk_table_attach (GTK_TABLE (table6), combo34, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo34_items = g_list_append (combo34_items, _("ix86"));
  combo34_items = g_list_append (combo34_items, _("sun"));
  combo34_items = g_list_append (combo34_items, _("mac"));
  combo34_items = g_list_append (combo34_items, _("sgi"));
  combo34_items = g_list_append (combo34_items, _("dec"));
  combo34_items = g_list_append (combo34_items, _("ibm"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo34), combo34_items);
  g_list_free (combo34_items);

  entry37 = GTK_COMBO (combo34)->entry;
  gtk_widget_ref (entry37);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry37", entry37,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry37);
  gtk_entry_set_text (GTK_ENTRY (entry37), data->arch);

  combo35 = gtk_combo_new ();
  gtk_widget_ref (combo35);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "combo35", combo35,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo35);
  gtk_table_attach (GTK_TABLE (table6), combo35, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo35_items = g_list_append (combo35_items, _("105"));
  combo35_items = g_list_append (combo35_items, _("101"));
  combo35_items = g_list_append (combo35_items, _("102"));
  combo35_items = g_list_append (combo35_items, _("450"));
  combo35_items = g_list_append (combo35_items, _("84"));
  combo35_items = g_list_append (combo35_items, _("mklinux"));
  combo35_items = g_list_append (combo35_items, _("type5"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo35), combo35_items);
  g_list_free (combo35_items);

  entry38 = GTK_COMBO (combo35)->entry;
  gtk_widget_ref (entry38);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry38", entry38,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry38);
  gtk_entry_set_text (GTK_ENTRY (entry38), data->type);

  combo36 = gtk_combo_new ();
  gtk_widget_ref (combo36);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "combo36", combo36,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (combo36);
  gtk_table_attach (GTK_TABLE (table6), combo36, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  combo36_items = g_list_append (combo36_items, _("iso-8859-1"));
  combo36_items = g_list_append (combo36_items, _("iso-8859-2"));
  combo36_items = g_list_append (combo36_items, _("iso-8859-9"));
  combo36_items = g_list_append (combo36_items, _("am-armscii8"));
  combo36_items = g_list_append (combo36_items, _("be-latin1"));
  combo36_items = g_list_append (combo36_items, _("cp1251"));
  combo36_items = g_list_append (combo36_items, _("georgian-academy"));
  combo36_items = g_list_append (combo36_items, _("koi8-r"));
  combo36_items = g_list_append (combo36_items, _("tis620"));
  gtk_combo_set_popdown_strings (GTK_COMBO (combo36), combo36_items);
  g_list_free (combo36_items);

  entry39 = GTK_COMBO (combo36)->entry;
  gtk_widget_ref (entry39);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry39", entry39,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry39);
  gtk_entry_set_text (GTK_ENTRY (entry39), data->codepage);

  entry40 = gtk_entry_new ();
  gtk_widget_ref (entry40);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "entry40", entry40,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_entry_set_text (GTK_ENTRY (entry40), data->command);
  gtk_widget_show (entry40);
  gtk_table_attach (GTK_TABLE (table6), entry40, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label46 = gtk_label_new ("");
  gtk_label_parse_uline (GTK_LABEL (label46),
                         _("Where command can be:\n"
                         "* xmodmap /full/path/xmodmap.hu\n"
                         "* gkb__xmmap hu\n"
                         "* setxkbmap hu"));
  gtk_widget_ref (label46);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "label46", label46,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label46);
  gtk_box_pack_start (GTK_BOX (vbox3), label46, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label46), GTK_JUSTIFY_LEFT);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "hseparator1", hseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox2), hseparator1, TRUE, TRUE, 0);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (hbuttonbox1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "hbuttonbox1", hbuttonbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox2), hbuttonbox1, TRUE, TRUE, 0);

  button4 = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "button4", button4,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_signal_connect (GTK_OBJECT (button4), "clicked",
  		      GTK_SIGNAL_FUNC(ok_edited_cb),
  		      GINT_TO_POINTER(pos));

  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  button5 = gnome_stock_button (GNOME_STOCK_BUTTON_APPLY);
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "button5", button5,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_signal_connect (GTK_OBJECT (button5), "clicked",
  		      GTK_SIGNAL_FUNC (apply_edited_cb), 
  		      GINT_TO_POINTER(pos));

  gtk_widget_show (button5);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  button6 = gnome_stock_button (GNOME_STOCK_BUTTON_CLOSE);
  gtk_widget_ref (button6);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "button6", button6,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_signal_connect (GTK_OBJECT(button6), "clicked",
                      GTK_SIGNAL_FUNC(wdestroy_cb),
                      GTK_OBJECT(gkb->mapedit));

  gtk_widget_show (button6);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button6);
  GTK_WIDGET_SET_FLAGS (button6, GTK_CAN_DEFAULT);

  button7 = gnome_stock_button (GNOME_STOCK_BUTTON_HELP);
  gtk_widget_ref (button7);
  gtk_object_set_data_full (GTK_OBJECT (gkb->mapedit), "button7", button7,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_signal_connect (GTK_OBJECT (button7), "clicked",
  		      GTK_SIGNAL_FUNC (edithelp_cb), 
  		      NULL);

  gtk_widget_show (button7);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button7);
  GTK_WIDGET_SET_FLAGS (button7, GTK_CAN_DEFAULT);

  gtk_widget_show(gkb->mapedit);
  }
  return;
}

gint 
wdestroy_cb (GtkWidget * closebutton, GtkWidget * window)
{
 if (window == gkb->addwindow)
  gkb->addwindow=NULL;
 else
  gkb->mapedit=NULL;
 gtk_widget_destroy(window);
  
 return FALSE;
}

void
list_show(gint pos)
{
 GtkWidget * hbox1;
 GtkWidget * label3, * pixmap1, * list_item;
 GKBpreset * tdata;
 gchar *pixmap1_filename;
 gint counter;
 GList * list;
 
 gtk_list_clear_items (GTK_LIST (gkb->list1), 0, -1);
 
 counter=0;
 
 for (list=gkb->tempmaps;list!=NULL;list = g_list_next (list))
  {
  char buf[30];

  tdata = list->data;

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox1);
  gtk_widget_show (hbox1);

  pixmap1 = gtk_type_new (gnome_pixmap_get_type ());
  sprintf(buf,"gkb/%s",tdata->flag);
  pixmap1_filename =  gnome_unconditional_pixmap_file (buf);
  if (pixmap1_filename)
    gnome_pixmap_load_file_at_size (GNOME_PIXMAP (pixmap1), pixmap1_filename, 28, 20);
   else
    {
     gtk_widget_destroy(pixmap1);
     pixmap1 = gtk_label_new (tdata->flag);
     gtk_widget_set_usize (pixmap1, 28, 20);
    }
  g_free (pixmap1_filename);
  gtk_widget_ref (pixmap1); 
  gtk_widget_show (pixmap1);
  gtk_box_pack_start (GTK_BOX (hbox1), pixmap1, FALSE, TRUE, 0);

  label3 = gtk_label_new (tdata->name);
  gtk_widget_ref (label3);
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (hbox1), label3, TRUE, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label3), 3, 0);

  list_item=gtk_list_item_new();
  gtk_container_add(GTK_CONTAINER(list_item), hbox1);
  gtk_object_set_data (GTK_OBJECT(list_item),"hbox",hbox1);
  gtk_object_set_data (GTK_OBJECT(list_item),"flag",tdata->flag);
  gtk_object_set_data (GTK_OBJECT(list_item),"name",tdata->name);
  gtk_container_add(GTK_CONTAINER(gkb->list1), list_item);
  gtk_widget_show(list_item);
  }

/* TODO:
  if (counter == pos)
   gtk_real_list_select_child (GTK_LIST(gkb->list1), list_item);
*/
 
 gtk_widget_show(gkb->list1);

}

gint 
addwadd_cb (GtkWidget * addbutton, GtkWidget * ctree)
{
 GKBpreset * tdata;

 if (a_keymap)
 {
  tdata = g_new0 (GKBpreset,1);

  tdata->name = g_strdup (a_keymap->name);
  tdata->flag = g_strdup (a_keymap->flag);
  tdata->command = g_strdup (a_keymap->command);
  tdata->country = g_strdup (a_keymap->country);
  tdata->label = g_strdup (a_keymap->label);
  tdata->lang = g_strdup (a_keymap->lang);

  gkb->tempmaps = g_list_append (gkb->tempmaps, tdata);
 }

 list_show(g_list_length(gkb->tempmaps) - 1);
 
 if ( g_list_length(gkb->tempmaps) > 1 )
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
 return FALSE;
}

static void
preadd_cb (GtkWidget * tree, GtkWidget * button)
{
  GList *i;
  GtkLabel *label;
  GtkWidget *item;

  g_return_if_fail (tree != NULL);
  g_return_if_fail (button != NULL);

  gtk_widget_set_sensitive (button, FALSE);
 
  if((i = GTK_TREE_SELECTION(tree)) != NULL)
  {
   item = GTK_WIDGET (i->data);
   label = GTK_LABEL (GTK_BIN (item)->child);
   if (GTK_IS_LABEL(label))
    {
     gtk_widget_set_sensitive (button, TRUE);
     a_keymap = gtk_object_get_data (GTK_OBJECT(item),"item");
    }
   }
 return;
}

static void
addwindow_cb (GtkWidget *addbutton)
{
  GtkWidget *vbox1;
  GtkWidget *tree1;
  GtkWidget *scrolled1;
  GtkWidget *hbuttonbox1;
  GtkWidget *button4;
  GtkWidget *button5;
  GtkWidget *button6;

  if (gkb->addwindow)
    {
      gtk_widget_show_now (gkb->addwindow);
      gdk_window_raise (gkb->addwindow->window);
      return;
    }

  gkb->addwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(gkb->addwindow), TRUE);
  gtk_object_set_data (GTK_OBJECT (gkb->addwindow), "addwindow", gkb->addwindow);
  gtk_window_set_title (GTK_WINDOW (gkb->addwindow), _("Select layout"));

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->addwindow), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gkb->addwindow), vbox1);

  scrolled1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled1),
                                  GTK_POLICY_AUTOMATIC,
	                	  GTK_POLICY_AUTOMATIC);
  gtk_widget_set_usize (scrolled1, 315, 202);
  gtk_box_pack_start (GTK_BOX (vbox1), scrolled1, TRUE, TRUE, 0);
  gtk_widget_show (scrolled1);

  tree1 = gtk_tree_new ();
  gtk_widget_ref (tree1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->addwindow), "tree1", tree1,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show (tree1);

  treeitems_create (tree1);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW(scrolled1),
                                         tree1);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (hbuttonbox1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->addwindow), "hbuttonbox1", hbuttonbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, FALSE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_SPREAD);

  button4 = gtk_button_new_with_label (_("Add"));
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (gkb->addwindow), "button4", button4,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);
  gtk_widget_set_sensitive (button4, FALSE);

  button5 = gnome_stock_button (GNOME_STOCK_BUTTON_CLOSE);
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (gkb->addwindow), "button5", button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button5);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  button6 = gnome_stock_button (GNOME_STOCK_BUTTON_HELP);
  gtk_widget_ref (button6);
  gtk_object_set_data_full (GTK_OBJECT (gkb->addwindow), "button6", button6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button6);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button6);
  GTK_WIDGET_SET_FLAGS (button6, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT(tree1), "selection_changed",
                      GTK_SIGNAL_FUNC(preadd_cb),
                      GTK_OBJECT(button4));
  gtk_signal_connect (GTK_OBJECT(button4), "clicked",
                      GTK_SIGNAL_FUNC(addwadd_cb),
                      GTK_OBJECT(tree1));
  gtk_signal_connect (GTK_OBJECT(button5), "clicked",
                      GTK_SIGNAL_FUNC(wdestroy_cb),
                      GTK_OBJECT(gkb->addwindow));
  gtk_signal_connect (GTK_OBJECT(button6), "clicked",
                      GTK_SIGNAL_FUNC(addhelp_cb),
                      GTK_OBJECT(tree1));

  gtk_widget_show(gkb->addwindow);

  return;
}

static void
list_select_cb ()
{
 GtkWidget * editbutton;
 GtkWidget * deletebutton;
 GtkWidget * upbutton;
 GtkWidget * downbutton;
 GList * mlist;
 gint pos;
 
 mlist = GTK_LIST(gkb->list1)->selection;

 deletebutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"deletebutton");
 downbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"downbutton");
 upbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"upbutton");
 editbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"editbutton");
 if (mlist){
  gtk_widget_set_sensitive (deletebutton, TRUE);
  gtk_widget_set_sensitive (editbutton, TRUE);
  pos = gtk_list_child_position (GTK_LIST(gkb->list1), GTK_WIDGET(mlist->data));
  if (pos == 0) 
   {
    gtk_widget_set_sensitive (downbutton, TRUE);
    gtk_widget_set_sensitive (upbutton, FALSE);
   } else 
   {
     gtk_widget_set_sensitive (upbutton, TRUE);
     gtk_widget_set_sensitive (downbutton, FALSE);
   if (pos < (g_list_length(gkb->tempmaps) - 1)) 
     {
      gtk_widget_set_sensitive (downbutton, TRUE);
     }
   }
 }
  else
 {
  gtk_widget_set_sensitive (editbutton, FALSE);
  gtk_widget_set_sensitive (deletebutton, FALSE);
  gtk_widget_set_sensitive (downbutton, FALSE);
  gtk_widget_set_sensitive (upbutton, FALSE);
 }
 return;
}

static void
del_select_cb (GtkWidget * button)
{ 
 GtkWidget * deletebutton;
 GList *mlist;
 GtkObject *list_item;
 GtkWidget *hbox;
 gint pos;
 
 mlist=GTK_LIST(gkb->list1)->selection;

 g_return_if_fail (mlist != NULL);

 if (mlist){
  list_item=GTK_OBJECT(mlist->data);
  hbox = gtk_object_get_data (GTK_OBJECT(list_item),"hbox");
  pos = gtk_list_child_position (GTK_LIST(gkb->list1), GTK_WIDGET(mlist->data));

  gkb->tempmaps = g_list_remove (gkb->tempmaps,g_list_nth_data(gkb->tempmaps, pos));

  deletebutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"deletebutton");
  gtk_widget_set_sensitive (deletebutton, FALSE); 
 }

 list_show(-1);

 if ( g_list_length(gkb->tempmaps) > 1 )
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
 
 return;
}

static void
move_select_cb (GtkWidget * button)
{
 GtkWidget * upbutton;
 GtkWidget * downbutton;
 GtkWidget * list_item;
 GList *mlist;
 GKBpreset * data;
 gint pos;
 GList * row, * nextr, * prevr;
 
 mlist=GTK_LIST(gkb->list1)->selection;
 g_return_if_fail (mlist != NULL);

 downbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"downbutton");
 upbutton = gtk_object_get_data(GTK_OBJECT(gkb->propbox),"upbutton");
 
 if (mlist) {
  list_item=GTK_WIDGET(mlist->data);
  pos = gtk_list_child_position (GTK_LIST(gkb->list1), list_item);

  row = g_list_nth(gkb->tempmaps,pos);
  data = g_list_nth_data(gkb->tempmaps,pos);

  printf("POS:%d, N: %s\n",pos, data->name);fflush(stdout);

  if (button == upbutton) 
   { 
    prevr = row->prev;
    if ((row->prev = prevr->prev) == NULL)
     {
      gkb->tempmaps = row;
     }
     else
     {
      prevr->prev->next = row;
     }

    if ((nextr = row->next) != NULL) {
      nextr->prev = prevr;
    }

    row->next = prevr;

    prevr->prev = row;

    prevr->next = nextr;

    pos--;
   }
   else 
   {
    nextr = row->next;
    if ((prevr = row->prev) != NULL) {
     prevr->next = nextr;
    } else {
     gkb->tempmaps = nextr;
    }
    row->next = nextr->next;
    row->prev = nextr;
    nextr->next = row;
    nextr->prev = prevr;
    pos++;
   }

  list_show(pos);

  if (pos == 0) 
    {
     gtk_widget_set_sensitive (downbutton, TRUE);
     gtk_widget_set_sensitive (upbutton, FALSE);
    } else 
    {
     gtk_widget_set_sensitive (upbutton, TRUE);
     gtk_widget_set_sensitive (downbutton, FALSE);
     if (pos < (g_list_length(gkb->tempmaps) - 1)) 
      {
      gtk_widget_set_sensitive (downbutton, TRUE);
      }
    }
  }

 if ( g_list_length(gkb->tempmaps) > 1 )
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
 return;
}

void
properties_dialog (AppletWidget * applet)
{
  gchar *pixmap1_filename;
  GtkWidget *button1, *button2, *button3, *button11;
  GtkWidget *button4, *button5, *button6;
  GtkWidget *entry1;
  GtkWidget *frame1, *frame2;
  GtkWidget *prop_menuitem;
  GtkWidget *hbox10, *hbox11, *hbox8;
  GtkWidget *label16, *label17;
  GtkWidget *label23, *label24;
  GtkWidget *optionmenu1, *optionmenu1_menu;
  GtkWidget *optionmenu2, *optionmenu2_menu;
  GtkWidget *pixmap1;
  GtkWidget *propnotebook;
  GtkWidget *scrolledwindow3;
  GtkWidget *table1;
  GtkWidget *vbox2, *vbox3;
  GtkWidget *vbuttonbox4;
  GList * list;

  if (gkb->propbox)
    {
	gtk_widget_destroy (gkb->propbox);
	gkb->propbox= NULL;
    }

  for (list = gkb->tempmaps; list != NULL; list = list->next)
    {
      PropWg *actdata = list->data;
      if (actdata)
	{
	  g_free (actdata->name);
	  g_free (actdata->flag);
	  g_free (actdata->command);
	  g_free (actdata);
	}
    }
  g_list_free (gkb->tempmaps);

  gkb->tn = gkb->n;

  gkb->propbox = gnome_property_box_new ();

  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "propbox", gkb->propbox);
  gtk_window_set_title (GTK_WINDOW (gkb->propbox), _("GKB Properties"));
  gtk_window_set_position (GTK_WINDOW (gkb->propbox), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (gkb->propbox), FALSE, FALSE, FALSE);

  propnotebook = GNOME_PROPERTY_BOX (gkb->propbox)->notebook;
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "propnotebook", propnotebook);
  gtk_widget_show (propnotebook);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (propnotebook), vbox2);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox3);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbox3", vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox3);
  gtk_box_pack_start (GTK_BOX (vbox2), vbox3, TRUE, FALSE, 0);

  frame2 = gtk_frame_new (_("Display"));
  gtk_widget_ref (frame2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame2", frame2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox3), frame2, TRUE, TRUE, 0);

  hbox11 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox11);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox11", hbox11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox11);
  gtk_container_add (GTK_CONTAINER (frame2), hbox11);

  pixmap1 = gtk_type_new (gnome_pixmap_get_type ());
  pixmap1_filename = gnome_pixmap_file ("gkb.png");
  if (pixmap1_filename)
    gnome_pixmap_load_file (GNOME_PIXMAP (pixmap1), pixmap1_filename);
  g_free (pixmap1_filename);
  gtk_widget_ref (pixmap1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "pixmap1", pixmap1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (pixmap1);
  gtk_box_pack_start (GTK_BOX (hbox11), pixmap1, FALSE, FALSE, 23);

  table1 = gtk_table_new (2, 2, TRUE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (hbox11), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 15);

  optionmenu1 = gtk_option_menu_new ();
  gtk_widget_ref (optionmenu1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "optionmenu1", optionmenu1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (optionmenu1);
  gtk_table_attach (GTK_TABLE (table1), optionmenu1, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  optionmenu1_menu = gtk_menu_new ();
  prop_menuitem = gtk_menu_item_new_with_label (_("Normal"));
  gtk_signal_connect (GTK_OBJECT (prop_menuitem), "activate",
                        (GtkSignalFunc) switch_normal_cb, NULL);
  gtk_widget_show (prop_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu1_menu), prop_menuitem);
  prop_menuitem = gtk_menu_item_new_with_label (_("Small"));
  gtk_signal_connect (GTK_OBJECT (prop_menuitem), "activate",
                        (GtkSignalFunc) switch_small_cb, NULL);
  gtk_widget_show (prop_menuitem);

  gtk_menu_append (GTK_MENU (optionmenu1_menu), prop_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu1), optionmenu1_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionmenu1), 0);

  optionmenu2 = gtk_option_menu_new ();
  gtk_widget_ref (optionmenu2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "optionmenu2", optionmenu2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (optionmenu2);
  gtk_table_attach (GTK_TABLE (table1), optionmenu2, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  optionmenu2_menu = gtk_menu_new ();
  prop_menuitem = gtk_menu_item_new_with_label (_("Label"));
  gtk_widget_show (prop_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), prop_menuitem);
  prop_menuitem = gtk_menu_item_new_with_label (_("Flag"));
  gtk_widget_show (prop_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), prop_menuitem);
  prop_menuitem = gtk_menu_item_new_with_label (_("Flag and label"));
  gtk_widget_show (prop_menuitem);
  gtk_menu_append (GTK_MENU (optionmenu2_menu), prop_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu2), optionmenu2_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionmenu2), 1);

  label23 = gtk_label_new (_("Appearance"));
  gtk_widget_ref (label23);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "label23", label23,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label23);
  gtk_table_attach (GTK_TABLE (table1), label23, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 10, 0);
  gtk_label_set_justify (GTK_LABEL (label23), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label23), 1, 0.5);

  label24 = gtk_label_new (_("Applet size"));
  gtk_widget_ref (label24);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "label24", label24,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label24);
  gtk_table_attach (GTK_TABLE (table1), label24, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 10, 0);
  gtk_label_set_justify (GTK_LABEL (label24), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_alignment (GTK_MISC (label24), 1, 0.5);

  frame1 = gtk_frame_new (_("Hotkey for switching between layouts"));
  gtk_widget_ref (frame1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame1", frame1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox2), frame1, TRUE, FALSE, 2);

  hbox10 = gtk_hbox_new (TRUE, 0);
  gtk_widget_ref (hbox10);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox10", hbox10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox10);
  gtk_container_add (GTK_CONTAINER (frame1), hbox10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox10), 10);

  entry1 = gtk_entry_new ();
  gtk_widget_ref (entry1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "entry1", entry1,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_signal_connect (GTK_OBJECT(entry1), "changed", 
                      GTK_SIGNAL_FUNC (changed_cb),
		      NULL);

  gtk_entry_set_text (GTK_ENTRY(entry1), gkb->key);

  gtk_widget_show (entry1);
  gtk_box_pack_start (GTK_BOX (hbox10), entry1, FALSE, TRUE, 0);

  button6 = gtk_button_new_with_label (_("Grab hotkey"));
  gtk_widget_ref (button6);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "button6", button6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_signal_connect (GTK_OBJECT(button6), "clicked", 
                      GTK_SIGNAL_FUNC (grab_button_pressed),
		      entry1);

  gtk_widget_show (button6);
  gtk_box_pack_start (GTK_BOX (hbox10), button6, TRUE, FALSE, 0);

  label16 = gtk_label_new (_("General"));
  gtk_widget_ref (label16);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "label16", label16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label16);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (propnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (propnotebook), 0), label16);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox8);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox8", hbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox8);
  gtk_container_add (GTK_CONTAINER (propnotebook), hbox8);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow3);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "scrolledwindow3", scrolledwindow3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow3);
  gtk_box_pack_start (GTK_BOX (hbox8), scrolledwindow3, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gkb->list1=gtk_list_new();
  gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(scrolledwindow3),
                                           gkb->list1);

  list_init();

  gtk_signal_connect(GTK_OBJECT(gkb->list1),
                       "selection_changed",
                       GTK_SIGNAL_FUNC(list_select_cb),
                       NULL);

  vbuttonbox4 = gtk_vbutton_box_new ();
  gtk_widget_ref (vbuttonbox4);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbuttonbox4", vbuttonbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbuttonbox4);
  gtk_box_pack_start (GTK_BOX (hbox8), vbuttonbox4, FALSE, TRUE, 0);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (vbuttonbox4), 0);
  gtk_button_box_set_child_size (GTK_BUTTON_BOX (vbuttonbox4), 75, 25);
  gtk_button_box_set_child_ipadding (GTK_BUTTON_BOX (vbuttonbox4), 2, 0);

  button1 = gtk_button_new_with_label (_("Add"));
  gtk_widget_ref (button1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "button1", button1,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_signal_connect (GTK_OBJECT(button1), "clicked", 
                      GTK_SIGNAL_FUNC (addwindow_cb),
                      NULL);

  gtk_widget_show (button1);
  gtk_container_add (GTK_CONTAINER (vbuttonbox4), button1);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button11 = gtk_button_new_with_label (_("New"));
  gtk_widget_ref (button11);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "newbutton", button11,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_object_set_data (GTK_OBJECT (button11),"wlabel","New");

  gtk_signal_connect (GTK_OBJECT(button11), "clicked", 
                      GTK_SIGNAL_FUNC (mapedit_cb),
                      NULL);

  gtk_widget_show (button11);
  gtk_container_add (GTK_CONTAINER (vbuttonbox4), button11);
  GTK_WIDGET_SET_FLAGS (button11, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_with_label (_("Edit"));
  gtk_widget_ref (button2);

  gtk_widget_set_sensitive (button2, FALSE);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "editbutton", button2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button2);

  gtk_object_set_data (GTK_OBJECT (button2),"wlabel","Edit");

  gtk_signal_connect (GTK_OBJECT(button2), "clicked", 
                      GTK_SIGNAL_FUNC (mapedit_cb),
                      NULL);

  gtk_container_add (GTK_CONTAINER (vbuttonbox4), button2);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  button3 = gtk_button_new_with_label (_("Up"));
  gtk_widget_ref (button3);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "upbutton", button3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_set_sensitive (button3, FALSE);

  gtk_signal_connect (GTK_OBJECT(button3), "clicked", 
                      GTK_SIGNAL_FUNC (move_select_cb),
                      NULL);

  gtk_widget_show (button3);
  gtk_container_add (GTK_CONTAINER (vbuttonbox4), button3);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  button4 = gtk_button_new_with_label (_("Down"));
  gtk_widget_ref (button4);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "downbutton", button4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_set_sensitive (button4, FALSE);

  gtk_signal_connect (GTK_OBJECT(button4), "clicked", 
                      GTK_SIGNAL_FUNC (move_select_cb),
                      NULL);

  gtk_widget_show (button4);
  gtk_container_add (GTK_CONTAINER (vbuttonbox4), button4);
  GTK_WIDGET_SET_FLAGS (button4, GTK_CAN_DEFAULT);

  button5 = gtk_button_new_with_label (_("Delete"));
  gtk_widget_ref (button5);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "deletebutton", button5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_set_sensitive (button5, FALSE);

  gtk_signal_connect (GTK_OBJECT(button5), "clicked", 
                      GTK_SIGNAL_FUNC (del_select_cb),
                      NULL);

  gtk_widget_show (button5);
  gtk_container_add (GTK_CONTAINER (vbuttonbox4), button5);
  GTK_WIDGET_SET_FLAGS (button5, GTK_CAN_DEFAULT);

  label17 = gtk_label_new (_("Keymaps"));
  gtk_widget_ref (label17);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "label17", label17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label17);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (propnotebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (propnotebook), 1), label17);

  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "apply", GTK_SIGNAL_FUNC (apply_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "destroy", GTK_SIGNAL_FUNC (gtk_widget_destroyed),&gkb->propbox);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "help", GTK_SIGNAL_FUNC (prophelp_cb), NULL);
  
  gtk_widget_show_now (gkb->propbox);
  gdk_window_raise (gkb->propbox->window);

  return;
}

static void
prophelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS" };

  gnome_help_display (NULL, &help_entry);
}

static void
addhelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-ADD" };

  gnome_help_display (NULL, &help_entry);
}

static void
edithelp_cb (AppletWidget * applet)
{
  GnomeHelpMenuEntry help_entry =
    { "gkb_applet", "index.html#GKBAPPLET-PREFS-EDIT" };

  gnome_help_display (NULL, &help_entry);
}
