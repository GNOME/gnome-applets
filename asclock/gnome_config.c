#include "asclock.h"


void set_gnome_config(asclock *my, char *config_path)
{

  gnome_config_push_prefix (config_path);

  gnome_config_set_string("ASClock/theme_filename", my->theme_filename);

  gnome_config_set_string("ASClock/timezone", my->timezone);

  gnome_config_set_int(   "ASClock/itblinks", my->itblinks);

  gnome_config_set_int(   "ASClock/showampm", my->showampm);

  gnome_config_pop_prefix();

  gnome_config_sync();
  
  get_gnome_config(my, config_path);
}

void get_gnome_config(asclock *my, char *config_path)
{
  char *tmp;
  gboolean is_default;

  gnome_config_push_prefix (config_path);

  tmp =
  gnome_config_get_string_with_default("ASClock/theme_filename=", &is_default);
  strncpy(my->theme_filename, tmp, 512);
  my->theme_filename[511] = '\0';

  g_free(tmp);

  my->itblinks =
  gnome_config_get_int_with_default("ASClock/itblinks=1", &is_default);

  my->showampm = 
  gnome_config_get_int_with_default("ASClock/showampm=0", &is_default);

  tmp =
  gnome_config_get_string_with_default("ASClock/timezone=", &is_default);
  strncpy(my->timezone, tmp, 512);
  my->timezone[511] = '\0';

  g_free(tmp);

  gnome_config_pop_prefix();
}

