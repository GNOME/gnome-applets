/* Fifteen sliding pieces game
 *
 * Sam Lloyd's classic game for the Gnome Panel
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#include <config.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>
#include <assert.h>

/* the piece size is for 48 and will be scaled to the proper size */
#define PIECE_SIZE 11
#define PIECE_FONT "5x8"
#define SCRAMBLE_MOVES 256

gboolean scrambled = FALSE; /* whether or not the board has been
                               scrambled since the last win */
unsigned moves = 0; /* player's moves so far */

static void
free_stuff (GtkObject *object, gpointer data)
{
	if (data)
		g_free (data);
	return;
}

static void
test_win (GnomeCanvasItem **board)
{
        GtkWidget *dlg;
	int i;
	gchar *buf;

	if (!scrambled)
		return;

	for (i = 0; i < 15; i++)
		if (!board[i] || (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (board[i]), "piece_num")) != i))
			return;

	buf = g_strdup_printf("%s : %u", _("You win!\nMoves"), moves);
	dlg = gnome_ok_dialog(buf);
	gtk_window_set_modal(GTK_WINDOW(dlg),TRUE);
	gnome_dialog_run (GNOME_DIALOG (dlg));
	g_free(buf);
	moves = 0;
	scrambled = FALSE;
}

static char *
get_piece_color (int piece)
{
	static char buf[50];
	int x, y;
	int r, g, b;

	y = piece / 4;
	x = piece % 4;

	r = ((4 - x) * 255) / 4;
	g = ((4 - y) * 255) / 4;
	b = 128;

	g_snprintf (buf, sizeof(buf), "#%02x%02x%02x", r, g, b);

	return buf;
}

static gint
piece_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
	GnomeCanvas *canvas;
	GnomeCanvasItem **board;
	GnomeCanvasItem *text;
	int num, pos;
	int x, y;
	int dx, dy;
	int bx, by;
	int blank, dist;

	canvas = item->canvas;
	board = gtk_object_get_user_data (GTK_OBJECT (canvas));
	num = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "piece_num"));
	pos = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "piece_pos"));
	text = gtk_object_get_data (GTK_OBJECT (item), "text");

	switch (event->type) {
	case GDK_ENTER_NOTIFY:
		gnome_canvas_item_set (text, "fill_color", "white", NULL);
		break;

	case GDK_LEAVE_NOTIFY:
		gnome_canvas_item_set (text, "fill_color", "black", NULL);
		break;

	case GDK_BUTTON_PRESS:
		for (blank = 0; blank < 16; blank++)
			if (board[blank] == NULL)
				break;
		assert(blank != 16); /* no blank on board? */
		bx = blank % 4;
		by = blank / 4;

		y = pos / 4;
		x = pos % 4;

		if (x == bx && y > by) { /* piece up */
			dx = 0;
			dy = -1;
			dist = y - by;
		} else if (x == bx && y < by) { /* down */
			dx = 0;
			dy = +1;
			dist = by - y;
		} else if (x > bx && y == by) { /* left */
			dx = -1;
			dy = 0;
			dist = x - bx;
		} else if (x < bx && y == by) { /* right */
			dx = +1;
			dy = 0;
			dist = bx - x;
		} else { /* no move */
			break;
		}

		moves +=  dist;
		while (dist--) {
			pos = blank - 4 * dy - dx;
			gtk_object_set_data (GTK_OBJECT (board[pos]),
					     "piece_pos",
					     GINT_TO_POINTER (blank));
			gnome_canvas_item_move (board[pos], dx * PIECE_SIZE,
						dy * PIECE_SIZE);
			board[blank] = board[pos];
			board[pos] = NULL;
			blank = pos;
		}
		test_win (board);

		break;

	default:
		break;
	}

	return FALSE;
}

static GtkWidget *
create_fifteen (int size, GtkWidget **canvas)
{
	GtkWidget *frame;
	GnomeCanvasItem **board;
	GnomeCanvasItem *text;
	int i, x, y;
	char buf[20];
	double scale_factor = size/48.0;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	*canvas = gnome_canvas_new ();
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (*canvas),
					  scale_factor);
	gtk_widget_set_usize (*canvas, scale_factor * PIECE_SIZE * 4,
			      PIECE_SIZE * 4);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (*canvas), 0, 0,
					PIECE_SIZE * 4, PIECE_SIZE * 4);
	gnome_canvas_scroll_to (GNOME_CANVAS (*canvas), 0, 0);
	gtk_container_add (GTK_CONTAINER (frame), *canvas);
	gtk_widget_show (*canvas);

	gtk_object_set_user_data (GTK_OBJECT (frame), *canvas);

	/* Create the board */

	board = g_new (GnomeCanvasItem *, 16);
	gtk_object_set_user_data (GTK_OBJECT (*canvas), board);
	gtk_signal_connect (GTK_OBJECT (*canvas), "destroy",
			    (GtkSignalFunc) free_stuff,
			    board);

	for (i = 0; i < 15; i++) {
		y = i / 4;
		x = i % 4;

		board[i] = gnome_canvas_item_new (GNOME_CANVAS_GROUP (GNOME_CANVAS (*canvas)->root),
						  gnome_canvas_group_get_type (),
						  "x", (double) (x * PIECE_SIZE),
						  "y", (double) (y * PIECE_SIZE),
						  NULL);

		gnome_canvas_item_new (GNOME_CANVAS_GROUP (board[i]),
				       gnome_canvas_rect_get_type (),
				       "x1", 0.0,
				       "y1", 0.0,
				       "x2", (double) PIECE_SIZE,
				       "y2", (double) PIECE_SIZE,
				       "fill_color", get_piece_color (i),
				       "outline_color", "black",
				       "width_pixels", 0,
				       NULL);

		g_snprintf (buf, sizeof(buf), "%d", i + 1);

		text = gnome_canvas_item_new (GNOME_CANVAS_GROUP (board[i]),
					      gnome_canvas_text_get_type (),
					      "text", buf,
					      "x", (double) PIECE_SIZE / 2.0,
					      "y", (double) PIECE_SIZE / 2.0,
					      "font", PIECE_FONT,
					      "anchor", GTK_ANCHOR_CENTER,
					      "fill_color", "black",
					      NULL);

		gtk_object_set_data (GTK_OBJECT (board[i]), "piece_num", GINT_TO_POINTER (i));
		gtk_object_set_data (GTK_OBJECT (board[i]), "piece_pos", GINT_TO_POINTER (i));
		gtk_object_set_data (GTK_OBJECT (board[i]), "text", text);
		gtk_signal_connect (GTK_OBJECT (board[i]), "event",
				    (GtkSignalFunc) piece_event,
				    NULL);
	}

	board[15] = NULL;

	/* Done */

	return frame;
}

static void
scramble (AppletWidget *applet, gpointer data)
{
	GnomeCanvas *canvas;
	GnomeCanvasItem **board;
	int i;
	int pos, oldpos;
	int dir;
	int x, y;

	srandom (time (NULL));

	canvas = gtk_object_get_user_data (data);
	board = gtk_object_get_user_data (GTK_OBJECT (canvas));

	/* First, find the blank spot */

	for (pos = 0; pos < 16; pos++)
		if (board[pos] == NULL)
			break;

	/* "Move the blank spot" around in order to scramble the pieces */

	for (i = 0; i < SCRAMBLE_MOVES; i++) {
/* retry_scramble: */
/* Yuck ;) --Tom. */
		dir = random () % 4;

		x = y = 0;

		if ((dir == 0) && (pos > 3)) /* up */
			y = -1;
		else if ((dir == 1) && (pos < 12)) /* down */
			y = 1;
		else if ((dir == 2) && ((pos % 4) != 0)) /* left */
			x = -1;
		else if ((dir == 3) && ((pos % 4) != 3)) /* right */
			x = 1;
		else
			continue; /* was: goto retry_scramble; --Tom */

		oldpos = pos + y * 4 + x;
		board[pos] = board[oldpos];
		board[oldpos] = NULL;
		gtk_object_set_data (GTK_OBJECT (board[pos]), "piece_pos", GINT_TO_POINTER (pos));
		gnome_canvas_item_move (board[pos], -x * PIECE_SIZE, -y * PIECE_SIZE);
		gnome_canvas_update_now (canvas);
		pos = oldpos;
	}
	moves = 0;
	scrambled = TRUE;

	return;
}

static void
about (AppletWidget *applet, gpointer data)
{
	static const char *authors[] = { "Federico Mena", NULL };
	static GtkWidget *about_box = NULL;

	if (about_box != NULL)
	{
		gdk_window_show(about_box->window);
		gdk_window_raise(about_box->window);
		return;
	}
	about_box = gnome_about_new (_("Fifteen sliding pieces"),
				     VERSION,
				     _("Copyright (C) The Free Software Foundation"),
				     authors,
				     _("Sam Lloyd's all-time favorite game, "
				       "now for your delight in the Gnome Panel. "
				       "Guaranteed to be a productivity buster."),
				     NULL);
	gtk_signal_connect( GTK_OBJECT(about_box), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box );

	gtk_widget_show(about_box);
	return;
}

static void
change_pixel_size(GtkWidget *w, int size, gpointer data)
{
	GnomeCanvas *canvas = data;
	double scale_factor = size/48.0;

	gnome_canvas_set_pixels_per_unit (canvas, scale_factor);

	gtk_widget_set_usize (GTK_WIDGET (canvas),
			      scale_factor * PIECE_SIZE * 4,
			      scale_factor * PIECE_SIZE * 4);
	gnome_canvas_scroll_to (canvas, 0, 0);
	return;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
    GnomeHelpMenuEntry help_entry = { "fifteen_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
}

int
main (int argc, char **argv)
{
	GtkWidget *applet;
	GtkWidget *fifteen;
	GtkWidget *canvas = NULL;
	int size;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init ("fifteen_applet", VERSION, argc,
			    argv, NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-fifteen.png");

	applet = applet_widget_new ("fifteen_applet");
	if (!applet)
		g_error (_("Can't create fifteen applet!"));

	size = applet_widget_get_panel_pixel_size(APPLET_WIDGET(applet));
	fifteen = create_fifteen (size, &canvas);
	applet_widget_add (APPLET_WIDGET (applet), fifteen);
	gtk_widget_show (fifteen);

	/* here it is ok to connect here, this is because we have already
	 * gotten the size before, and thus don't care about the initial
	 * signal */
	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
			   GTK_SIGNAL_FUNC(change_pixel_size),
			   canvas);


	gtk_widget_show (applet);

	applet_widget_register_callback (APPLET_WIDGET (applet),
					 "scramble",
					 _("Scramble pieces"),
					 scramble,
					 fifteen);

	applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"), help_cb, NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       about,
					       NULL);

	applet_widget_gtk_main ();

	return 0;
}
