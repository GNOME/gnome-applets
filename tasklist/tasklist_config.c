#include <gnome.h>
#include "tasklist_applet.h"

extern Config config;

void config_save (const gchar *privcfgpath)
{
  gnome_config_push_prefix (privcfgpath);
  gnome_config_set_bool ("tasks/all_tasks", config.all_tasks);
  gnome_config_set_bool ("tasks/minimized_tasks", config.minimized_tasks);
  gnome_config_set_bool ("tasks/show_all", config.show_all);
  gnome_config_set_bool ("tasks/show_normal", config.show_normal);
  gnome_config_set_bool ("tasks/show_minimized", config.show_minimized);

  gnome_config_set_bool ("general/show_winops", config.show_winops);
  gnome_config_set_bool ("general/confirm_kill", config.confirm_kill);
  
  gnome_config_set_bool ("geomestroy/numrows_follows_panel", config.numrows_follows_panel);

  gnome_config_pop_prefix ();
  gnome_config_sync ();
  gnome_config_drop_all ();

}

void config_load (const gchar *privcfgpath)
{
  static guint loaded = FALSE;
 
  if (loaded)
    return;

  gnome_config_push_prefix (privcfgpath);
  config.all_tasks = gnome_config_get_bool("tasks/all_tasks=false");
  config.minimized_tasks = gnome_config_get_bool("tasks/minimized_tasks=false");
  config.show_all = gnome_config_get_bool("tasks/show_all=true");
  config.show_normal = gnome_config_get_bool("tasks/show_normal=false");
  config.show_minimized = gnome_config_get_bool("tasks/show_minimized=false");
  config.show_winops = gnome_config_get_bool("general/show_winops=true");
  config.confirm_kill = gnome_config_get_bool("general/confirm_kill=true");
  config.numrows_follows_panel = gnome_config_get_bool("geometry/numrows_follows_panel=true");

  gnome_config_pop_prefix ();
}
