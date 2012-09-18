#	ubuntu_package.sh
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

# Tested on Ubuntu 10.10, 11.04, 11.10

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

