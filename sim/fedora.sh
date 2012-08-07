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
SOURCEDIR=.  
ARCH_TYPE=`uname -m`  
 
sudo yum groupinstall "Development Tools"  
sudo yum install rpmdevtools gtk2-devel glib2-devel libpng-devel  
 
if [ ${ARCH_TYPE} == "x86_64" ]  
then  
	LIB=lib64  
else  
	LIB=lib  
fi  
 
if [ $# -ge 1 -a "$1" == "--debug" ]  
then  
	CFLAGS=-g  
else  
	CFLAGS=-O2  
fi  
 
INCLUDES="-I/usr/include/openmotif -I/usr/include/gtk-2.0 -I/usr/${LIB}/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/lib/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/include"  
 
LIBS="-L/usr/${LIB}/openmotif -L/usr/${LIB}/gdm -L/usr/${LIB} -L/usr/${LIB}/glib-2.0 -L/usr/${LIB}/gdk-pixbuf-2.0 -L/usr/${LIB}/gtk-2.0 -lXm -lXt -lX11 -lm"  
 
 LIBS2="`pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`"

gcc ${CFLAGS} -c $SOURCEDIR/noble/io.c -o io.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/math.c -o math.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/parse.c -o parse.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/interpret.c -o interpret.o
gcc ${CFLAGS} -c $SOURCEDIR/noble/land.c -o land.o

gcc ${CFLAGS} -c $SOURCEDIR/entity/being.c -o being.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/brain.c -o brain.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/social.c -o social.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/episodic.c -o episodic.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/food.c -o food.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/drives.c -o drives.o
gcc ${CFLAGS} -c $SOURCEDIR/universe/sim.c -o sim.o
gcc ${CFLAGS} -c $SOURCEDIR/universe/file.c -o file.o
gcc ${CFLAGS} -c $SOURCEDIR/command/graph.c -o graph.o
gcc ${CFLAGS} -c $SOURCEDIR/command/genealogy.c -o genealogy.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/body.c -o body.o
gcc ${CFLAGS} -c $SOURCEDIR/universe/console.c -o console.o
gcc ${CFLAGS} -c $SOURCEDIR/entity/metabolism.c -o metabolism.o

gcc ${CFLAGS} -c $SOURCEDIR/cle.c -o cle.o
gcc ${CFLAGS} -c $SOURCEDIR/longterm.c -o longterm.o

gcc ${CFLAGS} -c $SOURCEDIR/gui/draw.c -o draw.o
gcc ${CFLAGS} -c $SOURCEDIR/gui/control.c -o control.o

gcc ${CFLAGS} ${INCLUDES} -c $SOURCEDIR/gtk/platform.c -o platform.o ${LIBS2}

gcc ${CFLAGS} ${INCLUDES} -o $SOURCEDIR/../na io.o math.o parse.o interpret.o being.o brain.o metabolism.o land.o social.o episodic.o food.o drives.o sim.o file.o genealogy.o body.o draw.o control.o platform.o console.o ${LIBS2}

gcc ${CFLAGS} -c $SOURCEDIR/contrib/motters/cle.c -o cleweb.o
gcc ${CFLAGS} -c $SOURCEDIR/contrib/motters/web.c -o web.o
gcc ${CFLAGS} -c $SOURCEDIR/contrib/motters/pnglite.c -o pnglite.o

gcc -Wall -ansi -pedantic -O3 -o ../naweb cleweb.o pnglite.o -lz sim.o file.o genealogy.o body.o being.o metabolism.o brain.o console.o land.o social.o episodic.o food.o drives.o math.o io.o parse.o interpret.o graph.o web.o

gcc ${CFLAGS} ${INCLUDES} -o ../nalongterm io.o math.o parse.o interpret.o being.o brain.o land.o social.o episodic.o food.o drives.o metabolism.o console.o sim.o file.o genealogy.o body.o graph.o longterm.o pnglite.o -lz

rm *.o 

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

