/*
 * GNOME Desktop Pager Applet
 * (C) 1998 The Free Software Foundation
 *
 * Author: M.Watson
 *
 * A panel applet that switches between multiple desktops using
 * Marko Macek's XA_WIN_* hints proposal. Compatible with icewm => 0.91
 *
 */


#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include "WinMgr.h"
#include "applet-lib.h"
#include "applet-widget.h"

void   setup();
void   switch_cb(GtkWidget *widget, gpointer data);
void   about_cb(AppletWidget *widget, gpointer data);
void   properties_dialog(AppletWidget *widget, gpointer data);
void   change_workspace(gint ws);
void   make_sticky(GtkWidget *widget, gpointer data);
gint   check_workspace(gpointer data);
GList* get_workspaces();
gint   get_current_workspace();

GtkWidget *applet;
GList *workspace_list, *button_list;
gint current_workspace;
Atom _XA_WIN_WORKSPACE;
Atom _XA_WIN_WORKSPACE_NAMES;
Atom _XA_WIN_STATE;


int main(int argc, char *argv[])
{

    panel_corba_register_arguments();

    applet_widget_init_defaults("wmpager_applet", NULL, argc, argv, 0, NULL, argv[0]);

    /* Get the Atom for making a window sticky, so that the detached pager appears on all screens */
    _XA_WIN_STATE = XInternAtom(GDK_DISPLAY(), XA_WIN_STATE, False);
    
    if(((_XA_WIN_WORKSPACE = XInternAtom(GDK_DISPLAY(), XA_WIN_WORKSPACE, True))== None) ||
       (_XA_WIN_WORKSPACE_NAMES = XInternAtom(GDK_DISPLAY(), XA_WIN_WORKSPACE_NAMES, True))==None)
    {
        /* FIXME: This should check to make sure icewm (or equiv) is running.
           Not sure if it is working though... */
        g_error(_("This applet requires you to run a window manager with the XA_ extensions.\n"));
        exit(1);
    }

    applet = applet_widget_new();
    if (!applet)
        g_error("Can't create applet!\n");

    gtk_widget_realize(applet);

    workspace_list = get_workspaces();
    setup();
    current_workspace = get_current_workspace();
    gtk_toggle_button_set_state
        (GTK_TOGGLE_BUTTON(g_list_nth(button_list, current_workspace)->data), 1);

    gtk_timeout_add(1000, check_workspace, NULL);
    gtk_widget_show(applet);
    /*
    gtk_signal_connect(GTK_OBJECT(applet),"destroy",
                       GTK_SIGNAL_FUNC(destroy_applet),
                       NULL);
    gtk_signal_connect(GTK_OBJECT(applet),"session_save",
                       GTK_SIGNAL_FUNC(applet_session_save),
                       NULL);
    */
    applet_widget_register_callback(APPLET_WIDGET(applet),
                                    "about",
                                    _("About..."),
                                    about_cb,
                                    NULL);


    applet_widget_register_callback(APPLET_WIDGET(applet),
                                    "properties",
                                    _("Properties..."),
                                    properties_dialog,
                                    NULL);

    applet_widget_gtk_main();

    return 0;
}




GList *get_workspaces()
{
    GList *tmp_list;
    XTextProperty tp;
    char **list;
    int count, i;
    char tmpbuf[1024];

    XGetTextProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), &tp, _XA_WIN_WORKSPACE_NAMES);

    XTextPropertyToStringList(&tp, &list, &count);

    tmp_list = g_list_alloc();
    for(i=0; i<count; i++)
    {
        tmp_list = g_list_append(tmp_list, g_strdup(list[i]));
    }

    return tmp_list;
}

void setup()
{
    GtkWidget *frame;
    GtkWidget *hb, *vb;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *handle;
    
    gint num_ws;      /* the number of workspaces */
    gint even;        /* even number of workspaces? */
    gint i;           /* counter for loops */
    char ws_name[1024];

    workspace_list = g_list_first(workspace_list);
    button_list = g_list_alloc();
    num_ws = g_list_length(workspace_list)-1;
    even = (((num_ws/2)*2)<num_ws) ? 0 : 1;

    handle = gtk_handle_box_new();
    /* applet_widget_add should be called after all children are added, but it screws up the buttons
     and the handle box. If it annoys you to have to middle- and right- click on the handle only, comment
     this one out and uncomment the one at the end of this function. */
    applet_widget_add(APPLET_WIDGET(applet), handle);
    gtk_signal_connect (GTK_OBJECT (handle), "child_detached",
                        GTK_SIGNAL_FUNC (make_sticky), NULL);
    gtk_widget_show(handle);

    
    
    frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(handle), frame);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_widget_show(frame);

    /* FIXME: If the panel is vertical, switch the hboxs and vboxs. */
    /* This is not really a problem, just a cosmetic nit.           */
    vb = gtk_vbox_new(TRUE, 0);
    gtk_container_add(GTK_CONTAINER(frame), vb);
    gtk_widget_show(vb);

    hb = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vb), hb, TRUE, TRUE, 0);
    gtk_widget_show(hb);

    for(i=1; i < ((num_ws + !even)/2)+1; i++)
    {
        workspace_list = g_list_next(workspace_list);
        sprintf(ws_name, "%s", (char*)(workspace_list->data));
        button = gtk_toggle_button_new_with_label(ws_name);
        button_list = g_list_append(button_list, button);
        gtk_signal_connect(GTK_OBJECT(button), "clicked",
                           GTK_SIGNAL_FUNC(switch_cb), (int*)i);
        gtk_box_pack_start(GTK_BOX(hb), button, TRUE, TRUE, 0);
        gtk_widget_show(button);
    }

        
    hb = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vb), hb, TRUE, TRUE, 0);
    gtk_widget_show(hb);

    for(i=((num_ws + !even)/2)+1; i < num_ws+1; i++)
    {
        workspace_list = g_list_next(workspace_list);
        button = gtk_toggle_button_new_with_label((gchar*)(workspace_list->data));
        button_list = g_list_append(button_list, button);
        gtk_signal_connect(GTK_OBJECT(button), "clicked",
                           GTK_SIGNAL_FUNC(switch_cb), (int*)i);
        gtk_box_pack_start(GTK_BOX(hb), button, TRUE, TRUE, 0);
        gtk_widget_show(button);
    }

    if(!even)
    {
        label = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(hb), label, TRUE, TRUE, 0);
        button_list = g_list_append(button_list, button);
        gtk_signal_connect(GTK_OBJECT(button), "clicked",
                           GTK_SIGNAL_FUNC(switch_cb), (int*)i);
        gtk_widget_show(label);
    }

    /* This is where the applet_widget_add belongs, but it seems to screw up the buttons and the handlebox. */
    /*
    applet_widget_add(APPLET_WIDGET(applet), handle);
    */
    return;
}

void about_cb (AppletWidget *widget, gpointer data)
{
    GtkWidget *about;
    gchar *authors[] = {"M.Watson", NULL};

    about = gnome_about_new ( _("Desktop Pager Applet"), "0.0",
                              _("Copyright (C)1998 M.Watson"),
                              authors,
                              "Pager for icewm (or other GNOME hints compliant) window manager",
                              NULL);
    gtk_widget_show (about);

    return;
}

void properties_dialog(AppletWidget *widget, gpointer data)
{

    return;
}


void switch_cb(GtkWidget *widget, gpointer data)
{
    gint button_num;
    gint tmp_ws;

    tmp_ws = get_current_workspace();
    button_num = (int)data;
    if(GTK_TOGGLE_BUTTON(widget)->active)
    {
        if(button_num != tmp_ws)
            gtk_toggle_button_set_state
                (GTK_TOGGLE_BUTTON(g_list_nth(button_list, tmp_ws)->data), 0);
        change_workspace(button_num);
        gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(button_list, button_num)->data), 0);
        return;
    }
    else
        gtk_widget_set_sensitive(GTK_WIDGET(g_list_nth(button_list, button_num)->data), 1);

    return;
}


gint get_current_workspace()
{
    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop;
    CARD32 wws = 0;



    if((XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
                           _XA_WIN_WORKSPACE,
                           0, 1, False, XA_CARDINAL,
                           &type, &format, &nitems,
                           &bytes_after, &prop)) == 1)
    {
        g_error(_("Failed to retrieve workspace property."));
        exit(1);
    }
    
    wws = *(CARD32 *)prop;

    return wws+1;
}

void change_workspace(gint ws)
{
    XEvent xev;


    xev.type = ClientMessage;
    xev.xclient.type = ClientMessage;
    xev.xclient.window = GDK_ROOT_WINDOW();
    xev.xclient.message_type = _XA_WIN_WORKSPACE;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = ws-1;
    xev.xclient.data.l[1] = gdk_time_get();

    XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
               SubstructureNotifyMask, (XEvent*) &xev);
    current_workspace = ws;

    return;
}

gint check_workspace(gpointer data)
{
    gint i;  /* loop counter */
    gint tmp_ws;

    /* FIXME: Is there some way to be notified when the XA_WIN_WORKSPACE
       property on the ROOT window changes? That may be more efficient
       than polling once a second.            */

    tmp_ws = get_current_workspace();
    if(tmp_ws != current_workspace)
    {
        gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(g_list_nth(button_list, current_workspace)->data), 0);
        gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(g_list_nth(button_list, tmp_ws)->data), 1);
        current_workspace = tmp_ws;
    }
    return 1;
}


void make_sticky(GtkWidget *widget, gpointer data)
{
    GdkWindowPrivate *privwin;
    XEvent xev;

    privwin = (GdkWindowPrivate*)(GTK_HANDLE_BOX(widget)->float_window);
    if(privwin != NULL)
    {
        xev.type = ClientMessage;
        xev.xclient.type = ClientMessage;
        xev.xclient.window = privwin->xwindow;
        xev.xclient.message_type = _XA_WIN_STATE;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = WinStateAllWorkspaces;
        xev.xclient.data.l[1] = (1 << 0);
        xev.xclient.data.l[2] = gdk_time_get();

        XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, SubstructureNotifyMask, (XEvent*) &xev);
    }
    
    return;
}
