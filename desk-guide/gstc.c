/* gstc.c - G(something) stacking cache
 * Copyright (C) 1999 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#define G_LOG_DOMAIN "GStC"
#include	"gstc.h"
#include        <gdk/gdkx.h>
#include	<string.h>


/* --- prototypes --- */
static GdkFilterReturn	gstc_parent_event_monitor (GdkXEvent  *gdk_xevent,
						   GdkEvent   *event,
						   gpointer    sparent_pointer);
static void		gstc_parent_add_child	  (GstcParent *sparent,
						   Window      xchild);
static void		gstc_parent_remove_child  (GstcParent *sparent,
						   Window      xchild);
static void		gstc_parent_restack_child (GstcParent *sparent,
						   Window      xchild,
						   Window      xbelow);


/* --- variables --- */
static GSList *gstc_parents = NULL;


/* --- functions --- */
GstcParent*
gstc_parent_from_window (GdkWindow *window)
{
  GSList *node;

  g_return_val_if_fail (window != NULL, NULL);

  for (node = gstc_parents; node; node = node->next)
    {
      GstcParent *sparent = node->data;

      if (sparent->window == window)
	return sparent;
    }

  return NULL;
}

GstcParent*
gstc_parent_add_watch (GdkWindow *window)
{
  GstcParent *sparent;

  g_return_val_if_fail (window != NULL, NULL);

  sparent = gstc_parent_from_window (window);
  if (!sparent)
    {
      XWindowAttributes attribs = { 0, };
      Window xroot = None, xparent = None, *children = NULL;
      int n_children = 0;
      guint i;

      gdk_error_trap_push ();
      XGetWindowAttributes (GDK_WINDOW_XDISPLAY (window),
			    GDK_WINDOW_XWINDOW (window),
			    &attribs);
      XSelectInput (GDK_WINDOW_XDISPLAY (window),
		    GDK_WINDOW_XWINDOW (window),
		    attribs.your_event_mask |
		    SubstructureNotifyMask);

      XQueryTree (GDK_WINDOW_XDISPLAY (window),
		  GDK_WINDOW_XWINDOW (window),
		  &xroot,
		  &xparent,
		  &children,
		  &n_children);

      if (gdk_error_trap_pop ())
	{
	  if (children)
	    XFree (children);

	  return NULL;
	}

      sparent = g_new (GstcParent, 1);
      gstc_parents = g_slist_prepend (gstc_parents, sparent);
      sparent->window = window;
      gdk_window_ref (sparent->window);
      sparent->ref_count = 0;
      gdk_window_add_filter (window, gstc_parent_event_monitor, sparent);
      sparent->n_children = n_children;
      sparent->children = g_renew (gulong, NULL, n_children);
      for (i = 0; i < sparent->n_children; i++)
	sparent->children[i] = children[i];
      if (children)
	XFree (children);
    }
  sparent->ref_count++;

  return sparent;
}

void
gstc_parent_delete_watch (GdkWindow *window)
{
  GstcParent *sparent;

  g_return_if_fail (window != NULL);
return;
  sparent = gstc_parent_from_window (window);
  if (sparent)
    sparent->ref_count--;
  if (sparent && !sparent->ref_count)
    {
      gdk_window_remove_filter (sparent->window, gstc_parent_event_monitor, sparent);
      g_free (sparent->children);
      gdk_window_unref (sparent->window);
      gstc_parents = g_slist_remove (gstc_parents, sparent);
      sparent->ref_count = 42;
      g_free (sparent);
    }
}

static void
gstc_parent_add_child (GstcParent *sparent,
		       Window      xchild)
{
  gint i, ci = -1;

  /* sanity checks, find child */
  for (i = 0; i < sparent->n_children; i++)
    {
      if (sparent->children[i] == xchild)
	{
	  ci = i;
	  break;
	}
    }
  if (ci >= 0)
    {
      g_warning (G_GNUC_PRETTY_FUNCTION "(): can't add known window %ld",
		 xchild);
      return;
    }

  /* provide space for new child */
  sparent->n_children++;
  sparent->children = g_renew (gulong,
			       sparent->children,
			       sparent->n_children);

  /* append child */
  sparent->children[sparent->n_children - 1] = xchild;
}

static void
gstc_parent_remove_child (GstcParent *sparent,
			  Window      xchild)
{
  gint i, ci = -1;

  /* find child */
  for (i = 0; i < sparent->n_children; i++)
    {
      if (sparent->children[i] == xchild)
	{
	  ci = i;
	  break;
	}
    }

  /* sanity checks */
  if (ci < 0)
    {
      g_warning (G_GNUC_PRETTY_FUNCTION "(): can't remove unknown window %ld",
		 xchild);
      return;
    }

  /* remove child from children array */
  sparent->n_children--;
  g_memmove (sparent->children + ci,
	     sparent->children + ci + 1,
	     sizeof (gulong) * (sparent->n_children - ci));
  sparent->children = g_renew (gulong,
			       sparent->children,
			       sparent->n_children);
}

static void
gstc_parent_restack_child (GstcParent *sparent,
			   Window      xchild,
			   Window      xbelow)
{
  gint i, ci = -1, bi = -1;

  /* find both children */
  for (i = 0; i < sparent->n_children; i++)
    {
      if (sparent->children[i] == xchild)
	{
	  ci = i;
	  if (bi >= 0 || !xbelow)
	    break;
	}
      if (sparent->children[i] == xbelow)
	{
	  bi = i;
	  if (ci >= 0)
	    break;
	}
    }

  /* sanity checks */
  if (ci < 0)
    {
      g_warning (G_GNUC_PRETTY_FUNCTION "(): can't restack unknown window %ld",
		 xchild);
      return;
    }
  if (xbelow && bi < 0)
    {
      g_warning (G_GNUC_PRETTY_FUNCTION "(): can't restack window %ld on top of unknown window %ld",
		 xchild,
		 xbelow);
      return;
    }

  /* bail out if stacking order is correct already */
  if (bi + 1 == ci)
    return;

  /* remove child from children array */
  g_memmove (sparent->children + ci,
	     sparent->children + ci + 1,
	     sizeof (gulong) * (sparent->n_children - 1 - ci));

  /* assign new child index */
  ci = bi < ci ? bi + 1 : bi;

  /* provide space for child */
  g_memmove (sparent->children + ci + 1,
	     sparent->children + ci,
	     sizeof (gulong) * (sparent->n_children - 1 - ci));

  /* insert child */
  sparent->children[ci] = xchild;
}

static GdkFilterReturn
gstc_parent_event_monitor (GdkXEvent *gdk_xevent,
			   GdkEvent  *event,
			   gpointer   sparent_pointer)
{
  XEvent *xevent = gdk_xevent;
  GstcParent *sparent = sparent_pointer;
  Window xparent = GDK_WINDOW_XWINDOW (sparent->window);
  GdkFilterReturn filter_return = GDK_FILTER_CONTINUE;

  g_return_val_if_fail (g_slist_find (gstc_parents, sparent), GDK_FILTER_CONTINUE);

  switch (xevent->type)
    {
    case CreateNotify:
      if (xevent->xcreatewindow.parent != xparent)
	g_warning (G_GNUC_PRETTY_FUNCTION "(): now what is THIS? "
		   "i receive a SubstructureNotify XCreateWindowEvent "
		   "for a *foreign* child (%ld)??? X is on drugs!",
		   xevent->xcreatewindow.window);
      else
	gstc_parent_add_child (sparent, xevent->xcreatewindow.window);
      break;
    case CirculateNotify:
      if (xevent->xcirculate.event == xparent)
	gstc_parent_restack_child (sparent,
				   xevent->xcirculate.window,
				   (xevent->xcirculate.place == PlaceOnTop &&
				    sparent->n_children
				    ? sparent->children[sparent->n_children - 1]
				    : None));
      break;
    case ConfigureNotify:
      if (xevent->xconfigure.event == xparent)
	gstc_parent_restack_child (sparent,
				   xevent->xconfigure.window,
				   xevent->xconfigure.above);
      break;
    case ReparentNotify:
      if (xevent->xreparent.event == xparent)
	{
	  if (xevent->xreparent.parent == xparent)
	    gstc_parent_add_child (sparent, xevent->xreparent.window);
	  else
	    gstc_parent_remove_child (sparent, xevent->xreparent.window);
	}
      break;
    case DestroyNotify:
      if (xevent->xdestroywindow.event == xparent)
	gstc_parent_remove_child (sparent, xevent->xdestroywindow.window);
      break;
    default:
      break;
    }

  return filter_return;
}
