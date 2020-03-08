/*
 * Copyright (C) 2004 Carlos Garcia Campos
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

#include "cpufreq-selector-service.h"

static void
quit_cb (CPUFreqSelectorService *self,
         GMainLoop              *loop)
{
  if (g_main_loop_is_running (loop))
    g_main_loop_quit (loop);
}

int
main (void)
{
  GMainLoop *loop;
  CPUFreqSelectorService *service;

  loop = g_main_loop_new (NULL, FALSE);
  service = cpufreq_selector_service_new ();

  g_signal_connect (service, "quit", G_CALLBACK (quit_cb), loop);

  g_main_loop_run (loop);

  g_main_loop_unref (loop);

  return 0;
}
