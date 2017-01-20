Summary: converter of character encodings
Name: konwert
Version: 1.8
Release: 1
Copyright: GPL
Group: Utilities/Text
Source: http://qrczak.home.ml.org/programy/linux/konwert/konwert-%{PACKAGE_VERSION}.tar.gz
URL: http://qrczak.home.ml.org/
BuildRoot: /var/tmp/konwert-root
Requires: perl >= 5.001

%description
Konwert is a package for conversion between various character encodings.

%package devel
Summary: development of Konwert's filters
Group: Utilities/Text
Requires: konwert

%description devel
Konwert is a package for conversion between various character encodings.
This package contains scripts and data files useful for development new
filters. They are not needed for normal usage.

%changelog
* Sun Jul 19 1998 Marcin 'Qrczak' Kowalczyk <qrczak@knm.org.pl>

- Split into konwert and konwert-devel packages

* Sat Jul 4 1998 Marcin 'Qrczak' Kowalczyk <qrczak@knm.org.pl>

- First release in RPM (thanks to Ziemek Borowski
  <ziembor@FAQ-bot.Ziembor.waw.pl>)

%prep
%setup

%build
make prefix=/usr perl=/usr/bin/perl

%install
rm -rf $RPM_BUILD_ROOT
make install prefix=$RPM_BUILD_ROOT/usr perl=/usr/bin/perl nofixmanconfig=1

%clean
rm -rf $RPM_BUILD_ROOT

%post
/usr/share/konwert/aux/fixmanconfig

%files
%attr(755, root, root) /usr/bin/trs
%attr(755, root, root) /usr/bin/konwert
%attr(755, root, root) /usr/bin/filterm
%attr(-, root, root) %dir /usr/share/konwert
%attr(-, root, root) /usr/share/konwert/filters
%attr(-, root, root) /usr/share/konwert/aux
%attr(-, root, root) %dir /usr/lib/konwert
%attr(-, root, root) /usr/lib/konwert/aux
%attr(-, root, root) %docdir /usr/doc/konwert-%{PACKAGE_VERSION}
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/BUGS
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/CHANGES
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/COPYING
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/README
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/TODO
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/filterm
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/filters
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/konwert
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/thanks
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/trs
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/BLEDY
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/COPYING
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/CZYTAJTO
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/DO_ZROBIENIA
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/ZMIANY
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/filterm
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/filtry
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/konwert
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/podziekowania
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/trs
%attr(-, root, root) /usr/man/man*/*
%attr(-, root, root) /usr/man/*/man*/*

%files devel
%attr(-, root, root) /usr/share/konwert/devel
%attr(-, root, root) %docdir /usr/doc/konwert-%{PACKAGE_VERSION}
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/en/devel
%attr(-, root, root) /usr/doc/konwert-%{PACKAGE_VERSION}/pl/tworzenie
