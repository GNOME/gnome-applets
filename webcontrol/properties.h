/* Webcontrol properties dialog and actions */

#ifndef __PROPERTIES_H_
#define __PROPERTIES_H_

#include <gnome.h>
#include <applet-widget.h>

#define B_LIST_NAME(b) ((browser *) b->data)->name
#define B_LIST_COMMAND(b) ((browser *) b->data)->command
#define B_LIST_NEWWIN(b) ((browser *) b->data)->newwin
#define B_LIST_NO_NEWWIN(b) ((browser *) b->data)->no_newwin

#define WC_CFG_OP_ADD 0
#define WC_CFG_OP_EDIT 1

typedef struct _browser
{
	gchar *name;
	gchar *command;
	gchar *newwin;
	gchar *no_newwin;
} browser;

typedef struct _cfg_op
{
	gint operation;
	GtkWidget *list;
} cfg_op;

typedef struct _webcontrol_properties 
{
	gboolean newwindow;		/* do we launch a new window */
	gboolean show_check;		/* do we show "launch new window" */
	gboolean show_go;               /* show the "GO" button */
        gint width;                     /* horizontal size of applet */
	gboolean show_clear;            /* show the "Clear" button */
	gboolean clear_top;             /* clear button on top? */
	gint hist_len;                  /* length of URL history */
	gboolean use_mime;              /* use MIME handler or custom */
	GSList *curr_browser;           /* current browser to launch */
	GSList *browsers;               /* list of installed browsers */
} webcontrol_properties;

extern void
properties_box (AppletWidget *widget, gpointer data);

extern void 
check_box_toggled (GtkWidget *check, int *data);

#endif





