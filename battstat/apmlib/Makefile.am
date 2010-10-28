# Makefile.am
#
# Author: Jörgen Pehrson <jp@spektr.eu.org>
# Created: Thu Jun 15 23:08:41 CEST 2000
#

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/apmlib			\
	$(GNOME_APPLETS_CFLAGS)			\
	-DG_LOG_DOMAIN=\"battstat_applet\"

noinst_LIBRARIES = libapm.a

libapm_a_SOURCES = apmlib.c apm.h


-include $(top_srcdir)/git.mk
