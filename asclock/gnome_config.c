#include "asclock.h"


void set_gnome_config(asclock *my, char *config_path)
{
  printf("save config path = %s\n", config_path);

  gnome_config_push_prefix (config_path);

  gnome_config_set_string("ASClock/theme_filename", my->theme_filename);
  printf("theme = %s\n", my->theme_filename);
  gnome_config_set_string("ASClock/timezone", my->timezone);
  printf("timezone = %s\n", my->timezone);
  gnome_config_set_int(   "ASClock/itblinks", my->itblinks);
  printf("itblinks = %d\n", my->itblinks);
  gnome_config_set_int(   "ASClock/isampm", my->showampm);
  printf("showampm = %d\n", my->showampm);
  gnome_config_pop_prefix();

  gnome_config_sync();
  
  get_gnome_config(my, config_path);
}

void get_gnome_config(asclock *my, char *config_path)
{
  char *tmp;
  gboolean is_default;

  printf("load config path = %s\n", config_path);
  gnome_config_push_prefix (config_path);

  tmp =
  gnome_config_get_string_with_default("ASClock/theme_filename=", &is_default);
  strncpy(my->theme_filename, tmp, 512);
  printf("theme = %s\n", my->theme_filename);
  g_free(tmp);

  my->itblinks =
  gnome_config_get_int_with_default("ASClock/itblinks=1", &is_default);
  printf("itblinks = %d\n", my->itblinks);

  my->showampm = 
  gnome_config_get_int_with_default("ASClock/showampm=0", &is_default);
  printf("showampm = %d\n", my->showampm);

  tmp =
  gnome_config_get_string_with_default("ASClock/timezone=", &is_default);
  strncpy(my->timezone, tmp, 512);
  printf("timezone = %s\n", my->timezone);
  g_free(tmp);

  gnome_config_pop_prefix();
}

