/* Fifteen sliding pieces game
 *
 * Sam Lloyd's classic game for the Gnome Panel
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 */

#include <config.h>
#include <applet-lib.h>


#define PIECE_SIZE 11
#define PIECE_FONT "5x8"
#define SCRAMBLE_MOVES 256


static void
free_stuff (GtkObject *object, gpointer data)
{
	if (data)
		g_free (data);
}

static void
test_win (GnomeCanvasItem **board)
{
	int i;

	for (i = 0; i < 15; i++)
		if (!board[i] || (GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (board[i]), "piece_num")) != i))
			return;

	gnome_dialog_run_modal (GNOME_DIALOG (gnome_ok_dialog (_("You win!"))));
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

	sprintf (buf, "#%02x%02x%02x", r, g, b);

	return buf;
}

static gint
piece_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
	GnomeCanvas *canvas;
	GnomeCanvasItem **board;
	GnomeCanvasItem *text;
	int num, pos, newpos;
	int x, y;
	double dx = 0.0, dy = 0.0;
	int move;

	canvas = item->canvas;
	board = gtk_object_get_user_data (GTK_OBJECT (canvas));
	num = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "piece_num"));
	pos = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (item), "piece_pos"));
	text = gtk_object_get_data (GTK_OBJECT (item), "text");

	switch (event->type) {
	case GDK_ENTER_NOTIFY:
		gnome_canvas_item_set (text,
				       "fill_color", "white",
				       NULL);
		break;

	case GDK_LEAVE_NOTIFY:
		gnome_canvas_item_set (text,
				       "fill_color", "black",
				       NULL);
		break;

	case GDK_BUTTON_PRESS:
		y = pos / 4;
		x = pos % 4;

		move = TRUE;

		if ((y > 0) && (board[(y - 1) * 4 + x] == NULL)) {
			dx = 0.0;
			dy = -1.0;
			y--;
		} else if ((y < 3) && (board[(y + 1) * 4 + x] == NULL)) {
			dx = 0.0;
			dy = 1.0;
			y++;
		} else if ((x > 0) && (board[y * 4 + x - 1] == NULL)) {
			dx = -1.0;
			dy = 0.0;
			x--;
		} else if ((x < 3) && (board[y * 4 + x + 1] == NULL)) {
			dx = 1.0;
			dy = 0.0;
			x++;
		} else
			move = FALSE;

		if (move) {
			newpos = y * 4 + x;
			board[pos] = NULL;
			board[newpos] = item;
			gtk_object_set_data (GTK_OBJECT (item), "piece_pos", GINT_TO_POINTER (newpos));
			gnome_canvas_item_move (item, dx * PIECE_SIZE, dy * PIECE_SIZE);
			test_win (board);
		}

		break;

	default:
		break;
	}

	return FALSE;
}

static GtkWidget *
create_fifteen (void)
{
	GtkWidget *frame;
	GtkWidget *canvas;
	GnomeCanvasItem **board;
	GnomeCanvasItem *text;
	int i, x, y;
	char buf[20];

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	canvas = gnome_canvas_new ();
	gnome_canvas_set_size (GNOME_CANVAS (canvas), PIECE_SIZE * 4, PIECE_SIZE * 4);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0, 0, PIECE_SIZE * 4, PIECE_SIZE * 4);
	gtk_container_add (GTK_CONTAINER (frame), canvas);
	gtk_widget_show (canvas);

	gtk_object_set_user_data (GTK_OBJECT (frame), canvas);

	/* Create the board */

	board = g_new (GnomeCanvasItem *, 16);
	gtk_object_set_user_data (GTK_OBJECT (canvas), board);
	gtk_signal_connect (GTK_OBJECT (canvas), "destroy",
			    (GtkSignalFunc) free_stuff,
			    board);

	for (i = 0; i < 15; i++) {
		y = i / 4;
		x = i % 4;

		board[i] = gnome_canvas_item_new (GNOME_CANVAS_GROUP (GNOME_CANVAS (canvas)->root),
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

		sprintf (buf, "%d", i + 1);

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

	srand (time (NULL));

	canvas = gtk_object_get_user_data (data);
	board = gtk_object_get_user_data (GTK_OBJECT (canvas));

	/* First, find the blank spot */

	for (pos = 0; pos < 16; pos++)
		if (board[pos] == NULL)
			break;

	/* "Move the blank spot" around in order to scramble the pieces */

	for (i = 0; i < SCRAMBLE_MOVES; i++) {
retry_scramble:
		dir = rand () % 4;

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
			goto retry_scramble;

		oldpos = pos + y * 4 + x;
		board[pos] = board[oldpos];
		board[oldpos] = NULL;
		gtk_object_set_data (GTK_OBJECT (board[pos]), "piece_pos", GINT_TO_POINTER (pos));
		gnome_canvas_item_move (board[pos], -x * PIECE_SIZE, -y * PIECE_SIZE);
		gnome_canvas_update_now (canvas);
		pos = oldpos;
	}
}

static void
about (AppletWidget *applet, gpointer data)
{
	static const char *authors[] = { "Federico Mena", NULL };
	GtkWidget *about_box;

	about_box = gnome_about_new (_("Fifteen sliding pieces"),
				     _("1.0"),
				     _("Copyright (C) The Free Software Foundation"),
				     authors,
				     _("Sam Lloyd's all-time favorite game, "
				       "now for your delight in the Gnome Panel. "
				       "Guaranteed to be a productivity buster."),
				     NULL);

	gtk_widget_show(about_box);
}

int
main (int argc, char **argv)
{
	GtkWidget *applet;
	GtkWidget *fifteen;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init_defaults ("fifteen_applet", VERSION, argc,
				     argv, NULL, 0, NULL);

	applet = applet_widget_new ();
	if (!applet)
		g_error (_("Can't create fifteen applet!"));

	fifteen = create_fifteen ();
	applet_widget_add (APPLET_WIDGET (applet), fifteen);
	gtk_widget_show (fifteen);

	gtk_widget_show (applet);

	applet_widget_register_callback (APPLET_WIDGET (applet),
					 "scramble",
					 _("Scramble pieces"),
					 scramble,
					 fifteen);

	applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       about,
					       NULL);

	applet_widget_gtk_main ();

	return 0;
}
