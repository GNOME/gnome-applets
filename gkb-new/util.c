/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
 *  File: util.c
 *
 * utilities for gkb
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * Authors: Szabolcs Ban  <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 */

#include "gkb.h"

const gchar *
gkb_util_get_text_from_mode (GkbMode mode)
{
  if (mode == GKB_LABEL)
    return _("Label");

  if (mode == GKB_FLAG)
    return _("Flag");

  if (mode == GKB_FLAG_AND_LABEL)
    return _("Flag and Label");

  g_warning ("Invalid mode [%i]\n", mode);

  return _("Flag");
}

gint gkb_util_get_int_from_mode (GkbMode mode)
{
  if (mode == GKB_LABEL)
    return 1;

  if (mode == GKB_FLAG)
    return 0;

  if (mode == GKB_FLAG_AND_LABEL)
    return 2;

  g_warning ("Invalid mode [%i]\n", mode);

  return 0;
}

GkbMode gkb_util_get_mode_from_text (const gchar * text)
{
  g_return_val_if_fail (text != NULL, GKB_FLAG);

  if (strcmp (text, _("Label")) == 0)
    return GKB_LABEL;

  if (strcmp (text, _("Flag")) == 0)
    return GKB_FLAG;

  if (strcmp (text, _("Flag and Label")) == 0)
    return GKB_FLAG_AND_LABEL;

  g_warning ("Could not interpret size change [%s]\n", text);

  return GKB_FLAG;
}
