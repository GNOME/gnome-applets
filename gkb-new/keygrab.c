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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <panel-applet-gconf.h>

#include <X11/keysym.h>
#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include "gkb.h"

int NumLockMask, CapsLockMask, ScrollLockMask;

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
      else if (strcmp (p, "Alt") == 0)
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
      g_string_append (gs, "Alt");
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
  XKeyEvent *xkevent = (XKeyEvent *) gdk_xevent;
  GtkEntry *entry;
  KeySym keysym;
  guint  state;
  char buf[10];
  char *key;
  guint newkey;
  GdkWindow *root_window;
  GkbPropertyBoxInfo *pbi = (GkbPropertyBoxInfo *)data;
  GKB *gkb = pbi->gkb;
  static gboolean key_was_pressed = FALSE;
  
  entry = GTK_ENTRY (pbi->hotkey_entry);

  root_window = gdk_get_default_root_window ();

  if (xevent->type != KeyPress && xevent->type != KeyRelease)
    {
      gdk_display_pointer_ungrab (gdk_drawable_get_display (root_window),
                                  GDK_CURRENT_TIME);
      gdk_keyboard_ungrab (GDK_CURRENT_TIME);
      gtk_widget_destroy (grab_dialog);
      gdk_window_remove_filter (root_window, grab_key_filter, data);
      key_was_pressed = FALSE;
      return GDK_FILTER_REMOVE;
    }

  state = xevent->xkey.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK);

  XLookupString (&xevent->xkey, buf, 0, &keysym, NULL);

  if (xevent->type == KeyRelease) {

    if (!key_was_pressed)
      return GDK_FILTER_CONTINUE;

   /* grabbing went thro fine, update the old_* variables */
   g_free (gkb->old_key);
   gkb->old_key = g_strdup (gkb->key);
   gkb->old_keysym = gkb->keysym;
   gkb->old_state = gkb->state;
   gdk_display_pointer_ungrab (gdk_drawable_get_display (root_window),
                             GDK_CURRENT_TIME);
   gdk_keyboard_ungrab (GDK_CURRENT_TIME);
   gtk_widget_destroy (grab_dialog);
   gdk_window_remove_filter (root_window, grab_key_filter, data);
   key_was_pressed = FALSE;
   return GDK_FILTER_REMOVE;
  }
 
   key_was_pressed = TRUE;
 
  /* Esc cancels . We have got the keysym by XLookupString() */
  if (keysym == GDK_Escape) 
    return GDK_FILTER_REMOVE;
  
  key = convert_keysym_state_to_string (XKeycodeToKeysym(GDK_DISPLAY(),
                  xkevent->keycode,0), xkevent->state);
  gtk_entry_set_text (GTK_ENTRY (entry), key ? key : "");
  
  newkey = XKeysymToKeycode (GDK_DISPLAY (), gkb->keysym);

  gkb->key = g_strdup (key);
  g_free (key);

  gdk_error_trap_push ();
  gkb_xungrab (gkb->keycode, gkb->state, root_window);
  gdk_flush ();
  gdk_error_trap_pop ();
 
  convert_string_to_keysym_state (gkb->key, &gkb->keysym, &gkb->state);

  newkey = XKeysymToKeycode (GDK_DISPLAY (), gkb->keysym);

  gdk_error_trap_push ();
  gkb_xgrab ( newkey, gkb->state, root_window);
 
  /* In case we are unable to grab revert to the the previous hot key*/
  gdk_flush ();
  if (gdk_error_trap_pop () != 0) {

    g_free (gkb->key);
    gkb->key = g_strdup (gkb->old_key);
    gtk_entry_set_text (GTK_ENTRY (entry), gkb->key ? gkb->key : "");
    gkb->keysym = gkb->old_keysym;
    gkb->state = gkb->old_state;

    /* Grab the key which was used just before popping the window */
    /* Hopefully we are still able to grab the old key back */
    gdk_error_trap_push ();
    gkb_xgrab ( gkb->keycode, gkb->state, root_window);
    gdk_flush ();
    gdk_error_trap_pop ();

    /* No point in continuing further. Close the popup window */
    gdk_display_pointer_ungrab (gdk_drawable_get_display (root_window),
                                GDK_CURRENT_TIME);
    gdk_keyboard_ungrab (GDK_CURRENT_TIME);
    gtk_widget_destroy (grab_dialog);
    gdk_window_remove_filter (root_window, grab_key_filter, data);
    return GDK_FILTER_REMOVE;
  }

  gkb->keycode = newkey; 
  gconf_applet_set_string (PANEL_APPLET (gkb->applet), "key", gkb->key, NULL);
  
  return GDK_FILTER_REMOVE;
}

void
grab_button_pressed (GtkButton *button,
		     gpointer   data)
{
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *label;
  GdkWindow *root_window;
  GkbPropertyBoxInfo *pbi = (GkbPropertyBoxInfo *)data;

  g_return_if_fail (pbi != NULL);

  root_window = gdk_screen_get_root_window (
			gtk_widget_get_screen (GTK_WIDGET (button)));

  if ((gdk_pointer_grab (root_window, FALSE,
                        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
                        NULL, NULL, GDK_CURRENT_TIME) == 0))
    {
      if (gdk_keyboard_grab (root_window, FALSE, GDK_CURRENT_TIME) != 0)
        {
          gdk_display_pointer_ungrab (gdk_drawable_get_display (root_window),
                                      GDK_CURRENT_TIME);
          return;
        }
    }
    else
     return;

  gdk_window_add_filter (root_window, grab_key_filter, data);

  grab_dialog = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_screen (GTK_WINDOW (grab_dialog),
			 gtk_widget_get_screen (GTK_WIDGET (button)));
  g_object_set (G_OBJECT (grab_dialog),
    		"allow_grow", FALSE,
    	        "allow_shrink", FALSE,
                "resizable", FALSE,
                NULL);

  gtk_window_set_transient_for (GTK_WINDOW (grab_dialog), GTK_WINDOW (pbi->box));
  gtk_window_set_position (GTK_WINDOW (grab_dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (grab_dialog), TRUE);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (grab_dialog), frame);

  box = gtk_hbox_new (0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (box), 20);
  gtk_container_add (GTK_CONTAINER (frame), box);

  label = gtk_label_new (_("Press a key or press Esc to cancel"));
  gtk_container_add (GTK_CONTAINER (box), label);

  gtk_widget_show_all (grab_dialog);
  return;
}

static void
init_xmaps ()
{
  XModifierKeymap *xmk=NULL;
  KeyCode *map;
  int m, k;

  xmk=XGetModifierMapping(GDK_DISPLAY());
  if(xmk)
  {
    map=xmk->modifiermap;
    for(m=0;m<8;m++)
	for(k=0;k<xmk->max_keypermod; k++, map++)
	{
	if(*map==XKeysymToKeycode(GDK_DISPLAY(), XK_Num_Lock))
		NumLockMask=(1<<m);
	if(*map==XKeysymToKeycode(GDK_DISPLAY(), XK_Caps_Lock))
	  CapsLockMask=(1<<m);
        if(*map==XKeysymToKeycode(GDK_DISPLAY(), XK_Scroll_Lock))
          ScrollLockMask=(1<<m);
      }
    XFreeModifiermap(xmk);
  }
}

void
gkb_xgrab (int        keycode,
	   int        modifiers,
	   GdkWindow *root_window)
{
  init_xmaps ();

#define GRAB_KEY(mods) XGrabKey (GDK_DISPLAY (), keycode, modifiers|mods,	\
				 GDK_WINDOW_XWINDOW (root_window), True,	\
				 GrabModeAsync, GrabModeAsync)

  GRAB_KEY (0);
  GRAB_KEY (NumLockMask);
  GRAB_KEY (CapsLockMask);
  GRAB_KEY (ScrollLockMask);
  GRAB_KEY (NumLockMask|CapsLockMask);
  GRAB_KEY (NumLockMask|ScrollLockMask);
  GRAB_KEY (CapsLockMask|ScrollLockMask);
  GRAB_KEY (NumLockMask|CapsLockMask|ScrollLockMask);

#undef GRAB_KEY
}

void
gkb_xungrab (int        keycode,
	     int        modifiers,
	     GdkWindow *root_window)
{
  init_xmaps ();

#define UNGRAB_KEY(mods) XUngrabKey (GDK_DISPLAY (), keycode, modifiers|mods,	\
				     GDK_WINDOW_XWINDOW (root_window))

  UNGRAB_KEY (0);
  UNGRAB_KEY (NumLockMask);
  UNGRAB_KEY (CapsLockMask);
  UNGRAB_KEY (ScrollLockMask);
  UNGRAB_KEY (NumLockMask|CapsLockMask);
  UNGRAB_KEY (NumLockMask|ScrollLockMask);
  UNGRAB_KEY (CapsLockMask|ScrollLockMask);
  UNGRAB_KEY (NumLockMask|CapsLockMask|ScrollLockMask);

#undef UNGRAB_KEY
}
