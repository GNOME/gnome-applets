NULL =

noinst_LTLIBRARIES = libscreen-exec.la

INCLUDES =				\
	$(WARN_CFLAGS)			\
	$(GNOME_LIBS2_CFLAGS)		\
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
