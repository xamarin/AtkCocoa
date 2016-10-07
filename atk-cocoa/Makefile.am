SUBDIRS = libatkcocoa-util

moduledir = $(libdir)/gtk-2.0/modules
module_LTLIBRARIES = libatkcocoa.la

BUILT_SOURCES = acmarshal.h acmarshal.c

atkcocoa_c_sources =			\
	ACAccessibilityCellElement.c	\
	ACAccessibilityElement.c	\
	ACAccessibilityOutlineElement.c \
	ACAccessibilityTreeCellElement.c \
	ACAccessibilityTreeColumnElement.c \
	ACAccessibilityTreeRowElement.c \
	acelement.c			\
	acmarshal.c			\
	acutils.c			\
	gail-private-macros.h		\
	gail.c				\
	gailadjustment.c		\
	gailarrow.c			\
	gailbooleancell.c		\
	gailbox.c			\
	gailbutton.c			\
	gailcalendar.c			\
	gailcell.c			\
	gailcellparent.c		\
	gailcheckmenuitem.c		\
	gailchecksubmenuitem.c		\
	gailclist.c			\
	gailclistcell.c			\
	gailcombo.c			\
	gailcombobox.c			\
	gailcontainer.c			\
	gailcontainercell.c		\
	gailentry.c			\
	gailexpander.c			\
	gailframe.c			\
	gailimage.c			\
	gailimagecell.c			\
	gailitem.c			\
	gaillabel.c			\
	gaillist.c			\
	gailmenu.c			\
	gailmenushell.c			\
	gailmenuitem.c			\
	gailnotebook.c			\
	gailnotebookpage.c		\
	gailobject.c			\
	gailoptionmenu.c		\
	gailpaned.c			\
	gailpixmap.c			\
	gailprogressbar.c		\
	gailradiobutton.c		\
	gailradiomenuitem.c		\
	gailradiosubmenuitem.c		\
	gailrange.c			\
	gailrenderercell.c		\
	gailscale.c			\
	gailscalebutton.c		\
	gailscrollbar.c			\
	gailscrolledwindow.c		\
	gailseparator.c			\
	gailspinbutton.c		\
	gailsubmenuitem.c		\
	gailstatusbar.c			\
	gailtextcell.c			\
	gailtextview.c			\
	gailtogglebutton.c		\
	gailtoplevel.c			\
	gailtreeview.c			\
	gailutil.c			\
	gailwidget.c			\
	gailwindow.c			\
	NSAccessibilityElement+AtkCocoa.c

libatkcocoaincludedir=$(includedir)/atkcocoa-1.0/atkcocoa

atkcocoa_private_h_sources =		\
	ACAccessibilityCellElement.h	\
	ACAccessibilityElement.h	\
	ACAccessibilityOutlineElement.h \
	ACAccessibilityTreeCellElement.h \
	ACAccessibilityTreeColumnElement.h \
	ACAccessibilityTreeRowElement.h \
	acdebug.h			\
	acelement.h			\
	acmarshal.h			\
	acutils.h			\
	gail.h				\
	gailadjustment.h		\
	gailarrow.h			\
	gailbooleancell.h		\
	gailbox.h			\
	gailbutton.h			\
	gailcalendar.h			\
	gailcell.h			\
	gailcellparent.h		\
	gailcheckmenuitem.h		\
	gailchecksubmenuitem.h		\
	gailclist.h			\
	gailclistcell.h			\
	gailcombo.h			\
	gailcombobox.h			\
	gailcontainercell.h		\
	gailcontainer.h			\
	gailentry.h			\
	gailexpander.h			\
	gailfactory.h			\
	gailframe.h			\
	gailimage.h			\
	gailimagecell.h			\
	gailitem.h			\
	gaillabel.h			\
	gaillist.h			\
	gailmenu.h			\
	gailmenushell.h			\
	gailmenuitem.h			\
	gailnotebook.h			\
	gailnotebookpage.h		\
	gailobject.h			\
	gailoptionmenu.h		\
	gailpaned.h			\
	gailpixmap.h			\
	gailprogressbar.h		\
	gailradiobutton.h		\
	gailradiomenuitem.h		\
	gailradiosubmenuitem.h		\
	gailrange.h			\
	gailrenderercell.h		\
	gailscale.h			\
	gailscalebutton.h		\
	gailscrollbar.h			\
	gailscrolledwindow.h		\
	gailseparator.h			\
	gailspinbutton.h		\
	gailsubmenuitem.h		\
	gailstatusbar.h			\
	gailtextcell.h			\
	gailtextview.h			\
	gailtogglebutton.h		\
	gailtoplevel.h			\
	gailtreeview.h			\
	gailutil.h			\
	gailwindow.h			\
	NSAccessibilityElement+AtkCocoa.h

atkcocoa_public_h_sources =	\
	gailwidget.h

libatkcocoa_la_SOURCES = 			\
	$(atkcocoa_c_sources)		\
	$(atkcocoa_public_h_sources)	\
	$(atkcocoa_private_h_sources)

libatkcocoainclude_HEADERS =		\
	$(atkcocoa_public_h_sources)

libatkcocoa_la_CPPFLAGS = \
	-I$(top_srcdir)			\
	-DGTK_VERSION=\"$(GTK_VERSION)\"\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	$(AM_CPPFLAGS)

libatkcocoa_la_CFLAGS = \
	-xobjective-c   \
	$(GTK_CFLAGS)	\
	$(AM_CFLAGS)

libatkcocoa_la_LIBADD =  \
	$(top_builddir)/atk-cocoa/libatkcocoa-util/libatkcocoautil.la	\
	$(GTK_LIBS)

libatkcocoa_la_LDFLAGS =    \
	-rpath $(moduledir) -module -avoid-version \
	$(no_undefined) \
        $(LDFLAGS)

acmarshal.h: stamp-acmarshal.h
	@true
stamp-acmarshal.h: acmarshal.list
	$(GLIB_GENMARSHAL) --prefix=_ac_marshal $(srcdir)/acmarshal.list --header > xgen-acmlh \
	&& (cmp -s xgen-acmlh acmarshal.h || cp xgen-acmlh acmarshal.h) \
	&& rm -f xgen-acmlh \
	&& echo timestamp > $(@F)
acmarshal.c: acmarshal.list
	(echo "#include \"acmarshal.h\""; \
	 $(GLIB_GENMARSHAL) --prefix=_ac_marshal $(srcdir)/acmarshal.list --body) >> xgen-acmlc \
	&& cp xgen-acmlc acmarshal.c \
	&& rm -f xgen-acmlc
