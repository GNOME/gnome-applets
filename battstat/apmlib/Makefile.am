# Makefile.am
#
# Author: Jörgen Pehrson <jp@spektr.eu.org>
# Created: Thu Jun 15 23:08:41 CEST 2000
#

NULL =

noinst_LIBRARIES = libapm.a

libapm_a_SOURCES = apmlib.c apm.h

libapm_a_CFLAGS = \
	-I$(top_srcdir)				\
	-I$(top_srcdir)/apmlib			\
	-DG_LOG_DOMAIN=\"battstat_applet\" \
	$(GNOME_APPLETS_CFLAGS) \
	$(WARN_CFLAGS) \
	$(AM_CFLAGS) \
	$(NULL)

libapm_a_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(AM_LDFLAGS) \
	$(NULL)

CLEANFILES = \
	libapm.a

-include $(top_srcdir)/git.mk
