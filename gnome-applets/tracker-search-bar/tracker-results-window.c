/*
 * Copyright (C) 2009, Nokia <ivan.frade@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "config.h"

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <libtracker-sparql/tracker-sparql.h>

#include "tracker-results-window.h"
#include "tracker-aligned-window.h"
#include "tracker-regex.h"

#define MAX_ITEMS 10

#define FILE_SEARCH_QUERY(rdftype) \
	"SELECT DISTINCT" \
	"  ?uri ?title ?tooltip ?uri '' " \
	"WHERE {" \
	"  { " \
	"    { " \
	"      ?urn a " #rdftype "; " \
	"	 nie:isStoredAs ?file . " \
	"      ?file nie:url ?uri ; " \
	"	 nfo:belongsToContainer ?tooltip ; " \
	"	 nfo:fileName ?title . " \
	"    } UNION { " \
	"      ?urn a nfo:FileDataObject ; " \
	"	 nie:url ?uri ; " \
	"	 nfo:belongsToContainer ?tooltip ; " \
	"	 nfo:fileName ?title . " \
	"      ?content a " #rdftype "; " \
	"	 nie:isStoredAs ?urn . " \
	"    } " \
	"  } " \
	"  ?urn fts:match \"%s*\" . " \
	"}" \
	"ORDER BY DESC(fts:rank(?urn)) " \
	"OFFSET 0 LIMIT %d"

#define MUSIC_QUERY FILE_SEARCH_QUERY(nfo:Audio)
#define IMAGE_QUERY FILE_SEARCH_QUERY(nfo:Image)
#define VIDEO_QUERY FILE_SEARCH_QUERY(nmm:Video)
#define DOCUMENT_QUERY FILE_SEARCH_QUERY (nfo:Document)
#define FOLDER_QUERY FILE_SEARCH_QUERY (nfo:Folder)

#define APP_QUERY	  \
	"SELECT" \
	"  ?urn ?title ?tooltip ?link fts:rank(?urn) nfo:softwareIcon(?urn)" \
	"WHERE {" \
	"  ?urn a nfo:Software ;" \
	"  nie:title ?title ;" \
	"  nie:comment ?tooltip ;" \
	"  nfo:softwareCmdLine ?link ." \
	"  ?urn fts:match \"%s*\" " \
	"} " \
	"ORDER BY DESC(fts:rank(?urn)) " \
	"OFFSET 0 LIMIT %d"
#define TAGS_QUERY	  \
	"SELECT" \
	"  ?urn ?title ?title ?urn fts:rank(?urn) " \
	"WHERE {" \
	"  ?urn a nao:Tag ;" \
	"  nao:prefLabel ?title ." \
	"  ?urn fts:match \"%s*\" " \
	"} " \
	"ORDER BY DESC(fts:rank(?urn)) " \
	"OFFSET 0 LIMIT %d"
/* #define TAGS_QUERY                                   \ */
/*      "SELECT"                                        \ */
/*      "  ?urn ?title COUNT(?files) ?urn fts:rank(?urn) " \ */
/*      "WHERE {"                                       \ */
/*      "  ?urn a nao:Tag ;"                            \ */
/*      "  nao:prefLabel ?title ."                      \ */
/*      "  ?urn fts:match \"%s*\" ."                    \ */
/*      "  ?files nao:hasTag ?urn "                     \ */
/*      "} "                                            \ */
/*      "GROUP BY ?urn "                                \ */
/*      "ORDER BY DESC(fts:rank(?urn)) "                \ */
/*      "OFFSET 0 LIMIT %d" */
#define BOOKMARK_QUERY	  \
	"SELECT" \
	"  ?urn ?title ?link ?link fts:rank(?urn) " \
	"WHERE {" \
	"  ?urn a nfo:Bookmark ;" \
	"  nie:title ?title ;" \
	"  nie:links ?link ." \
	"  ?urn fts:match \"%s*\" " \
	"} " \
	"ORDER BY DESC(fts:rank(?urn)) " \
	"OFFSET 0 LIMIT %d"
#define WEBSITE_QUERY	  \
	"SELECT" \
	"  ?urn ?title ?link ?link fts:rank(?urn) " \
	"WHERE {" \
	"  ?urn a nfo:Website ;" \
	"  nie:title ?title ;" \
	"  nie:links ?link ." \
	"  ?urn fts:match \"%s*\" " \
	"} " \
	"ORDER BY DESC(fts:rank(?urn)) " \
	"OFFSET 0 LIMIT %d"
#define CONTACT_QUERY	  \
	"SELECT" \
	"  ?urn ?title ?link ?link fts:rank(?urn) " \
	"WHERE {" \
	"  ?urn a nco:Contact ;" \
	"  nco:fullname ?title ;" \
	"  nco:hasEmailAddress ?link ." \
	"  ?urn fts:match \"%s*\" " \
	"} " \
	"ORDER BY DESC(fts:rank(?urn)) " \
	"OFFSET 0 LIMIT %d"

#undef USE_SEPARATOR_FOR_SPACING

struct _TrackerResultsWindow
{
	TrackerAlignedWindow parent;

	GtkWidget *frame;
	GtkWidget *treeview;
	GtkWidget *scrolled_window;
	GObject *store;

	GtkWidget *label;

	GtkIconTheme *icon_theme;

	TrackerSparqlConnection *connection;
	GCancellable *cancellable;
	gchar *query;

	gboolean first_category_populated;

	GList *search_queries;
	gint queries_pending;
	gint request_id;
};

typedef enum {
	CATEGORY_NONE                  = 1 << 0,
	CATEGORY_CONTACT               = 1 << 1,
	CATEGORY_TAG                   = 1 << 2,
	CATEGORY_EMAIL_ADDRESS         = 1 << 3,
	CATEGORY_DOCUMENT              = 1 << 4,
	CATEGORY_APPLICATION           = 1 << 5,
	CATEGORY_IMAGE                 = 1 << 6,
	CATEGORY_AUDIO                 = 1 << 7,
	CATEGORY_FOLDER                = 1 << 8,
	CATEGORY_FONT                  = 1 << 9,
	CATEGORY_VIDEO                 = 1 << 10,
	CATEGORY_ARCHIVE               = 1 << 11,
	CATEGORY_BOOKMARK              = 1 << 12,
	CATEGORY_WEBSITE               = 1 << 13
} TrackerCategory;

typedef struct {
	gchar *urn;
	gchar *title;
	gchar *tooltip;
	gchar *link;
	gchar *icon_name;
	TrackerCategory category;
} ItemData;

typedef struct {
	GCancellable *cancellable;
	gint request_id;
	TrackerCategory category;
	TrackerResultsWindow *window;
	GSList *results;
} SearchQuery;

struct FindCategory {
	const gchar *category_str;
	gboolean found;
};

static void     results_window_constructed          (GObject              *object);
static void     results_window_finalize             (GObject              *object);
static void     results_window_set_property         (GObject              *object,
                                                     guint                 prop_id,
                                                     const GValue         *value,
                                                     GParamSpec           *pspec);
static void     results_window_get_property         (GObject              *object,
                                                     guint                 prop_id,
                                                     GValue               *value,
                                                     GParamSpec           *pspec);
static gboolean results_window_key_press_event      (GtkWidget            *widget,
                                                     GdkEventKey          *event);
static gboolean results_window_button_press_event   (GtkWidget            *widget,
                                                     GdkEventButton       *event);
static void     results_window_get_preferred_width  (GtkWidget            *widget,
                                                     gint                 *minimal_width,
                                                     gint                 *natural_width);
static void     results_window_get_preferred_height (GtkWidget            *widget,
                                                     gint                 *minimal_height,
                                                     gint                 *natural_height);
static void     results_window_screen_changed       (GtkWidget            *widget,
                                                     GdkScreen            *prev_screen);
static void     model_set_up                        (TrackerResultsWindow *window);
static void     search_get                          (TrackerResultsWindow *window,
                                                     TrackerCategory       category);
static void     search_start                        (TrackerResultsWindow *window);
static void     search_query_free                   (SearchQuery          *sq);
static gchar *  category_to_string                  (TrackerCategory       category);

enum {
	COL_CATEGORY_ID,
	COL_IMAGE,
	COL_IMAGE_REQUESTED,
	COL_URN,
	COL_TITLE,
	COL_TOOLTIP,
	COL_LINK,
	COL_ICON_NAME,
	COL_COUNT
};

enum {
	PROP_0,
	PROP_QUERY
};

G_DEFINE_TYPE (TrackerResultsWindow, tracker_results_window, TRACKER_TYPE_ALIGNED_WINDOW)

static void
tracker_results_window_class_init (TrackerResultsWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->constructed = results_window_constructed;
	object_class->finalize = results_window_finalize;
	object_class->set_property = results_window_set_property;
	object_class->get_property = results_window_get_property;

	widget_class->key_press_event = results_window_key_press_event;
	widget_class->button_press_event = results_window_button_press_event;
	widget_class->get_preferred_width = results_window_get_preferred_width;
	widget_class->get_preferred_height = results_window_get_preferred_height;
	widget_class->screen_changed = results_window_screen_changed;

	g_object_class_install_property (object_class,
	                                 PROP_QUERY,
	                                 g_param_spec_string ("query",
	                                                      "Query",
	                                                      "Query",
	                                                      NULL,
	                                                      G_PARAM_READWRITE));
}

static gboolean
launch_application_for_uri (GtkWidget   *widget,
                            const gchar *uri)
{
	GdkAppLaunchContext *launch_context;
	GdkScreen *screen;
	GError *error = NULL;
	gboolean success;

	launch_context = gdk_app_launch_context_new ();

	screen = gtk_widget_get_screen (widget);
	gdk_app_launch_context_set_screen (launch_context, screen);

	g_app_info_launch_default_for_uri (uri,
	                                   G_APP_LAUNCH_CONTEXT (launch_context),
	                                   &error);

	if (error) {
		g_critical ("Could not launch application for uri '%s': %s",
		            uri, error->message);
		g_error_free (error);
		success = FALSE;
	} else {
		success = TRUE;
	}

	g_object_unref (launch_context);

	return success;
}

static void
tree_view_row_activated_cb (GtkTreeView       *treeview,
                            GtkTreePath       *path,
                            GtkTreeViewColumn *column,
                            gpointer           user_data)
{
	TrackerResultsWindow *window;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *link;
	gboolean success;

	window = user_data;
	model = GTK_TREE_MODEL (window->store);

	if (!gtk_tree_model_get_iter (model, &iter, path)) {
		return;
	}

	gtk_tree_model_get (model, &iter,
	                    COL_LINK, &link,
	                    -1);

	if (!link) {
		return;
	}

	if (tracker_regex_match (TRACKER_REGEX_ALL, link, NULL, NULL) > 0) {
		success = launch_application_for_uri (GTK_WIDGET (window), link);
	} else {
		GError *error = NULL;

		success = g_spawn_command_line_async (link, &error);

		if (error) {
			g_critical ("Could not launch command line:'%s', %s",
			            link,
			            error->message);
			g_error_free (error);
		}
	}

	if (success) {
		gtk_widget_hide (GTK_WIDGET (window));
	}

	g_free (link);
}

static void
tracker_results_window_init (TrackerResultsWindow *self)
{
	GtkWidget *vbox;
	GError *error = NULL;

	self->cancellable = g_cancellable_new ();

#ifdef HAVE_TRACKER3
	self->connection = tracker_sparql_connection_bus_new ("org.freedesktop.Tracker3.Miner.Files",
	                                                      NULL,
	                                                      NULL,
	                                                      &error);
#else
	self->connection = tracker_sparql_connection_get (self->cancellable, &error);
#endif

	if (error != NULL) {
		g_warning ("Cannot connect to tracker: %s\n", error->message);
		g_error_free (error);
		g_object_unref (self->cancellable);
	}

	self->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (self), self->frame);
	gtk_frame_set_shadow_type (GTK_FRAME (self->frame), GTK_SHADOW_IN);
	gtk_widget_set_size_request (self->frame, 500, 500);
	gtk_widget_show (self->frame);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
	gtk_container_add (GTK_CONTAINER (self->frame), vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_widget_show (vbox);

	self->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), self->scrolled_window, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self->scrolled_window),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	self->treeview = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (self->scrolled_window), self->treeview);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self->treeview), FALSE);
	g_signal_connect (self->treeview, "row-activated",
	                  G_CALLBACK (tree_view_row_activated_cb), self);

	self->label = gtk_label_new (NULL);
	gtk_widget_set_sensitive (self->label, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), self->label, TRUE, TRUE, 0);

	self->icon_theme = gtk_icon_theme_get_default ();

	model_set_up (self);

	gtk_widget_show_all (self->scrolled_window);
}

static void
results_window_constructed (GObject *object)
{
	TrackerResultsWindow *window;

	window = TRACKER_RESULTS_WINDOW (object);

	search_start (window);
}

static void
results_window_finalize (GObject *object)
{
	TrackerResultsWindow *self;

	self = TRACKER_RESULTS_WINDOW (object);

	g_free (self->query);

	if (self->cancellable) {
		g_cancellable_cancel (self->cancellable);
		g_object_unref (self->cancellable);
	}

	if (self->connection) {
		g_object_unref (self->connection);
	}

	/* Clean up previous requests, this will call
	 * g_cancellable_cancel() on each query still running.
	 */
	g_list_free_full (self->search_queries, (GDestroyNotify) search_query_free);
	self->search_queries = NULL;

	G_OBJECT_CLASS (tracker_results_window_parent_class)->finalize (object);
}

static void
results_window_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	TrackerResultsWindow *self;

	self = TRACKER_RESULTS_WINDOW (object);

	switch (prop_id) {
	case PROP_QUERY:
		/* Don't do the search_start() call if the window was
		 * just set up.
		 */
		g_free (self->query);
		self->query = g_value_dup_string (value);
		search_start (TRACKER_RESULTS_WINDOW (object));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
results_window_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	TrackerResultsWindow *self;

	self = TRACKER_RESULTS_WINDOW (object);

	switch (prop_id) {
	case PROP_QUERY:
		g_value_set_string (value, self->query);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gboolean
results_window_key_press_event (GtkWidget   *widget,
                                GdkEventKey *event)
{
	if (event->keyval == GDK_KEY_Escape) {
		gtk_widget_hide (widget);

		return TRUE;
	}

        if (event->keyval != GDK_KEY_Return &&
            (*event->string != '\0' ||
             event->keyval == GDK_KEY_BackSpace)) {
                GtkWidget *entry;

                entry = tracker_aligned_window_get_widget (TRACKER_ALIGNED_WINDOW (widget));
                gtk_propagate_event (entry, (GdkEvent *) event);
                return TRUE;
        }

	if (GTK_WIDGET_CLASS (tracker_results_window_parent_class)->key_press_event (widget, event)) {
		return TRUE;
	}

	return FALSE;
}

static gboolean
results_window_button_press_event (GtkWidget      *widget,
                                   GdkEventButton *event)
{
	GtkAllocation alloc;

	gtk_widget_get_allocation (widget, &alloc);

	if (event->x < 0 || event->x > alloc.width ||
	    event->y < 0 || event->y > alloc.height) {
		/* Click happened outside window, pop it down */
		gtk_widget_hide (widget);
		return TRUE;
	}

	if (GTK_WIDGET_CLASS (tracker_results_window_parent_class)->button_press_event &&
	    GTK_WIDGET_CLASS (tracker_results_window_parent_class)->button_press_event (widget, event)) {
		return TRUE;
	}

	return FALSE;
}

static void
results_window_size_request (GtkWidget      *widget,
                             GtkRequisition *requisition)
{
	GtkRequisition child_req;
	guint border_width;

	gtk_widget_size_request (gtk_bin_get_child (GTK_BIN (widget)), &child_req);
	border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));

	requisition->width = child_req.width + (2 * border_width);
	requisition->height = child_req.height + (2 * border_width);

	if (gtk_widget_get_realized (widget)) {
		GdkScreen *screen;
		GdkRectangle monitor_geom;
		guint monitor_num;

		/* make it no larger than half the monitor size */
		screen = gtk_widget_get_screen (widget);
		monitor_num = gdk_screen_get_monitor_at_window (screen, gtk_widget_get_window (widget));

		gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor_geom);

		requisition->width = MIN (requisition->width, monitor_geom.width / 2);
		requisition->height = MIN (requisition->height, monitor_geom.height / 2);
	}
}

static void
results_window_get_preferred_width  (GtkWidget *widget,
                                     gint      *minimal_width,
                                     gint      *natural_width)
{
	GtkRequisition requisition;

	results_window_size_request (widget, &requisition);

	*minimal_width = *natural_width = requisition.width;
}

static void
results_window_get_preferred_height (GtkWidget *widget,
                                     gint      *minimal_height,
                                     gint      *natural_height)
{
	GtkRequisition requisition;

	results_window_size_request (widget, &requisition);

	*minimal_height = *natural_height = requisition.height;
}

static void
results_window_screen_changed (GtkWidget *widget,
                               GdkScreen *prev_screen)
{
	TrackerResultsWindow *self;
	GdkScreen *screen;

	self = TRACKER_RESULTS_WINDOW (widget);

	if (self->icon_theme) {
		self->icon_theme = NULL;
	}

	screen = gtk_widget_get_screen (widget);

	if (screen) {
		self->icon_theme = gtk_icon_theme_get_for_screen (screen);
		/* FIXME: trigger the model to update icons */
	}

	GTK_WIDGET_CLASS (tracker_results_window_parent_class)->screen_changed (widget, prev_screen);
}

static ItemData *
item_data_new (const gchar     *urn,
               const gchar     *title,
               const gchar     *tooltip,
               const gchar     *link,
               const gchar     *icon_name,
               TrackerCategory  category)
{
	ItemData *id;

	id = g_new0 (ItemData, 1);

	id->urn = g_strdup (urn);
	id->title = g_strdup (title);
	id->tooltip = g_strdup (tooltip);
	id->link = g_strdup (link);
	id->icon_name = g_strdup (icon_name);
	id->category = category;

	return id;
}

static void
item_data_free (ItemData *id)
{
	g_free (id->urn);
	g_free (id->title);
	g_free (id->tooltip);
	g_free (id->link);
	g_free (id->icon_name);

	g_free (id);
}

static SearchQuery *
search_query_new (gint                  request_id,
                  TrackerCategory       category,
                  TrackerResultsWindow *window)
{
	SearchQuery *sq;

	sq = g_new0 (SearchQuery, 1);

	sq->request_id = request_id;
	sq->cancellable = g_cancellable_new ();
	sq->category = category;
	sq->window = window;
	sq->results = NULL;

	return sq;
}

static void
search_query_free (SearchQuery *sq)
{
	if (sq->cancellable) {
		g_cancellable_cancel (sq->cancellable);
		g_object_unref (sq->cancellable);
	}

	sq->window->queries_pending--;
	g_slist_free_full (sq->results, (GDestroyNotify) item_data_free);
	g_free (sq);
}

static gchar *
category_to_string (TrackerCategory category)
{
	switch (category) {
	case CATEGORY_NONE: return _("Other");
	case CATEGORY_CONTACT: return _("Contacts");
	case CATEGORY_TAG: return _("Tags");
	case CATEGORY_EMAIL_ADDRESS: return _("Email Addresses");
	case CATEGORY_DOCUMENT: return _("Documents");
	case CATEGORY_APPLICATION: return _("Applications");
	case CATEGORY_IMAGE: return _("Images");
	case CATEGORY_AUDIO: return _("Audio");
	case CATEGORY_FOLDER: return _("Folders");
	case CATEGORY_FONT: return _("Fonts");
	case CATEGORY_VIDEO: return _("Videos");
	case CATEGORY_ARCHIVE: return _("Archives");
	case CATEGORY_BOOKMARK: return _("Bookmarks");
	case CATEGORY_WEBSITE: return _("Links");
	default: break;
	}

	return _("Other");
}

inline static void
category_from_string (const gchar *type,
                      guint       *categories)
{
	if (g_str_has_suffix (type, "nao#Tag")) {
		*categories |= CATEGORY_TAG;
	}

	if (g_str_has_suffix (type, "nfo#TextDocument") ||
	    g_str_has_suffix (type, "nfo#PaginatedTextDocument")) {
		*categories |= CATEGORY_DOCUMENT;
	}

	if (g_str_has_suffix (type, "nco#Contact")) {
		*categories |= CATEGORY_CONTACT;
	}

	if (g_str_has_suffix (type, "nco#EmailAddress")) {
		*categories |= CATEGORY_EMAIL_ADDRESS;
	}

	if (g_str_has_suffix (type, "nfo#Image") ||
	    g_str_has_suffix (type, "nfo#RosterImage") ||
	    g_str_has_suffix (type, "nfo#VectorImage") ||
	    g_str_has_suffix (type, "nfo#FilesystemImage")) {
		*categories |= CATEGORY_IMAGE;
	}

	if (g_str_has_suffix (type, "nfo#Audio") ||
	    g_str_has_suffix (type, "nmm#MusicPiece")) {
		*categories |= CATEGORY_AUDIO;
	}

	if (g_str_has_suffix (type, "nfo#Folder")) {
		*categories |= CATEGORY_FOLDER;
	}

	if (g_str_has_suffix (type, "nfo#Font")) {
		*categories |= CATEGORY_FONT;
	}

	if (g_str_has_suffix (type, "nfo#Video") ||
	    g_str_has_suffix (type, "nmm#Video")) {
		*categories |= CATEGORY_VIDEO;
	}

	if (g_str_has_suffix (type, "nfo#Archive")) {
		*categories |= CATEGORY_ARCHIVE;
	}

	if (g_str_has_suffix (type, "nfo#Bookmark")) {
		*categories |= CATEGORY_BOOKMARK;
	}

	if (g_str_has_suffix (type, "nfo#Website")) {
		*categories |= CATEGORY_WEBSITE;
	}
}

static GdkPixbuf *
pixbuf_get (TrackerResultsWindow *window,
            const gchar          *uri,
            const gchar          *icon_name,
            TrackerCategory       category)
{
	const gchar *attributes;
	GFile *file;
	GFileInfo *info;
	GIcon *icon;
	GdkPixbuf *pixbuf = NULL;
	GError *error = NULL;

	file = g_file_new_for_uri (uri);

        if (category & CATEGORY_TAG) {
                icon_name = GTK_STOCK_INDEX;
        } else if (category & CATEGORY_BOOKMARK) {
                icon_name = "user-bookmarks";
        }

	if (icon_name) {
		if (strrchr (icon_name, '.') == NULL) {
			pixbuf = gtk_icon_theme_load_icon (window->icon_theme,
			                                   icon_name, 24,
			                                   GTK_ICON_LOOKUP_USE_BUILTIN,
			                                   &error);

			if (error) {
				g_printerr ("Couldn't get icon name '%s': %s\n",
				            icon_name, error->message);
				g_error_free (error);
			}
		} else {
			const gchar * const *xdg_dirs;
			gchar *path;
			gint i;

			/* Icon name is actually some filename in $(sharedir)/icons */
			xdg_dirs = g_get_system_data_dirs ();

			for (i = 0; !pixbuf && xdg_dirs[i]; i++) {
				path = g_build_filename (xdg_dirs[i], "icons", icon_name, NULL);
				pixbuf = gdk_pixbuf_new_from_file_at_size (path, 24, 24, NULL);
				g_free (path);
			}
		}
        } else if (category & CATEGORY_IMAGE) {
		gchar *path;

		path = g_file_get_path (file);
		pixbuf = gdk_pixbuf_new_from_file_at_size (path, 24, 24, &error);
		g_free (path);

		if (error) {
			g_printerr ("Couldn't get pixbuf for uri:'%s', %s\n",
			            uri,
			            error->message);
			g_clear_error (&error);
		} else {
			g_object_unref (file);
			return pixbuf;
		}
	} else if (category &
		   (CATEGORY_DOCUMENT |
		    CATEGORY_IMAGE |
		    CATEGORY_AUDIO |
		    CATEGORY_FOLDER |
		    CATEGORY_VIDEO |
		    CATEGORY_ARCHIVE)) {
		attributes =
			G_FILE_ATTRIBUTE_STANDARD_ICON;

		info = g_file_query_info (file,
		                          attributes,
		                          G_FILE_QUERY_INFO_NONE,
		                          NULL,
		                          &error);

		if (error) {
			g_printerr ("Couldn't get pixbuf for uri:'%s', %s\n",
			            uri,
			            error->message);
			g_object_unref (file);
			g_error_free (error);

			return NULL;
		}

		icon = g_file_info_get_icon (info);

		if (icon && G_IS_THEMED_ICON (icon)) {
			GtkIconInfo *icon_info;
			const gchar **names;

			names = (const gchar**) g_themed_icon_get_names (G_THEMED_ICON (icon));
			icon_info = gtk_icon_theme_choose_icon (window->icon_theme,
			                                        names,
			                                        24,
			                                        GTK_ICON_LOOKUP_USE_BUILTIN);

			if (icon_info) {
				pixbuf = gtk_icon_info_load_icon (icon_info, NULL);
				gtk_icon_info_free (icon_info);
			}
		}

		g_object_unref (info);
	} else {
		g_message ("No pixbuf could be retrieved for category %s (URI: %s)\n",
		           category_to_string (category), uri);
	}

	g_object_unref (file);

	return pixbuf;
}

static void
model_category_cell_data_func (GtkTreeViewColumn    *tree_column,
                               GtkCellRenderer      *cell,
                               GtkTreeModel         *model,
                               GtkTreeIter          *iter,
                               TrackerResultsWindow *window)
{
	GtkTreePath *path;
	GtkTreeIter prev_iter;
	TrackerCategory category;
	gboolean previous_path;
	gboolean print = FALSE;

	gtk_tree_model_get (model, iter, COL_CATEGORY_ID, &category, -1);

	/* Get the previous iter */
	path = gtk_tree_model_get_path (model, iter);

	previous_path = gtk_tree_path_prev (path);

	if (!previous_path) {
		print = TRUE;
	} else if (previous_path && gtk_tree_model_get_iter (model, &prev_iter, path)) {
		TrackerCategory prev_category = CATEGORY_NONE;

		gtk_tree_model_get (model, &prev_iter,
		                    COL_CATEGORY_ID, &prev_category,
		                    -1);

		if (prev_category == CATEGORY_NONE) {
			print = TRUE;
		}
	}

	g_object_set (cell,
	              "text", print ? category_to_string (category) : "",
	              "visible", print,
	              NULL);

	gtk_tree_path_free (path);
}

static void
model_pixbuf_cell_data_func (GtkTreeViewColumn    *tree_column,
                             GtkCellRenderer      *cell,
                             GtkTreeModel         *model,
                             GtkTreeIter          *iter,
                             TrackerResultsWindow *window)
{
	GdkPixbuf *pixbuf = NULL;
	gboolean requested = FALSE;

	gtk_tree_model_get (model, iter, COL_IMAGE_REQUESTED, &requested, -1);

	if (!requested) {
		TrackerCategory category = CATEGORY_NONE;
		gchar *urn, *icon_name;

		gtk_tree_model_get (model, iter,
		                    COL_CATEGORY_ID, &category,
		                    COL_URN, &urn,
				    COL_ICON_NAME, &icon_name,
		                    -1);

		if (urn) {
			/* FIXME: Should use category */
			pixbuf = pixbuf_get (window, urn, icon_name, category);
			g_free (urn);
			g_free (icon_name);
		}

		/* Cache it in the store */
		gtk_list_store_set (GTK_LIST_STORE (model), iter,
		                    COL_IMAGE, pixbuf,
		                    COL_IMAGE_REQUESTED, TRUE,
		                    -1);
	} else {
		/* We do this because there may be no image for a file
		 * and we don't want to keep requesting the same
		 * file's image.
		 */
		gtk_tree_model_get (model, iter, COL_IMAGE, &pixbuf, -1);
	}

	g_object_set (cell,
	              "visible", TRUE,
	              "pixbuf", pixbuf,
	              NULL);

	if (pixbuf) {
		g_object_unref (pixbuf);
	}
}

static gboolean
model_separator_func (GtkTreeModel *model,
                      GtkTreeIter  *iter,
                      gpointer      user_data)
{
#ifdef USE_SEPARATOR_FOR_SPACING
	gchar *urn;

	gtk_tree_model_get (model, iter, COL_URN, &urn, -1);

	if (!urn) {
		return TRUE;
	}

	g_free (urn);

	return FALSE;
#else  /* USE_SEPARATOR_FOR_SPACING */
	return FALSE;
#endif /* USE_SEPARATOR_FOR_SPACING */
}

static gboolean
model_selection_func (GtkTreeSelection *selection,
                      GtkTreeModel     *model,
                      GtkTreePath      *path,
                      gboolean          path_currently_selected,
                      gpointer          data)
{
	GtkTreeIter iter;
	gchar *urn;

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COL_URN, &urn, -1);

	if (!urn) {
		return FALSE;
	}

	g_free (urn);

	return TRUE;
}

static void
model_set_up (TrackerResultsWindow *window)
{
	GtkTreeView *view;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	GtkListStore *store;
	GtkCellRenderer *cell;

	view = GTK_TREE_VIEW (window->treeview);

	/* View */
	gtk_tree_view_set_enable_search (view, FALSE);

	/* Store */
	store = gtk_list_store_new (COL_COUNT,
	                            G_TYPE_INT,            /* Category ID */
	                            GDK_TYPE_PIXBUF,       /* Image */
	                            G_TYPE_BOOLEAN,        /* Image requested */
	                            G_TYPE_STRING,         /* URN */
	                            G_TYPE_STRING,         /* Title */
	                            G_TYPE_STRING,         /* Tooltip */
	                            G_TYPE_STRING,         /* Link */
				    G_TYPE_STRING);	   /* Icon name */

	gtk_tree_view_set_model (view, GTK_TREE_MODEL (store));

	gtk_tree_view_set_row_separator_func (view,
	                                      model_separator_func,
	                                      window,
	                                      NULL);

	/* Selection */
	selection = gtk_tree_view_get_selection (view);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	gtk_tree_selection_set_select_function (selection,
	                                        model_selection_func,
	                                        window,
	                                        NULL);

	/* Column: Category */
	column = gtk_tree_view_column_new ();
	cell = gtk_cell_renderer_text_new ();

	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_set_cell_data_func (column, cell,
	                                         (GtkTreeCellDataFunc)
	                                         model_category_cell_data_func,
	                                         window,
	                                         NULL);

	gtk_tree_view_column_set_title (column, _("Category"));
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_sort_column_id (column, COL_CATEGORY_ID);
	gtk_tree_view_append_column (view, column);

	/* Column: Icon + Title */
	column = gtk_tree_view_column_new ();

	cell = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, cell, FALSE);
	gtk_tree_view_column_set_cell_data_func (column, cell,
	                                         (GtkTreeCellDataFunc)
	                                         model_pixbuf_cell_data_func,
	                                         window,
	                                         NULL);
        g_object_set (cell,
                      "height", 24,
                      "width", 24,
                      NULL);

	cell = gtk_cell_renderer_text_new ();
	g_object_set (cell,
	              "xpad", 4,
	              "ypad", 1,
	              NULL);
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	gtk_tree_view_column_add_attribute (column, cell, "text", COL_TITLE);

	gtk_tree_view_column_set_title (column, _("Title"));
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
	gtk_tree_view_column_set_sort_column_id (column, COL_TITLE);
	gtk_tree_view_append_column (view, column);

	/* Sorting */
	/* gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), */
	/*                                    COL_CATEGORY_ID, */
	/*                                    GTK_SORT_ASCENDING); */

	/* Tooltips */
	gtk_tree_view_set_tooltip_column (view, COL_TOOLTIP);

	/* Save */
	window->store = G_OBJECT (store);
}

static void
model_add (TrackerResultsWindow *window,
           TrackerCategory       category,
           const gchar          *urn,
           const gchar          *title,
           const gchar          *tooltip,
           const gchar          *link,
           const gchar          *icon_name)
{
	GtkTreeIter iter;

	gtk_list_store_append (GTK_LIST_STORE (window->store), &iter);
	gtk_list_store_set (GTK_LIST_STORE (window->store), &iter,
	                    COL_CATEGORY_ID, category,
	                    COL_IMAGE, NULL,
	                    COL_URN, urn,
	                    COL_TITLE, title,
	                    COL_TOOLTIP, tooltip,
	                    COL_LINK, link,
			    COL_ICON_NAME, icon_name,
	                    -1);

	/* path = gtk_tree_model_get_path (GTK_TREE_MODEL (window->store), &iter); */
	/* gtk_tree_view_set_tooltip_row (GTK_TREE_VIEW (window->treeview), tooltip, path); */
	/* gtk_tree_path_free (path); */

	/* gtk_tree_selection_select_iter (selection, &iter); */
}

static void
search_window_ensure_not_blank (TrackerResultsWindow *self)
{
	if (self->queries_pending == 0) {
		GtkTreeIter iter;

		/* No more queries pending */
		if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->store), &iter)) {
			gchar *str;

			str = g_strdup_printf (_("No results found for “%s”"), self->query);
			gtk_label_set_text (GTK_LABEL (self->label), str);
			g_free (str);

			gtk_widget_hide (self->scrolled_window);
			gtk_widget_show (self->label);
		} else {
			gtk_widget_show_all (self->scrolled_window);
			gtk_widget_hide (self->label);
		}
	}
}

inline static void
search_get_foreach (SearchQuery         *sq,
		    TrackerSparqlCursor *cursor)
{
	ItemData *id;
	const gchar *urn, *title, *tooltip, *link, *rank, *icon_name;

	urn = tracker_sparql_cursor_get_string (cursor, 0, NULL);
	title = tracker_sparql_cursor_get_string (cursor, 1, NULL);
	tooltip = tracker_sparql_cursor_get_string (cursor, 2, NULL);
	link = tracker_sparql_cursor_get_string (cursor, 3, NULL);
	rank = tracker_sparql_cursor_get_string (cursor, 4, NULL);
	icon_name = tracker_sparql_cursor_get_string (cursor, 5, NULL);

	/* App queries don't return rank or belongs */
	if (!rank) {
		rank = "0.0";
	}

	if (icon_name && g_str_has_prefix (icon_name, "urn:theme-icon:")) {
		icon_name += strlen ("urn:theme-icon:");
	}

	g_print ("urn:'%s' found (rank:'%s')\n", urn, rank);
	g_print ("  title:'%s'\n", title);
	g_print ("  tooltip:'%s'\n", tooltip);
	g_print ("  link:'%s'\n", link);

	id = item_data_new (urn, title, tooltip, link, icon_name, sq->category);
	sq->results = g_slist_append (sq->results, id);

	/* category_from_string (type, &id->categories); */
	/* g_print ("  type:'%s', new categories:%d\n", type, id->categories); */
}

static void
search_get_cb (GObject      *source_object,
	       GAsyncResult *res,
	       gpointer      user_data)
{
	TrackerSparqlCursor *cursor;
	TrackerResultsWindow *window;
	SearchQuery *sq;
	GError *error = NULL;

	sq = user_data;

	cursor = tracker_sparql_connection_query_finish (TRACKER_SPARQL_CONNECTION (source_object),
							 res,
							 &error);

	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		g_error_free (error);
		return;
	}

	window = sq->window;

	/* If request IDs don't match, data is no longer needed */
	if (window->request_id != sq->request_id) {
		g_message ("Received data from request id:%d, now on request id:%d",
		           sq->request_id,
		           window->request_id);

		window->search_queries = g_list_remove (window->search_queries, sq);
		search_query_free (sq);

		/* We don't care about errors if we're not interested
		 * in the results.
		 */
		g_clear_error (&error);

		if (cursor) {
			g_object_unref (cursor);
		}

		return;
	}

	if (error) {
		g_printerr ("Could not get search results, %s\n", error->message);
		g_error_free (error);

		if (cursor) {
			g_object_unref (cursor);
		}

		window->search_queries = g_list_remove (window->search_queries, sq);
		search_query_free (sq);
		search_window_ensure_not_blank (window);

		return;
	}

	if (!cursor) {
		g_print ("No results were found matching the query in category:'%s'\n",
		         category_to_string (sq->category));
	} else {
		GSList *l;

		g_print ("Results: category:'%s'\n",
		         category_to_string (sq->category));

		/* FIXME: make async */
		while (tracker_sparql_cursor_next (cursor,
						   window->cancellable,
						   &error)) {
			search_get_foreach (sq, cursor);
		}

		/* Add separator */
		if (window->first_category_populated && g_slist_length (sq->results) > 0) {
			model_add (window, CATEGORY_NONE, NULL, NULL, NULL, NULL, NULL);
		} else {
			window->first_category_populated = TRUE;
		}

		for (l = sq->results; l; l = l->next) {
			ItemData *id = l->data;

			model_add (window,
			           sq->category,
			           id->urn,
			           id->title,
			           id->tooltip,
			           id->link,
			           id->icon_name);
		}

		g_object_unref (cursor);
	}

	window->search_queries = g_list_remove (window->search_queries, sq);
	search_query_free (sq);
	search_window_ensure_not_blank (window);

	if (window->queries_pending < 1) {
		g_print ("\n\n\n");
	}
}

static void
search_get (TrackerResultsWindow *window,
            TrackerCategory       category)
{
	SearchQuery *sq;
	gchar *sparql;

	if (!window->connection) {
		return;
	}

	switch (category) {
	case CATEGORY_IMAGE:
		sparql = g_strdup_printf (IMAGE_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_AUDIO:
		sparql = g_strdup_printf (MUSIC_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_VIDEO:
		sparql = g_strdup_printf (VIDEO_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_DOCUMENT:
		sparql = g_strdup_printf (DOCUMENT_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_FOLDER:
		sparql = g_strdup_printf (FOLDER_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_APPLICATION:
		sparql = g_strdup_printf (APP_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_TAG:
		sparql = g_strdup_printf (TAGS_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_BOOKMARK:
		sparql = g_strdup_printf (BOOKMARK_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_WEBSITE:
		sparql = g_strdup_printf (WEBSITE_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_CONTACT:
		sparql = g_strdup_printf (CONTACT_QUERY, window->query, MAX_ITEMS);
		break;
	case CATEGORY_NONE:
	case CATEGORY_EMAIL_ADDRESS:
	case CATEGORY_FONT:
	case CATEGORY_ARCHIVE:
	default:
		sparql = NULL;
		break;
	}

	if (sparql == NULL) {
		return;
	}

	sq = search_query_new (window->request_id, category, window);
	window->search_queries = g_list_prepend (window->search_queries, sq);

	tracker_sparql_connection_query_async (window->connection,
					       sparql,
					       sq->cancellable,
					       search_get_cb,
					       sq);
	g_free (sparql);

	window->queries_pending++;
}

static void
search_start (TrackerResultsWindow *window)
{
	GtkTreeModel *model;
	GtkListStore *store;

	/* Cancel current requests */
	window->request_id++;
	g_message ("Incrementing request ID to %d", window->request_id);

	/* Clear current data */
	g_message ("Clearing previous results");
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (window->treeview));
	store = GTK_LIST_STORE (model);
	gtk_list_store_clear (store);

	if (!window->query || strlen (window->query) < 1) {
		gtk_widget_show (window->scrolled_window);
		gtk_widget_hide (window->label);
		gtk_widget_hide (GTK_WIDGET (window));
		return;
	}

	window->first_category_populated = FALSE;

	/* Clean up previous requests, this will call
	 * g_cancellable_cancel() on each query still running.
	 */
	g_list_free_full (window->search_queries, (GDestroyNotify) search_query_free);
	window->search_queries = NULL;

	/* SPARQL requests */
	search_get (window, CATEGORY_IMAGE);
	search_get (window, CATEGORY_AUDIO);
	search_get (window, CATEGORY_VIDEO);
	search_get (window, CATEGORY_DOCUMENT);
	search_get (window, CATEGORY_FOLDER);
	search_get (window, CATEGORY_APPLICATION);
	search_get (window, CATEGORY_TAG);
	search_get (window, CATEGORY_BOOKMARK);
	search_get (window, CATEGORY_WEBSITE);
	search_get (window, CATEGORY_CONTACT);
}

static gboolean
grab_popup_window (TrackerResultsWindow *window)
{
	GdkGrabStatus status;
	GtkWidget *widget;
	guint32 time;

	widget = GTK_WIDGET (window);
	time = gtk_get_current_event_time ();

	/* Grab pointer */
	status = gdk_pointer_grab (gtk_widget_get_window (widget),
	                           TRUE,
	                           GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
	                           NULL, NULL,
	                           time);

	if (status == GDK_GRAB_SUCCESS) {
		status = gdk_keyboard_grab (gtk_widget_get_window (widget), TRUE, time);
	}

	if (status == GDK_GRAB_SUCCESS) {
		gtk_widget_grab_focus (widget);
	} else if (status == GDK_GRAB_NOT_VIEWABLE) {
		/* window is not viewable yet, retry */
		return TRUE;
	} else {
		gtk_widget_hide (widget);
	}

	return FALSE;
}

GtkWidget *
tracker_results_window_new (GtkWidget   *parent,
                            const gchar *query)
{
	return g_object_new (TRACKER_TYPE_RESULTS_WINDOW,
	                     "align-widget", parent,
	                     "query", query,
	                     NULL);
}

void
tracker_results_window_popup (TrackerResultsWindow *window)
{
        GtkAdjustment *vadj, *hadj;

        g_return_if_fail (TRACKER_IS_RESULTS_WINDOW (window));

	gtk_widget_realize (GTK_WIDGET (window));
	gtk_widget_show (GTK_WIDGET (window));

        /* Force scroll to top-left */
        vadj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (window->scrolled_window));
        gtk_adjustment_set_value (vadj, gtk_adjustment_get_lower (vadj));

        hadj = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (window->scrolled_window));
        gtk_adjustment_set_value (hadj, gtk_adjustment_get_lower (hadj));

        g_idle_add ((GSourceFunc) grab_popup_window, window);
}
