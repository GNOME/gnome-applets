static GList *
copy_props ()
{
  GList * tempmaps = NULL;
  GList * list;

  gkb->tempsize=gkb->size;  
  gkb->tempsmall=gkb->small;

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
copy_propwgs ()
{
  GList * tempmaps = NULL;
  GList * list;

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
