#ifndef HTTP_GET_H
#define HTTP_GET_H

#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>

int
http_get_to_file(gchar *a_host, gint a_port, gchar *a_resource, FILE *a_file);

#endif
