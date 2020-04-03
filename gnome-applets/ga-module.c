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

#include "accessx-status/accessx-status-applet.h"
#include "battstat/battstat-applet.h"
#include "brightness/brightness-applet.h"
#include "charpick/charpick-applet.h"
#include "command/command-applet.h"
#ifdef BUILD_CPUFREQ_APPLET
#include "cpufreq/cpufreq-applet.h"
#endif
#include "drivemount/drivemount-applet.h"
#include "geyes/geyes-applet.h"
#include "gweather/gweather-applet.h"
#include "inhibit/inhibit-applet.h"
#include "mini-commander/mini-commander-applet.h"
#include "multiload/multiload-applet.h"
#include "netspeed/netspeed-applet.h"
#include "sticky-notes/sticky-notes-applet.h"
#include "timer/timer-applet.h"
#ifdef HAVE_TRACKER_SEARCH_BAR
#include "tracker-search-bar/tracker-applet.h"
#endif
#include "trash/trash-applet.h"
#include "window-buttons/window-buttons.h"
#include "window-picker/wp-applet.h"
#include "window-title/window-title.h"

static GpAppletInfo *
ga_get_applet_info (const char *id)
{
  GpGetAppletTypeFunc type_func;
  const char *name;
  const char *description;
  const char *icon_name;
  GpAboutDialogFunc about_func;
  const char *help_uri;
  GpAppletInfo *info;

  about_func = NULL;
  help_uri = NULL;

  if (g_strcmp0 (id, "accessx-status") == 0)
    {
      type_func = accessx_status_applet_get_type;
      name = _("Keyboard Accessibility Status");
      description = _("Shows the status of keyboard accessibility features");
      icon_name = "ax-applet";

      about_func = accessx_status_applet_setup_about;
      help_uri = "help:accessx-status";
    }
  else if (g_strcmp0 (id, "battstat") == 0)
    {
      type_func = battstat_applet_get_type;
      name = _("Battery Charge Monitor");
      description = _("Monitor a laptop's remaining power");
      icon_name = "battery";

      about_func = battstat_applet_setup_about;
      help_uri = "help:battstat";
    }
  else if (g_strcmp0 (id, "brightness") == 0)
    {
      type_func = gpm_brightness_applet_get_type;
      name = _("Brightness Applet");
      description = _("Adjusts Laptop panel brightness");
      icon_name = "gnome-brightness-applet";

      about_func = gpm_brightness_applet_setup_about;
    }
  else if (g_strcmp0 (id, "charpick") == 0)
    {
      type_func = charpick_applet_get_type;
      name = _("Character Palette");
      description = _("Insert characters");
      icon_name = "accessories-character-map";

      about_func = charpick_applet_setup_about;
      help_uri = "help:char-palette";
    }
  else if (g_strcmp0 (id, "command") == 0)
    {
      type_func = command_applet_get_type;
      name = _("Command");
      description = _("Shows the output of a command");
      icon_name = "utilities-terminal";

      about_func = command_applet_setup_about;
    }
#ifdef BUILD_CPUFREQ_APPLET
  else if (g_strcmp0 (id, "cpufreq") == 0)
    {
      type_func = cpufreq_applet_get_type;
      name = _("CPU Frequency Scaling Monitor");
      description = _("Monitor the CPU Frequency Scaling");
      icon_name = "gnome-cpu-frequency-applet";

      about_func = cpufreq_applet_setup_about;
      help_uri = "help:cpufreq-applet";
    }
#endif
  else if (g_strcmp0 (id, "drivemount") == 0)
    {
      type_func = drivemount_applet_get_type;
      name = _("Disk Mounter");
      description = _("Mount local disks and devices");
      icon_name = "media-floppy";

      about_func = drivemount_applet_setup_about;
      help_uri = "help:drivemount";
    }
  else if (g_strcmp0 (id, "geyes") == 0)
    {
      type_func = eyes_applet_get_type;
      name = _("Eyes");
      description = _("A set of eyeballs for your panel");
      icon_name = "gnome-eyes-applet";

      about_func = eyes_applet_setup_about;
      help_uri = "help:geyes";
    }
  else if (g_strcmp0 (id, "gweather") == 0)
    {
      type_func = gweather_applet_get_type;
      name = _("Weather Report");
      description = _("Monitor the current weather conditions, and forecasts");
      icon_name = "weather-storm";

      about_func = gweather_applet_setup_about;
      help_uri = "help:gweather";
    }
  else if (g_strcmp0 (id, "inhibit") == 0)
    {
      type_func = inhibit_applet_get_type;
      name = _("Inhibit Applet");
      description = _("Allows user to inhibit automatic power saving");
      icon_name = "gnome-inhibit-applet";

      about_func = inhibit_applet_setup_about;
    }
  else if (g_strcmp0 (id, "mini-commander") == 0)
    {
      type_func = mini_commander_applet_get_type;
      name = _("Command Line");
      description = _("Mini-Commander");
      icon_name = "gnome-mini-commander";

      about_func = mini_commander_applet_setup_about;
      help_uri = "help:command-line";
    }
  else if (g_strcmp0 (id, "multiload") == 0)
    {
      type_func = multiload_applet_get_type;
      name = _("System Monitor");
      description = _("A system load indicator");
      icon_name = "utilities-system-monitor";

      about_func = multiload_applet_setup_about;
      help_uri = "help:multiload";
    }
  else if (g_strcmp0 (id, "netspeed") == 0)
    {
      type_func = netspeed_applet_get_type;
      name = _("Network Monitor");
      description = _("Netspeed Applet");
      icon_name = "netspeed-applet";

      about_func = netspeed_applet_setup_about;
      help_uri = "help:netspeed_applet";
    }
  else if (g_strcmp0 (id, "sticky-notes") == 0)
    {
      type_func = sticky_notes_applet_get_type;
      name = _("Sticky Notes");
      description = _("Create, view, and manage sticky notes on the desktop");
      icon_name = "gnome-sticky-notes-applet";

      about_func = stickynotes_applet_setup_about;
      help_uri = "help:stickynotes_applet";
    }
  else if (g_strcmp0 (id, "timer") == 0)
    {
      type_func = timer_applet_get_type;
      name = _("Timer");
      description = _("Start a timer and receive a notification when it is finished");
      icon_name = "gnome-panel-clock";

      about_func = timer_applet_setup_about;
    }
#ifdef HAVE_TRACKER_SEARCH_BAR
  else if (g_strcmp0 (id, "tracker-search-bar") == 0)
    {
      type_func = tracker_applet_get_type;
      name = _("Tracker Search Bar");
      description = _("Find your data quickly using Tracker");
      icon_name = "system-search";

      about_func = tracker_applet_setup_about;
    }
#endif
  else if (g_strcmp0 (id, "trash") == 0)
    {
      type_func = trash_applet_get_type;
      name = _("Trash");
      description = _("Go to Trash");
      icon_name = "user-trash-full";

      about_func = trash_applet_setup_about;
      help_uri = "help:trashapplet";
    }
  else if (g_strcmp0 (id, "window-buttons") == 0)
    {
      type_func = wb_applet_get_type;
      name = _("Window Buttons");
      description = _("Window buttons for your GNOME Panel");
      icon_name = "windowbuttons-applet";

      about_func = wb_applet_setup_about;
    }
  else if (g_strcmp0 (id, "window-picker") == 0)
    {
      type_func = wp_applet_get_type;
      name = _("Window Picker");
      description = _("Shows a list of icons for the open windows.");
      icon_name = "preferences-system-windows";

      about_func = wp_applet_setup_about;
    }
  else if (g_strcmp0 (id, "window-title") == 0)
    {
      type_func = wt_applet_get_type;
      name = _("Window Title");
      description = _("Window title for your GNOME Panel");
      icon_name = "windowtitle-applet";

      about_func = wt_applet_setup_about;
    }
  else
    {
      g_assert_not_reached ();
      return NULL;
    }

  info = gp_applet_info_new (type_func, name, description, icon_name);

  if (about_func != NULL)
    gp_applet_info_set_about_dialog (info, about_func);

  if (help_uri != NULL)
    gp_applet_info_set_help_uri (info, help_uri);

  return info;
}

static const char *
ga_get_applet_id_from_iid (const char *iid)
{
  if (g_strcmp0 (iid, "AccessxStatusAppletFactory::AccessxStatusApplet") == 0)
    return "accessx-status";
  else if (g_strcmp0 (iid, "BattstatAppletFactory::BattstatApplet") == 0)
    return "battstat";
  else if (g_strcmp0 (iid, "BrightnessAppletFactory::BrightnessApplet") == 0)
    return "brightness";
  else if (g_strcmp0 (iid, "CharpickerAppletFactory::CharpickerApplet") == 0)
    return "charpick";
  else if (g_strcmp0 (iid, "CommandAppletFactory::CommandApplet") == 0)
    return "command";
#ifdef BUILD_CPUFREQ_APPLET
  else if (g_strcmp0 (iid, "CPUFreqAppletFactory::CPUFreqApplet") == 0)
    return "cpufreq";
#endif
  else if (g_strcmp0 (iid, "DriveMountAppletFactory::DriveMountApplet") == 0)
    return "drivemount";
  else if (g_strcmp0 (iid, "GeyesAppletFactory::GeyesApplet") == 0)
    return "geyes";
  else if (g_strcmp0 (iid, "GWeatherAppletFactory::GWeatherApplet") == 0)
    return "gweather";
  else if (g_strcmp0 (iid, "InhibitAppletFactory::InhibitApplet") == 0)
    return "inhibit";
  else if (g_strcmp0 (iid, "MiniCommanderAppletFactory::MiniCommanderApplet") == 0)
    return "mini-commander";
  else if (g_strcmp0 (iid, "MultiLoadAppletFactory::MultiLoadApplet") == 0)
    return "multiload";
  else if (g_strcmp0 (iid, "NetspeedAppletFactory::NetspeedApplet") == 0)
    return "netspeed";
  else if (g_strcmp0 (iid, "StickyNotesAppletFactory::StickyNotesApplet") == 0)
    return "sticky-notes";
  else if (g_strcmp0 (iid, "TimerAppletFactory::TimerApplet") == 0)
    return "timer";
#ifdef HAVE_TRACKER_SEARCH_BAR
  else if (g_strcmp0 (iid, "SearchBarFactory::SearchBar") == 0)
    return "tracker-search-bar";
#endif
  else if (g_strcmp0 (iid, "TrashAppletFactory::TrashApplet") == 0)
    return "trash";
  else if (g_strcmp0 (iid, "WindowButtonsAppletFactory::WindowButtonsApplet") == 0)
    return "window-buttons";
  else if (g_strcmp0 (iid, "WindowPickerFactory::WindowPicker") == 0 ||
           g_strcmp0 (iid, "org.gnome.gnome-applets.window-picker::window-picker") == 0)
    return "window-picker";
  else if (g_strcmp0 (iid, "WindowTitleAppletFactory::WindowTitleApplet") == 0)
    return "window-title";

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
                            "accessx-status",
                            "battstat",
                            "brightness",
                            "charpick",
                            "command",
#ifdef BUILD_CPUFREQ_APPLET
                            "cpufreq",
#endif
                            "drivemount",
                            "geyes",
                            "gweather",
                            "inhibit",
                            "mini-commander",
                            "multiload",
                            "netspeed",
                            "sticky-notes",
                            "timer",
#ifdef HAVE_TRACKER_SEARCH_BAR
                            "tracker-search-bar",
#endif
                            "trash",
                            "window-buttons",
                            "window-picker",
                            "window-title",
                            NULL);

  gp_module_set_get_applet_info (module, ga_get_applet_info);
  gp_module_set_compatibility (module, ga_get_applet_id_from_iid);
}
