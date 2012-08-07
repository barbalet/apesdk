#	ubuntu.sh
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

UBUNTU_VERSION="11.10"

if [ ${UBUNTU_VERSION} == "11.10" ]
then
    sudo apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgnome2-dev libpng12-dev
else
    sudo apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgdk-pixbuf2.0-dev libxprintutil-dev libgnome2-dev libpng12-dev
fi

if [ $# -ge 1 -a "$1" == "--debug" ]
then
    CFLAGS=-g
else
    CFLAGS=-O2 
fi

INCLUDES="-I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/include"

LIBS="-L/usr/lib/gdm -L/usr/lib -L/usr/lib/glib-2.0 -L/usr/lib/gdk-pixbuf-2.0 -L/usr/lib/gtk-2.0 -L/usr/lib/pango-1.0 -L/usr/lib/cairo -lXm -lXt -lX11 -lm"

LIBS2="`pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`"

gcc ${CFLAGS} -c noble/io.c -o io.o
gcc ${CFLAGS} -c noble/math.c -o math.o
gcc ${CFLAGS} -c noble/parse.c -o parse.o
gcc ${CFLAGS} -c noble/interpret.c -o interpret.o
gcc ${CFLAGS} -c noble/land.c -o land.o

gcc ${CFLAGS} -c entity/being.c -o being.o
gcc ${CFLAGS} -c entity/brain.c -o brain.o
gcc ${CFLAGS} -c entity/social.c -o social.o
gcc ${CFLAGS} -c entity/episodic.c -o episodic.o
gcc ${CFLAGS} -c entity/food.c -o food.o
gcc ${CFLAGS} -c entity/drives.c -o drives.o
gcc ${CFLAGS} -c universe/sim.c -o sim.o
gcc ${CFLAGS} -c universe/file.c -o file.o
gcc ${CFLAGS} -c command/graph.c -o graph.o
gcc ${CFLAGS} -c command/genealogy.c -o genealogy.o
gcc ${CFLAGS} -c entity/body.c -o body.o
gcc ${CFLAGS} -c universe/console.c -o console.o
gcc ${CFLAGS} -c entity/metabolism.c -o metabolism.o

gcc ${CFLAGS} -c cle.c -o cle.o
gcc ${CFLAGS} -c longterm.c -o longterm.o

gcc ${CFLAGS} -c gui/draw.c -o draw.o
gcc ${CFLAGS} -c gui/control.c -o control.o

gcc ${CFLAGS} ${INCLUDES} -c gtk/platform.c -o platform.o ${LIBS2}

gcc ${CFLAGS} ${INCLUDES} -o ../na io.o math.o parse.o interpret.o being.o body.o brain.o metabolism.o land.o social.o episodic.o food.o drives.o sim.o file.o genealogy.o draw.o control.o platform.o console.o ${LIBS2}

gcc ${CFLAGS} -c contrib/motters/cle.c -o cleweb.o
gcc ${CFLAGS} -c contrib/motters/web.c -o web.o
gcc ${CFLAGS} -c contrib/motters/pnglite.c -o pnglite.o

gcc -Wall -ansi -pedantic -O3 -o ../naweb cleweb.o pnglite.o -lz sim.o file.o genealogy.o body.o metabolism.o being.o brain.o console.o land.o social.o episodic.o food.o drives.o math.o io.o parse.o interpret.o graph.o web.o

gcc ${CFLAGS} ${INCLUDES} -o ../nalongterm io.o math.o parse.o interpret.o being.o brain.o land.o social.o episodic.o food.o drives.o metabolism.o console.o sim.o file.o genealogy.o body.o graph.o longterm.o pnglite.o -lz

rm *.o

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

