/* File: prop.c
 * Purpose: GNOME Keyboard switcher property box
 *
 * Copyright (C) 1999 Free Software Foundation
 * Author: Szabolcs BAN <shooby@gnome.hu>, 1998-2000
 *
 * Thanks for aid of Balazs Nagy <julian7@kva.hu>,
 * Charles Levert <charles@comm.polymtl.ca>,
 * George Lebl <jirka@5z.com> and solidarity
 * Emese Kovacs <emese@eik.bme.hu>.
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

#include "gkb.h"

typedef struct _PropWg PropWg;
struct _PropWg
{
  GdkPixmap *pix;
 
  char *name;
  char *command;
  char *iconpath;
  
  GtkWidget *propbox; 
  GtkWidget *notebook;
  GtkWidget *label1;
  GtkWidget *iconentry;
  GtkWidget *keymapname;
  GtkWidget *commandinput;
  GtkWidget *iconpathinput;
  GtkWidget *vbox1, *hbox1, *vbox2, *hbox2, *hbox3, *hboxmap;
  GtkWidget *frame1, *frame2, *frame3, *frame4, *frame6;
  GtkWidget *newkeymap, *delkeymap;
};

static void prophelp_cb (AppletWidget * widget, gpointer data);
static void makenotebook (GKB * gkb, PropWg * actdata, int i);
static PropWg * cp_prop (Prop * data);
static GList * copy_props (GKB * gkb);
static Prop * cp_propwg (PropWg * data);
static GList * copy_propwgs (GKB * gkb);

static GList *
copy_props (GKB * gkb)
{
  GList * tempmaps = NULL;
  GList * list;

  for(list = gkb->maps; list != NULL; list = list->next) {
	  Prop * prop = list->data;
	  if(prop) {
		  PropWg * p2 = cp_prop (prop);
		  tempmaps = g_list_prepend (tempmaps, p2);
	  }
  }
  return g_list_reverse(tempmaps);
}

static GList *
copy_propwgs (GKB * gkb)
{
  GList * tempmaps = NULL;
  GList * list = gkb->tempmaps;

  for(list = gkb->tempmaps; list != NULL; list = list->next) {
	  PropWg * prop = list->data;
	  if(prop != NULL) {
		  Prop * p2;
		  p2 = cp_propwg (prop);
		  tempmaps = g_list_prepend (tempmaps, p2);
	  }
  }
  return g_list_reverse(tempmaps);
}

static PropWg *
cp_prop (Prop * data)
{

  PropWg * tempdata = g_new0 (PropWg, 1);

  tempdata->name = g_strdup (data->name);
  tempdata->command = g_strdup (data->command);
  tempdata->iconpath = g_strdup (data->iconpath);

  return tempdata;
}

static Prop *
cp_propwg (PropWg * data)
{

  Prop *tempdata = g_new0 (Prop, 1);

  tempdata->name = g_strdup (data->name);
  tempdata->command = g_strdup (data->command);
  tempdata->iconpath = g_strdup (data->iconpath);

  return tempdata;
}

static void
delmap_cb (GnomePropertyBox * pb,GKB * gkb)
{
  gint page;
  PropWg *actdata;

  if (gkb->tempmaps->next == NULL) return;

  page = gtk_notebook_get_current_page(GTK_NOTEBOOK(gkb->notebook));
  gtk_notebook_remove_page (GTK_NOTEBOOK(gkb->notebook), page);
  actdata = g_list_nth_data(gkb->tempmaps, page);
  gkb->tempmaps = g_list_remove(gkb->tempmaps, actdata);
  g_free(actdata->name);
  g_free(actdata->iconpath);
  g_free(actdata->command);
  g_free(actdata);
  gtk_widget_draw(GTK_WIDGET(gkb->notebook), NULL);

  gkb->tn--;
  
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
  
}

static void
icontopath_cb (GnomePropertyBox * pb, PropWg * actdata)
{
	char *itext;

	g_return_if_fail (GTK_WIDGET_REALIZED (actdata->iconpathinput));
	g_return_if_fail (GTK_WIDGET_REALIZED (actdata->iconentry));

	itext = gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (actdata->iconentry));

	if(itext != NULL) {
		gtk_entry_set_text (GTK_ENTRY(actdata->iconpathinput), itext);
		g_free(itext);
	}

	gtk_widget_show(actdata->iconpathinput);

	gnome_property_box_changed (GNOME_PROPERTY_BOX (actdata->propbox));

}

static void
pathtoicon_cb (GnomePropertyBox * pb, PropWg * actdata)
{
	char *etext = gtk_entry_get_text(GTK_ENTRY(actdata->iconpathinput));
	char *itext = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY (actdata->iconentry));

	if(etext && itext && strcmp(etext, itext) == 0) {
		g_free(itext);
		return;
	} /* :]]] Thanx */
	g_free(itext);

	gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry), etext);

	gnome_property_box_changed (GNOME_PROPERTY_BOX (actdata->propbox));
}

static void
newmap_cb (GnomePropertyBox * pb, GKB * gkb)
{
  PropWg *actdata;

  gnome_config_push_prefix (APPLET_WIDGET (gkb->applet)->privcfgpath);
  actdata = cp_prop(loadprop (gkb, 0));
  gnome_config_pop_prefix ();

  makenotebook (gkb, actdata, gkb->tn);

  gkb->tempmaps = g_list_append (gkb->tempmaps, actdata);
  gkb->tn ++;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));

}

static void
changed_cb (GnomePropertyBox * pb, GKB * gkb)
{
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
apply_cb (GtkWidget * pb, gint page, GKB * gkb)
{
  PropWg * actdata;
  GList * list;
  int i = 0;

  if (page != -1)
    return;			/* Thanks Havoc -- Julian7 */

  list = gkb->tempmaps;
  while (list)
    {
      if((actdata = list->data) != NULL) {

        g_free (actdata->name);
        g_free (actdata->iconpath);
        g_free (actdata->command);

        actdata->name = g_strdup (gtk_entry_get_text (GTK_ENTRY (actdata->keymapname)));

	/* gnome_icon_entry_get_filename returns a newly allocated string,
	 * so no need to strdup */
        actdata->iconpath = 
	  gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (actdata->iconentry));

        actdata->command = g_strdup (
	  gtk_entry_get_text (GTK_ENTRY (actdata->commandinput)));

        gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (actdata->notebook),
             gtk_notebook_get_nth_page (GTK_NOTEBOOK (actdata->notebook), i++),
	       gtk_entry_get_text (GTK_ENTRY(actdata->keymapname)));

        gtk_widget_show (actdata->notebook);
      }
      list = list->next;
    }

  for(list = gkb->maps; list != NULL; list = list->next) {
	  PropWg *p = list->data;
	  if(p) {
		  g_free(p->name);
		  g_free(p->command);
		  g_free(p->iconpath);
		  g_free(p);
	  }
  }
  g_list_free(gkb->maps);

  gkb->maps = copy_propwgs (gkb);
  gkb->n = gkb->tn;

  gkb->dact = g_list_nth_data (gkb->maps, gkb->cur);

  sized_render (gkb);
  gkb_draw (gkb);

  /* execute in a shell but don't wait for the thing to end */
  gnome_execute_shell (NULL, gkb->dact->command);

  /* tell the panel to save our configuration data */
  applet_widget_sync_config(APPLET_WIDGET(gkb->applet));
}

static void
destroy_cb (GtkWidget * widget, GKB * gkb)
{
  gkb->propbox = NULL;
}

static void
makenotebook (GKB * gkb, PropWg * actdata, int i)
{
  actdata->notebook = gkb->notebook;
  actdata->propbox = gkb->propbox;
  actdata->vbox1 = gtk_vbox_new (FALSE, 0);

  gtk_widget_ref (actdata->vbox1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbox1",
			    actdata->vbox1,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->vbox1);
  gtk_container_add (GTK_CONTAINER (gkb->notebook), actdata->vbox1);

  actdata->hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (actdata->hbox1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox1",
			    actdata->hbox1,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->hbox1);

  actdata->frame4 = gtk_frame_new (_("Keymap name"));
  gtk_widget_ref (actdata->frame4);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame4",
			    actdata->frame4,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->frame4);
  gtk_box_pack_start (GTK_BOX (actdata->hbox1), actdata->frame4, FALSE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame4), 2);

  gtk_container_add (GTK_CONTAINER (actdata->vbox1), actdata->hbox1);

  actdata->keymapname = gtk_entry_new ();
  gtk_widget_ref (actdata->keymapname);

  gtk_entry_set_text (GTK_ENTRY(actdata->keymapname), actdata->name);

  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "keymapname",
			    actdata->keymapname,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->keymapname);
  gtk_container_add (GTK_CONTAINER (actdata->frame4), actdata->keymapname);

  gtk_signal_connect (GTK_OBJECT (actdata->keymapname),
		      "changed", GTK_SIGNAL_FUNC (changed_cb), gkb);

  actdata->frame6 = gtk_frame_new (_("Keymap control"));
  gtk_widget_ref (actdata->frame6);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame6",
			    actdata->frame6,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->frame6);
  gtk_box_pack_start (GTK_BOX (actdata->hbox1), actdata->frame6, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame6), 2);


  actdata->hboxmap = gtk_hbox_new (FALSE, 0);

  actdata->newkeymap = gtk_button_new_with_label (_("New keymap"));

  gtk_widget_ref (actdata->newkeymap);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "newkeymap",
			    actdata->newkeymap,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->newkeymap);
  gtk_signal_connect (GTK_OBJECT (actdata->newkeymap),
		      "clicked", (GtkSignalFunc) newmap_cb, gkb);

  gtk_container_add (GTK_CONTAINER (actdata->hboxmap), actdata->newkeymap);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->newkeymap), 2);

  actdata->delkeymap = gtk_button_new_with_label (_("Delete this keymap"));

  gtk_widget_ref (actdata->delkeymap);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "delkeymap",
			    actdata->delkeymap,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->delkeymap);
  gtk_signal_connect (GTK_OBJECT (actdata->delkeymap),
		      "clicked", (GtkSignalFunc) delmap_cb, gkb);

  gtk_container_add (GTK_CONTAINER (actdata->hboxmap), actdata->delkeymap);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->delkeymap), 2);
  gtk_container_add (GTK_CONTAINER (actdata->frame6), actdata->hboxmap);

  gtk_widget_show (actdata->hboxmap);

  actdata->hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (actdata->hbox2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "hbox2",
			    actdata->hbox2,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->hbox2);
  gtk_box_pack_start (GTK_BOX (actdata->vbox1), actdata->hbox2, TRUE, TRUE,
		      0);

  actdata->iconentry = gnome_icon_entry_new (NULL, _("Keymap icon"));
  gtk_widget_ref (actdata->iconentry);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "iconentry",
			    actdata->iconentry,
			    (GtkDestroyNotify) gtk_widget_unref);
  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry),
                             actdata->iconpath);
  gtk_signal_connect (GTK_OBJECT (gnome_icon_entry_gtk_entry(
                   GNOME_ICON_ENTRY(actdata->iconentry))), "changed",
		      GTK_SIGNAL_FUNC (icontopath_cb), actdata);

  gtk_widget_show (actdata->iconentry);

  gtk_box_pack_end (GTK_BOX (actdata->hbox2), actdata->iconentry, FALSE,
		    FALSE, 0);
  gtk_widget_set_usize (actdata->iconentry, 60, 40);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->iconentry), 2);

  actdata->vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (actdata->vbox2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "vbox2",
			    actdata->vbox2,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->vbox2);
  gtk_box_pack_start (GTK_BOX (actdata->hbox2), actdata->vbox2, TRUE, TRUE,
		      0);

  actdata->frame1 = gtk_frame_new (_("Iconpath"));
  gtk_widget_ref (actdata->frame1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame1",
			    actdata->frame1,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->frame1);
  gtk_box_pack_start (GTK_BOX (actdata->vbox2), actdata->frame1, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame1), 2);

  actdata->iconpathinput = gtk_entry_new ();
  gtk_widget_ref (actdata->iconpathinput);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "iconpathinput",
			    actdata->iconpathinput,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_signal_connect (GTK_OBJECT (actdata->iconpathinput), "changed",
		      GTK_SIGNAL_FUNC (pathtoicon_cb), actdata);

  gtk_entry_set_text (GTK_ENTRY(actdata->iconpathinput), actdata->iconpath);
  gtk_widget_show (actdata->iconpathinput);
  gtk_container_add (GTK_CONTAINER (actdata->frame1), actdata->iconpathinput);

  actdata->frame2 = gtk_frame_new (_("Full command"));
  gtk_widget_ref (actdata->frame2);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "frame2",
			    actdata->frame2,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->frame2);
  gtk_box_pack_start (GTK_BOX (actdata->vbox2), actdata->frame2, TRUE, TRUE,
		      0);
  gtk_container_set_border_width (GTK_CONTAINER (actdata->frame2), 2);

  actdata->commandinput = gtk_entry_new ();
  gtk_widget_ref (actdata->commandinput);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "commandinput",
			    actdata->commandinput,
			    (GtkDestroyNotify) gtk_widget_unref);

  gtk_entry_set_text (GTK_ENTRY(actdata->commandinput), actdata->command);

  gtk_signal_connect (GTK_OBJECT (actdata->commandinput), "changed",
		      GTK_SIGNAL_FUNC (changed_cb), gkb);
  gtk_widget_show (actdata->commandinput);
  gtk_container_add (GTK_CONTAINER (actdata->frame2), actdata->commandinput);

  actdata->label1 = gtk_label_new (actdata->name);
  gtk_widget_ref (actdata->label1);
  gtk_object_set_data_full (GTK_OBJECT (gkb->propbox), "label1",
			    actdata->label1,
			    (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (actdata->label1);

  gtk_notebook_set_tab_label (GTK_NOTEBOOK (gkb->notebook),
			      gtk_notebook_get_nth_page (GTK_NOTEBOOK
		              (gkb->notebook), i), actdata->label1);
}

void
properties_dialog (AppletWidget * applet, gpointer gkbx)
{

  GKB *gkb = (GKB *) gkbx;
  int i = 0;

  GList * list;

  if (gkb->propbox)
    {
      gtk_widget_show_now (gkb->propbox);
      gdk_window_raise (gkb->propbox->window);
      return;
     }

  for(list = gkb->tempmaps; list != NULL; list = list->next) {
	  PropWg * actdata = list->data;
	  if(actdata) {
		  g_free(actdata->name);
		  g_free(actdata->iconpath);
		  g_free(actdata->command);
		  g_free(actdata);
	  }
  }
  g_list_free(gkb->tempmaps);

  gkb->tempmaps = copy_props (gkb);
  gkb->tn  = gkb->n;

  gkb->propbox = gnome_property_box_new ();
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "propbox", gkb->propbox);
  gtk_window_set_title (GTK_WINDOW (gkb->propbox), _("GKB Properties"));
  gtk_window_set_policy (GTK_WINDOW (gkb->propbox), FALSE, FALSE, TRUE);
  gtk_widget_show_all (GTK_WIDGET (gkb->propbox));

  gkb->notebook = GNOME_PROPERTY_BOX (gkb->propbox)->notebook;
  gtk_object_set_data (GTK_OBJECT (gkb->propbox), "notebook", gkb->notebook);
  gtk_widget_show (gkb->notebook);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (gkb->notebook), TRUE);
  gtk_notebook_popup_enable (GTK_NOTEBOOK (gkb->notebook));
  gtk_widget_show_all (gkb->propbox);

  list = gkb->tempmaps;
  while (list)
    {
      if (list->data) makenotebook (gkb, list->data,i++);
      list = list->next;
    }

  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "apply", GTK_SIGNAL_FUNC (apply_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "destroy", GTK_SIGNAL_FUNC (destroy_cb), gkb);
  gtk_signal_connect (GTK_OBJECT (gkb->propbox),
		      "help", GTK_SIGNAL_FUNC (prophelp_cb), NULL);
  gtk_widget_show_all (gkb->propbox);

  return;
  applet = NULL;
}

static void
prophelp_cb (AppletWidget * applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry = { "gkb_applet", "index.html#GKBAPPLET-PREFS" };

  gnome_help_display (NULL, &help_entry);
}
