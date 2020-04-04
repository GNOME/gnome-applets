/*
 * Copyright 2003, 2004 Sun Microsystems Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "accessx-status-applet.h"

#include <stdlib.h>
#include <string.h>

#include <glib/gi18n-lib.h>
#include <glib-object.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <X11/XKBlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

static int xkb_base_event_type = 0;

static GtkIconSize icon_size_spec;

#define ALT_GRAPH_LED_MASK (0x10)

#define ACCESSX_APPLET "ax-applet"
#define ACCESSX_BASE_ICON "ax-base"
#define ACCESSX_ACCEPT_BASE "ax-accept"
#define ACCESSX_REJECT_BASE "ax-reject"
#define MOUSEKEYS_BASE_ICON "ax-mouse-base"
#define MOUSEKEYS_BUTTON_LEFT "ax-button-left"
#define MOUSEKEYS_BUTTON_MIDDLE "ax-button-middle"
#define MOUSEKEYS_BUTTON_RIGHT "ax-button-right"
#define MOUSEKEYS_DOT_LEFT "ax-dot-left"
#define MOUSEKEYS_DOT_MIDDLE "ax-dot-middle"
#define MOUSEKEYS_DOT_RIGHT "ax-dot-right"
#define SHIFT_KEY_ICON "ax-shift-key"
#define CONTROL_KEY_ICON "ax-control-key"
#define ALT_KEY_ICON "ax-alt-key"
#define META_KEY_ICON "ax-meta-key"
#define SUPER_KEY_ICON "ax-super-key"
#define HYPER_KEY_ICON "ax-hyper-key"
#define ALTGRAPH_KEY_ICON "ax-altgraph-key"
#define SLOWKEYS_IDLE_ICON "ax-sk-idle"
#define SLOWKEYS_PENDING_ICON "ax-sk-pending"
#define SLOWKEYS_ACCEPT_ICON "ax-sk-accept"
#define SLOWKEYS_REJECT_ICON "ax-sk-reject"
#define BOUNCEKEYS_ICON "ax-bouncekeys"

typedef struct
{
	const char *stock_id;
	const char *name;
	GtkStateType state;
	gboolean     wildcarded;
} AppletStockIcon;

typedef enum
{
  ACCESSX_STATUS_ERROR_NONE = 0,
  ACCESSX_STATUS_ERROR_XKB_DISABLED
} AccessxStatusErrorType;

typedef enum
{
  ACCESSX_STATUS_MODIFIERS = 1 << 0,
  ACCESSX_STATUS_SLOWKEYS = 1 << 1,
  ACCESSX_STATUS_BOUNCEKEYS = 1 << 2,
  ACCESSX_STATUS_MOUSEKEYS = 1 << 3,
  ACCESSX_STATUS_ENABLED = 1 << 4,
  ACCESSX_STATUS_ALL = 0xFFFF
} AccessxStatusNotifyType;

struct _AccessxStatusApplet
{
  GpApplet                parent;

  GtkWidget              *box;
  GtkWidget              *idlefoo;
  GtkWidget              *mousefoo;
  GtkWidget              *stickyfoo;
  GtkWidget              *slowfoo;
  GtkWidget              *bouncefoo;
  GtkWidget              *shift_indicator;
  GtkWidget              *ctrl_indicator;
  GtkWidget              *alt_indicator;
  GtkWidget              *meta_indicator;
  GtkWidget              *hyper_indicator;
  GtkWidget              *super_indicator;
  GtkWidget              *alt_graph_indicator;
  GtkIconFactory         *icon_factory;
  XkbDescRec             *xkb;
  Display                *xkb_display;
  AccessxStatusErrorType  error_type;
  gint                    size;
};

G_DEFINE_TYPE (AccessxStatusApplet, accessx_status_applet, GP_TYPE_APPLET)

static AppletStockIcon stock_icons [] = {
        { ACCESSX_APPLET, "ax-applet.png", GTK_STATE_NORMAL, True },
        { ACCESSX_BASE_ICON, "ax-key-base.png", GTK_STATE_NORMAL, True },
        { ACCESSX_BASE_ICON, "ax-key-none.png", GTK_STATE_INSENSITIVE, False },
        { ACCESSX_BASE_ICON, "ax-key-inverse.png", GTK_STATE_SELECTED, False },
        { ACCESSX_ACCEPT_BASE, "ax-key-yes.png", GTK_STATE_NORMAL, True },
        { ACCESSX_REJECT_BASE, "ax-key-no.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_BASE_ICON, "mousekeys-base.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_BUTTON_LEFT, "mousekeys-pressed-left.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_BUTTON_MIDDLE, "mousekeys-pressed-middle.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_BUTTON_RIGHT, "mousekeys-pressed-right.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_DOT_LEFT, "mousekeys-default-left.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_DOT_MIDDLE, "mousekeys-default-middle.png", GTK_STATE_NORMAL, True },
        { MOUSEKEYS_DOT_RIGHT, "mousekeys-default-right.png", GTK_STATE_NORMAL, True },
        { SHIFT_KEY_ICON, "sticky-shift-latched.png", GTK_STATE_NORMAL, False },
        { SHIFT_KEY_ICON, "sticky-shift-locked.png", GTK_STATE_SELECTED, False },
        { SHIFT_KEY_ICON, "sticky-shift-none.png", GTK_STATE_INSENSITIVE, True },
        { CONTROL_KEY_ICON, "sticky-ctrl-latched.png", GTK_STATE_NORMAL, False },
        { CONTROL_KEY_ICON, "sticky-ctrl-locked.png", GTK_STATE_SELECTED, False },
        { CONTROL_KEY_ICON, "sticky-ctrl-none.png", GTK_STATE_INSENSITIVE, True },
        { ALT_KEY_ICON, "sticky-alt-latched.png", GTK_STATE_NORMAL, False },
        { ALT_KEY_ICON, "sticky-alt-locked.png", GTK_STATE_SELECTED, False },
        { ALT_KEY_ICON, "sticky-alt-none.png", GTK_STATE_INSENSITIVE, True },
        { META_KEY_ICON, "sticky-meta-latched.png", GTK_STATE_NORMAL, False },
        { META_KEY_ICON, "sticky-meta-locked.png", GTK_STATE_SELECTED, False },
        { META_KEY_ICON, "sticky-meta-none.png", GTK_STATE_INSENSITIVE, True },
        { SUPER_KEY_ICON, "sticky-super-latched.png", GTK_STATE_NORMAL, False },
        { SUPER_KEY_ICON, "sticky-super-locked.png", GTK_STATE_SELECTED, False },
        { SUPER_KEY_ICON, "sticky-super-none.png", GTK_STATE_INSENSITIVE, True },
        { HYPER_KEY_ICON, "sticky-hyper-latched.png", GTK_STATE_NORMAL, False },
        { HYPER_KEY_ICON, "sticky-hyper-locked.png", GTK_STATE_SELECTED, False },
        { HYPER_KEY_ICON, "sticky-hyper-none.png", GTK_STATE_INSENSITIVE, True },
        { SLOWKEYS_IDLE_ICON, "ax-slowkeys.png", GTK_STATE_NORMAL, True },
        { SLOWKEYS_PENDING_ICON, "ax-slowkeys-pending.png", GTK_STATE_NORMAL, True },
        { SLOWKEYS_ACCEPT_ICON, "ax-slowkeys-yes.png", GTK_STATE_NORMAL, True },
        { SLOWKEYS_REJECT_ICON, "ax-slowkeys-no.png", GTK_STATE_NORMAL, True },
        { BOUNCEKEYS_ICON, "ax-bouncekeys.png", GTK_STATE_NORMAL, True }
};

typedef struct {
	unsigned int mask;
	GtkWidget   *indicator;
} ModifierStruct;

static ModifierStruct modifiers [] = {
	{ShiftMask, NULL},
	{ControlMask, NULL},
	{Mod1Mask, NULL},
	{Mod2Mask, NULL},
	{Mod3Mask, NULL},
	{Mod4Mask, NULL},
	{Mod5Mask, NULL}
};

typedef struct {
	unsigned int mask;
	const char *stock_id;
} ButtonIconStruct;

static ButtonIconStruct button_icons [] = {
	{Button1Mask, MOUSEKEYS_BUTTON_LEFT},
	{Button2Mask, MOUSEKEYS_BUTTON_MIDDLE},
	{Button3Mask, MOUSEKEYS_BUTTON_RIGHT}
};

static void popup_error_dialog (AccessxStatusApplet        *sapplet);

/* cribbed from geyes */
static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static void
help_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static void
dialog_cb (GSimpleAction *action,
           GVariant      *parameter,
           gpointer       user_data)
{
	AccessxStatusApplet *sapplet = (AccessxStatusApplet *) user_data;
	GError *error = NULL;
	GdkScreen *screen;
	GdkAppLaunchContext *launch_context;
	GAppInfo *appinfo;

	if (sapplet->error_type != ACCESSX_STATUS_ERROR_NONE) {
		popup_error_dialog (sapplet);
		return;
	}

	screen = gtk_widget_get_screen (GTK_WIDGET (sapplet));
	appinfo = g_app_info_create_from_commandline ("gnome-control-center universal-access",
						      _("Open the universal access preferences dialog"),
						      G_APP_INFO_CREATE_NONE,
						      &error);

	if (!error) {
		launch_context = gdk_app_launch_context_new ();
		gdk_app_launch_context_set_screen (launch_context, screen);
		g_app_info_launch (appinfo, NULL, G_APP_LAUNCH_CONTEXT (launch_context), &error);

		g_object_unref (launch_context);
	}

	if (error != NULL) {
		GtkWidget *dialog = 
			gtk_message_dialog_new (NULL,
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_CLOSE,
						_("There was an error launching the keyboard preferences dialog: %s"), 
						error->message);

		g_signal_connect (G_OBJECT (dialog),
				  "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);

		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (sapplet)));
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

		gtk_widget_show (dialog);
		g_error_free (error);
	}

	g_object_unref (appinfo);
}

static const GActionEntry accessx_status_applet_menu_actions [] = {
	{ "dialog", dialog_cb, NULL, NULL, NULL },
	{ "help",   help_cb,   NULL, NULL, NULL },
	{ "about",  about_cb,  NULL, NULL, NULL },
	{ NULL }
};

static XkbDescPtr 
accessx_status_applet_get_xkb_desc (AccessxStatusApplet *sapplet)
{
	Display *display;

	if (sapplet->xkb == NULL) {
		int ir, reason_return;
		char *display_name = getenv ("DISPLAY");
		display = XkbOpenDisplay (display_name,
					  &xkb_base_event_type,
					  &ir, NULL, NULL, 
					  &reason_return);
		g_assert (display); /* TODO: change error message below to something user-viewable */
		if (display == NULL)
			g_warning ("Xkb extension could not be initialized! (error code %x)", reason_return);
		else 
			sapplet->xkb = XkbGetMap (display, 
						  XkbAllComponentsMask,
						  XkbUseCoreKbd);
		g_assert (sapplet->xkb);
		if (sapplet->xkb == NULL)
			g_warning ("Xkb keyboard description not available!");
		sapplet->xkb_display = display;
	}
	return sapplet->xkb;
}

static gboolean
accessx_status_applet_xkb_select (AccessxStatusApplet *self)
{
  GdkDisplay *display;
  Display *xdisplay;
  int opcode_rtn;
  int error_rtn;
  gboolean retval;

  display = gtk_widget_get_display (GTK_WIDGET (self));
  xdisplay = gdk_x11_display_get_xdisplay (display);

  retval = XkbQueryExtension (xdisplay,
                              &opcode_rtn,
                              &xkb_base_event_type,
                              &error_rtn,
                              NULL,
                              NULL);

  if (retval)
    {
      retval = XkbSelectEvents (xdisplay,
                                XkbUseCoreKbd,
                                XkbAllEventsMask,
                                XkbAllEventsMask);

      self->xkb = accessx_status_applet_get_xkb_desc (self);
    }
  else
    {
      self->error_type = ACCESSX_STATUS_ERROR_XKB_DISABLED;
    }

  return retval;
}

static void
accessx_status_applet_init_modifiers (AccessxStatusApplet *sapplet) 
{
	unsigned int i;
	unsigned int alt_mask, meta_mask, hyper_mask, super_mask, alt_gr_mask;
	alt_mask = XkbKeysymToModifiers (sapplet->xkb_display, XK_Alt_L);
	meta_mask = XkbKeysymToModifiers (sapplet->xkb_display, XK_Meta_L);

	g_assert (sapplet->meta_indicator);
	if (meta_mask && (meta_mask != alt_mask)) gtk_widget_show (sapplet->meta_indicator);
	else gtk_widget_hide (sapplet->meta_indicator);

	hyper_mask = XkbKeysymToModifiers (sapplet->xkb_display, XK_Hyper_L);
	if (hyper_mask) gtk_widget_show (sapplet->hyper_indicator);
	else gtk_widget_hide (sapplet->hyper_indicator);

	super_mask = XkbKeysymToModifiers (sapplet->xkb_display, XK_Super_L);
	if (super_mask) gtk_widget_show (sapplet->super_indicator);
	else gtk_widget_hide (sapplet->super_indicator);
	alt_gr_mask = XkbKeysymToModifiers (sapplet->xkb_display, XK_Mode_switch) | 
	    XkbKeysymToModifiers (sapplet->xkb_display, XK_ISO_Level3_Shift) |
	    XkbKeysymToModifiers (sapplet->xkb_display, XK_ISO_Level3_Latch) | 
	    XkbKeysymToModifiers (sapplet->xkb_display, XK_ISO_Level3_Lock);

	if (alt_gr_mask) gtk_widget_show (sapplet->alt_graph_indicator);
	else gtk_widget_hide (sapplet->alt_graph_indicator);

	for (i = 0; i < G_N_ELEMENTS (modifiers); ++i) {
		if (modifiers[i].mask == ShiftMask)
			modifiers[i].indicator = sapplet->shift_indicator;
		else if (modifiers[i].mask == ControlMask)
			modifiers[i].indicator = sapplet->ctrl_indicator;
		else if (modifiers[i].mask == alt_mask) 
			modifiers[i].indicator = sapplet->alt_indicator;
		else if (modifiers[i].mask == meta_mask)
			modifiers[i].indicator = sapplet->meta_indicator;
		else if (modifiers[i].mask == hyper_mask)
			modifiers[i].indicator = sapplet->hyper_indicator;
		else if (modifiers[i].mask == super_mask)
			modifiers[i].indicator = sapplet->super_indicator;
		else if (modifiers[i].mask == alt_gr_mask)
			modifiers[i].indicator = sapplet->alt_graph_indicator;
	}
}

static guint _sk_timeout = 0, _bk_timeout = 0;

static gboolean
timer_reset_slowkeys_image (gpointer user_data)
{
	GdkPixbuf *pixbuf = gtk_widget_render_icon (GTK_WIDGET (user_data),
						    SLOWKEYS_IDLE_ICON,
						    icon_size_spec,
						    NULL);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (user_data), pixbuf); 
	g_object_unref (pixbuf);

	_sk_timeout = 0;

	return G_SOURCE_REMOVE;
}

static gboolean
timer_reset_bouncekeys_image (gpointer user_data)
{
	GdkPixbuf *pixbuf = gtk_widget_render_icon (GTK_WIDGET (user_data),
						    BOUNCEKEYS_ICON,
						    icon_size_spec,
						    NULL);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (user_data), pixbuf); 
	g_object_unref (pixbuf);

	_bk_timeout = 0;

	return G_SOURCE_REMOVE;
}

static GdkPixbuf *
accessx_status_applet_get_glyph_pixbuf (AccessxStatusApplet *sapplet,
                                        GtkWidget           *widget,
                                        GdkPixbuf           *base,
                                        GdkColor            *fg,
                                        GdkColor            *bg,
                                        const char          *glyphstring)
{
	GdkPixbuf *glyph_pixbuf;
	cairo_surface_t *surface;
	PangoLayout *layout;
	PangoRectangle ink, logic;
	PangoContext *pango_context;
	gint w = gdk_pixbuf_get_width (base);
	gint h = gdk_pixbuf_get_height (base);
        cairo_t *cr;

        surface = gdk_window_create_similar_surface (gdk_get_default_root_window (),
                                                     CAIRO_CONTENT_COLOR_ALPHA, w, h);
	pango_context = gtk_widget_get_pango_context (widget);
	layout = pango_layout_new (pango_context);
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
	pango_layout_set_text (layout, glyphstring, -1);

        cr = cairo_create (surface);
        gdk_cairo_set_source_color (cr, bg);
        cairo_paint (cr);

        gdk_cairo_set_source_color (cr, fg);
	pango_layout_get_pixel_extents (layout, &ink, &logic);
        cairo_move_to (cr, 
                       (w - ink.x - ink.width)/2, 
                       (h - ink.y - ink.height)/2);
        pango_cairo_show_layout (cr, layout);

        cairo_destroy (cr);
	g_object_unref (layout);

	glyph_pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, w, h);
	cairo_surface_destroy (surface);

	return glyph_pixbuf;
}

static GdkPixbuf *
accessx_status_applet_slowkeys_image (AccessxStatusApplet *sapplet, 
				      XkbAccessXNotifyEvent *event)
{
	GdkPixbuf *ret_pixbuf;
	GdkWindow *window;
	gboolean is_idle = TRUE;
	const char *stock_id = SLOWKEYS_IDLE_ICON;
	GtkStyle *style = gtk_widget_get_style (GTK_WIDGET (sapplet));
	GdkColor bg = style->bg[GTK_STATE_NORMAL];

	if (event != NULL) {
		is_idle = FALSE;
		switch (event->detail) {
		case XkbAXN_SKPress:
			stock_id = ACCESSX_BASE_ICON;
			if (_sk_timeout) {
				g_source_remove (_sk_timeout);
				_sk_timeout = 0;
			}
			break;
		case XkbAXN_SKAccept:
			stock_id = ACCESSX_ACCEPT_BASE;
			gdk_color_parse ("#009900", &bg);
			break;
		case XkbAXN_SKReject:
			stock_id = ACCESSX_REJECT_BASE;
			gdk_color_parse ("#990000", &bg);
			_sk_timeout = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, 
							  MAX (event->sk_delay, 150),
							  timer_reset_slowkeys_image, 
							  sapplet->slowfoo, NULL);
			break;
		case XkbAXN_SKRelease:
		default:
			stock_id = SLOWKEYS_IDLE_ICON;
			is_idle = TRUE;
			break;
		}
	}
	ret_pixbuf = gtk_widget_render_icon (GTK_WIDGET (sapplet),
					     stock_id,
					     icon_size_spec,
					     NULL);
	if (!is_idle) {
		GdkPixbuf *glyph_pixbuf, *tmp_pixbuf;
		GdkColor  fg;
		const char * glyphstring = N_("a");
		tmp_pixbuf = ret_pixbuf;
		ret_pixbuf = gdk_pixbuf_copy (tmp_pixbuf);
		g_object_unref (tmp_pixbuf);

		window = gtk_widget_get_window (GTK_WIDGET (sapplet));

		if (event && window) {
			int keysyms_return;
			KeySym *keysyms;
			KeySym keysym;

			keysyms = XGetKeyboardMapping (GDK_WINDOW_XDISPLAY (window),
			                               event->keycode,
			                               1,
			                               &keysyms_return);

			keysym = keysyms[0];
			XFree (keysyms);

			glyphstring = XKeysymToString (keysym);
			if ((!g_utf8_validate (glyphstring, -1, NULL)) || 
			    (g_utf8_strlen (glyphstring, -1) > 1))
				glyphstring = "";
		}
		fg = style->fg[gtk_widget_get_state (GTK_WIDGET (sapplet))];
		glyph_pixbuf = accessx_status_applet_get_glyph_pixbuf (sapplet, 
								       GTK_WIDGET (sapplet),
								       ret_pixbuf, 
								       &fg,
								       &bg,
								       glyphstring);
		gdk_pixbuf_composite (glyph_pixbuf, ret_pixbuf, 0, 0, 
				      gdk_pixbuf_get_width (glyph_pixbuf),
				      gdk_pixbuf_get_height (glyph_pixbuf), 
				      0., 0., 1.0, 1.0, GDK_INTERP_NEAREST, 255);
		g_object_unref (glyph_pixbuf);
	}
	return ret_pixbuf;
}

static GdkPixbuf *
accessx_status_applet_bouncekeys_image (AccessxStatusApplet *sapplet, XkbAccessXNotifyEvent *event)
{
	GtkStyle  *style;
	GdkColor   fg, bg;
	GdkPixbuf *icon_base = NULL, *tmp_pixbuf;
	/* Note to translators: the first letter of the alphabet, not the indefinite article */
	const char *glyphstring = N_("a");
	const char *stock_id = ACCESSX_BASE_ICON;

	style = gtk_widget_get_style (GTK_WIDGET (sapplet));
	fg = style->text[gtk_widget_get_state (GTK_WIDGET (sapplet))];
	bg = style->base[GTK_STATE_NORMAL];

	if (event != NULL) {
		switch (event->detail) {
		case XkbAXN_BKReject:
			stock_id = SLOWKEYS_REJECT_ICON;
			_bk_timeout = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, 
							  MAX (event->debounce_delay, 150),
							  timer_reset_bouncekeys_image, 
							  sapplet->bouncefoo, NULL); 
			break;
		default:
			stock_id = ACCESSX_BASE_ICON;
			break;
		}
	}
	tmp_pixbuf = gtk_widget_render_icon (GTK_WIDGET (sapplet),
					     stock_id,
					     icon_size_spec,
					     NULL);
	if (tmp_pixbuf) {
		GdkPixbuf   *glyph_pixbuf;
		icon_base = gdk_pixbuf_copy (tmp_pixbuf);
		g_object_unref (tmp_pixbuf);
		glyph_pixbuf = accessx_status_applet_get_glyph_pixbuf (sapplet, 
								       GTK_WIDGET (sapplet),
								       icon_base, 
								       &fg,
								       &bg,
								       glyphstring);
		gdk_pixbuf_composite (glyph_pixbuf, icon_base, 2, 2, 
				      gdk_pixbuf_get_width (glyph_pixbuf) - 2,
				      gdk_pixbuf_get_height (glyph_pixbuf) - 2, 
				      -2., -2., 1.0, 1.0, GDK_INTERP_NEAREST, 96);
		gdk_pixbuf_composite (glyph_pixbuf, icon_base, 1, 1, 
				      gdk_pixbuf_get_width (glyph_pixbuf) - 1,
				      gdk_pixbuf_get_height (glyph_pixbuf) - 1, 
				      1., 1., 1.0, 1.0, GDK_INTERP_NEAREST, 255);

		g_object_unref (glyph_pixbuf);
	}
	return icon_base;
}

static GdkPixbuf *
accessx_status_applet_mousekeys_image (AccessxStatusApplet *sapplet, XkbStateNotifyEvent *event)
{
	GdkPixbuf  *mouse_pixbuf = NULL, *button_pixbuf, *dot_pixbuf, *tmp_pixbuf;
	const char *which_dot = MOUSEKEYS_DOT_LEFT;
	tmp_pixbuf = gtk_widget_render_icon (GTK_WIDGET (sapplet),
					       MOUSEKEYS_BASE_ICON,
					       icon_size_spec, 
					       NULL);
	mouse_pixbuf = gdk_pixbuf_copy (tmp_pixbuf);
	g_object_unref (tmp_pixbuf);
	/* composite in the buttons */
	if (mouse_pixbuf && event && event->ptr_buttons) {
		unsigned int i;
		for (i = 0; i < G_N_ELEMENTS (button_icons); ++i) {
			if (event->ptr_buttons & button_icons[i].mask) {
				button_pixbuf = gtk_widget_render_icon (GTK_WIDGET (sapplet),
									button_icons[i].stock_id,
									icon_size_spec,
									NULL);
				gdk_pixbuf_composite (button_pixbuf, mouse_pixbuf, 0, 0, 
						      gdk_pixbuf_get_width (button_pixbuf),
						      gdk_pixbuf_get_height (button_pixbuf), 
						      0.0, 0.0, 1.0, 1.0, GDK_INTERP_NEAREST, 255);
			}
		}
	}
	if (event) {
		switch (sapplet->xkb->ctrls->mk_dflt_btn) {
		case Button2:
			which_dot = MOUSEKEYS_DOT_MIDDLE;
			break;
		case Button3: 
			which_dot = MOUSEKEYS_DOT_RIGHT;
			break;
		case Button1:
		default:
			which_dot = MOUSEKEYS_DOT_LEFT;
			break;
		}
	}
	dot_pixbuf = gtk_widget_render_icon (GTK_WIDGET (sapplet),
					     which_dot,
					     icon_size_spec,
					     NULL);

	gdk_pixbuf_composite (dot_pixbuf, mouse_pixbuf, 0, 0, 
			      gdk_pixbuf_get_width (dot_pixbuf),
			      gdk_pixbuf_get_height (dot_pixbuf), 
			      0.0, 0.0, 1.0, 1.0, GDK_INTERP_NEAREST, 255);
	return mouse_pixbuf;
}


static void
accessx_status_applet_update (AccessxStatusApplet *sapplet, 
			      AccessxStatusNotifyType notify_type,
			      XkbEvent *event)
{
	GdkWindow * window;
	unsigned int i;

	window = gtk_widget_get_window (GTK_WIDGET (sapplet));

	if (notify_type & ACCESSX_STATUS_MODIFIERS) {
		unsigned int locked_mods = 0, latched_mods = 0;
		if (event != NULL) {
			locked_mods = event->state.locked_mods;
			latched_mods = event->state.latched_mods;
		}
		else if (window != NULL) {
			XkbStateRec state;			
			XkbGetState (GDK_WINDOW_XDISPLAY (window), 
				     XkbUseCoreKbd, &state); 
			locked_mods = state.locked_mods;
			latched_mods = state.latched_mods;
		}
		/* determine which modifiers are locked, and set state accordingly */
		for (i = 0; i < G_N_ELEMENTS (modifiers); ++i) {
			if (modifiers[i].indicator != NULL && modifiers[i].mask) {
				if (locked_mods & modifiers[i].mask) {
					gtk_widget_set_sensitive (modifiers[i].indicator, TRUE);
					gtk_widget_set_state (modifiers[i].indicator, GTK_STATE_SELECTED);
				}
				else if (latched_mods & modifiers[i].mask) {
					gtk_widget_set_sensitive (modifiers[i].indicator, TRUE);
					gtk_widget_set_state (modifiers[i].indicator, GTK_STATE_NORMAL);
				}
				else {
					gtk_widget_set_sensitive (modifiers[i].indicator, FALSE);
				}
			}
		}
	}
	if ((notify_type & ACCESSX_STATUS_SLOWKEYS) && (event != NULL)) {
		GdkPixbuf *pixbuf = accessx_status_applet_slowkeys_image (
			sapplet, &event->accessx);
		gtk_image_set_from_pixbuf (GTK_IMAGE (sapplet->slowfoo), pixbuf);
		g_object_unref (pixbuf);
	}
	if ((notify_type & ACCESSX_STATUS_BOUNCEKEYS) && (event != NULL)) {
		GdkPixbuf *pixbuf = accessx_status_applet_bouncekeys_image (
			sapplet, &event->accessx);
		gtk_image_set_from_pixbuf (GTK_IMAGE (sapplet->bouncefoo), pixbuf);
		g_object_unref (pixbuf);
	}
	if (notify_type & ACCESSX_STATUS_MOUSEKEYS) {
		GdkPixbuf *pixbuf = accessx_status_applet_mousekeys_image (
			sapplet, &event->state);
		gtk_image_set_from_pixbuf (GTK_IMAGE (sapplet->mousefoo),  pixbuf);
		g_object_unref (pixbuf);
	}

	if (notify_type & ACCESSX_STATUS_ENABLED) {
		/* Update the visibility of widgets in the box */
		XkbGetControls (GDK_WINDOW_XDISPLAY (window), 
				/* XkbMouseKeysMask | XkbStickyKeysMask | 
				   XkbSlowKeysMask | XkbBounceKeysMask, */
				XkbAllControlsMask,
				sapplet->xkb);
		if (!(sapplet->xkb->ctrls->enabled_ctrls & 
		      (XkbMouseKeysMask | XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask)))
			gtk_widget_show (sapplet->idlefoo);
		else gtk_widget_hide (sapplet->idlefoo);
		if (sapplet->xkb->ctrls->enabled_ctrls & XkbMouseKeysMask) 
			gtk_widget_show (sapplet->mousefoo);
		else
			gtk_widget_hide (sapplet->mousefoo);
		if (sapplet->xkb->ctrls->enabled_ctrls & XkbStickyKeysMask) 
			gtk_widget_show (sapplet->stickyfoo);
		else
			gtk_widget_hide (sapplet->stickyfoo);
		if (sapplet->xkb->ctrls->enabled_ctrls & XkbSlowKeysMask) 
			gtk_widget_show (sapplet->slowfoo);
		else
			gtk_widget_hide (sapplet->slowfoo);
		if (sapplet->xkb->ctrls->enabled_ctrls & XkbBounceKeysMask) 
			gtk_widget_show (sapplet->bouncefoo);
		else
			gtk_widget_hide (sapplet->bouncefoo);
	}
}

static void
accessx_status_applet_notify_xkb_ax (AccessxStatusApplet *sapplet, 
				     XkbAccessXNotifyEvent *event) 
{
	AccessxStatusNotifyType notify_mask = 0;

	switch (event->detail) {
	case XkbAXN_SKPress:
	case XkbAXN_SKAccept:
	case XkbAXN_SKRelease:
	case XkbAXN_SKReject:
		notify_mask |= ACCESSX_STATUS_SLOWKEYS;
		break;
	case XkbAXN_BKAccept:
	case XkbAXN_BKReject:
		notify_mask |= ACCESSX_STATUS_BOUNCEKEYS;
		break;
	case XkbAXN_AXKWarning:
		break;
	default:
		break;
	}
	accessx_status_applet_update (sapplet, notify_mask, (XkbEvent *) event);
}

static void
accessx_status_applet_notify_xkb_state (AccessxStatusApplet *sapplet, 
					XkbStateNotifyEvent *event) 
{
	AccessxStatusNotifyType notify_mask = 0;

	if (event->changed & XkbPointerButtonMask) {
		notify_mask |= ACCESSX_STATUS_MOUSEKEYS;
	} 
	if (event->changed & (XkbModifierLatchMask | XkbModifierLockMask)) {
		notify_mask |= ACCESSX_STATUS_MODIFIERS;
	}
	accessx_status_applet_update (sapplet, notify_mask, (XkbEvent *) event);
}

static void
accessx_status_applet_notify_xkb_device (AccessxStatusApplet *sapplet, 
					 XkbExtensionDeviceNotifyEvent *event) 
{
	if (event->reason == XkbXI_IndicatorStateMask) {
		if (event->led_state &= ALT_GRAPH_LED_MASK) {
			gtk_widget_set_sensitive (sapplet->alt_graph_indicator, TRUE);
			gtk_widget_set_state (sapplet->alt_graph_indicator, GTK_STATE_NORMAL);
		}
		else {
			gtk_widget_set_sensitive (sapplet->alt_graph_indicator, FALSE);
		}
	}
}

static void
accessx_status_applet_notify_xkb_controls (AccessxStatusApplet *sapplet, 
					   XkbControlsNotifyEvent *event) 
{
	unsigned int mask = XkbStickyKeysMask | XkbSlowKeysMask | XkbBounceKeysMask | XkbMouseKeysMask;
	unsigned int notify_mask = 0;
	XkbGetControls (sapplet->xkb_display, 
			XkbMouseKeysMask,
			sapplet->xkb);
	if (event->enabled_ctrl_changes & mask)
		notify_mask = ACCESSX_STATUS_ENABLED;
	if (event->changed_ctrls & XkbMouseKeysMask)
		notify_mask |= ACCESSX_STATUS_MOUSEKEYS;
	if (notify_mask)
		accessx_status_applet_update (sapplet, 
					      notify_mask, 
					      (XkbEvent *) event);
}

static void
accessx_status_applet_notify_xkb_event (AccessxStatusApplet *sapplet,
					XkbEvent *event)
{
	switch (event->any.xkb_type) {
	case XkbStateNotify:
		accessx_status_applet_notify_xkb_state (sapplet, &event->state);
		break;
	case XkbAccessXNotify:
		accessx_status_applet_notify_xkb_ax (sapplet, &event->accessx);
		break;
	case XkbControlsNotify:
		accessx_status_applet_notify_xkb_controls (sapplet, &event->ctrls);
		break;
	case XkbExtensionDeviceNotify:
		/* This is a hack around the fact that XFree86's XKB doesn't give AltGr notifications */
		accessx_status_applet_notify_xkb_device (sapplet, &event->device);
		break;
	default:
		break;
	}
}

static GdkFilterReturn
accessx_status_xkb_filter (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer user_data)
{
	AccessxStatusApplet *sapplet = user_data;
	XkbEvent *xevent = gdk_xevent;

	if (xevent->any.type == xkb_base_event_type)
	{
		accessx_status_applet_notify_xkb_event (sapplet, xevent);
	}
	return GDK_FILTER_CONTINUE;
}

static GtkIconSet *
accessx_status_applet_altgraph_icon_set (AccessxStatusApplet *sapplet, GtkWidget *widget)
{
	GtkIconSet  *icon_set = gtk_icon_set_new ();
	gint         i;
	GtkStateType states[3] = {GTK_STATE_NORMAL, GTK_STATE_INSENSITIVE, GTK_STATE_SELECTED};
	GtkStyle    *style = gtk_widget_get_style (widget);
	GdkPixbuf   *icon_base;

	gtk_widget_set_sensitive (widget, TRUE);
	for (i = 0; i < 3; ++i) {
		int alpha;
		GdkColor *fg, *bg;
		GtkIconSource *source = gtk_icon_source_new ();
		GdkPixbuf *pixbuf, *glyph_pixbuf;
		gboolean wildcarded = FALSE;
		fg = &style->text[states[i]];
		bg = &style->white;
		switch (states[i]) {
		case GTK_STATE_NORMAL:
			alpha = 255;
			gtk_widget_set_sensitive (widget, TRUE);
			gtk_widget_set_state (widget, GTK_STATE_NORMAL);
			break;
		case GTK_STATE_SELECTED:
		    /* FIXME: should use text/base here, for selected ? */
			fg = &style->white; 
			bg = &style->black;
			alpha = 255;
			gtk_widget_set_sensitive (widget, TRUE);
			gtk_widget_set_state (widget, GTK_STATE_SELECTED);
			break;

		case GTK_STATE_ACTIVE:
		case GTK_STATE_PRELIGHT:
		case GTK_STATE_INCONSISTENT:
		case GTK_STATE_FOCUSED:
			g_assert_not_reached ();
			break;

		case GTK_STATE_INSENSITIVE:
		default:
			alpha = 63;
			gtk_widget_set_sensitive (widget, FALSE);
			wildcarded = TRUE;
			break;
		}
		icon_base = gtk_widget_render_icon (widget,
			ACCESSX_BASE_ICON, icon_size_spec, NULL);
		pixbuf = gdk_pixbuf_copy (icon_base);
		g_object_unref (icon_base);
	       /* 
		* should be N_("ae"));
		* need en_ locale for this.
		*/
	       /* 
		* Translators: substitute an easily-recognized single glyph 
		* from Level 2, i.e. an AltGraph character from a common keyboard 
		* in your locale. 
		*/
		glyph_pixbuf = accessx_status_applet_get_glyph_pixbuf (sapplet, 
                                                                       widget,
								       pixbuf, 
								       fg, 
								       bg,
								       ("æ"));
		gdk_pixbuf_composite (glyph_pixbuf, pixbuf, 0, 0, 
				      gdk_pixbuf_get_width (glyph_pixbuf),
				      gdk_pixbuf_get_height (glyph_pixbuf),
				      0., 0., 1.0, 1.0, GDK_INTERP_NEAREST, alpha);
		g_object_unref (glyph_pixbuf);
		gtk_icon_source_set_pixbuf (source, pixbuf);
		gtk_icon_source_set_state (source, states[i]);
		gtk_icon_source_set_state_wildcarded (source, wildcarded);
		gtk_icon_set_add_source (icon_set, source);
		gtk_icon_source_free (source);
	}
	/* we mucked about with the box's state to create the icons; restore it to normal */
	gtk_widget_set_state (widget, GTK_STATE_NORMAL);
	gtk_widget_set_sensitive (widget, TRUE);
	return icon_set;
}

static void
accessx_applet_add_stock_icons (AccessxStatusApplet *sapplet, GtkWidget *widget)
{
	GtkIconFactory *factory = gtk_icon_factory_new ();
        unsigned int i = 0;
        GtkIconSet     *icon_set;
                                                                                
	gtk_icon_factory_add_default (factory);

        while (i <  G_N_ELEMENTS (stock_icons)) {
		const char *set_name = stock_icons[i].stock_id;
                icon_set = gtk_icon_set_new ();
		do {
			GtkIconSource *source;
			char *resource_path;
			GdkPixbuf *pixbuf;

			source = gtk_icon_source_new ();

			resource_path = g_build_filename (GRESOURCE_PREFIX "/icons/",
			                                  stock_icons[i].name,
			                                  NULL);

			pixbuf = gdk_pixbuf_new_from_resource (resource_path, NULL);
			g_free (resource_path);

			gtk_icon_source_set_pixbuf (source, pixbuf);
			g_object_unref (pixbuf);

			gtk_icon_source_set_state (source, stock_icons[i].state);
			gtk_icon_source_set_state_wildcarded (source, stock_icons[i].wildcarded);
			gtk_icon_set_add_source (icon_set, source);
			gtk_icon_source_free (source);
			++i;
		} while (set_name == stock_icons[i].stock_id);
		gtk_icon_factory_add (factory, set_name, icon_set);
		gtk_icon_set_unref (icon_set);
	}
	/* now create the stock icons for AltGr, which are internationalized */
	icon_set = accessx_status_applet_altgraph_icon_set (sapplet, widget);
	gtk_icon_factory_add (factory, ALTGRAPH_KEY_ICON, icon_set);
	gtk_icon_set_unref (icon_set);
	sapplet->icon_factory = factory;
}

static void 
disable_applet (AccessxStatusApplet* sapplet)
{
	gtk_widget_hide (sapplet->meta_indicator);
	gtk_widget_hide (sapplet->hyper_indicator);
	gtk_widget_hide (sapplet->super_indicator);
	gtk_widget_hide (sapplet->alt_graph_indicator);
	gtk_widget_hide (sapplet->shift_indicator);
	gtk_widget_hide (sapplet->ctrl_indicator);
	gtk_widget_hide (sapplet->alt_indicator);
	gtk_widget_hide (sapplet->meta_indicator);
	gtk_widget_hide (sapplet->mousefoo);
	gtk_widget_hide (sapplet->stickyfoo);
	gtk_widget_hide (sapplet->slowfoo);
	gtk_widget_hide (sapplet->bouncefoo);
}

static void 
popup_error_dialog (AccessxStatusApplet* sapplet)
{
	GtkWidget *dialog;
	gchar *error_txt;

	switch (sapplet->error_type) {
		case ACCESSX_STATUS_ERROR_XKB_DISABLED : 
			error_txt = g_strdup (_("XKB Extension is not enabled"));
			break;

		case ACCESSX_STATUS_ERROR_NONE :
			g_assert_not_reached ();
			break;

		default	: error_txt = g_strdup (_("Unknown error"));
			  break;				
	}

	dialog = gtk_message_dialog_new (NULL,
				   	 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 _("Error: %s"),
					 error_txt);

	g_signal_connect (G_OBJECT (dialog),
			  "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);

	gtk_window_set_screen (GTK_WINDOW (dialog),
	gtk_widget_get_screen (GTK_WIDGET (sapplet)));

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gtk_widget_show (dialog);
	g_free (error_txt);
}

static void
create_applet (AccessxStatusApplet *sapplet)
{
	AtkObject           *atko;
	GdkPixbuf	    *pixbuf;

	sapplet->xkb = NULL;
	sapplet->xkb_display = NULL;
	sapplet->error_type = ACCESSX_STATUS_ERROR_NONE;

	if (gp_applet_get_orientation (GP_APPLET (sapplet)) == GTK_ORIENTATION_VERTICAL) {
		sapplet->box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		sapplet->stickyfoo = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	}
	else {
		sapplet->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
		sapplet->stickyfoo = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	}

	sapplet->size = 24;
	icon_size_spec = GTK_ICON_SIZE_LARGE_TOOLBAR;       

	accessx_applet_add_stock_icons (sapplet, sapplet->box);
	pixbuf = accessx_status_applet_mousekeys_image (sapplet, NULL);
	sapplet->mousefoo = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (pixbuf);
	gtk_widget_hide (sapplet->mousefoo);

	sapplet->shift_indicator = 
		gtk_image_new_from_stock (SHIFT_KEY_ICON, icon_size_spec); 
	gtk_widget_hide (sapplet->mousefoo);
	sapplet->ctrl_indicator = 
		gtk_image_new_from_stock (CONTROL_KEY_ICON, icon_size_spec); 
	sapplet->alt_indicator = 
		gtk_image_new_from_stock (ALT_KEY_ICON, icon_size_spec); 
	sapplet->meta_indicator =
		gtk_image_new_from_stock (META_KEY_ICON, icon_size_spec); 
	gtk_widget_set_sensitive (sapplet->meta_indicator, FALSE);
	gtk_widget_hide (sapplet->meta_indicator);
	sapplet->hyper_indicator =
		gtk_image_new_from_stock (HYPER_KEY_ICON, icon_size_spec); 
	gtk_widget_set_sensitive (sapplet->hyper_indicator, FALSE);
	gtk_widget_hide (sapplet->hyper_indicator);
	sapplet->super_indicator =
		gtk_image_new_from_stock (SUPER_KEY_ICON, icon_size_spec); 
	gtk_widget_set_sensitive (sapplet->super_indicator, FALSE);
	gtk_widget_hide (sapplet->super_indicator);
	sapplet->alt_graph_indicator =
		gtk_image_new_from_stock (ALTGRAPH_KEY_ICON, icon_size_spec);
	gtk_widget_set_sensitive (sapplet->alt_graph_indicator, FALSE);

	pixbuf = accessx_status_applet_slowkeys_image (sapplet, NULL);
	sapplet->slowfoo = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (pixbuf);
	gtk_widget_hide (sapplet->slowfoo);

	pixbuf = accessx_status_applet_bouncekeys_image (sapplet, NULL);
	sapplet->bouncefoo = gtk_image_new_from_pixbuf (pixbuf);
	g_object_unref (pixbuf);
	gtk_widget_hide (sapplet->bouncefoo);

	sapplet->idlefoo = gtk_image_new_from_stock (ACCESSX_APPLET, icon_size_spec);
	gtk_widget_show (sapplet->slowfoo);

	gtk_container_add (GTK_CONTAINER (sapplet), sapplet->box);
	gtk_widget_show (sapplet->box);

	gtk_container_add (GTK_CONTAINER (sapplet->box), sapplet->idlefoo);
	gtk_container_add (GTK_CONTAINER (sapplet->box), sapplet->mousefoo);
	gtk_container_add (GTK_CONTAINER (sapplet->box), sapplet->stickyfoo);
	gtk_container_add (GTK_CONTAINER (sapplet->box), sapplet->slowfoo);
	gtk_container_add (GTK_CONTAINER (sapplet->box), sapplet->bouncefoo);

	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->shift_indicator);
	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->ctrl_indicator);
	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->alt_indicator);
	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->meta_indicator);
	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->hyper_indicator);
	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->super_indicator);
	gtk_container_add (GTK_CONTAINER (sapplet->stickyfoo), sapplet->alt_graph_indicator);

	gtk_box_set_homogeneous (GTK_BOX (sapplet->stickyfoo), TRUE);
	gtk_widget_show (sapplet->stickyfoo);

	gtk_widget_show (GTK_WIDGET (sapplet));

	atko = gtk_widget_get_accessible (GTK_WIDGET (sapplet));
	atk_object_set_name (atko, _("AccessX Status"));
	atk_object_set_description (atko, _("Shows keyboard status when accessibility features are used."));

	atko = gtk_widget_get_accessible (sapplet->box);
	atk_object_set_name (atko, _("AccessX Status"));
	atk_object_set_description (atko, _("Shows keyboard status when accessibility features are used."));
}

static void
placement_changed_cb (GpApplet            *applet,
                      GtkOrientation       orientation,
                      GtkPositionType      position,
                      AccessxStatusApplet *sapplet)
{
	if (orientation == GTK_ORIENTATION_VERTICAL) {
		gtk_orientable_set_orientation (GTK_ORIENTABLE (sapplet->box),
		                                GTK_ORIENTATION_VERTICAL);
		gtk_orientable_set_orientation (GTK_ORIENTABLE (sapplet->stickyfoo),
		                                GTK_ORIENTATION_VERTICAL);
	}
	else {
		gtk_orientable_set_orientation (GTK_ORIENTABLE (sapplet->box),
		                                                GTK_ORIENTATION_HORIZONTAL);
		gtk_orientable_set_orientation (GTK_ORIENTABLE (sapplet->stickyfoo),
		                                                GTK_ORIENTATION_HORIZONTAL);
	}

	if (gtk_widget_get_realized (GTK_WIDGET (sapplet)))
		accessx_status_applet_update (sapplet, ACCESSX_STATUS_ALL, NULL);
}

static void
accessx_status_applet_resize (GtkWidget           *widget,
                              GtkAllocation       *allocation,
                              AccessxStatusApplet *sapplet)
{
	gint old_size = sapplet->size;

	if (gp_applet_get_orientation (GP_APPLET (sapplet)) == GTK_ORIENTATION_VERTICAL) {
		sapplet->size = allocation->width;
	} else {
		sapplet->size = allocation->height;
	}

	if (sapplet->size != old_size) {
		/* TODO: either rescale icons to fit panel, or tile them when possible */
	}
}

static gboolean
button_press_cb (GtkWidget *widget, GdkEventButton *event, AccessxStatusApplet *sapplet)
{
	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) 
		dialog_cb (NULL, NULL, sapplet);

	return FALSE;
}

static gboolean
key_press_cb (GtkWidget *widget, GdkEventKey *event, AccessxStatusApplet *sapplet)
{
	switch (event->keyval) {
	case GDK_KEY_KP_Enter:
	case GDK_KEY_ISO_Enter:
	case GDK_KEY_3270_Enter:
	case GDK_KEY_Return:
	case GDK_KEY_space:
	case GDK_KEY_KP_Space:
		dialog_cb (NULL, NULL, sapplet);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

static void
accessx_status_applet_realize (GtkWidget           *widget,
                               AccessxStatusApplet *self)
{
  if (!accessx_status_applet_xkb_select (self))
    {
      disable_applet (self);
      popup_error_dialog (self);
      return;
    }

  accessx_status_applet_init_modifiers (self);
  accessx_status_applet_update (self, ACCESSX_STATUS_ALL, NULL);
}

static void
accessx_status_applet_fill (AccessxStatusApplet *sapplet)
{
	AtkObject           *atk_object;
	const char          *menu_resource;
	GAction             *action;

	create_applet (sapplet);

	g_signal_connect_after (sapplet,
	                        "realize",
	                        G_CALLBACK (accessx_status_applet_realize),
	                        sapplet);

	g_signal_connect (sapplet,
	                  "placement-changed",
	                  G_CALLBACK (placement_changed_cb),
	                  sapplet);

	g_signal_connect (sapplet,
	                  "size-allocate",
	                  G_CALLBACK (accessx_status_applet_resize),
	                  sapplet);

	g_signal_connect (sapplet, "button_press_event",
				   G_CALLBACK (button_press_cb), sapplet);
	g_signal_connect (sapplet, "key_press_event",
				   G_CALLBACK (key_press_cb), sapplet);				   

	menu_resource = GRESOURCE_PREFIX "/ui/accessx-status-applet-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (sapplet),
	                                    menu_resource,
	                                    accessx_status_applet_menu_actions);

	action = gp_applet_menu_lookup_action (GP_APPLET (sapplet), "dialog");
	g_object_bind_property (sapplet, "locked-down", action, "enabled",
                            G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

	gtk_widget_set_tooltip_text (GTK_WIDGET (sapplet), _("Keyboard Accessibility Status"));

	atk_object = gtk_widget_get_accessible (GTK_WIDGET (sapplet));
	atk_object_set_name (atk_object, _("AccessX Status"));
	atk_object_set_description (atk_object,
				    _("Displays current state of keyboard accessibility features"));

	gtk_widget_show_all (GTK_WIDGET (sapplet));
}

static void
accessx_status_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (accessx_status_applet_parent_class)->constructed (object);
  accessx_status_applet_fill (ACCESSX_STATUS_APPLET (object));
}

static void
accessx_status_applet_finalize (GObject *object)
{
  AccessxStatusApplet *self;

  self = ACCESSX_STATUS_APPLET (object);

  gdk_window_remove_filter (NULL, accessx_status_xkb_filter, self);

  if (self->xkb != NULL)
    {
      XkbFreeKeyboard (self->xkb, 0, True);
      self->xkb = NULL;
    }

  if (self->xkb_display)
    {
      XCloseDisplay (self->xkb_display);
      self->xkb_display = NULL;
    }

  G_OBJECT_CLASS (accessx_status_applet_parent_class)->finalize (object);
}

static void
accessx_status_applet_class_init (AccessxStatusAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = accessx_status_applet_constructed;
  object_class->finalize = accessx_status_applet_finalize;
}

static void
accessx_status_applet_init (AccessxStatusApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);

  gdk_window_add_filter (NULL, accessx_status_xkb_filter, self);
}

void
accessx_status_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("Shows the state of AccessX features such as latched modifiers");

  authors = (const char *[])
    {
      "Calum Benson <calum.benson@sun.com>",
      "Bill Haneman <bill.haneman@sun.com>",
      NULL
    };

  documenters = (const char *[])
    {
      "Bill Haneman <bill.haneman@sun.com>",
      "Sun GNOME Documentation Team <gdocteam@sun.com>",
      NULL
    };

  copyright = "\xC2\xA9 2003 Sun Microsystems";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
