static void
advanced_show()
{
  GList * list;
  PropWg * prop;

  for(list = gkb->tempmaps; list != NULL; list = list->next) {
    prop = list->data;    
    if (gkb->advconf)     
      {
       gtk_widget_hide(prop->hfn);
       gtk_widget_show(prop->hfa);
      }
	else
      {  
       gtk_widget_hide(prop->hfa);
       gtk_widget_show(prop->hfn);
      }
  }
}

static void
prop_show ()
{
  int i=1;
  GList * list;

  gtk_widget_show(gkb->propbox);
  for(list = gkb->tempmaps; list != NULL; list = list->next) {
    gtk_widget_show(gtk_notebook_get_nth_page (GTK_NOTEBOOK (gkb->notebook),
    					      (i++)));
  }
  advanced_show();
}
