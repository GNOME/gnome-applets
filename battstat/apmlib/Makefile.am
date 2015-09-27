# Makefile.am
#
# Author: Jörgen Pehrson <jp@spektr.eu.org>
# Created: Thu Jun 15 23:08:41 CEST 2000
#

NULL =

noinst_LTLIBRARIES = libapm.la

libapm_la_SOURCES = \
	apmlib.c \
	apm.h \
	$(NULL)

libapm_la_CFLAGS = \
	-I$(top_srcdir)				\
	-I$(top_srcdir)/apmlib			\
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libapm_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

-include $(top_srcdir)/git.mk
