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

void gnotes_init();
void gnote_signal_handler(int);
void gnote_destroy(gpointer);
void gnotes_raise(AppletWidget*, gpointer);
void gnote_raise(gpointer);
void gnotes_lower(AppletWidget*, gpointer);
void gnote_lower(gpointer);
void gnotes_show(AppletWidget*, gpointer);
void gnote_show(gpointer);
void gnotes_hide(AppletWidget*, gpointer);
void gnote_hide(gpointer);
void gnotes_save(AppletWidget*, gpointer);
void gnote_save(gpointer);
void gnotes_load(AppletWidget*, gpointer);
static gboolean gnote_load_xml(const gchar *);
static gboolean gnote_load_xml_v10(xmlDocPtr);
static gboolean gnote_load_old(const gchar *);

static void do_on_all_notes(GPtrOperator oper)
{
    int i;

    if(note_list == 0)
    {
        return;
    }

    g_debug("Working on %d notes.\n", note_list->len);
    
    for(i = 0;i < note_list->len; i++)
    {
        g_debug("Doing %d in list\n", i);
        (*oper)(g_ptr_array_index(note_list, i));
    }
}
        
const gchar *get_gnotes_dir()
{
    if(save_dir_name == 0)
    {
        save_dir_name = g_strdup_printf("%s/%s", gnome_util_user_home(), GNOTES_DIR);
        g_debug("Setting save directory to %s\n", save_dir_name);
    }
    
    return save_dir_name;
}

gchar *make_gnote_filename(GNote* note)
{
    gchar* ret;
    gchar* file_part;

    file_part = g_strdup_printf("%ld", note->timestamp);
    
    ret = g_strdup_printf("%s/%s", get_gnotes_dir(), file_part);

    g_free(file_part);
    
    return ret;
}


void gnotes_init()
{
    note_list = g_ptr_array_new();
}

void gnote_signal_handler(int signal)
{
    switch (signal)
    {
    case SIGHUP:   /* zap all the notes and reload 'em */
        do_on_all_notes(&gnote_destroy);
        gnotes_load(0, 0);
        break;
    default:
        printf("gnote: SIGNAL %d received\n", signal);
        break;
    };
};

void gnote_destroy(gpointer note)
{
    gtk_widget_destroy(((GNote*)note)->window);
}


static gint gnote_changed_cb(GtkWidget *text, GdkEventButton *event,
                             gpointer data)
{
    int index=0;

    while ((index < note_list->len) &&
           (text != ((GNote*)g_ptr_array_index(note_list, index))->text))
        index++;
    ((GNote*)g_ptr_array_index(note_list, index))->timestamp = time(NULL);
    return(TRUE);
};


gint gnote_delete_cb(GtkWidget *widget, gpointer handle_boxptr)
{
    int index;

    g_debug("gnote_delete_cb();");

    for (index=0; index < note_list->len; index++)
    {
        GNote* the_note = (GNote*)g_ptr_array_index(note_list, index);
        if (handle_boxptr == the_note->handle_box)
        {
            char *fname = make_gnote_filename(the_note);
            g_debug("  deleted [%s]", fname);
            unlink(fname);
            free(fname);

            gtk_widget_destroy(the_note->window);
            g_free(the_note->title);

            g_ptr_array_remove_fast(note_list, the_note);

            g_free(the_note);

            /* we don't need to keep looking.  We found it */
            break;
        };
    };

    return(FALSE);
};


void gnote_menu(GtkWidget *handle_box, GdkEventButton *event)
{
    GtkWidget *menu;
    GtkWidget *menuitem;
  
    g_debug("gnote_menu();");

    menu = gtk_menu_new();

    {
        int index=0;
        time_t timestamp;
        char *timestr;
        GNote* the_note;
        
        timestamp = time(NULL);
        while ((index < note_list->len) &&
               (handle_box !=
                ((GNote*)g_ptr_array_index(note_list, index))->handle_box))
            index++;

        the_note = (GNote*)g_ptr_array_index(note_list, index);
        
        if (the_note->title == 0)
        {
            the_note->title = g_strdup("GNotes!");
        };

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

 
    menuitem = gtk_menu_item_new_with_label(_("Raise Notes"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnotes_raise), 0);
    gtk_menu_append(GTK_MENU(menu), menuitem);
  
    menuitem = gtk_menu_item_new_with_label(_("Lower Notes"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnotes_lower), 0);
    gtk_menu_append(GTK_MENU(menu), menuitem);
  
    menuitem = gtk_menu_item_new_with_label(_("Hide Notes"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnotes_hide), 0);
    gtk_menu_append(GTK_MENU(menu), menuitem);

    menuitem = gtk_menu_item_new_with_label(_("Delete Note"));
    gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                       GTK_SIGNAL_FUNC(gnote_delete_cb), handle_box);
    gtk_menu_append(GTK_MENU(menu), menuitem);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button,
                   event->time);
};

static gint ptr_x=0, ptr_y=0;

gint gnote_motion_cb(GtkWidget *widget, GdkEventButton *event,
                     gpointer data)
{
    gint win_x=0, win_y=0, win_w=0, win_h=0, new_ptr_x=0, new_ptr_y=0;

    gdk_window_get_origin(widget->parent->window, &win_x, &win_y);
    gdk_window_get_pointer(widget->parent->window, &new_ptr_x, &new_ptr_y,
                           NULL);
    gdk_window_get_size(widget->parent->window, &win_w, &win_h);
  
    /* new position minus the pointer position in the widget */
    win_x += new_ptr_x - ptr_x;
    win_y += new_ptr_y - ptr_y;

    /* set new size */
    win_w = new_ptr_x;
    win_h = new_ptr_y;

    gdk_window_raise(widget->parent->window);

    if (event->state & ShiftMask)
    {
        int index;

        for (index=0; index < note_list->len; index++)
        {
            GNote* the_note = (GNote*)g_ptr_array_index(note_list, index);
            
            if (widget == the_note->handle_box)
            {
            	gtk_widget_set_usize(the_note->window, win_w, win_h);
            	break;
            };
        };
    }
    else
    {
        gdk_window_move(widget->parent->window, win_x, win_y);
    };

    return(TRUE);
};

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
            gnote_new_cb(NULL, (gpointer)GNOTE_NEW_1x1);
            break;
        case 3:
            return(FALSE);
        };
        cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
        gdk_window_set_cursor(widget->window, cursor);
        gdk_cursor_destroy(cursor);
        break;
    default:
        g_debug("Unhandled type in gnote_handle_button_cb(): %d\n", event->type);
        break;
    };

    return(TRUE);
};


void gnote_new(gint width, gint height, gint x, gint y, gboolean hidden,
               const gchar *text, time_t timestamp,
               const gchar *title, gboolean loaded_from_file,
               const gchar *type)
{
    GNote* the_note = (GNote*)g_malloc(sizeof(GNote));

    g_ptr_array_add(note_list, the_note);
    
    the_note->hidden = hidden;
    the_note->timestamp = timestamp;
    the_note->title = g_strdup(title);
    the_note->already_saved = loaded_from_file;
    the_note->type = g_strdup(type);
    
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
    gtk_text_set_word_wrap(GTK_TEXT(the_note->text), TRUE);
    gtk_signal_connect(GTK_OBJECT(the_note->text), "changed",
                       GTK_SIGNAL_FUNC(gnote_changed_cb), NULL);
    if (text != NULL)
    {
        gtk_text_insert(GTK_TEXT(the_note->text), NULL,
                        NULL, NULL, text, strlen(text));
    }

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

        style = gtk_style_new();
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

    gtk_widget_show_all(the_note->window);

    gdk_window_set_decorations(the_note->window->window, 0);
    gtk_window_set_focus(GTK_WINDOW(the_note->window),
                         the_note->text);

    {
        GdkCursor *cursor;

        cursor = gdk_cursor_new(GDK_XTERM);
        gdk_window_set_cursor(the_note->text->window, cursor);
        gdk_cursor_destroy(cursor);
    };

    /* heinous hack follows... for some reason, even though I tell it
       not to, the window manager gives the gnote a border... so I found
       out that if I hide it then reshow it, the border goes away  :-O  */
    gdk_window_hide(the_note->window->window);
    /* only reshow it if we're supposed to */
    if (the_note->hidden == FALSE)
    {
        gdk_window_show(the_note->window->window);
        /* gtk_widget_set_uposition(the_note->window, the_note->x, */
        /* the_note->y); */
    }
    else
    {
        gdk_window_hide(the_note->window->window);
    }
}

void gnote_new_cb(AppletWidget *applet, gpointer data)
{
    char *sdata;
    gint width;
    gint height;

    sdata = g_strdup((gchar *)data);
    sprintf(sdata, "%s", (char *)data);

    if(strcmp(sdata, GNOTE_NEW_DEFAULT) == 0)
    {
        width = gnotes.default_width;
        height = gnotes.default_height;
    }
    else
    {
        width = atoi(strtok(sdata, "x")) * 100;
        height = atoi(strtok(NULL, "x")) * 100;
    }

    gnote_new(width, height, -1, -1, FALSE, "", time(NULL), "GNotes!", FALSE, "");

    g_free(sdata);
};

void gnotes_raise(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_raise);
}

void gnote_raise(gpointer note)
{
    gdk_window_raise(((GNote*)note)->window->window);
}

void gnotes_lower(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_lower);
}

void gnote_lower(gpointer note)
{
    gdk_window_lower(((GNote*)note)->window->window);
}

void gnotes_show(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_show);
}

void gnote_show(gpointer prenote)
{
    GNote *note = prenote;
    gdk_window_show(note->window->window);
    /* gtk_widget_set_uposition(note->window, note->x, */
    /* note->y); */
}

void gnotes_hide(AppletWidget *wid, gpointer data)
{
    do_on_all_notes(&gnote_hide);
}

void gnote_hide_cb(GtkWidget *widget, gpointer gnote)
{
    gnote_hide(gnote);
}

void gnote_hide(gpointer note)
{
    gdk_window_hide(((GNote*)note)->window->window);
}

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
    int width, height;
    
    the_note = prenote;
    

    /*
      make sure file doesn't already exist - since this is only
      possible with NEW notes, we change the filename too so it
      doesn't get unlinked below
    */
    fname = make_gnote_filename(the_note);

    while (!the_note->already_saved && !stat(fname, &stats))
    {
        the_note->timestamp++;
        g_free(fname);
        fname = make_gnote_filename(the_note);
        the_note->already_saved = TRUE;
    }

    g_debug("Saving into %s\n", fname);
    fflush(stdout);
    fflush(stderr);
    
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

        xmlNewChild(tree, 0, gnotes_data_node_name,
                    GTK_TEXT(the_note->text)->text.ch);

        xmlDocDump(fptr, doc);

        xmlFreeDoc(doc);
        fclose(fptr);
        /* g_free(fptr); */
    }
    else
    {
        g_warning("GNotes: gnotes_save(): cannot open file %s", fname);
    }
    g_free(fname);
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
            g_debug("Considering Loading `%s'\n", filename);
            
            if(stat(filename, &stats) == 0)
            {
                if(S_ISREG(stats.st_mode) && contains_only_nums(filename))
                {
                    g_debug("Loading %s\n", filename);
                    
                    if(!gnote_load_xml(filename))
                    {
                        if(!gnote_load_old(filename))
                        {
                            gchar *newfilename = g_strdup_printf("%sbad_file",
                                                                 filename);
                            g_warning("Gnotes: gnotes_load(): file not in "
                                      "old or new(xml) format.  Renameing "
                                      "from %s to %s\n", filename, newfilename);
                            rename(filename, newfilename);
                            g_free(newfilename);
                        }
                    }
                }
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

static gboolean gnote_load_xml(const char *filename)
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
                g_warning("Gnotes: gnote_load_xml(%s): error reading %ld bytes\n",
                        filename, stats.st_size);
                ret_val = FALSE;
            }
            else
            {
                buffer[stats.st_size] = '\0';
                
                doc = xmlParseMemory(buffer, stats.st_size);
                
                tmp_string1 = xmlGetProp(doc->root, gnotes_version_property_name);
                
                if(strcmp(gnotes_file_format, tmp_string1) == 0)
                {
                    ret_val = gnote_load_xml_v10(doc);
                }
                else
                {
                    g_warning("GNotes: gnote_load_xml(%s): unknown file format: %s\n",
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

static gboolean gnote_load_xml_v10(xmlDocPtr doc)
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
    xmlNodePtr tmp_node;
    
    tmp_string = xmlGetProp(doc->root, gnotes_id_property_name);
    if(tmp_string == 0)
    {
        g_warning("GNotes: gnote_load_xml_v10(): no id property");
        ret_val = FALSE;
    }

    if(ret_val)
    {
        /* this has a problem of not knowing how large a time_t is (I think) */
        /* y2038 problem? */
        sscanf(tmp_string, "%ld", &timestamp);
    }

    if(ret_val)
    {
        /* title */
        tmp_node = doc->root->childs;

        if(strcmp(tmp_node->name, gnotes_title_node_name) == 0)
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
        
        if((tmp_node != 0) && (strcmp(tmp_node->name, gnotes_loc_node_name) == 0))
        {
            xmlNodePtr start_node = tmp_node->childs;
            xmlNodePtr intmp_node = tmp_node->childs;

            while((intmp_node != 0) && ret_val)
            {
                if(strcmp(intmp_node->name, gnotes_x_node_name) == 0)
                {
                    if(sscanf(intmp_node->childs->content, "%d", &x) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad X format (%s)\n",
                                  intmp_node->childs->content);
                        ret_val = FALSE;
                    }
                }
                else if(strcmp(intmp_node->name, gnotes_y_node_name) == 0)
                {
                    if(sscanf(intmp_node->childs->content, "%d", &y) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad Y format (%s)\n",
                                  intmp_node->childs->content);
                        ret_val = FALSE;
                    }
                }
                else if(strcmp(intmp_node->name, gnotes_height_node_name) == 0)
                {
                    if(sscanf(intmp_node->childs->content, "%d", &height) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad height format (%s)\n",
                                  intmp_node->childs->content);
                        ret_val = FALSE;
                    }
                }
                else if(strcmp(intmp_node->name, gnotes_width_node_name) == 0)
                {
                    if(sscanf(intmp_node->childs->content, "%d", &width) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad width format (%s)\n",
                                  intmp_node->childs->content);
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
            g_warning("GNotes: gnote_load_xml_v10(): location data flakey: %s\n",
                      tmp_node->name);
            ret_val = FALSE;
        }
    }

    if(ret_val)
    {
        /* state (including hidden) */
        tmp_node = tmp_node->next;
        
        if((tmp_node != 0) && (strcmp(tmp_node->name, gnotes_state_node_name) == 0))
        {
            xmlNodePtr start_node = tmp_node->childs;
            xmlNodePtr intmp_node = tmp_node->childs;

            while((intmp_node != 0) && ret_val)
            {
                if(strcmp(intmp_node->name, gnotes_hidden_node_name) == 0)
                {
                    if(sscanf(intmp_node->childs->content, "%d", &hidden) != 1)
                    {
                        g_warning("GNotes: gnote_load_xml_v10(): bad hidden format (%s)\n",
                                  intmp_node->childs->content);
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

        if((tmp_node != 0) && (strcmp(tmp_node->name, gnotes_contents_node_name) == 0))
        {
            xmlNodePtr start_node = tmp_node->childs;
            xmlNodePtr intmp_node = tmp_node->childs;

            while((intmp_node != 0) && ret_val)
            {
                if(strcmp(intmp_node->name, gnotes_data_type_node_name) == 0)
                {
                    type = intmp_node->childs->content;
                }
                else if(strcmp(intmp_node->name, gnotes_data_node_name) == 0)
                {
                    text = intmp_node->childs->content;
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
        // FIXME: is this correct?
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
            // FIXME: is this correct?
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
    
