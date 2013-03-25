#	linux_req.sh
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

# Tested on SolusOS 1.2

OS=$(awk '/DISTRIB_ID=/' /etc/*-release | sed 's/DISTRIB_ID=//' | tr '[:upper:]' '[:lower:]')
if [ -z "$OS" ]; then    
    OS=$(awk '/ID=/' /etc/*-release | sed 's/ID=//' | tr '[:upper:]' '[:lower:]')
    set -- $OS
    OS=$1
fi

ARCH=$(uname -m | sed 's/x86_//;s/i[3-6]86/32/')
VERSION=$(awk '/DISTRIB_RELEASE=/' /etc/*-release | sed 's/DISTRIB_RELEASE=//' | sed 's/[.]0/./')
if [ -z "$VERSION" ]; then
    VERSION=$(awk '/VERSION_ID=/' /etc/*-release | sed 's/VERSION_ID=//' | sed 's/[.]0/./')
fi

if [ -z "$OS" ]; then
    OS=$(awk '{print $1}' /etc/*-release | tr '[:upper:]' '[:lower:]')
fi

if [ -z "$VERSION" ]; then
    VERSION=$(awk '{print $3}' /etc/*-release)
fi

echo $OS "Arch:" $ARCH "Version" $VERSION

# Convert floating point version into an integer
INT_VERSION=`echo $VERSION '*100' | bc -l | awk -F '.' '{ print $1; exit; }'`

if [[ $OS == "solusos" && $INT_VERSION -le "120" ]]; then
	sudo apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgnome2-dev libpng12-dev
elif [[ $OS == "ubuntu" && $INT_VERSION -ge "1110" ]]; then
	sudo apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgnome2-dev libpng12-dev
elif [[ $OS == "ubuntu" && $INT_VERSION -lt "1110" ]]; then
    sudo apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgdk-pixbuf2.0-dev libxprintutil-dev libgnome2-dev libpng12-dev
elif [ $OS == "debian" ]; then
	apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgnome2-dev libpng12-dev
elif [[ $OS == "fedora" || $OS == "redhat" || $OS == "centos" ]]; then
	sudo yum groupinstall "Development Tools"  
	sudo yum install rpmdevtools gtk2-devel glib2-devel libpng-devel
elif [[ $OS == "linuxmint" && $INT_VERSION -ge "1300" ]]; then
	sudo apt-get install build-essential libmotif-dev libgtk2.0-dev libglib2.0-dev libgdk-pixbuf2.0-dev libgnome2-dev libpng12-dev	
else
	echo "Operating system not recognised"
fi
