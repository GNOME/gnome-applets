/*
#
#   GNotes!
#   Copyright (C) 1998-1999 spoon <spoon@ix.netcom.com>
#   Copyright (C) 1999 dres <dres@debian.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <config.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>

#include <X11/X.h>

#include <glib.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

#include "gnotes_applet.h"
#include "gnote.h"

const char *gnotes_file_format = "1.0";

/* XML node names */
const gchar *gnotes_top_level_node_name = "GNote";     /* can't change */
const gchar *gnotes_version_property_name = "Version"; /* these and be */
const gchar *gnotes_id_property_name    = "ID";        /* backwards compat */
const gchar *gnotes_title_node_name     = "Title";
const gchar *gnotes_loc_node_name       = "Location";
const gchar *gnotes_x_node_name         = "X";
const gchar *gnotes_y_node_name         = "Y";
const gchar *gnotes_width_node_name     = "Width";
const gchar *gnotes_height_node_name    = "Height";
const gchar *gnotes_state_node_name     = "State";
const gchar *gnotes_hidden_node_name    = "Hidden";
const gchar *gnotes_contents_node_name  = "Contents";
const gchar *gnotes_data_type_node_name = "Type";
const gchar *gnotes_data_node_name      = "Data";


/* THE notes */
static GPtrArray* note_list;
extern GNotes gnotes;

gchar *save_dir_name = 0;

typedef void (*GPtrOperator)(gpointer);

void gnotes_init(void);
void gnote_destroy_cb(GtkWidget *, gpointer);
void gnote_delete_cb(GtkWidget *, gpointer);
void gnotes_raise(AppletWidget*, gpointer);
void gnote_raise(gpointer);
gint gnote_raise_cb(GtkWidget *, gpointer);
void gnotes_lower(AppletWidget*, gpointer);
void gnote_lower(gpointer);
gint gnote_lower_cb(GtkWidget *, gpointer);
void gnotes_show(AppletWidget*, gpointer);
void gnote_show(gpointer);
void gnotes_hide(AppletWidget*, gpointer);
void gnote_hide(gpointer);
gint gnote_hide_cb(GtkWidget *, gpointer);
void gnote_save(gpointer);
void gnotes_load(AppletWidget*, gpointer);
static gboolean gnote_load_xml(const gchar *);
static gboolean gnote_load_xml_v10(xmlDocPtr, const gchar*);
static gboolean gnote_load_old(const gchar *);
const gchar *get_gnotes_dir(void);
gchar *make_gnote_filename_string(const gchar *);
gchar *make_gnote_filename(const GNote *);
GNote *get_gnote_based_on_boxptr(gpointer);


/*----------------------------------------------------------------------*/
/*
 * INIT
 */
void gnotes_init()
{
    note_list = g_ptr_array_new();
}

/*----------------------------------------------------------------------*/
/*
 * Utilities
 */
static void do_on_all_notes(GPtrOperator oper)
{
    int i;

    if(note_list == 0)
    {
        return;
    }

    g_debug("Working on %d notes.", note_list->len);
    
    for(i = 0;i < note_list->len; i++)
    {
        g_debug("Doing %d in list", i);
        (*oper)(g_ptr_array_index(note_list, i));
    }
}
        
const gchar *get_gnotes_dir()
{
    if(save_dir_name == 0)
    {
        save_dir_name = g_strdup_printf("%s/%s", gnome_util_user_home(), GNOTES_DIR);
        g_debug("Setting save directory to %s", save_dir_name);
    }
    
    return save_dir_name;
}

gchar *make_gnote_filename_string(const gchar *file_part)
{
    return g_strdup_printf("%s/%s", get_gnotes_dir(), file_part);
}

gchar *make_gnote_filename(const GNote *note)
{
    return g_strdup_printf("%s/%ld", get_gnotes_dir(), note->timestamp);
}

GNote *get_gnote_based_on_boxptr(gpointer handle_boxptr)
{
    int index;
    
    for (index=0; index < note_list->len; index++)
    {
        GNote* the_note = (GNote*)g_ptr_array_index(note_list, index);
        if (handle_boxptr == the_note->handle_box)
        {
            return the_note;
        }
    }

    return 0;
}

static gboolean contains_only_nums(gchar *test_string)
{
    int i;

    if(test_string == 0)
    {
        return FALSE;
    }
    
    for(i = strlen(test_string) - 1; i >=0; i--)
    {
        if(!isdigit(test_string[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

static guchar* get_text_text(GtkText* text)
{
    return gtk_editable_get_chars(GTK_EDITABLE(text), 0, -1);
}


/*----------------------------------------------------------------------*/
GtkWidget* gnote_create_menu(GNote* the_note)
{
    GtkWidget *menu;
    GtkWidget *menuitem;
  
    g_debug("gnote_menu();");

    menu = gtk_menu_new();

    {
        char *timestr;
        /* put the title menu item */
        menuitem = gtk_menu_item_new_with_label(the_note->title);
        gtk_menu_append(GTK_MENU(menu), menuitem);

        gtk_signal_connect_object(GTK_OBJECT(menuitem),"select",
                                  GTK_SIGNAL_FUNC(gtk_item_deselect),
                                  GTK_OBJECT(menuitem));

        timestr = asctime(localtime(&the_note->timestamp));
        timestr[strlen(timestr)-1] = '\0';  /* get rid of '\n' */

        menuitem = gtk_menu_item_new_with_label(timestr);
        gtk_menu_append(GTK_MENU(menu), menuitem);
        gtk_signal_connect_object(GTK_OBJECT(menuitem),"select",
                                  GTK_SIGNAL_FUNC(gtk_item_deselect),
                                  GTK_OBJECT(menuitem));
    };

    /* separator */
    menuitem = gtk_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), menuitem);

 
    menuitem = gtk_menu_item_new_with_label(_("Raise Note"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnote_raise_cb), the_note->handle_box);
    gtk_menu_append(GTK_MENU(menu), menuitem);
  
    menuitem = gtk_menu_item_new_with_label(_("Lower Note"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnote_lower_cb), the_note->handle_box);
    gtk_menu_append(GTK_MENU(menu), menuitem);
  
    menuitem = gtk_menu_item_new_with_label(_("Hide Note"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnote_hide_cb), the_note->handle_box);
    gtk_menu_append(GTK_MENU(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Delete Note"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnote_delete_cb), the_note->handle_box);
    gtk_menu_append(GTK_MENU(menu), menuitem);

    gtk_widget_show_all(menu);

    return menu;
}

void gnote_menu(GtkWidget *handle_box, GdkEventButton *event)
{
    GNote* the_note = get_gnote_based_on_boxptr(handle_box);
    gtk_menu_popup(GTK_MENU(the_note->menu), NULL, NULL, NULL, NULL,
                   event->button, event->time);
};

static gint ptr_x=0, ptr_y=0;
static gpointer motion_last_box = 0;
static GNote *motion_note = 0;

gint gnote_handle_button_cb(GtkWidget *widget, GdkEventButton *event,
                            gpointer data)
{
    GdkCursor *cursor;

    g_debug("gnote_handle_button_cb();");

    /* get current position in case we're moved! */
    gdk_window_get_pointer(widget->parent->window, &ptr_x, &ptr_y, NULL);

    switch (event->type)
    {
    case GDK_BUTTON_PRESS:
        switch (event->button)
        {
        case 1:
            cursor = gdk_cursor_new(GDK_FLEUR);
            gdk_pointer_grab(widget->window, FALSE, GDK_BUTTON1_MOTION_MASK |
                             GDK_BUTTON_RELEASE_MASK, NULL, cursor, event->time);
            gdk_cursor_destroy(cursor);
            break;
        case 2:
            cursor = gdk_cursor_new(GDK_DOT);
            gdk_window_set_cursor(widget->window, cursor);
            gdk_cursor_destroy(cursor);
            break;
        case 3:
            gnote_menu(widget, event);
            break;
        };
        break;
    case GDK_BUTTON_RELEASE:
        switch (event->button)
        {
        case 1:
            gdk_pointer_ungrab(event->time);
            break;
        case 2:
            gnote_new_cb(NULL, 0);
            break;
        case 3:
            return(FALSE);
        };
        cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
        gdk_window_set_cursor(widget->window, cursor);
        gdk_cursor_destroy(cursor);
        break;
    default:
        g_debug("Unhandled type in gnote_handle_button_cb(): %d", event->type);
        break;
    };

    gnote_save(motion_note);
    return(TRUE);
};

gint gnote_motion_cb(GtkWidget *widget, GdkEventButton *event,
                     gpointer data)
{
    gint win_x=0, win_y=0, win_w=0, win_h=0, new_ptr_x=0, new_ptr_y=0;

    gdk_window_get_pointer(widget->parent->window, &new_ptr_x, &new_ptr_y,
                           NULL);

    if (event->state & ShiftMask)
    {
        g_debug("Doing a shift motion event");
        if((widget != motion_last_box) || (motion_note == 0))
        {
            motion_note = get_gnote_based_on_boxptr(widget);
            motion_last_box = motion_note->handle_box;
        }
        /* set new size */
        win_w = new_ptr_x;
        win_h = new_ptr_y;
        g_debug("Size is now W:%d H:%d with widget %p", win_w, win_h,
                motion_note->window);
        gtk_widget_set_usize(motion_note->window, win_w, win_h);
    }
    else
    {
        gdk_window_get_origin(widget->parent->window, &win_x, &win_y);
        /* new position minus the pointer position in the widget */
        win_x += new_ptr_x - ptr_x;
        win_y += new_ptr_y - ptr_y;
        gdk_window_move(widget->parent->window, win_x, win_y);
    };

    return(TRUE);
};


/*----------------------------------------------------------------------*/
void gnote_new(gint width, gint height, gint x, gint y, gboolean hidden,
               const gchar *text, time_t timestamp,
               const gchar *title, gboolean loaded_from_file,
               const gchar *type)
{
    GNote* the_note = g_new0(GNote, 1);

    g_debug("gnote_new(%d, %d, %d, %d, %d, %s, %ld, %s, %d, %s) called.",
            width, height, x, y, hidden, text, timestamp, title,
            loaded_from_file, type);
    
    g_ptr_array_add(note_list, the_note);
    
    the_note->hidden = hidden;
    the_note->timestamp = timestamp;
    the_note->title = g_strdup(title);
    the_note->already_saved = loaded_from_file;
    the_note->type = g_strdup(type);
    the_note->menu = NULL;
    
    /* create window */
    the_note->window = gtk_window_new(GTK_WINDOW_DIALOG);
    if(the_note->window == 0)
    {
        return;
    }
    
    gtk_window_set_policy(GTK_WINDOW(the_note->window), TRUE, TRUE,
                          TRUE);

    if ((width >= 0) && (height >= 0))
    {
        gtk_widget_set_usize(the_note->window, width, height);
    }
    if ((x >= 0) && (y >= 0))
    {
        gtk_widget_set_uposition(the_note->window, x, y);
    }

    /* create text */ 
    the_note->text = gtk_text_new(NULL, NULL);
    gtk_text_set_editable(GTK_TEXT(the_note->text), TRUE);
    /* gtk_text_set_word_wrap(GTK_TEXT(the_note->text), TRUE); */
    
    gtk_text_freeze(GTK_TEXT(the_note->text));
    if (text != 0)
    {
        gtk_text_insert(GTK_TEXT(the_note->text), NULL,
                        NULL, NULL, text, -1);
    }
    else
    {
        gtk_text_insert(GTK_TEXT(the_note->text), NULL,
                        NULL, NULL, "", -1);
    }
    gtk_text_thaw(GTK_TEXT(the_note->text));
    
    /* set text color */
    {
        GtkStyle *style;
        GdkColor *yellow;

        yellow = (GdkColor *)g_malloc(sizeof(GdkColor));
        yellow->red = 0xffff;
        yellow->green = 0xffff;
        yellow->blue = 0x0000;
        yellow->pixel = (gulong)255*750000;

        gdk_color_alloc(gtk_widget_get_colormap(the_note->text), yellow);

        style = gtk_style_copy(gtk_widget_get_style(the_note->text));
        memcpy(&style->base[GTK_STATE_NORMAL], yellow, sizeof(GdkColor));
        gtk_widget_set_style(the_note->text, style);
        g_free(yellow);
    };

    /* create handle_box */
    the_note->handle_box = gtk_handle_box_new();
    gtk_signal_connect(GTK_OBJECT(the_note->handle_box),
                       "motion_notify_event",
                       GTK_SIGNAL_FUNC(gnote_motion_cb), NULL);
    gtk_signal_connect(GTK_OBJECT(the_note->handle_box),
                       "button_press_event",
                       GTK_SIGNAL_FUNC(gnote_handle_button_cb), NULL);
    gtk_signal_connect(GTK_OBJECT(the_note->handle_box),
                       "button_release_event",
                       GTK_SIGNAL_FUNC(gnote_handle_button_cb), NULL);
    gtk_signal_connect(GTK_OBJECT(the_note->handle_box),
                       "destroy",
                       GTK_SIGNAL_FUNC(gnote_destroy_cb),
                       the_note->handle_box);
    /* FIXME: can't do this because the window gets destroyed when
    shutting down which deletes the notes.  very bad.  Way around? */
/*     gtk_signal_connect(GTK_OBJECT(the_note->handle_box), */
/*                        "delete_event", */
/*                        GTK_SIGNAL_FUNC(gnote_delete_cb), */
/*                        the_note->handle_box); */
/*     gtk_signal_connect(GTK_OBJECT(the_note->handle_box), */
/*                        "destroy_event", */
/*                        GTK_SIGNAL_FUNC(gnote_delete_cb), */
/*                        the_note->handle_box); */
    gtk_widget_set_usize(the_note->handle_box, 10, 0);

    /* create hbox */
    the_note->hbox = gtk_hbox_new(FALSE, 0);

    /* pack it up! */
    gtk_box_pack_start(GTK_BOX(the_note->hbox),
                       the_note->handle_box, FALSE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(the_note->hbox),
                     the_note->text, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(the_note->window),
                      the_note->hbox);

    gtk_widget_show_all(the_note->hbox);
    gtk_widget_realize(the_note->window);
    
    /*
     * setup window parameters.
     */
    gdk_window_set_decorations (the_note->window->window, 0);

    /* Set the proper GNOME hints */
    
    gnome_win_hints_init ();

    if (gnome_win_hints_wm_exists ()) {
        if(gnotes.defaults.onbottom)
        {
            gnome_win_hints_set_layer (the_note->window, WIN_LAYER_DESKTOP);
        }
        
        gnome_win_hints_set_state (the_note->window,
                                   WIN_STATE_STICKY);
        gnome_win_hints_set_hints (the_note->window,
                                   WIN_HINTS_SKIP_FOCUS
                                   | WIN_HINTS_SKIP_WINLIST
                                   | WIN_HINTS_SKIP_TASKBAR);
    }

    gtk_window_set_focus(GTK_WINDOW(the_note->window),
                         the_note->text);

    /* make sure it will all be realized here */
    gtk_widget_show_now(the_note->window);

    {
        GdkCursor *cursor;

        cursor = gdk_cursor_new(GDK_XTERM);
        gdk_window_set_cursor(the_note->text->window, cursor);
        gdk_cursor_destroy(cursor);
    };

    if (the_note->hidden != FALSE)
    {
        gdk_window_hide(the_note->window->window);
    }

    the_note->menu = gnote_create_menu(the_note);
    
    gnote_save(the_note);
}

void gnote_new_cb(AppletWidget *applet, gpointer data)
{
    char *sdata;
    gint width, height;

    if(data != 0)
    {
        sdata = g_strdup((gchar *)data);
        width = atoi(strtok(sdata, "x")) * 100;
        height = atoi(strtok(NULL, "x")) * 100;
        g_free(sdata);
    }
    else
    {
        GNotes *gnotes = gnotes_get_main_info();
        width = gnotes->defaults.width;
        height = gnotes->defaults.height;
    }

    gnote_new(width, height, 0, 0, FALSE, "",
              time(NULL), "GNotes!", FALSE, "");

}

/*----------------------------------------------------------------------*/

void gnote_delete_cb(GtkWidget *widget, gpointer handle_boxptr)
{
    GNote *the_note = get_gnote_based_on_boxptr(handle_boxptr);
    /* the window's desctruction handler will take care of everything */
    if(the_note->window)
        gtk_widget_destroy(the_note->window);
}


void gnote_destroy_cb(GtkWidget *widget, gpointer handle_boxptr)
{
    GNote *the_note = get_gnote_based_on_boxptr(handle_boxptr);
    char *fname; 

    fname = make_gnote_filename(the_note);
    
    g_debug("  deleted [%s]", fname);
    unlink(fname);
    free(fname);

    if(the_note->menu)
        gtk_widget_destroy(GTK_WIDGET(the_note->menu));
    
    g_free(the_note->title);
    g_free(the_note->type);
    g_ptr_array_remove(note_list, the_note);
    g_free(the_note);
}

/*----------------------------------------------------------------------*/
void gnotes_raise(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_raise);
}

void gnote_raise(gpointer note)
{
    gdk_window_raise(((GNote*)note)->window->window);
}

gint gnote_raise_cb(GtkWidget *widget, gpointer handle_boxptr)
{
    gnote_raise(get_gnote_based_on_boxptr(handle_boxptr));
    return(FALSE);
}

/*----------------------------------------------------------------------*/
void gnotes_lower(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_lower);
}

void gnote_lower(gpointer note)
{
    gdk_window_lower(((GNote*)note)->window->window);
}

gint gnote_lower_cb(GtkWidget *widget, gpointer handle_boxptr)
{
    gnote_lower(get_gnote_based_on_boxptr(handle_boxptr));
    return(FALSE);
}

/*----------------------------------------------------------------------*/
void gnotes_show(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_show);
}

void gnote_show(gpointer prenote)
{
    GNote *note = prenote;
    gdk_window_show(note->window->window);
}

/*----------------------------------------------------------------------*/
void gnotes_hide(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_hide);
}

void gnote_hide(gpointer note)
{
    gdk_window_hide(((GNote*)note)->window->window);
}

gint gnote_hide_cb(GtkWidget *widget, gpointer handle_boxptr)
{
    gnote_hide(get_gnote_based_on_boxptr(handle_boxptr));
    return(FALSE);
}

/*----------------------------------------------------------------------*/
/*
 * SAVING
 */
void gnotes_save(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_save);
}

void gnote_save(gpointer prenote)
{
    GNote *the_note;
    FILE *fptr;
    char *fname;
    struct stat stats;
    xmlDocPtr doc;
    xmlNodePtr tree;
    int x,y;

    if(prenote == 0)
    {
        return;
    }
    
    the_note = (GNote*)prenote;

    /*
      make sure file doesn't already exist - since this is only
      possible with NEW notes, we change the filename too so it
      doesn't get unlinked below
    */
    fname = make_gnote_filename(the_note);

    if(!the_note->already_saved)
    {
        while (!stat(fname, &stats))
        {
            the_note->timestamp++;
            g_free(fname);
            fname = make_gnote_filename(the_note);
        }
    }

    g_debug("Saving into %s", fname);
    
    if ((fptr=fopen(fname,"w")) != NULL)
    {
        gchar *tmp_string;
        
        doc = xmlNewDoc("1.0");

        /* Top level */
        doc->root = xmlNewDocNode(doc, 0, gnotes_top_level_node_name, 0);
        xmlSetProp(doc->root, gnotes_version_property_name, gnotes_file_format);
        tmp_string = g_strdup_printf("%ld", the_note->timestamp);
        xmlSetProp(doc->root, gnotes_id_property_name, tmp_string);
        g_free(tmp_string);
        
        /* title */
        tree = xmlNewChild(doc->root, 0, gnotes_title_node_name, the_note->title);
        
        /* location */
        tree = xmlNewChild(doc->root, 0, gnotes_loc_node_name, 0);

        gdk_window_get_position(the_note->window->window, &x, &y);
        tmp_string = g_strdup_printf("%d", x);
        /* GTK_WIDGET(the_note->window)->allocation.x); */
        xmlNewChild(tree, 0, gnotes_x_node_name, tmp_string);
        g_free(tmp_string);

        tmp_string = g_strdup_printf("%d", y);
        /* GTK_WIDGET(the_note->window)->allocation.y); */
        xmlNewChild(tree, 0, gnotes_y_node_name, tmp_string);
        g_free(tmp_string);

        tmp_string = g_strdup_printf(
            "%d", GTK_WIDGET(the_note->window)->allocation.width);
        xmlNewChild(tree, 0, gnotes_width_node_name, tmp_string);
        g_free(tmp_string);

        tmp_string = g_strdup_printf(
            "%d",GTK_WIDGET(the_note->window)->allocation.height);
        xmlNewChild(tree, 0, gnotes_height_node_name, tmp_string);
        g_free(tmp_string);

        /* state */
        tree = xmlNewChild(doc->root, 0, gnotes_state_node_name, 0);

        tmp_string = g_strdup_printf("%d", the_note->hidden);
        xmlNewChild(tree, 0, gnotes_hidden_node_name, tmp_string);
        g_free(tmp_string);

        /* data */
        tree = xmlNewChild(doc->root, 0, gnotes_contents_node_name, 0);

        xmlNewChild(tree, 0, gnotes_data_type_node_name, "Text");

        if(GTK_TEXT(the_note->text)->text_len < 1)
        {
            xmlNewChild(tree, 0, gnotes_data_node_name, "");
        }
        else
        {
            gchar *data = get_text_text(GTK_TEXT(the_note->text));
            xmlNewChild(tree, 0, gnotes_data_node_name, data);
            g_free(data);
        }

        xmlDocDump(fptr, doc);

        xmlFreeDoc(doc);
        fclose(fptr);
        /* g_free(fptr); */
        the_note->already_saved = TRUE;
    }
    else
    {
        g_warning("GNotes: gnotes_save(): cannot open file %s", fname);
    }
    g_free(fname);
}


/*----------------------------------------------------------------------*/
/*
 * LOADING
 */

void gnotes_load(AppletWidget *wid, gpointer data)
{
    DIR *dptr;
  
    g_debug("Load Notes");

    /* create a new note for each one */
    if ((dptr=opendir(get_gnotes_dir())) != NULL)
    {
        struct dirent *pdirent;
        struct stat stats;
        
        while ((pdirent=readdir(dptr)) != NULL)
        {
            gchar *filename = pdirent->d_name;
            g_debug("Considering Loading `%s'", filename);
            
            if(stat(filename, &stats) == 0)
            {
                if(S_ISREG(stats.st_mode) && contains_only_nums(filename))
                {
                    g_debug("Loading %s", filename);
                    
                    if(!gnote_load_xml(filename))
                    {
                        if(!gnote_load_old(filename))
                        {
                            gchar *newfilename = g_strdup_printf("%sbad_file",
                                                                 filename);
                            g_warning("Gnotes: gnotes_load(): file not in "
                                      "old or new(xml) format.  Renameing "
                                      "from %s to %s", filename, newfilename);
                            rename(filename, newfilename);
                            g_free(newfilename);
                        }
                    }
                }
                else
                {
                    g_debug("not regular file or contains more than nums");
                }
            }
            else
            {
                g_debug("Bad stat return");
            }
        }
        closedir(dptr);
    }
    else
    {
        g_warning("GNotes: gnotes_load(): cannot open directory %s",
                get_gnotes_dir());
    }
}

static gboolean gnote_load_xml(const gchar *filename)
{
    gchar *buffer;
    struct stat stats;
    gint fdes = -1;
    xmlDocPtr doc;
    gchar *tmp_string1;
    gboolean ret_val = TRUE;
    
    if(stat(filename, &stats) == 0)
    {
        buffer = (gchar *)g_malloc(stats.st_size + 1);

        if((fdes = open(filename, O_RDONLY)) >= 0)
        {
            if(read(fdes, buffer, stats.st_size) != stats.st_size)
            {
                g_warning("Gnotes: gnote_load_xml(%s): error reading %ld bytes",
                        filename, stats.st_size);
                ret_val = FALSE;
            }
            else
            {
                buffer[stats.st_size] = '\0';
                
                doc = xmlParseMemory(buffer, stats.st_size);

                if((doc == 0) || (doc->root == 0))
                {
                    return FALSE;
                }
                
                tmp_string1 = xmlGetProp(doc->root, gnotes_version_property_name);
                
                if(gnotes_file_format && tmp_string1 && strcmp(gnotes_file_format, tmp_string1) == 0)
                {
                    ret_val = gnote_load_xml_v10(doc, filename);
                }
                else
                {
                    g_warning("GNotes: gnote_load_xml(%s): unknown file format: %s",
                              filename, tmp_string1);
                    ret_val = FALSE;
                }
                g_free(tmp_string1);
                xmlFreeDoc(doc);
            }
        }
        if(fdes != -1)
            close(fdes);
        g_free(buffer);
    }
    else
    {
        g_warning("Gnotes: gnote_load_xml(%s): error stating file", filename);
        ret_val = FALSE;
    }

    return ret_val;
}

static gboolean gnote_load_xml_v10(xmlDocPtr doc, const gchar *filename)
{
    gboolean ret_val = TRUE;
    gint width;
    gint height;
    gint x;
    gint y;
    gboolean hidden;
    gchar *text = "";
    time_t timestamp;
    gchar *title = "";
    gchar *type = "";
    gchar *tmp_string;
    xmlNodePtr tmp_node = NULL;
     
    tmp_string = xmlGetProp(doc->root, gnotes_id_property_name);
    if(tmp_string == 0)
    {
        g_warning("GNotes: gnote_load_xml_v10(): no id property");
        ret_val = FALSE;
    }

    if(ret_val)
    {
        if(filename && strcmp(filename, tmp_string) != 0)
        {
            g_warning("Gnotes: gnote_load_xml_v10(): filename doesn't match "
                      "id property.  Resetting id to filename");
        }
        /* this has a problem of not knowing how large a time_t is (I think) */
        /* y2038 problem? */
        sscanf(filename, "%ld", &timestamp);

    }

    if(ret_val)
    {
        /* title */
        tmp_node = doc->root->childs;

        if(tmp_node == 0)
        {
            ret_val = FALSE;
        }
        else if((tmp_node->name && strcmp(tmp_node->name, gnotes_title_node_name) == 0) &&
                (tmp_node->childs != 0))
        {
            title = tmp_node->childs->content;
        }
        else
        {
            g_warning("GNotes: gnote_load_xml_v10(): title not apparent");
            ret_val = FALSE;
        }
    }

    if(ret_val)
    {
        /* location */
        tmp_node = tmp_node->next;
        
        if((tmp_node != 0) && (tmp_node->name && strcmp(tmp_node->name, gnotes_loc_node_name) == 0))
        {
            xmlNodePtr start_node = tmp_node->childs;
            xmlNodePtr intmp_node = tmp_node->childs;

            while((intmp_node != 0) && ret_val)
            {
                if(intmp_node->name == 0)
                {
                    /* ok. */
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_x_node_name) == 0)
                {
                    if((intmp_node->childs == 0) ||
                       sscanf(intmp_node->childs->content, "%d", &x) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad X format");
                        ret_val = FALSE;
                    }
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_y_node_name) == 0)
                {
                    if((intmp_node->childs == 0) ||
                       sscanf(intmp_node->childs->content, "%d", &y) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad Y format");
                        ret_val = FALSE;
                    }
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_height_node_name) == 0)
                {
                    if((intmp_node->childs == 0) ||
                       sscanf(intmp_node->childs->content, "%d", &height) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad height format");
                        ret_val = FALSE;
                    }
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_width_node_name) == 0)
                {
                    if((intmp_node->childs == 0) ||
                       sscanf(intmp_node->childs->content, "%d", &width) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad width format");
                        ret_val = FALSE;
                    }
                }
                intmp_node = intmp_node->next;
                if(intmp_node == start_node)
                {
                    intmp_node = 0;
                }
            }
        }
        else
        {
            g_warning("GNotes: gnote_load_xml_v10(): location data flakey");
            ret_val = FALSE;
        }
    }

    if(ret_val)
    {
        /* state (including hidden) */
        tmp_node = tmp_node->next;
        
        if((tmp_node != 0) && (tmp_node->name && strcmp(tmp_node->name, gnotes_state_node_name) == 0))
        {
            xmlNodePtr start_node = tmp_node->childs;
            xmlNodePtr intmp_node = tmp_node->childs;

            while((intmp_node != 0) && ret_val)
            {
                if(intmp_node->name == 0)
                {
                    /* ignore this.  It's OK (I think) */
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_hidden_node_name) == 0)
                {
                    if((intmp_node->childs == 0) ||
                       sscanf(intmp_node->childs->content, "%d", &hidden) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad hidden format");
                        ret_val = FALSE;
                    }
                }
                intmp_node = intmp_node->next;
                if(intmp_node == start_node)
                {
                    intmp_node = 0;
                }
            }
        }
        else
        {
            g_warning("GNotes: gnote_load_xml_v10(): state data flakey");
            ret_val = FALSE;
        }
    }
    

    if(ret_val)
    {
        /* contents (including type and data */
        tmp_node = tmp_node->next;

        if((tmp_node != 0) && (tmp_node->name && strcmp(tmp_node->name, gnotes_contents_node_name) == 0))
        {
            xmlNodePtr start_node = tmp_node->childs;
            xmlNodePtr intmp_node = tmp_node->childs;

            while((intmp_node != 0) && ret_val)
            {
                if(intmp_node->name == 0)
                {
                    /* this is an ok situation.  Empty data is normal */
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_data_type_node_name) == 0)
                {
                    if(intmp_node->childs != 0)
                    {
                        type = intmp_node->childs->content;
                    }
                    else
                    {
                        type = 0;
                    }
                }
                else if(intmp_node->name && strcmp(intmp_node->name, gnotes_data_node_name) == 0)
                {
                    if(intmp_node->childs != 0)
                    {
                        text = intmp_node->childs->content;
                    }
                    else
                    {
                        text = "";
                    }
                }
                intmp_node = intmp_node->next;
                if(intmp_node == start_node)
                {
                    intmp_node = 0;
                }
            }
        }
        else
        {
            g_warning("GNotes: gnote_load_xml_v10(): data flakey");
            ret_val = FALSE;
        }
    }

    if(ret_val)
    {
        gnote_new(width, height, x, y, hidden, text, timestamp, title, TRUE, type);
    }
    
    return ret_val;
}


static gboolean gnote_load_old(const char *filename)
{
    FILE *fptr;
    gboolean ret_val = TRUE;
    
    if((fptr = fopen(filename, "r")) != 0)
    {
        gint width;
        gint height;
        gint x;
        gint y;
        gint hidden, index, current_size;
        gchar *string, letter, *title, *format;
        time_t timestamp;

                /* read in file format indicator */
        current_size = 16;
        format = (gchar *)g_malloc(current_size);
        index = 0;
        letter = fgetc(fptr);
        while (!feof(fptr) && !ferror(fptr) && (letter != '\n'))
        {
            format[index++] = letter;
            if(index + 1 >= current_size)
            {
                gchar *tmp_format =
                    (gchar*)g_realloc(format, current_size * 2);
                if(tmp_format == 0)
                {
                    break;
                }
                current_size *= 2;
                format = tmp_format;
            }
            letter = fgetc(fptr);
        }
        /* FIXME: is this correct? */
        format[index + 1] = '\0';
                
        /* if we have a proper GNOTE file, continue */
        if (!strncmp(GNOTE_FORMAT, format, strlen(GNOTE_FORMAT)))
        {
            if ((timestamp=atol(filename)) == 0)
            {
                timestamp = time(NULL);
            }

            /* read in geometry, etc */
            fscanf(fptr, "%dx%d+%d+%d %d\n", &width, &height, &x, &y,
                   &hidden);

            current_size = 16;
            title = (gchar *)g_malloc(current_size);
            index = 0;
            letter = fgetc(fptr);
            while (!feof(fptr) && !ferror(fptr) && (letter != '\n'))
            {
                title[index++] = letter;
                if(index + 1 >= current_size)
                {
                    gchar *tmp_title =
                        (gchar*)g_realloc(title, current_size * 2);
                    if(tmp_title == 0)
                    {
                        break;
                    }
                    current_size *= 2;
                    title = tmp_title;
                }
                letter = fgetc(fptr);
            }
            /* FIXME: is this correct? */
            title[index + 1] = '\0';

            current_size = 16;
            string = (gchar *)g_malloc(current_size);
            index = 0;
            letter = fgetc(fptr);
            while (!feof(fptr) && !ferror(fptr))
            {
                string[index++] = letter;
                if(index + 1 >= current_size)
                {
                    gchar *tmp_string =
                        (gchar*)g_realloc(string, current_size * 2);
                    if(tmp_string == 0)
                    {
                        break;
                    }
                    current_size *= 2;
                    string = tmp_string;
                }
                letter = fgetc(fptr);
            }
            string[index + 1] = '\0';
                    
            gnote_new(width, height, x, y, (gboolean)hidden, string,
                      timestamp, title, TRUE, "");
            
            g_free(title);
            g_free(string);
        }
        else
        {
            ret_val = FALSE;
        }
        g_free(format);
    }
    if(fptr != 0)
        fclose(fptr);

    return ret_val;
}
    
