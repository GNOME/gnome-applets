#include <applet-widget.h>

#define HISTORY_DEPTH 50
#define MAX_COMMAND_LENGTH 250
#define MAX_PREFIXES 99
#define MAX_PREFIX_LENGTH 50

typedef struct structProperties properties;
struct structProperties
{
    int showTime;
    int showDate;
    int normalSizeX;
    int normalSizeY;
    int reducedSizeX;
    int reducedSizeY;
    int cmdLineY;
    int cmdLineColorFgR;
    int cmdLineColorFgG;
    int cmdLineColorFgB;
    int cmdLineColorBgR;
    int cmdLineColorBgG;
    int cmdLineColorBgB;
    char *prefix[MAX_PREFIXES];
    char *command[MAX_PREFIXES];
};

extern properties prop;
extern properties propTmp;

extern GtkWidget *applet;

void loadSession(void);

void saveSession(void);

void saveSession(void);

gint saveSession_signal(GtkWidget *widget, const char *privcfgpath, const char *globcfgpath);

void propertiesBox(AppletWidget *applet, gpointer data);
