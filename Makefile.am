SUBDIRS = po

INCLUDES = \
	-I$(top_srcdir)	\
	$(EVOLUTION_CFLAGS) \
	$(CAMEL_CFLAGS)	\
	$(CURL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS)

plugin_DATA = signal-spam.eplug signal-spam.xml
plugin_LTLIBRARIES = signal-spam.la

signal_spam_la_SOURCES = signal-spam.c report.c
signal_spam_la_LDFLAGS = $(CURL_LIBS) $(GLIB_LIBS) $(GTK_LIBS) $(EVOLUTION_LIBS) $(CAMEL_LIBS) -module -avoid-version

EXTRA_DIST = \
	signal-spam.eplug.xml \
	signal-spam.xml \
	TODO \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

DISTCLEANFILES = \
  intltool-extract \
  intltool-merge \
  intltool-update

BUILT_SOURCES = signal-spam.eplug
CLEANFILES = $(BUILT_SOURCES)

%.eplug: %.eplug.in
	sed -e 's|\@PLUGINDIR\@|$(plugindir)|'          \
	-e 's|\@SOEXT\@|$(SOEXT)|'                      \
	-e 's|\@GETTEXT_PACKAGE\@|$(GETTEXT_PACKAGE)|'  \
	-e 's|\@LOCALEDIR\@|$(localedir)|' $< > $@

%.eplug.in: %.eplug.xml
	LC_ALL=C $(INTLTOOL_MERGE) -x -u /tmp $< $@
