#include <stdio.h>
#include <signal.h>
#include <fvwm/module.h>
#include <fvwm/fvwm.h>
#include <libs/fvwmlib.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#include "gtkpager.h"

#include "applet-lib.h"
#include "applet-widget.h"

void DeadPipe(int);
static void ParseOptions(void);
static void switch_to_desktop(GtkFvwmPager* pager, int desktop_offset);


static Atom _XA_WIN_WORKSPACE;
static Atom _XA_WIN_WORKSPACE_NAMES;
static Atom _XA_WIN_WORKSPACE_COUNT;
static Atom _XA_WIN_LAYER;

static GtkWidget* pager = 0;
static int        desk1;
static int        desk2;
static int        fd[2];
static char*      pgm_name;

unsigned long xprop_long_data[20];
unsigned char xprop_char_data[128];


void process_message(unsigned long, unsigned long*);

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

  if ((count = ReadFvwmPacket(source, header, &body)) > 0)
    {
      process_message(header[1], body);
      free(body);
    }
}

void
about_cb(AppletWidget* widget, gpointer data)
{
  GtkWidget* about;
  gchar* authors[] ={"M. Lausch", NULL};

  about = gnome_about_new( _("Fvwm Pager Applet"), "0.1",
			   _("Copyrhight (C) 1998 M. Lausch"),
			   authors,
			   "Pager for Fvwm2 window manager",
			   NULL);
  gtk_widget_show(about);
}

void properties_dialog(AppletWidget *widget, gpointer data)
{

    return;
}

    
    

main(int argc, char* argv[])
{
  GtkWidget* window;
  char bfr[128];
  int  idx;
  int  ndesks;
  char* tmp;
  char* dummy_argv[2] = {NULL};

  dummy_argv[0] = "fvwm-pager";
  
  panel_corba_register_arguments();
  
  applet_widget_init_defaults("fvwmpager_applet", NULL, 1, dummy_argv, 0, NULL, argv[0]);

  window = applet_widget_new();

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
  
  
#if 0
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window),
		       "GnomePager");
#endif
  gtk_widget_set_usize(GTK_WIDGET(window), 150, 60);

  pager = gtk_fvwmpager_new();
  
  gtk_signal_connect(GTK_OBJECT(pager), "switch_desktop",
		     GTK_SIGNAL_FUNC(switch_to_desktop), NULL);

  gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		     GTK_SIGNAL_FUNC(destroy), NULL);
  
  applet_widget_add(APPLET_WIDGET(window), pager);

  gtk_widget_show(window);
  gtk_widget_show(pager);

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
					properties_dialog,
					NULL);
  
  for (idx = 0; idx < ndesks; idx++)
    {
      sprintf(bfr, _("Desk %d"), idx);
      gtk_fvwmpager_add_desk(GTK_FVWMPAGER(pager), bfr);
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

  ParseOptions();
#if 1
  {
    GList* elem;
    XTextProperty tp;
    char*  desk_names[ndesks];
    int i;

    elem = GTK_FVWMPAGER(pager)->desktops;
    
    for (i = 0; i < ndesks; i++)
      {
	struct Desk* desk;
	desk = elem->data;
	
	desk_names[i] = desk->title;
	elem = g_list_next(elem);
      }
    if (!XStringListToTextProperty(desk_names, ndesks, &tp))
      {
	fprintf(stderr,"GnomePager: Cannot get textproperty for workspace names\n");
      }
    else
      {
	XSetTextProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), &tp, _XA_WIN_WORKSPACE_NAMES);
      }
  }
    
#endif  
  
  
  gdk_input_add(fd[1],
		GDK_INPUT_READ,
		fvwm_command_received,
		0);
  SendInfo(fd, "Send_WindowList", 0);
  applet_widget_gtk_main();
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


void configure_window(unsigned long* body)
{
  struct Xwin* win;
  
  win = malloc(sizeof (struct Xwin));

  memset(win, 0, sizeof(struct Xwin));
  
  win->xid   = body[0];
  win->x     = body[3];
  win->y     = body[4];
  win->w     = body[5];
  win->h     = body[6];
  win->icon_x = -1000;
  win->icon_y = -1000;
  win->icon_h = 10;
  win->icon_w = 10;
  win->desktop_idx = body[7];
  win->flags = 0;
  if (body[8] & ICONIFIED)
    win->flags |= GTKPAGER_WINDOW_ICONIFIED;
  gtk_fvwmpager_conf_window(GTK_FVWMPAGER(pager), win);
  
}

void configure_icon(unsigned long* body)
{
  struct Xwin* win;

  win = malloc(sizeof(struct Xwin));
  win->xid         = body[0];
  win->desktop_idx = -1;
  win->flags       = GTKPAGER_WINDOW_ICONIFIED;
  win->icon_x      = body[3];
  win->icon_y      = body[4];
  win->icon_w      = body[5];
  win->icon_h     = body[6];
  gtk_fvwmpager_conf_window(GTK_FVWMPAGER(pager), win);
}

    
void
destroy_window(unsigned long* body)
{
  unsigned long xid;
  xid = body[0];
  gtk_fvwmpager_destroy_window(pager, xid);
}

void
deiconify_window(unsigned long* body)
{
  unsigned long xid;

  xid = body[0];
  gtk_fvwmpager_deiconify_window(pager, xid);
}

void process_message(unsigned long type,unsigned long *body)
{
  switch(type)
    {
    case M_ADD_WINDOW:
      fprintf(stderr,"GnomePager: Unimplemented: M_ADD_WINDOW\n");
      /*list_configure(body); */
      break;
    case M_CONFIGURE_WINDOW:
      configure_window(body);
      break;
    case M_DESTROY_WINDOW:
      destroy_window(body);
      break;
    case M_FOCUS_CHANGE:
      fprintf(stderr,"GnomePager: Unimplemented: M_FOCUS_CHANGE\n");
      /* list_focus(body);*/
      break;
    case M_NEW_PAGE:
      fprintf(stderr,"GnomePager: Unimplmeneted: M_NEW_PAGE\n");
      break;
    case M_NEW_DESK:
      {
	XChangeProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
			_XA_WIN_WORKSPACE, _XA_WIN_WORKSPACE,
			32, PropModeReplace, (unsigned char*)body, 1);
	
	gtk_fvwmpager_set_current_desk(GTK_FVWMPAGER(pager), body[0]);
      }
      break;
    case M_RAISE_WINDOW:
      gtk_fvwmpager_raise_window(pager, body[0]);
      break;
    case M_LOWER_WINDOW:
      gtk_fvwmpager_lower_window(pager, body[0]);
      break;
    case M_ICONIFY:
    case M_ICON_LOCATION:
      configure_icon(body);
      break;
    case M_DEICONIFY:
      deiconify_window(body); 
      break;
    case M_ICON_NAME:
      fprintf(stderr,"GnomePager: Unimplemented: M_ICON_NAME\n");
      break;
    case M_MINI_ICON:
      fprintf(stderr,"GnomePager: Unimplmented: M_MINI_ICON\n");
      break;
    case M_END_WINDOWLIST:
      fprintf(stderr,"GnomePager: Unimplemented: M_END_WINDOWLIST\n");
      break;
    default:
      fprintf(stderr,"GnomePager: Unimplemented: default\n");
      break;
    }
}



void
DeadPipe(int signo)
{
  exit(0);
}

void ParseOptions(void)
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
	      fprintf(stderr,"GnomePager: Invalid Label line '%s'\n", tline);
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
