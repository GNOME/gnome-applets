/*
 * Copyright (C) 2009 Andrej Belcijan
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
 *
 * Authors:
 *     Andrej Belcijan <{andrejx} at {gmail.com}>
 */

#ifndef WINDOW_TITLE_PREFERENCE_H
#define WINDOW_TITLE_PREFERENCE_H

#include "window-title-private.h"

WTPreferences *wt_applet_load_preferences (WTApplet *wtapplet);

void wt_applet_properties_cb (GSimpleAction *action,
                              GVariant      *parameter,
                              gpointer       user_data);

#endif
