/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
 *  File: system.c
 *
 * system call for setting the keyboard
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



/**
 * gkb_system_set_keymap_idle:
 * @keymap: 
 * 
 * Set the keymap, this function should be called in an idle loop
 * 
 * Return Value: 
 **/
static gboolean
gkb_system_set_keymap_idle (GkbKeymap * keymap)
{
  if (system (gkb->keymap->command))
    gnome_error_dialog (_
			("The keymap switching command returned with error!"));

  return FALSE;
}


/**
 * gkb_system_set_keymap:
 * @gkb: 
 * 
 * Executes the system comand to set the keymap selected
 * used the keymap info in gkb->dact->kcode.
 * TODO: Implement advanced settings so that the user can specify
 *       the GKB_COMMAND to use, hardcoded for now.
 * TODO: The system call blocks the execution of the applet, trow it
 *       inside a gtk_idle loop to make it "seem" faster. This means
 *       that we are going to display the flag, but the command is not
 *       going to get executed imidiatelly (which is not a problem at all)
 **/
void
gkb_system_set_keymap (GKB * gkb)
{
  g_return_if_fail (gkb->keymap != NULL);

  gtk_idle_add ((GtkFunction) gkb_system_set_keymap_idle, gkb->keymap);
}
