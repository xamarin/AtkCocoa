lib_LTLIBRARIES = libatkcocoautil.la

util_c_sources =		\
	gailmisc.c		\
	gailtextutil.c

libatkcocoautilincludedir=$(includedir)/atkcocoa-1.0/libatkcocoa

util_public_h_sources =		\
	gailmisc.h		\
	gailtextutil.h		\
	gail-util.h

libatkcocoautil_la_SOURCES = 	\
	$(util_c_sources)

libatkcocoautilinclude_HEADERS =	\
	$(util_public_h_sources)

libatkcocoautil_la_CPPFLAGS = \
	-I$(top_srcdir)		\
	$(AM_CPPFLAGS) 		\
	-DGDK_DISABLE_DEPRECATED\
	-DGTK_DISABLE_DEPRECATED

libatkcocoautil_la_CFLAGS = \
	$(GTK_CFLAGS)	\
	$(AM_CFLAGS)

libatkcocoautil_la_LIBADD = \
	$(GTK_LIBS)

libatkcocoautil_la_LDFLAGS = \
	$(no_undefined)				\
	$(export_symbols)			\
        $(LDFLAGS)
