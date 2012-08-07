#
# rpm spec for nobleape
#

%define        __spec_install_post %{nil}
%define          debug_package %{nil}
%define        __os_install_post %{_dbpath}/brp-compress

Summary: An artificial life simulation
Name: nobleape
Version: 0.694
Release: 1
License: MIT
Group: Games
SOURCE0 : %{name}-%{version}.tar.gz
URL: http://www.nobleape.com
Packager: Tom Barbalet <tom@nobleape.com>
Requires: glib2, gtk2, libpng
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
Noble Ape features a number of autonomous simulation components including a landscape simulation, biological simulation, weather simulation, sentient creature (Noble Ape) simulation and a simple intelligent-agent scripting language (ApeScript).

%prep
%setup -q

%build
 # Empty section.

%install
rm -rf %{buildroot}
mkdir -p  %{buildroot}

# in builddir
cp -a * %{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%config(noreplace) %{_sysconfdir}/%{name}/%{name}.conf
%{_bindir}/*
%attr(644,root,root) /var/www/nobleape/images/*.*
%attr(755,root,root) /etc/cron.hourly/nobleapecron
%attr(644,root,root) /usr/share/applications/nobleape/nobleape.desktop
%attr(644,root,root) /usr/share/icons/hicolor/24x24/apps/nobleape.png
%attr(644,root,root) /usr/share/icons/hicolor/scalable/apps/nobleape.svg
%attr(644,root,root) /usr/share/pixmaps/nobleape.svg

%changelog
* Thu Apr 30 2011  Bob Mottram <fuzzgun@gmail.com> 0.693-1
- Spec file created

