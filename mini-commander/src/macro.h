#include "preferences.h"

void expand_command(char *command, properties *prop);
int prefix_length(char *command, properties *prop);
int prefix_length_Including_whithespaces(char *command, properties *prop);
char * get_prefix(char *command, properties *prop);
