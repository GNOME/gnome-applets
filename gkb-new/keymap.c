/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: prop.c
 * Purpose: GNOME Keyboard switcher property box
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs BAN <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
 *
 * Some of functions came from Helixcode's keyboard grabbing sections.
 * Other functions, ideas stolen from other applets, for example
 * from the fish applet, Wanda.
 *
 * Thanks for aid of George Lebl <jirka@5z.com> and solidarity
 * Balazs Nagy <js@lsc.hu>, Charles Levert <charles@comm.polymtl.ca>
 * and Emese Kovacs <emese@gnome.hu> for her docs and ideas.
 *
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
 */

#include "gkb.h"

#define free_and_null(x) g_free (x); x = NULL;

/**
 * gkb_util_free_keymap:
 * @keymap: 
 * 
 * Free a keymap
 **/
void
gkb_keymap_free (GkbKeymap * keymap)
{
  free_and_null (keymap->name);
  free_and_null (keymap->label);
  free_and_null (keymap->lang);
  free_and_null (keymap->country);
  free_and_null (keymap->codepage);
  free_and_null (keymap->type);
  free_and_null (keymap->arch);
  free_and_null (keymap->command);
  free_and_null (keymap->flag);
  if (keymap->pixbuf)
    g_object_unref (keymap->pixbuf);
  keymap->pixbuf = NULL;
  g_free (keymap);
}

void
gkb_keymap_free_list (GList * list_in)
{
  GList *ptr;
  for (ptr = list_in; ptr != NULL; ptr = ptr->next)
    gkb_keymap_free (ptr->data);
  g_list_free (list_in);
}

GkbKeymap *
gkb_keymap_copy (GkbKeymap *keymap)
{
  GkbKeymap *new_keymap = g_new0 (GkbKeymap, 1);
  new_keymap->name	= g_strdup (keymap->name);
  new_keymap->flag	= g_strdup (keymap->flag);
  new_keymap->country	= g_strdup (keymap->country);
  new_keymap->command	= g_strdup (keymap->command);
  new_keymap->lang	= g_strdup (keymap->lang);
  new_keymap->label	= g_strdup (keymap->label);
  /* FIXME - why was new_keymap->pix NULL'd before */
  new_keymap->pixbuf = NULL;

  return new_keymap;
}

GList *
gkb_keymap_copy_list (GList *list_in)
{
    GList *accum = NULL;
    GList *ptr;
    for (ptr = list_in; ptr != NULL; ptr = ptr->next)
	accum = g_list_prepend (accum, gkb_keymap_copy (ptr->data));
    return g_list_reverse (accum);
}
