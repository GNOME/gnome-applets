#ifndef BUSSIGN_H
#define BUSSIGN_H

#define IMAGE_FILENAME "test_image"

typedef struct bussign_properties_tag
{
  gchar *resource;
  gchar *host;
  gint   port;
  gchar *file;
} bussign_properties_t;

#endif
