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
  int i = 0;
  Prop * prop;
  PropWg * p2;
  GList * tempmaps = g_list_alloc();
  GList * list = gkb->maps;

  gkb->tn  = gkb->n;

  while (list)
    {
      if((prop = list->data) != NULL) 
       {
        p2 = cp_prop (prop);
        if (tempmaps->data == NULL) 
          tempmaps->data=p2;
         else
          tempmaps = g_list_insert (tempmaps, p2,i++);
       }
      list = list->next;
    }
  return tempmaps;
}

static GList *
copy_propwgs (GKB * gkb)
{
  int i = 0;
  PropWg * prop;
  Prop * p2;
  GList * tempmaps = g_list_alloc();
  GList * list = gkb->tempmaps;

  gkb->n= gkb->tn;

  while (list)
    {
      if((prop = list->data) != NULL) 
       {
        p2 = cp_propwg (prop);
        if (tempmaps->data == NULL)
          tempmaps->data=p2;
         else
          tempmaps = g_list_insert (tempmaps, p2,i++);
       }
      list = list->next;
    }
  return tempmaps;
}

static PropWg *
cp_prop (Prop * data)
{

  PropWg * tempdata = g_new (PropWg, 1);

  tempdata->name = (char *) strdup (data->name);
  tempdata->command = (char *) strdup (data->command);
  tempdata->iconpath = (char *) strdup (data->iconpath);

  return tempdata;
}

static Prop *
cp_propwg (PropWg * data)
{

  Prop *tempdata = g_new (Prop, 1);

  tempdata->name = (char *) strdup (data->name);
  tempdata->command = (char *) strdup (data->command);
  tempdata->iconpath = (char *) strdup (data->iconpath);

  return tempdata;
}

static void
tell_panel (void)
{
  CORBA_Environment ev;
  GNOME_Panel panel_client = CORBA_OBJECT_NIL;

  panel_client = goad_server_activate_with_repo_id (NULL,
						    "IDL:GNOME/Panel:1.0",
						    GOAD_ACTIVATE_EXISTING_ONLY,
						    NULL);
  if (!panel_client)
    return;
  CORBA_exception_init (&ev);
  GNOME_Panel_notice_config_changes (panel_client, &ev);
  CORBA_exception_free (&ev);
}

static void
delmap_cb (GnomePropertyBox * pb,GKB * gkb)
{
  gint page;

  if (gkb->tempmaps->next == NULL) return;

  page = gtk_notebook_get_current_page(gkb->notebook);
  gtk_notebook_remove_page (gkb->notebook, page);
  gkb->tempmaps = g_list_remove(gkb->tempmaps, 
  			g_list_nth_data(gkb->tempmaps, page));
  gtk_widget_draw(GTK_WIDGET(gkb->notebook), NULL);

  gkb->tn--;
  
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
  
}

static void
icontopath_cb (GnomePropertyBox * pb, PropWg * actdata)
{
 g_return_if_fail (GTK_WIDGET_REALIZED (actdata->iconpathinput));
 g_return_if_fail (GTK_WIDGET_REALIZED (actdata->iconentry));
 gtk_entry_set_text (GTK_ENTRY(actdata->iconpathinput),
	 gnome_icon_entry_get_filename (
	     GNOME_ICON_ENTRY (actdata->iconentry)));
 
 gtk_widget_show(actdata->iconpathinput);
 
 gnome_property_box_changed (GNOME_PROPERTY_BOX (actdata->propbox));

}

static void
pathtoicon_cb (GnomePropertyBox * pb, PropWg * actdata)
{
 int i;
 
 g_return_if_fail (GTK_WIDGET_REALIZED (actdata->iconpathinput));
 g_return_if_fail (GTK_WIDGET_REALIZED (actdata->iconentry));

 i = strcmp (gtk_entry_get_text(GTK_ENTRY(actdata->iconpathinput)),
   gnome_icon_entry_get_filename(GNOME_ICON_ENTRY (actdata->iconentry)));

 if (!i) return;
 
 gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry),
       gtk_entry_get_text (GTK_ENTRY (actdata->iconpathinput)));

 gnome_property_box_changed (GNOME_PROPERTY_BOX (actdata->propbox));

}

static void
newmap_cb (GnomePropertyBox * pb, GKB * gkb)
{

  PropWg *actdata = cp_prop(loadprop (gkb, 0));

  makenotebook (gkb, actdata, gkb->tn);

  gkb->tempmaps = g_list_insert (gkb->tempmaps, actdata, gkb->tn++);

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

      if (actdata->name) g_free (actdata->name);
      if (actdata->iconpath) g_free (actdata->iconpath);
      if (actdata->command) g_free (actdata->command);

      actdata->name = (char *) strdup (gtk_entry_get_text (GTK_ENTRY (actdata->keymapname)));

      actdata->iconpath = (char *) strdup (
	gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (actdata->iconentry)));

      actdata->command = (char *) strdup (
	gtk_entry_get_text (GTK_ENTRY (actdata->commandinput)));

      gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (actdata->notebook),
           gtk_notebook_get_nth_page (GTK_NOTEBOOK (actdata->notebook), i++),
	     gtk_entry_get_text (GTK_ENTRY(actdata->keymapname)));

      gtk_widget_show (actdata->notebook);
      }
      list = list->next;
    }

  gkb->maps = copy_propwgs (gkb);

  sized_render (gkb);
  gkb_draw (gkb);
  system (gkb->dact->command);

  applet_save_session (gkb->propbox,
                       APPLET_WIDGET (gkb->applet)->privcfgpath,
                       APPLET_WIDGET (gkb->applet)->globcfgpath, gkb);

  tell_panel ();
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
      gtk_widget_show (gkb->propbox);
      gdk_window_raise (gkb->propbox->window);
      return;
     }


  gkb->tempmaps = copy_props (gkb);
  list = gkb->tempmaps;

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
