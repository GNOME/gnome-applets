NULL =

noinst_LTLIBRARIES = libscreen-exec.la

INCLUDES =				\
	$(WARN_CFLAGS)			\
	$(GNOME_LIBS2_CFLAGS)		\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
        -DGTK_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED	\
	$(NULL)

EGGFILES =				\
	egg-screen-exec.h		\
	egg-screen-exec.c		\
	egg-screen-help.h		\
	egg-screen-help.c		\
	egg-screen-url.h		\
	egg-screen-url.c		\
	$(NULL)

libscreen_exec_la_SOURCES = $(EGGFILES)

EGGDIR = $(srcdir)/../../libegg/libegg/screen-exec

regenerate-built-sources:
	EGGFILES="$(EGGFILES)" EGGDIR="$(EGGDIR)" $(srcdir)/update-from-egg.sh
