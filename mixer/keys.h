/* GNOME Volume Applet
 * Copyright (C) 2004 Ronald Bultje <rbultje@ronald.bitfreak.net>
 *
 * keys.h: GConf key macros
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GVA_KEYS_H__
#define __GVA_KEYS_H__

G_BEGIN_DECLS

#define GNOME_VOLUME_APPLET_KEY(x) x
#define GNOME_VOLUME_APPLET_KEY_ACTIVE_ELEMENT \
  GNOME_VOLUME_APPLET_KEY ("active-element")
#define GNOME_VOLUME_APPLET_KEY_ACTIVE_TRACK \
  GNOME_VOLUME_APPLET_KEY ("active-track")

G_END_DECLS

#endif /* __GVA_KEYS_H__ */
