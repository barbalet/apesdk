#!/bin/bash
#	linux_package.sh
#
#	=============================================================
#
#   Copyright 1996-2013 Tom Barbalet. All rights reserved.
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use,
#   copy, modify, merge, publish, distribute, sublicense, and/or
#   sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following
#   conditions:
#
#   The above copyright notice and this permission notice shall be
#	included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#   OTHER DEALINGS IN THE SOFTWARE.
#
#   This software and Noble Ape are a continuing work of Tom Barbalet,
#   begun on 13 June 1996. No apes or cats were harmed in the writing
#   of this software.

VERSION=0.694  

OS=$(awk '/DISTRIB_ID=/' /etc/*-release | sed 's/DISTRIB_ID=//' | tr '[:upper:]' '[:lower:]')
if [ -z "$OS" ]; then    
    OS=$(awk '/ID=/' /etc/*-release | sed 's/ID=//' | tr '[:upper:]' '[:lower:]')
    set -- $OS
    OS=$1
fi

if [[ $OS == "ubuntu" || $OS == "debian" || $OS == "solusos" ]]; then

	# Note that this isn't strictly the official way in which Debian packages
	# are supposed to be created, but it works

	mkdir ~/natemp
	mkdir ~/natemp/deb
	mkdir ~/natemp/deb/DEBIAN
	mkdir ~/natemp/deb/usr
	mkdir ~/natemp/deb/usr/bin
	mkdir ~/natemp/deb/usr/share
	mkdir ~/natemp/deb/usr/share/nobleape
	cp ../na ~/natemp/deb/usr/bin
	cp ../nalongterm ~/natemp/deb/usr/bin

	mkdir ~/natemp/deb/bin
	mkdir ~/natemp/deb/var
	mkdir ~/natemp/deb/var/www
	mkdir ~/natemp/deb/var/www/nobleape
	mkdir ~/natemp/deb/var/www/nobleape/images
	mkdir ~/natemp/deb/etc
	mkdir ~/natemp/deb/etc/cron.hourly
	cp ../naweb ~/natemp/deb/usr/bin
	cp contrib/motters/nobleapecron ~/natemp/deb/etc/cron.hourly
	cp -r contrib/motters/images ~/natemp/deb/var/www/nobleape

	cp debian/control ~/natemp/deb/DEBIAN
	cp debian/copyright ~/natemp/deb/DEBIAN

    # desktop/menu/icon settings
	mkdir ~/natemp/deb/usr
	mkdir ~/natemp/deb/usr/share
	mkdir ~/natemp/deb/usr/share/applications
	mkdir ~/natemp/deb/usr/share/applications/nobleape
	mkdir ~/natemp/deb/usr/share/pixmaps
	mkdir ~/natemp/deb/usr/share/icons
	mkdir ~/natemp/deb/usr/share/icons/hicolor
	mkdir ~/natemp/deb/usr/share/icons/hicolor/scalable
	mkdir ~/natemp/deb/usr/share/icons/hicolor/scalable/apps
	mkdir ~/natemp/deb/usr/share/icons/hicolor/24x24
	mkdir ~/natemp/deb/usr/share/icons/hicolor/24x24/apps
	cp desktop/nobleape.desktop ~/natemp/deb/usr/share/applications/nobleape
	cp desktop/icon.png ~/natemp/deb/usr/share/icons/hicolor/24x24/apps/nobleape.png
	cp desktop/icon.svg ~/natemp/deb/usr/share/icons/hicolor/scalable/apps/nobleape.svg
	cp desktop/icon.svg ~/natemp/deb/usr/share/pixmaps/nobleape.svg

    #sudo chmod -R 0755 ~/natemp/deb/usr
    #sudo chmod -R 0755 ~/natemp/deb/bin
    #sudo chmod -R 0644 ~/natemp/deb/usr/share
    #sudo chown root:root ~/natemp/deb/bin/naweb
    #sudo chown root:root ~/natemp/deb/usr/bin/na
    #sudo chown -R www-data:www-data ~/natemp/deb/var
	dpkg -b ~/natemp/deb nobleape.deb
	rm -rf ~/natemp

	echo You may also wish to upload a screenshot for this version to http://screenshots.debian.net/upload


elif [[ $OS == "fedora" || $OS == "redhat" || $OS == "centos" ]]; then

	rm -rf ~/rpmbuild
	sudo rm -rf rpmpackage/nobleape-$VERSION
	rpmdev-setuptree
	mkdir rpmpackage/nobleape-$VERSION

	mkdir rpmpackage/nobleape-$VERSION/usr
	mkdir rpmpackage/nobleape-$VERSION/usr/bin
	install -m 755 ../na rpmpackage/nobleape-$VERSION/usr/bin
	install -m 755 ../nalongterm rpmpackage/nobleape-$VERSION/usr/bin

	mkdir rpmpackage/nobleape-$VERSION/bin
	mkdir rpmpackage/nobleape-$VERSION/var
	mkdir rpmpackage/nobleape-$VERSION/var/www
	mkdir rpmpackage/nobleape-$VERSION/var/www/nobleape
	mkdir rpmpackage/nobleape-$VERSION/var/www/nobleape/images
	mkdir rpmpackage/nobleape-$VERSION/etc
	mkdir rpmpackage/nobleape-$VERSION/etc/cron.hourly
	install -m 755 ../naweb rpmpackage/nobleape-$VERSION/usr/bin
	install -m 755 contrib/motters/nobleapecron rpmpackage/nobleape-$VERSION/etc/cron.hourly
	cp -r contrib/motters/images/*.png rpmpackage/nobleape-$VERSION/var/www/nobleape/images

    # desktop/menu/icon settings
	mkdir rpmpackage/nobleape-$VERSION/usr
	mkdir rpmpackage/nobleape-$VERSION/usr/share
	mkdir rpmpackage/nobleape-$VERSION/usr/share/applications
	mkdir rpmpackage/nobleape-$VERSION/usr/share/applications/nobleape
	mkdir rpmpackage/nobleape-$VERSION/usr/share/pixmaps
	mkdir rpmpackage/nobleape-$VERSION/usr/share/icons
	mkdir rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor
	mkdir rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor/scalable
	mkdir rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor/scalable/apps
	mkdir rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor/24x24
	mkdir rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor/24x24/apps
	cp desktop/nobleape.desktop rpmpackage/nobleape-$VERSION/usr/share/applications/nobleape
	cp desktop/icon.png rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor/24x24/apps/nobleape.png
	cp desktop/icon.svg rpmpackage/nobleape-$VERSION/usr/share/icons/hicolor/scalable/apps/nobleape.svg
	cp desktop/icon.svg rpmpackage/nobleape-$VERSION/usr/share/pixmaps/nobleape.svg

	cd rpmpackage
	mkdir nobleape-$VERSION/etc/nobleape
	install -m 644 nobleape.conf nobleape-$VERSION/etc/nobleape/
	tar -zcvf nobleape-$VERSION.tar.gz nobleape-$VERSION/

	rm -rf ~/rpmbuild/BUILD/nobleape-$VERSION
	rm ~/rpmbuild/SOURCES/nobleape*.*
	cp nobleape-$VERSION.tar.gz ~/rpmbuild/SOURCES/
	cp nobleape.spec ~/rpmbuild/SPECS/

	rpmbuild -ba ~/rpmbuild/SPECS/nobleape.spec

	sudo rm -rf nobleape-$VERSION
	rm nobleape-$VERSION.tar.gz
	cp -r ~/rpmbuild/RPMS/* .
	cd ..
	echo ---------------------------------------------------------
	echo RPM files can be found in the rpmpackage directory
	echo under architecture subdirectories.
	echo If you wish to enable the web server then remove
	echo the commented out line from /etc/cron.hourly/nobleapecron
	echo ---------------------------------------------------------

else

	echo "Operating system not recognised"

fi
