/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
 *  File: switcher.c
 *
 * keyboard layout switching functions
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
 *
 */

#include <config.h>

#include <X11/keysym.h>
#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <sys/stat.h>

#include <gtk/gtk.h>
#include <libbonobo.h>
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#include <panel-applet-gconf.h>
#include <egg-screen-help.h>
#include "gkb.h"
 
int NumLockMask, CapsLockMask, ScrollLockMask;

gint
gkb_button_press_event_cb (GtkWidget * widget, GdkEventButton * event, GKB *gkb)
{
   if (event->button != 1)	
    return FALSE;


  if (gkb->cur + 1 < gkb->n)
    gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);
  else
    {
      gkb->cur = 0;
      gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
    }

  gkb_update (gkb, TRUE);
  return TRUE;
}

static GdkFilterReturn
global_key_filter (GKB *gkb, GdkXEvent * gdk_xevent, GdkEvent * event)
{
  if (event->key.keyval == gkb->keysym && event->key.state == gkb->state)
    {
      if (gkb->cur + 1 < gkb->n)
	gkb->keymap = g_list_nth_data (gkb->maps, ++gkb->cur);
      else
	{
	  gkb->cur = 0;
	  gkb->keymap = g_list_nth_data (gkb->maps, gkb->cur);
	}

      gkb_update (gkb, TRUE);

      return GDK_FILTER_CONTINUE;
    }
  return GDK_FILTER_CONTINUE;
}

GdkFilterReturn
event_filter (GdkXEvent * gdk_xevent, GdkEvent * event, gpointer data)
{
  XEvent *xevent;
  GKB *gkb = (GKB *)data;

  xevent = (XEvent *) gdk_xevent;

  if (xevent->type == KeyRelease)
    {
      return global_key_filter (gkb, gdk_xevent, event);
    }
  return GDK_FILTER_CONTINUE;
}

static gboolean
check_client (Display * display, Window xwindow, Atom state_atom)
{
  gboolean valid_client = TRUE;

  if (valid_client)
    {
      Atom dummy1;
      int format = 0;
      unsigned long dummy2, nitems = 0, *prop = NULL;

      XGetWindowProperty (display, xwindow, state_atom, 0, 1024, False,
			  state_atom, &dummy1, &format, &nitems, &dummy2,
			  (unsigned char **) &prop);
      if (prop)
	{
	  valid_client = format == 32 && nitems > 0 && (prop[0] == NormalState
							|| prop[0] ==
							IconicState);
	  XFree (prop);
	}
    }

  if (valid_client)
    {
      XWindowAttributes attributes = { 0, };

      XGetWindowAttributes (display, xwindow, &attributes);
      valid_client = (attributes.class == InputOutput &&
		      attributes.map_state == IsViewable);
    }

  if (valid_client)
    {
      XWMHints *hints = XGetWMHints (display, xwindow);

      valid_client &= hints && (hints->flags & InputHint) && hints->input;
      if (hints)
	XFree (hints);
    }

  return valid_client;
}

static Window
find_input_client (Display * display, Window xwindow, Atom state_atom)
{
  Window dummy1, dummy2, *children = NULL;
  unsigned int n_children = 0;
  guint i;

  if (check_client (display, xwindow, state_atom))
    return xwindow;

  if (!XQueryTree (display, xwindow, &dummy1, &dummy2, &children, &n_children)
      || !children)
    return None;

  for (i = 0; i < n_children; i++)
    {
      xwindow = find_input_client (display, children[i], state_atom);
      if (xwindow)
	break;
    }
  XFree (children);

  return xwindow;
}
