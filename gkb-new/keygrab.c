/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* File: keygrab.c
 * Purpose: GNOME Keyboard switcher keyboard grabbing functions
 *
 * Copyright (C) 1998-2000 Free Software Foundation
 * Authors: Szabolcs Ban  <shooby@gnome.hu>
 *          Chema Celorio <chema@celorio.com>
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

#include <X11/keysym.h>
#include <gdk/gdkx.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include "gkb.h"

/* --- keygrab --- */

static gboolean
string_empty (const char *string)
{
  if (string == NULL || string[0] == '\0')
    return TRUE;
  else
    return FALSE;
}

gboolean
convert_string_to_keysym_state (const char *string,
				guint * keysym, guint * state)
{
  char *s, *p;

  g_return_val_if_fail (keysym != NULL, FALSE);
  g_return_val_if_fail (state != NULL, FALSE);

  *state = 0;
  *keysym = 0;

  if (string_empty (string) ||
      strcmp (string, "Disabled") == 0 || strcmp (string, _("Disabled")) == 0)
    return FALSE;

  s = g_strdup (string);

  gdk_error_trap_push ();

  p = strtok (s, "-");
  while (p != NULL)
    {
      if (strcmp (p, "Control") == 0)
	{
	  *state |= GDK_CONTROL_MASK;
	}
      else if (strcmp (p, "Lock") == 0)
	{
	  *state |= GDK_LOCK_MASK;
	}
      else if (strcmp (p, "Shift") == 0)
	{
	  *state |= GDK_SHIFT_MASK;
	}
      else if (strcmp (p, "Mod1") == 0)
	{
	  *state |= GDK_MOD1_MASK;
	}
      else if (strcmp (p, "Mod2") == 0)
	{
	  *state |= GDK_MOD2_MASK;
	}
      else if (strcmp (p, "Mod3") == 0)
	{
	  *state |= GDK_MOD3_MASK;
	}
      else if (strcmp (p, "Mod4") == 0)
	{
	  *state |= GDK_MOD4_MASK;
	}
      else if (strcmp (p, "Mod5") == 0)
	{
	  *state |= GDK_MOD5_MASK;
	}
      else
	{
	  *keysym = gdk_keyval_from_name (p);
	  if (*keysym == 0)
	    {
	      gdk_flush ();
	      gdk_error_trap_pop ();
	      g_free (s);
	      return FALSE;
	    }
	}
      p = strtok (NULL, "-");
    }

  gdk_flush ();
  gdk_error_trap_pop ();

  g_free (s);

  if (*keysym == 0)
    return FALSE;

  return TRUE;
}

char *
convert_keysym_state_to_string (guint keysym, guint state)
{
  GString *gs;
  char *sep = "";
  char *key;

  if (keysym == 0)
    return g_strdup (_("Disabled"));

  gdk_error_trap_push ();
  key = gdk_keyval_name (keysym);
  gdk_flush ();
  gdk_error_trap_pop ();
  if (!key)
    return NULL;

  gs = g_string_new (NULL);

  if (state & GDK_CONTROL_MASK)
    {
      /*g_string_append(gs, sep); */
      g_string_append (gs, "Control");
      sep = "-";
    }
  if (state & GDK_LOCK_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Lock");
      sep = "-";
    }
  if (state & GDK_SHIFT_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Shift");
      sep = "-";
    }
  if (state & GDK_MOD1_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Mod1");
      sep = "-";
    }
  if (state & GDK_MOD2_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Mod2");
      sep = "-";
    }
  if (state & GDK_MOD3_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Mod3");
      sep = "-";
    }
  if (state & GDK_MOD4_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Mod4");
      sep = "-";
    }
  if (state & GDK_MOD5_MASK)
    {
      g_string_append (gs, sep);
      g_string_append (gs, "Mod5");
      sep = "-";
    }

  g_string_append (gs, sep);
  g_string_append (gs, key);

  {
    char *ret = gs->str;
    g_string_free (gs, FALSE);
    return ret;
  }
}


static GtkWidget *grab_dialog;

static GdkFilterReturn
grab_key_filter (GdkXEvent * gdk_xevent, GdkEvent * event, gpointer data)
{
  XEvent *xevent = (XEvent *) gdk_xevent;
  GtkEntry *entry;
  char *key;

  if (xevent->type != KeyPress && xevent->type != KeyRelease)
    puts ("EXIT X");
  if (event->type != GDK_KEY_PRESS && event->type != GDK_KEY_RELEASE)
    puts ("EXIT GDK");

  if (xevent->type != KeyPress && xevent->type != KeyRelease)
    /*if (event->type != GDK_KEY_PRESS && event->type != GDK_KEY_RELEASE) */
    return GDK_FILTER_CONTINUE;

  entry = GTK_ENTRY (data);

  /* note: GDK has already translated the keycode to a keysym for us */
  g_message ("keycode: %d\tstate: %d", event->key.keyval, event->key.state);

  key = convert_keysym_state_to_string (event->key.keyval, event->key.state);
  gtk_entry_set_text (GTK_ENTRY (entry), key ? key : "");
  g_free (key);

  gdk_keyboard_ungrab (GDK_CURRENT_TIME);
  gtk_widget_destroy (grab_dialog);
  gdk_window_remove_filter (GDK_ROOT_PARENT (), grab_key_filter, data);

  return GDK_FILTER_REMOVE;
}

void
grab_button_pressed (GtkButton * button, gpointer data)
{
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *label;
  grab_dialog = gtk_window_new (GTK_WINDOW_POPUP);

  gdk_keyboard_grab (GDK_ROOT_PARENT (), TRUE, GDK_CURRENT_TIME);
  gdk_window_add_filter (GDK_ROOT_PARENT (), grab_key_filter, data);

  gtk_window_set_policy (GTK_WINDOW (grab_dialog), FALSE, FALSE, TRUE);
  gtk_window_set_position (GTK_WINDOW (grab_dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (grab_dialog), TRUE);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (grab_dialog), frame);

  box = gtk_hbox_new (0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (box), 20);
  gtk_container_add (GTK_CONTAINER (frame), box);

  label = gtk_label_new (_("Press a key..."));
  gtk_container_add (GTK_CONTAINER (box), label);

  gtk_widget_show_all (grab_dialog);
  return;
}
