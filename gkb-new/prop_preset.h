static GList *
find_presets ()
{
	DIR *dir;
	struct dirent *actfile;
	GList *diritems = NULL;

	dir = opendir (gnome_unconditional_datadir_file ("gnome/gkb/"));

	prefixdir=g_strdup(gnome_unconditional_datadir_file ("gnome/gkb/"));
  
	if (dir == NULL)
		return NULL;
	
	while ((actfile = readdir (dir)) != NULL) {
	        if (!strstr(actfile->d_name, ".keyprop"))
	                continue;
	
		if (actfile->d_name[0] != '.') {
			diritems = g_list_insert_sorted(diritems,
	                                     g_strdup(actfile->d_name),
			                     (GCompareFunc)strcmp);
		}
	}
	return diritems;
}

static GKBpreset *
gkb_preset_load (const char *filename)
{
        GKBpreset *retval;
        gchar * prefix;

 	g_assert (filename != NULL);

        retval = g_new0(GKBpreset, 1);

 	prefix = g_strconcat ("=",prefixdir,"/", filename, "=/Keymap Entry/", NULL);

 	gnome_config_push_prefix (prefix);
 	g_free (prefix);

	retval->name = gnome_config_get_translated_string ("Name");
	retval->lang = gnome_config_get_translated_string ("Language");
	retval->country = gnome_config_get_translated_string ("Country");
	retval->codepage = gnome_config_get_string ("Codepage");
	retval->flag = gnome_config_get_string ("Flag");
	retval->label = gnome_config_get_string ("Label");
	retval->type = gnome_config_get_string ("Type");
	retval->arch = gnome_config_get_string ("Arch");
	retval->command = gnome_config_get_string ("Command");

        return retval;
}
