make clean
make
rm -rf deb/bin
rm -rf deb/var
rm -rf deb/etc
mkdir deb/bin
mkdir deb/var
mkdir deb/var/www
mkdir deb/var/www/nobleape
mkdir deb/var/www/nobleape/images
mkdir deb/etc
mkdir deb/etc/cron.hourly
cp naweb deb/bin
cp nobleapecron deb/etc/cron.hourly
cp -r images deb/var/www/nobleape
dpkg -b deb nobleape-webserver.deb
