#include <config.h>
#include <gkb.h>

#include <gtk/gtk.h>
#include <libgnome/gnome-program.h>
#include <libgnomeui/gnome-ui-init.h>
#include <locale.h>

int
main (int argc, char *argv[])
{
	GKB *gkb;

	gtk_init (&argc, &argv);

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init ("gkb", VERSION,
		LIBGNOMEUI_MODULE, argc, argv,
		GNOME_PARAM_APP_DATADIR, DATADIR,
		NULL);

	gkb = gkb_new ();
	properties_dialog (NULL, gkb, NULL);
	gtk_main ();

	return 0;
}
