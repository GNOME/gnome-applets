/* fvwm-pager
 *
 * Copyright (C) 1998 Michael Lausch
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <signal.h>
#include <fvwm/module.h>
#include <fvwm/fvwm.h>
#include <libs/fvwmlib.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#include "properties.h"
#include "gtkpager.h"

#include "applet-lib.h"
#include "applet-widget.h"
#include "fvwm-pager.h"


void        DeadPipe             (int);
static void ParseOptions         (GtkFvwmPager* pager);
static void switch_to_desktop    (GtkFvwmPager* pager, int desktop_offset);
#if 0
static void move_window          (GtkFvwmPager* pager, unsigned long xid, int desktop, int x, int y);
#endif

PagerProps pager_props;

gint   pager_width  = 170;
gint   pager_height = 70;


static Atom _XA_WIN_WORKSPACE;
static Atom _XA_WIN_WORKSPACE_NAMES;
static Atom _XA_WIN_WORKSPACE_COUNT;
static Atom _XA_WIN_LAYER;

static int        desk1;
static int        desk2;
static int        fd[2];
static char*      pgm_name;

unsigned long xprop_long_data[20];
unsigned char xprop_char_data[128];


void process_message    (GtkFvwmPager* pager, unsigned long, unsigned long*);
void destroy            (GtkWidget* w, gpointer data);
void about_cb           (AppletWidget* widget, gpointer data);
void properties_dialog  (AppletWidget *widget, gpointer data);
void configure_window   (GtkFvwmPager* pager, unsigned long* body);
void configure_icon     (GtkFvwmPager* pager, unsigned long* body);
void destroy_window     (GtkFvwmPager* pager, unsigned long* body);
void deiconify_window   (GtkFvwmPager* pager, unsigned long* body);
void add_window         (GtkFvwmPager* pager, unsigned long* body);

void destroy(GtkWidget* w, gpointer data)
{
  gtk_main_quit();
}


static void
fvwm_command_received(gpointer data,
		      gint     source,
		      GdkInputCondition cond)
{
  int count;
  unsigned long header[HEADER_SIZE];
  unsigned long* body;
  GtkFvwmPager*  pager = GTK_FVWMPAGER(data);

  if ((count = ReadFvwmPacket(source, header, &body)) > 0)
    {
      process_message(pager, header[1], body);
      free(body);
    }
}

void
about_cb(AppletWidget* widget, gpointer data)
{
  GtkWidget* about;
  gchar* authors[] ={"M. Lausch", NULL};

  about = gnome_about_new( _("Fvwm Pager Applet"),
			   "0.1",
			   _("Copyright (C) 1998 M. Lausch"),
			   (const gchar**)authors,
			   "Pager for Fvwm2 window manager",
			   0);
  gtk_widget_show(about);
}

static gint
save_session (GtkWidget* widget, char* privcfgpath,
	      char* globcfgpath, gpointer data)
{
  save_fvwmpager_properties ("fvwmpager", &pager_props);
  return FALSE;
}


int
main(int argc, char* argv[])
{
  GtkWidget* window;
  char bfr[128];
  int  idx;
  int  ndesks;
  char* tmp;
  char* dummy_argv[2] = {NULL};
  static GtkWidget* pager = 0;
  GList* desktops;
  


  dummy_argv[0] = "fvwmpager";
  
  panel_corba_register_arguments();
  
  applet_widget_init_defaults("#fvwmpager", NULL, 1, dummy_argv, 0, NULL, argv[0]);

  window = applet_widget_new();

  load_fvwmpager_properties("fvwmpager", &pager_props);
  
  gtk_widget_realize(window);
  
  _XA_WIN_WORKSPACE       = XInternAtom(GDK_DISPLAY(), "WIN_WORKSPACE", False);
  _XA_WIN_WORKSPACE_NAMES = XInternAtom(GDK_DISPLAY(), "_WIN_WORKSPACE_NAMES", False);
  _XA_WIN_WORKSPACE_COUNT = XInternAtom(GDK_DISPLAY(), "WIN_WORKSPACE_COUNT", False);
  _XA_WIN_LAYER           = XInternAtom(GDK_DISPLAY(), "WIN_LAYER", False);

  
  tmp = strrchr(argv[0], '/');
  if (tmp)
    tmp++;
  else
    tmp = argv[0];
  pgm_name = strdup(tmp);
  
  signal(SIGPIPE, DeadPipe);

  fd[0] = atoi(argv[1]);
  fd[1] = atoi(argv[2]);

  desk1 = atoi(argv[6]);
  desk2 = atoi(argv[7]);

  if (desk1 > desk2)
    {
      int itemp;
      itemp = desk1;
      desk1 = desk2;
      desk2 = itemp;
    }
  ndesks = desk2 - desk1 + 1;

  xprop_long_data[0] = ndesks;
  
  XChangeProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), _XA_WIN_WORKSPACE_COUNT, _XA_WIN_WORKSPACE_COUNT,
		  32, PropModeReplace, (unsigned char*) xprop_long_data, 1);

  gtk_widget_set_usize(GTK_WIDGET(window), pager_props.width, pager_props.height);
  
  pager = gtk_fvwmpager_new(fd, pager_props.width, pager_props.height);
  
  gtk_signal_connect(GTK_OBJECT(pager), "switch_desktop",
		     GTK_SIGNAL_FUNC(switch_to_desktop), NULL);

  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		     GTK_SIGNAL_FUNC(destroy), NULL);
  
  gtk_signal_connect(GTK_OBJECT(window), "save_session",
		     GTK_SIGNAL_FUNC(save_session),
		     NULL);

  applet_widget_add(APPLET_WIDGET(window), pager);

  applet_widget_register_stock_callback(APPLET_WIDGET(window),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About"),
					about_cb,
					NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(window),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties"),
					pager_properties_dialog,
					pager);
  desktops = 0;
  for (idx = 0; idx < ndesks; idx++)
    {
      Desktop* new_desk;
      
      sprintf(bfr, _("Desk %d"), idx);
      new_desk = g_new0(Desktop, 1);
      new_desk->title = g_strdup(bfr);
      desktops = g_list_append(desktops, new_desk);
    }

  SetMessageMask(fd,
	     M_ADD_WINDOW       |
	     M_CONFIGURE_WINDOW |
	     M_DESTROY_WINDOW   |
	     M_FOCUS_CHANGE     |
	     M_NEW_PAGE         |
	     M_NEW_DESK         |
	     M_RAISE_WINDOW     |
	     M_LOWER_WINDOW     |
	     M_ICONIFY          |
	     M_ICON_LOCATION    |
	     M_DEICONIFY        |
	     M_ICON_NAME        |
	     M_CONFIG_INFO      |
	     M_END_CONFIG_INFO  |
	     M_MINI_ICON        |
	     M_END_WINDOWLIST);

  ParseOptions(GTK_FVWMPAGER(pager));
  {
    GList* elem;
    XTextProperty tp;
    char*  desk_names[ndesks];
    int i;

    elem = desktops;
    
    for (i = 0; i < ndesks; i++)
      {
	Desktop* desk;
	desk = elem->data;
	
	desk_names[i] = desk->title;
	elem = g_list_next(elem);
      }
    if (!XStringListToTextProperty(desk_names, ndesks, &tp))
      {
	g_log("fvwm-pager", G_LOG_LEVEL_ERROR, "Cannot get textproperty for workspace names\n");
      }
    else
      {
	XSetTextProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), &tp, _XA_WIN_WORKSPACE_NAMES);
      }
  }
    
  gtk_fvwmpager_display_desks(GTK_FVWMPAGER(pager), desktops);
  gdk_input_add(fd[1],
		GDK_INPUT_READ,
		fvwm_command_received,
		pager);

  gtk_widget_show(window);
  gtk_widget_show(pager);

  SendInfo(fd, "Send_WindowList", 0);
  applet_widget_gtk_main();
  save_fvwmpager_properties ("fvwmpager", &pager_props);
  return 0;
}

void
switch_to_desktop(GtkFvwmPager* pager, int offset)
{
  char    command[256];
  XEvent  xev;
  
  sprintf(command, "Desk 0 %d\n", offset + desk1);
  SendInfo(fd,command, 0);
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = GDK_ROOT_WINDOW();
  xev.xclient.message_type = _XA_WIN_WORKSPACE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = offset;
  xev.xclient.data.l[1] = gdk_time_get();
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
	     SubstructureNotifyMask, &xev);
  
}


void
configure_window(GtkFvwmPager* pager, unsigned long* body)
{
  PagerWindow* win;
  gint         xid = body[0];
  PagerWindow  old;
  gint         new_window = 0;
  
  win = g_hash_table_lookup(pager->windows, (gconstpointer)xid);
  if (!win)
    {
      win = g_new0(PagerWindow, 1);
      win->xid    = xid;
      win->ix     = -1000;
      win->iy     = -1000;
      win->iw     = 0;
      win->ih     = 0;
      win->flags  = 0;
      g_hash_table_insert(pager->windows, (gpointer)xid, win);
      new_window = 1;
    }
  old           = *win;
  win->x        = body[3];
  win->y        = body[4];
  win->w        = body[5];
  win->h        = body[6];
  win->th       = body[9];
  win->bw       = body[10];
  win->desk     = g_list_nth(pager->desktops, body[7])->data;
  win->flags    = 0;
  win->ixid     = body[20];
  if (body[8] & ICONIFIED)
    {
      win->flags |= GTKPAGER_WINDOW_ICONIFIED;
    }
  gtk_fvwmpager_display_window(GTK_FVWMPAGER(pager), new_window ? 0 : &old, xid);
}

void configure_icon(GtkFvwmPager* pager, unsigned long* body)
{
  PagerWindow* win;
  PagerWindow  old;
  
  gint         xid = body[0];
  
  win = g_hash_table_lookup(pager->windows, (gconstpointer)xid);
  if (!win)
    {
      return;
    }
  old = *win;
  win->flags  |= GTKPAGER_WINDOW_ICONIFIED;
  win->ix      = body[3];
  win->iy      = body[4];
  win->iw      = body[5];
  win->ih      = body[6];
  
  gtk_fvwmpager_display_window(GTK_FVWMPAGER(pager), &old, xid);
}

    
void
destroy_window(GtkFvwmPager* pager, unsigned long* body)
{
  unsigned long xid;
  xid = body[0];
  gtk_fvwmpager_destroy_window(GTK_FVWMPAGER(pager), xid);
}

void
set_focus(GtkFvwmPager* pager, unsigned long* body)
{
  unsigned long xid;

  xid = body[0];
  gtk_fvwmpager_set_current_window(GTK_FVWMPAGER(pager), xid);
}

void
deiconify_window(GtkFvwmPager* pager, unsigned long* body)
{
  unsigned long xid;
  PagerWindow   old;
  PagerWindow*  win;

  xid = body[0];
  win = g_hash_table_lookup(pager->windows, (gconstpointer)xid);
  if (!win)
    return;
  old = *win;
  win->flags &= ~GTKPAGER_WINDOW_ICONIFIED;
  gtk_fvwmpager_display_window(GTK_FVWMPAGER(pager), &old, xid);
}

void
add_window(GtkFvwmPager* pager, unsigned long* body)
{
  unsigned long xid;
  PagerWindow*  win;

  xid = body[0];
  win = g_hash_table_lookup(pager->windows, (gconstpointer)xid);
  if (!win)
    {
      return;
    }
  gtk_fvwmpager_display_window(GTK_FVWMPAGER(pager), 0, xid);
}


void process_message(GtkFvwmPager* pager, unsigned long type,unsigned long *body)
{
  switch(type)
    {
    case M_ADD_WINDOW:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "test-message: M_ADD_WINDOW received\n");
      add_window(pager, body);
      break;
    case M_CONFIGURE_WINDOW:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_CONFIGURE_WINDOW received\n");
      configure_window(pager, body);
      break;
    case M_DESTROY_WINDOW:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_DESTROY_WINDOW received\n");
      destroy_window(pager, body);
      break;
    case M_FOCUS_CHANGE:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: unhandled M_FOCUS_CHANGE received\n");
      set_focus(pager, body);
      break;
    case M_NEW_PAGE:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: unhandled M_ADD_WINDOW received\n");
      break;
    case M_NEW_DESK:
      {
	long desktop = (long)body[0];

	g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_NEW_DESK received\n");
	gtk_fvwmpager_set_current_desk(GTK_FVWMPAGER(pager), desktop);
	XChangeProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
			_XA_WIN_WORKSPACE, _XA_WIN_WORKSPACE,
			32, PropModeReplace, (unsigned char*)body, 1);
      }
      break;
    case M_RAISE_WINDOW:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_RAISE_WINDOW received\n");
      gtk_fvwmpager_raise_window(GTK_FVWMPAGER(pager), body[0]);
      break;
    case M_LOWER_WINDOW:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_LOWER_WINDOW received\n");
      gtk_fvwmpager_lower_window(GTK_FVWMPAGER(pager), body[0]);
      break;
    case M_ICONIFY:
    case M_ICON_LOCATION:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_ICONIFY || M_ICON_LOCATION received\n");
      configure_icon(pager, body);
      break;
    case M_DEICONIFY:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: M_DEICONIFY received\n");
      deiconify_window(pager, body); 
      break;
    case M_ICON_NAME:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: unhandled M_AICON_NAME received\n");
      break;
    case M_MINI_ICON:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: unhandled M_MINI_ICON received\n");
      break;
    case M_END_WINDOWLIST:
      g_log("fvwm-pager", G_LOG_LEVEL_DEBUG, "message: unhandled M_END_WINDOWLIST received\n");
      break;
    default:
      g_log("fvwm-pager", G_LOG_LEVEL_ERROR, "unknown FVWM2 test-message\n");
      break;
    }
}



void
DeadPipe(int signo)
{
  exit(0);
}

void ParseOptions(GtkFvwmPager* pager)
{
  char* tline;
  int   len = strlen(pgm_name);
    
  GetConfigLine(fd, &tline);

  while(tline != NULL)
    {
      char* ptr;
      int   desk;
      
      if ((strlen(&tline[0]) > 1) && mystrncasecmp(tline, CatString3("*", pgm_name, "Label"), len + 6) == 0)
	{
	  desk = desk1;
	  ptr = &tline[len + 6];

	  if (sscanf(ptr, "%d", &desk) != 1)
	    {
	      g_log("fvwm-pager", G_LOG_LEVEL_INFO,"Parse FVWM2 options: Invalid Label line '%s'\n", tline);
	    }
	  else
	    {
	      if ((desk > desk1) && (desk <= desk2))
		{
		  while (isspace(*ptr)) ptr++;
		  while (!isspace(*ptr)) ptr++;
		  if (ptr[strlen(ptr)-1] == '\n')
		    ptr[strlen(ptr)-1] = '\0';
		  gtk_fvwmpager_label_desk(pager, desk, ptr);
		}
	    }
	}
      GetConfigLine(fd, &tline);
    }
  
    
}
