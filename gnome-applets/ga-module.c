/*
 * Copyright (C) 2020 Alberts Muktupāvels
 * Copyright (C) 2020 Sebastian Geiger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <libgnome-panel/gp-module.h>

#include "battstat/battstat-applet.h"
#include "brightness/brightness-applet.h"
#include "command/command-applet.h"
#include "gweather/gweather-applet.h"
#include "netspeed/netspeed-applet.h"
#include "timer/timer-applet.h"
#include "trash/trash-applet.h"
#include "window-picker/wp-applet.h"

static GpAppletInfo *
ga_get_applet_info (const char *id)
{
  GpGetAppletTypeFunc type_func;
  const char *name;
  const char *description;
  const char *icon_name;
  GpAppletInfo *info;

  if (g_strcmp0 (id, "battstat") == 0)
    {
      type_func = battstat_applet_get_type;
      name = _("Battery Charge Monitor");
      description = _("Monitor a laptop's remaining power");
      icon_name = "battery";
    }
  else if (g_strcmp0 (id, "brightness") == 0)
    {
      type_func = gpm_brightness_applet_get_type;
      name = _("Brightness Applet");
      description = _("Adjusts Laptop panel brightness");
      icon_name = "gnome-brightness-applet";
    }
  else if (g_strcmp0 (id, "command") == 0)
    {
      type_func = command_applet_get_type;
      name = _("Command");
      description = _("Shows the output of a command");
      icon_name = "utilities-terminal";
    }
  else if (g_strcmp0 (id, "gweather") == 0)
    {
      type_func = gweather_applet_get_type;
      name = _("Weather Report");
      description = _("Monitor the current weather conditions, and forecasts");
      icon_name = "weather-storm";
    }
  else if (g_strcmp0 (id, "netspeed") == 0)
    {
      type_func = netspeed_applet_get_type;
      name = _("Network Monitor");
      description = _("Netspeed Applet");
      icon_name = "netspeed-applet";
    }
  else if (g_strcmp0 (id, "timer") == 0)
    {
      type_func = timer_applet_get_type;
      name = _("Timer");
      description = _("Start a timer and receive a notification when it is finished");
      icon_name = "gnome-panel-clock";
    }
  else if (g_strcmp0 (id, "trash") == 0)
    {
      type_func = trash_applet_get_type;
      name = _("Trash");
      description = _("Go to Trash");
      icon_name = "user-trash-full";
    }
  else if (g_strcmp0 (id, "window-picker") == 0)
    {
      type_func = wp_applet_get_type;
      name = _("Window Picker");
      description = _("Shows a list of icons for the open windows.");
      icon_name = "preferences-system-windows";
    }
  else
    {
      g_assert_not_reached ();
      return NULL;
    }

  info = gp_applet_info_new (type_func, name, description, icon_name);

  return info;
}

static const char *
ga_get_applet_id_from_iid (const char *iid)
{
  if (g_strcmp0 (iid, "BattstatAppletFactory::BattstatApplet") == 0)
    return "battstat";
  else if (g_strcmp0 (iid, "BrightnessAppletFactory::BrightnessApplet") == 0)
    return "brightness";
  else if (g_strcmp0 (iid, "CommandAppletFactory::CommandApplet") == 0)
    return "command";
  else if (g_strcmp0 (iid, "GWeatherAppletFactory::GWeatherApplet") == 0)
    return "gweather";
  else if (g_strcmp0 (iid, "NetspeedAppletFactory::NetspeedApplet") == 0)
    return "netspeed";
  else if (g_strcmp0 (iid, "TimerAppletFactory::TimerApplet") == 0)
    return "timer";
  else if (g_strcmp0 (iid, "WindowPickerFactory::WindowPicker") == 0 ||
           g_strcmp0 (iid, "org.gnome.gnome-applets.window-picker::window-picker") == 0)
    return "window-picker";
  else if (g_strcmp0 (iid, "TrashAppletFactory::TrashApplet") == 0)
    return "trash";

  return NULL;
}

void
gp_module_load (GpModule *module)
{
  bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  gp_module_set_gettext_domain (module, GETTEXT_PACKAGE);

  gp_module_set_abi_version (module, GP_MODULE_ABI_VERSION);

  gp_module_set_id (module, "org.gnome.gnome-applets");
  gp_module_set_version (module, PACKAGE_VERSION);

  gp_module_set_applet_ids (module,
                            "battstat",
                            "brightness",
                            "command",
                            "gweather",
                            "netspeed",
                            "timer",
                            "trash",
                            "window-picker",
                            NULL);

  gp_module_set_get_applet_info (module, ga_get_applet_info);
  gp_module_set_compatibility (module, ga_get_applet_id_from_iid);
}
