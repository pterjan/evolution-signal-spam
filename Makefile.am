INCLUDES =						\
	-I$(top_srcdir)					\
	$(SIGNAL_CFLAGS)			\
	$(CURL_CFLAGS)					\
	$(GTK_CFLAGS)					\
	$(GLIB_CFLAGS)

plugin_DATA = signal-spam.eplug signal-spam.xml
plugin_LTLIBRARIES = signal-spam.la

signal_spam_la_SOURCES = signal-spam.c report.c
signal_spam_la_LDFLAGS = $(CURL_LIBS) $(GLIB_LIBS) -module -avoid-version

EXTRA_DIST = \
	signal-spam.eplug.xml \
	signal-spam.xml \
	TODO 

BUILT_SOURCES = signal-spam.eplug
CLEANFILES = $(BUILT_SOURCES)

%.eplug: %.eplug.xml
	sed -e 's|\@PLUGINDIR\@|$(plugindir)|'          \
	-e 's|\@SOEXT\@|$(SOEXT)|'                      \
	-e 's|\@GETTEXT_PACKAGE\@|$(GETTEXT_PACKAGE)|'  \
	-e 's|\@LOCALEDIR\@|$(localedir)|' $< > $@
