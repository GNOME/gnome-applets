/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"
#include "modules.h"

/*
 *-------------------------------------------------------------------
 * If adding a new module, all that should be needed to include it is
 * to initialize it here.
 *-------------------------------------------------------------------
 */

GList *modules_build_list(AppData *ad)
{
	GList *list = NULL;

	list = g_list_append(list, (gpointer)mod_test_init(ad));
	list = g_list_append(list, (gpointer)mod_coredump_init(ad));
	list = g_list_append(list, (gpointer)mod_loadavg_init(ad));
	list = g_list_append(list, (gpointer)mod_tail_init(ad));
	list = g_list_append(list, (gpointer)mod_news_init(ad));

	return list;
}

