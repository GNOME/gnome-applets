/* webcontrol session saving and loading */

#ifndef __SESSION_H_
#define __SESSION_H_

#include <gnome.h>

#define WC_IS_BROWSER(string) !(g_strncasecmp (string, "browser:", 8))
#define WC_BROWSER_NAME(string) g_strchug ((strchr (string, ':') + 1))

void
wc_load_session (void);

void
wc_load_history (GtkCombo *);

gint
wc_save_session (GtkWidget *, gpointer);

#endif
