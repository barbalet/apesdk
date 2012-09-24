#!/bin/bash
#	fedora.sh
#
#	=============================================================
#
#   Copyright 1996-2012 Tom Barbalet. All rights reserved.
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

