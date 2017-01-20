#################################################################
#								#
#  Konwert - conversion between various character encodings	#
#								#
#  Konwert - konwersje miêdzy ró¿nymi kodowaniami znaków  	#
#								#
#################################################################

version = 1.8

SHELL = /bin/sh
CXXFLAGS = -Wall -O2 -fomit-frame-pointer -pipe
INSTALL = install

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
libdir = $(exec_prefix)/lib
sharedir = $(exec_prefix)/share

mylibdir = $(libdir)/konwert
mysharedir = $(sharedir)/konwert

# Instalowane programy:

srcbindir = bin

# Wykonywalne pliki z tego katalogu s± atomowymi filtrami:

filtersdir = $(mysharedir)/filters
srcfiltersdir = filters

# Pomocnicze programy i dane wykorzystywane przez filtry:

auxdir = $(mysharedir)/aux
libauxdir = $(mylibdir)/aux
srcauxdir = aux
srccharsetsdir = $(srcauxdir)/charsets

# Programy i dane pomagaj±ce tworzyæ tablice konwersji (nie s± potrzebne do
# u¿ywania konwertera):

develdir = $(mysharedir)/devel
libdeveldir = $(mylibdir)/devel
srcdeveldir = devel

# Dokumentacja:

mydocdir = $(prefix)/doc/konwert-$(version)
srcdocdir = doc
mandir = $(prefix)/man
srcmandir = man

# Wersje z poprawionymi ¶cie¿kami w #!, gotowe do zainstalowania:

installdir = install
installbindir = $(installdir)/bin
installlibdir = $(installdir)/lib
installsharedir = $(installdir)/share
installfiltersdir = $(installsharedir)/filters
installauxdir = $(installsharedir)/aux
installlibauxdir = $(installlibdir)/aux
installdeveldir = $(installsharedir)/devel
installlibdeveldir = $(installlibdir)/devel
installdocdir = $(installdir)/doc
installmandir = $(installdir)/man

# Perl:

perl := $(shell which perl)

###############################################################################

# Niektóre zestawy znaków powstaj± przez na³o¿enie ma³ej tablicy na inn±:

mergewithwhat := $(patsubst $(srcdeveldir)/mergewith%,%, \
	$(wildcard $(srcdeveldir)/mergewith*))
srcmergefiles := $(foreach dir,$(mergewithwhat), \
	$(wildcard $(srcdeveldir)/mergewith$(dir)/*))
mergefiles := $(patsubst %,$(srccharsetsdir)/%,$(notdir $(srcmergefiles)))

# Poprawka \[...\] w UTF-8:

fixutf8files := $(patsubst $(srcdeveldir)/fixutf8/%,%, \
	$(shell find $(srcdeveldir)/fixutf8 -type f))

# Zestawy znaków zdefiniowane bezpo¶redni± tabel± konwersji na UTF-8:

charsets := $(patsubst $(srccharsetsdir)/%,%, \
	$(sort $(wildcard $(srccharsetsdir)/*) $(mergefiles)))

# Konwersje zestawów znaków na UTF-8:

charset_utf8files := $(patsubst %,$(srcfiltersdir)/%-UTF8,$(charsets))

# Konwersje zestawów znaków z UTF-8 s± tworzone automatycznie, na podstawie
# konwersji odwrotnych (na UTF-8):

utf8_charsetfiles := $(patsubst %,$(srcfiltersdir)/UTF8-%,$(charsets))

# Pomiêdzy niektórymi parami zestawów znaków s± bezpo¶rednie konwersje,
# oprócz konwersji przez tymczasowy format UTF-8.

charset_charset := $(shell $(perl) -nae \
	' \
		if (@F == 1) {print} \
		else \
		{ \
			foreach $$a (grep {!/^-/} @F) \
			{ \
				$$a =~ s/-$$//; \
				foreach $$b (grep {!/-$$/} @F) \
				{ \
					$$b =~ s/^-//; \
					print "$$a-$$b\n" if $$a ne $$b; \
				} \
			} \
		} \
	' $(srcdeveldir)/charset-charset)
charset_charsetfiles := $(patsubst %,$(srcfiltersdir)/%,$(charset_charset))

# Opisy czêsto¶ci znaków w poszczególnych zestawach znaków ró¿nych jêzyków
# - do automatycznego rozpoznawania standardu:

anyfiles := $(patsubst $(srcdeveldir)/any/%,$(srcauxdir)/any/%, \
	$(wildcard $(srcdeveldir)/any/*))

# Wersje z poprawionymi #!/¶cie¿kami, gotowe do zainstalowania:

installfiles := \
	$(installbindir)/konwert \
	$(patsubst %,$(installsharedir)/%,\
		$(sort \
			$(shell find \
				$(srcfiltersdir) \
				$(srcauxdir) \
				$(srcdeveldir) \
				-type f \
			) \
			$(mergefiles) \
			$(fixutf8files) \
			$(charset_utf8files) \
			$(srcauxdir)/UTF8-ascii \
			$(srcauxdir)/UTF8-ascii1 \
			$(utf8_charsetfiles) \
			$(charset_charsetfiles) \
			$(anyfiles) \
		) \
	) \
	$(installlibauxdir)/512bold \
	$(installlibauxdir)/512bold1

# Wszystkie tworzone pliki:

targets := \
	$(srcbindir)/trs \
	$(srcbindir)/filterm \
	$(mergefiles) \
	$(fixutf8files) \
	$(charset_utf8files) \
	$(srcauxdir)/UTF8-ascii \
	$(srcauxdir)/UTF8-ascii1 \
	$(utf8_charsetfiles) \
	$(charset_charsetfiles) \
	$(srcbindir)/512bold \
	$(srcbindir)/512bold1 \
	$(anyfiles) \
	$(installfiles)

# Pliki, które mog± byæ utworzone, ale nie s± w³±czone do dystrybucji
# ani kasowane przy `make clean':

nodelete := \
	$(mergefiles) \
	$(utf8_charsetfiles) \
	$(charset_charsetfiles)

###############################################################################

all: $(targets)

###############################################################################

$(srcbindir)/filterm: LDLIBS=-lutil

$(installbindir)/konwert: $(srcbindir)/konwert
	sed 's|/usr/share/konwert/filters|$(filtersdir)|g' $< >$@
	chmod +x $@

$(installsharedir)/%: %
	$(INSTALL) -d $(dir $@)
	if [ -x $< ]; then sed -e '1s|^#!/usr/bin/trs|#!$(bindir)/trs|' \
	-e '1s|^#!/usr/bin/perl|#!$(perl)|' $< >$@; chmod +x $@; \
	else ln $< $@; fi

$(installlibauxdir)/512bold: $(srcbindir)/512bold
	$(INSTALL) -d $(dir $@)
	$(INSTALL) -m755 -s $< $@

$(installlibauxdir)/512bold1: $(srcbindir)/512bold1
	$(INSTALL) -d $(dir $@)
	$(INSTALL) -m755 -s $< $@

###############################################################################

mkmerge: $(patsubst %,$(srcdeveldir)/mergewith%,$(mergewithwhat))

	{ \
		for charset in $(mergewithwhat); do \
		echo "\$$(patsubst \$$(srcdeveldir)/mergewith$$charset/%,\$$(srccharsetsdir)/%, \\"; \
		echo "\$$(wildcard \$$(srcdeveldir)/mergewith$$charset/*)): \\"; \
		echo "\$$(srccharsetsdir)/%: \\"; \
		echo "\$$(srcdeveldir)/mergewith$$charset/% \\"; \
		echo "\$$(srccharsetsdir)/$$charset \\"; \
		echo "\$$(srcdeveldir)/mergetrs"; \
		echo -e "\t\$$(perl) \$$(srcdeveldir)/mergetrs" \
		"\$$< \$$(srccharsetsdir)/$$charset >\$$@"; \
		done; \
	} >$@

include mkmerge

###############################################################################

$(fixutf8files): \
%: $(srcdeveldir)/fixutf8/% \
$(srcdeveldir)/fixtrsutf8

	$(perl) $(srcdeveldir)/fixtrsutf8 $< >$@
	chmod +x $@

###############################################################################

$(charset_utf8files): \
$(srcfiltersdir)/%-UTF8: $(srcdeveldir)/charset-UTF8
	ln $< $@

###############################################################################

$(srcauxdir)/UTF8-ascii: $(srcdeveldir)/UTF8-charset \
$(srcdeveldir)/mkUTF8-ascii

	$(perl) $(srcdeveldir)/mkUTF8-ascii $< >$@

###############################################################################

$(srcauxdir)/UTF8-ascii1: $(srcdeveldir)/UTF8-charset \
$(srcdeveldir)/mkUTF8-ascii

	$(perl) $(srcdeveldir)/mkUTF8-ascii -1 $< >$@

###############################################################################

$(utf8_charsetfiles): \
$(srcfiltersdir)/UTF8-%: $(srccharsetsdir)/% \
$(srcdeveldir)/UTF8-charset \
$(srcdeveldir)/mkUTF8-charset $(srcdeveldir)/mime

	{ \
		echo '#!/bin/bash -'; \
		echo; \
		$(perl) $(srcdeveldir)/mkUTF8-charset $< \
			$(srcdeveldir)/UTF8-charset; \
		echo " MIME=`$(srcdeveldir)/mime $(patsubst $(srcfiltersdir)/UTF8-%,%,$@)`" \
			"\"\$${0%/*}/../aux/UTF8-charset\"" \
		-rf "\"\$${0%/*}/../aux/charsets/$(patsubst $(srcfiltersdir)/UTF8-%,%,$@)\""; \
	} >$@
	chmod +x $@

###############################################################################

$(srcfiltersdir)/UTF8-ascii: \
$(srcdeveldir)/UTF8-charset $(srcdeveldir)/UTF8-ascii \
$(srcdeveldir)/mkUTF8-charset

	{ \
		echo '#!/bin/bash -'; \
		echo; \
		$(perl) $(srcdeveldir)/mkUTF8-charset ascii \
			$(srcdeveldir)/UTF8-charset; \
		echo " MIME=us-ascii"; \
		cat $(srcdeveldir)/UTF8-ascii; \
	} >$@
	chmod +x $@

###############################################################################

define mkcharset-charset
	filter=$(notdir $@); \
	if [ $${filter##*-} = ascii ]; \
	then \
		b=ascii; \
	else \
		b=$(word 2,$^); \
	fi; \
	{ \
		echo '#!/bin/bash -'; \
		echo; \
		$(perl) $(srcdeveldir)/mkcharset-charset \
		$< $$b $(srcdeveldir)/UTF8-charset; \
		echo " MIME=`$(srcdeveldir)/mime $${filter##*-}`" \
		"\"\$${0%/*}/../aux/charset-charset\" $$filter"; \
	} >$@
	chmod +x $@
endef

mkcharset-charset: $(srcdeveldir)/charset-charset

	for filter in $(charset_charset); \
	do \
		echo "\$$(srcfiltersdir)/$$filter: \\"; \
		echo "\$$(srccharsetsdir)/$${filter%-*} \\"; \
		if [ $${filter##*-} != ascii ]; \
		then \
			echo "\$$(srccharsetsdir)/$${filter##*-}\\"; \
		fi; \
		echo "\$$(srcdeveldir)/UTF8-charset \\"; \
		echo "\$$(srcdeveldir)/mkcharset-charset"; \
		echo -e "\t\$$(mkcharset-charset)"; \
	done >$@

include mkcharset-charset

###############################################################################

$(anyfiles): \
$(srcauxdir)/any/%: $(srcdeveldir)/any/% \
$(patsubst %,$(srccharsetsdir)/%,$(charsets)) \
$(srcdeveldir)/mkany
	$(perl) $(srcdeveldir)/mkany $< >$@

###############################################################################

install: all
	$(INSTALL) -d $(bindir) $(filtersdir) $(auxdir) $(libauxdir) \
		$(develdir) $(libdeveldir) $(mydocdir) $(mandir)
	$(INSTALL) -m755 -s $(installbindir)/trs     $(bindir)
	$(INSTALL) -m755    $(installbindir)/konwert $(bindir)
	$(INSTALL) -m755 -s $(installbindir)/filterm $(bindir)
	cp -dRf $(installfiltersdir)/* $(filtersdir)
	$(perl) $(srcdeveldir)/mkaliases $(srcdeveldir)/aliases \
		$(installfiltersdir) $(filtersdir)
	cp -dRf $(installauxdir)/* $(auxdir)
	$(perl) $(srcdeveldir)/mkaliases $(srcdeveldir)/aliases \
		$(installauxdir)/charsets $(auxdir)/charsets
	cp -dRf $(installlibauxdir)/* $(libauxdir)
	cp -dRf $(installdeveldir)/* $(develdir)
	cp -dRf $(installdocdir)/* $(mydocdir)
	cp -dRf $(installmandir)/* $(mandir)
	if [ -z "$(nofixmanconfig)" ]; then $(srcauxdir)/fixmanconfig; fi

###############################################################################

uninstall:
	rm -f $(bindir)/{trs,konwert,filterm}
	rm -rf $(mylibdir) $(mysharedir) $(mydocdir)
	rm -f $(patsubst $(installmandir)/%,$(mandir)/%,\
		$(shell find $(installmandir)/* -not -type d))

###############################################################################

clean:
	-rm -f $(filter-out $(nodelete),$(targets))

###############################################################################

maintainer-clean:
	-rm -f $(targets)

###############################################################################

tgz: clean $(nodelete)
	cd .. && tar czf konwert-$(version).tar.gz konwert-$(version)

###############################################################################

rpm: tgz
	cd .. && rpm -ta konwert-$(version).tar.gz

###############################################################################
