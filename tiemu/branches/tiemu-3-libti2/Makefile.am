## Process this file with automake to produce Makefile.in

# subdirectories to scan
SUBDIRS = build glade help misc pedrom pixmaps skins src

if USE_GROFF
SUBDIRS += man
endif

dist_pkgdata_DATA = COPYING RELEASE AUTHORS LICENSES

EXTRA_DIST = README.* TODO docs

# install hook to install prebuilt Manpage.txt if groff is not available
install-data-local:
if !USE_GROFF
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir) 
	$(INSTALL_DATA) $(srcdir)/man/Manpage.txt $(DESTDIR)$(pkgdatadir)/Manpage.txt
endif

# custom targets
install-without-tcl-tk:
if USE_GROFF
	$(MAKE) install SUBDIRS="build glade help pedrom pixmaps skins man" DESTDIR="$(DESTDIR)"
else
	$(MAKE) install SUBDIRS="build glade help pedrom pixmaps skins" DESTDIR="$(DESTDIR)"
endif
	cd src && $(MAKE) install-without-tcl-tk DESTDIR="$(DESTDIR)"
