static void
delmap_cb (GnomePropertyBox * pb)
{
  gint page;
  PropWg *actdata;

  if (gkb->tempmaps->next == NULL) return;

  page = gtk_notebook_get_current_page(GTK_NOTEBOOK(gkb->notebook));
  page--;
  gtk_notebook_remove_page (GTK_NOTEBOOK(gkb->notebook), page+1);
  actdata = g_list_nth_data(gkb->tempmaps, page);
  gkb->tempmaps = g_list_remove(gkb->tempmaps, actdata);
  g_free(actdata->name);
  g_free(actdata->iconpath);
  g_free(actdata->command);
  g_free(actdata);
  gtk_widget_draw(GTK_WIDGET(gkb->notebook), NULL);

  gkb->tn--;

  prop_show ();
  
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
  
}

static void
newmap_cb (GnomePropertyBox * pb)
{
  PropWg *actdata;

  gnome_config_push_prefix (APPLET_WIDGET (gkb->applet)->privcfgpath);
  actdata = cp_prop(loadprop (0));
  gnome_config_pop_prefix ();

  makenotebook (actdata, gkb->tn);

  gkb->tempmaps = g_list_append (gkb->tempmaps, actdata);
  gkb->tn ++;

  prop_show ();

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

	gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry21), itext);
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
	gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry21), etext);

	gnome_property_box_changed (GNOME_PROPERTY_BOX (actdata->propbox));
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
switch_small_cb (GnomePropertyBox * pb)
{
 gkb->tempsmall = 1;
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
changed_cb (GnomePropertyBox * pb)
{
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
apply_cb (GtkWidget * pb, gint page)
{
  /* TODO: If gkb->adv... */

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

	if(gkb->advconf == 1)
	{
          actdata->name = g_strdup (gtk_entry_get_text (GTK_ENTRY (actdata->keymapname)));
          actdata->iconpath = 
	    gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (actdata->iconentry));
          actdata->command = g_strdup (
	    gtk_entry_get_text (GTK_ENTRY (actdata->commandinput)));
          gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (actdata->notebook),
             gtk_notebook_get_nth_page (GTK_NOTEBOOK (actdata->notebook), (i++)+1),
	       gtk_entry_get_text (GTK_ENTRY(actdata->keymapname)));
        }
	else
	{
	  if (strlen(gtk_entry_get_text (GTK_ENTRY (actdata->entry21))) < 1)
	   {
             actdata->name = g_strdup (gtk_entry_get_text (GTK_ENTRY (actdata->keymapname)));
             actdata->iconpath = 
             	    gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (actdata->iconentry));
             actdata->command = g_strdup (
	            gtk_entry_get_text (GTK_ENTRY (actdata->commandinput)));
             gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (actdata->notebook),
              gtk_notebook_get_nth_page (GTK_NOTEBOOK (actdata->notebook), (i++)+1),
	        gtk_entry_get_text (GTK_ENTRY(actdata->keymapname)));
	   }
	   else
	   {
          actdata->name = g_strdup (gtk_entry_get_text (GTK_ENTRY (actdata->entry21)));
          actdata->iconpath = 
	    gnome_icon_entry_get_filename (GNOME_ICON_ENTRY (actdata->iconentry21));
          actdata->command = 
            gtk_object_get_data (GTK_OBJECT (actdata->label25),"command");
          gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (actdata->notebook),
             gtk_notebook_get_nth_page (GTK_NOTEBOOK (actdata->notebook), (i++)+1),
	       gtk_entry_get_text (GTK_ENTRY(actdata->entry21)));
          }
        }
        gtk_widget_show (actdata->notebook);
      }
      list = list->next;
    }

  /* Free all maps before copy */
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

  gkb->maps = copy_propwgs ();
  gkb->n = gkb->tn;

  gkb->dact = g_list_nth_data (gkb->maps, gkb->cur);

  gkb->small = gkb->tempsmall;
  gkb->size = gkb->tempsize;

  gkb->advconf = gkb->tempadvconf;

  sized_render ();
  gkb_draw ();

  /* execute in a shell but don't wait for the thing to end is bad idea,
   * I think... */

  if (system(gkb->dact->command))
     gnome_error_dialog(_("The keymap switching command returned with error!"));

  advanced_show();

  /* tell the panel to save our configuration data */
  applet_widget_sync_config(APPLET_WIDGET(gkb->applet));
}

static void
destroy_cb (GtkWidget * widget)
{
  gkb->propbox = NULL;
}


static void
switch_adv_cb (GtkWidget * widget)
{
 gkb->tempadvconf = GTK_TOGGLE_BUTTON(gkb->advanced)->active;

 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static gint
keymap_select_cb(GtkWidget * widget, PropWg * actdata)
{
  gchar * labeltext, * flagpath, * flag, *command;
  GList * sel;
  GtkWidget *litem;
  gint i;

  sel = GTK_LIST(widget)->selection;

  if (!sel) {
   g_print("Selection cleared\n");
   return;
  }

  while (sel) /* Hehe run it once */
   {
     litem = GTK_OBJECT(sel->data);

     labeltext = gtk_object_get_data (litem, "label");
     flag = gtk_object_get_data (litem, "flag");
     command = gtk_object_get_data (litem, "command");
     sel=sel->next;
   }

  flagpath = g_strconcat ("gkb/",flag,NULL);
  gtk_entry_set_text (GTK_ENTRY(actdata->entry21), labeltext);
  gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry21),
    flagpath);

  gtk_object_set_data (GTK_OBJECT (actdata->label25), "command", command);

  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
  return FALSE;
}


static gint
country_select_cb(GtkWidget * widget, CountryData * cdata)
{
  GtkWidget * listwg;
  GList * list;
  GKBData * data;

  listwg = cdata->data->list;
  gtk_list_clear_items (GTK_LIST (listwg), 0, -1);
  list = cdata->keymaps;

  while (list != NULL)
   { 
    GKBpreset * item;
    GtkWidget *list_item;

    item = (GKBpreset *) list->data;
    list_item=gtk_list_item_new_with_label(item->name);
    gtk_object_set_data(GTK_OBJECT(list_item),
                        "label", item->label);
    gtk_object_set_data(GTK_OBJECT(list_item),
                        "flag", item->flag);
    gtk_object_set_data(GTK_OBJECT(list_item),
                        "command", item->command);
    gtk_container_add(GTK_CONTAINER(listwg), list_item);
    gtk_widget_show(list_item);
    data = g_new0(GKBData,1);
    data->preset = item;

    list = list->next;
   }

  gtk_list_select_item (listwg,0);
  return FALSE;
}
