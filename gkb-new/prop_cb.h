static void
delmap_cb (GnomePropertyBox * pb,GKB * gkb)
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

  prop_show (gkb);
  
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
  
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

  prop_show (gkb);

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
	gnome_icon_entry_set_icon (GNOME_ICON_ENTRY(actdata->iconentry21), etext);

	gnome_property_box_changed (GNOME_PROPERTY_BOX (actdata->propbox));
}

static void
switch_normal_cb (GnomePropertyBox * pb, GKB * gkb)
{
 gkb->tempsmall = 0;
 gkb->tempsize =
    applet_widget_get_panel_pixel_size (APPLET_WIDGET (gkb->applet));
 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}

static void
switch_small_cb (GnomePropertyBox * pb, GKB * gkb)
{
 gkb->tempsmall = 1;
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
             gtk_notebook_get_nth_page (GTK_NOTEBOOK (actdata->notebook), (i++)+1),
	       gtk_entry_get_text (GTK_ENTRY(actdata->keymapname)));

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

  gkb->maps = copy_propwgs (gkb);
  gkb->n = gkb->tn;

  gkb->dact = g_list_nth_data (gkb->maps, gkb->cur);

  gkb->small = gkb->tempsmall;
  gkb->size = gkb->tempsize;

  sized_render (gkb);
  gkb_draw (gkb);

  /* execute in a shell but don't wait for the thing to end is bad idea, I think... */
  if (system(gkb->dact->command))
     gnome_error_dialog(_("The keymap switching command returned with error!"));

  advanced_show(gkb);

  /* tell the panel to save our configuration data */
  applet_widget_sync_config(APPLET_WIDGET(gkb->applet));
}

static void
destroy_cb (GtkWidget * widget, GKB * gkb)
{
  gkb->propbox = NULL;
}


static void
switch_adv_cb (GtkWidget * widget, GKB * gkb)
{
 gkb->advconf = GTK_TOGGLE_BUTTON(gkb->advanced)->active;

 gnome_property_box_changed (GNOME_PROPERTY_BOX (gkb->propbox));
}
