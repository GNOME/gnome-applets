static GList *
find_presets ()
{
  DIR *dir;
  struct dirent *actfile;
  GList *diritems = NULL;

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
  GList * retlist, * templist, * newitem;
  gchar * prefix;
  gchar * tname, * tcodepage;
  gchar * ttype, * tarch, * tcommand;
  gchar * set, * filename;
  gint i, knum = 1;

  templist = list;
  retlist = NULL;

  while (templist = g_list_next (templist))
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
