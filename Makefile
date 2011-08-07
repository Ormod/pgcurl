MODULE_big = pgcurl
PGCURL_VERSION = 0.0.1

OBJS = pgcurl.o
DATA_built = $(MODULE_big).sql
SHLIB_LINK = -lcurl

PGXS := $(shell pg_config --pgxs)
include $(PGXS)

html:
	rst2html.py README README.html
dist:
	tar --exclude .git -cjf ../pgcurl_$(PGCURL_VERSION).tar.bz2 ../pgcurl/
deb84:
	sed -e s/PGVER/8.4/g < debian/packages.in > debian/packages
	yada rebuild
	debuild -uc -us -b
deb90:
	sed -e s/PGVER/9.0/g < debian/packages.in > debian/packages
	yada rebuild
	debuild -uc -us -b
deb91:
	sed -e s/PGVER/9.1/g < debian/packages.in > debian/packages
	yada rebuild
	debuild -uc -us -b
build-dep:
	apt-get install libpq-dev devscripts yada flex bison libcurl4-gnutls-dev devscripts

