/*
 * Copyright (C) 2020 Alberts Muktupāvels
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

#include "timer/timer-applet.h"

static GpAppletInfo *
ga_get_applet_info (const char *id)
{
  GpGetAppletTypeFunc type_func;
  const char *name;
  const char *description;
  const char *icon_name;
  GpAppletInfo *info;

  if (g_strcmp0 (id, "timer") == 0)
    {
      type_func = timer_applet_get_type;
      name = _("Timer");
      description = _("Start a timer and receive a notification when it is finished");
      icon_name = "gnome-panel-clock";
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
  if (g_strcmp0 (iid, "TimerAppletFactory::TimerApplet") == 0)
    return "timer";

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
                            "timer",
                            NULL);

  gp_module_set_get_applet_info (module, ga_get_applet_info);
  gp_module_set_compatibility (module, ga_get_applet_id_from_iid);
}
