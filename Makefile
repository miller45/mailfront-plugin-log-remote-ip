# Don't edit Makefile!  Use conf-* for configuration.
#
SHELL=/bin/sh

MYPROGNAME=plugin-log-remote-ip
_ARCH = $(shell uname -m)
_LIBDIR = /usr/lib
_INCDIR = /usr/include
_RPMOPTFLAGS = -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m32 -march=i686 -mtune=atom -fasynchronous-unwind-tables
ifeq ($(_ARCH), x86_64)
    _LIBDIR = /usr/lib64
    _RPMOPTFLAGS = -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches   -m64 -mtune=generic
endif

DEFAULT: all

all: libraries docs

clean: TARGETS
	rm -f `cat TARGETS`

docs: 

install: INSTHIER conf-modules
	bg-installer -v <INSTHIER
	bg-installer -c <INSTHIER

libraries: $(MYPROGNAME).so

makeso: conf-ccso conf-bgincs conf-mailfrontincs conf-bglibs
	( bgincs=`head -n 1 conf-bgincs`; \
	  mailfrontincs=`head -n 1 conf-mailfrontincs`; \
	  bglibs=`head -n 1 conf-bglibs`; \
	  echo '#!/bin/sh'; \
	  echo 'source=$$1; shift'; \
	  echo 'base=`echo "$$source" | sed -e s:\\\\.c$$::`'; \
	  echo exec `head -n 1 conf-ccso` -DSHARED -I. -I$(_INCDIR) "-I'$${bgincs}'" "-I'$${mailfrontincs}'" -L. -L$(_LIBDIR) "-L'$${bglibs}'" '-o $${base}.so $$source $${1+"$$@"}'; \
	) >makeso
	chmod 755 makeso

$(MYPROGNAME).so: makeso $(MYPROGNAME).c
	./makeso $(MYPROGNAME).c -lbg

reset:
	echo '/usr/local/bglibs/include' > conf-bgincs
	echo '/usr/local/bglibs/lib' > conf-bglibs
	echo '/usr/local/include/mailfront' > conf-mailfrontincs
	echo '/usr/local/lib/mailfront' > conf-modules
	echo >> conf-modules
	echo 'Modules will be installed in this directory.' >> conf-modules
	echo 'gcc -W -Wall -Wshadow -g -I/usr/include -fPIC -shared' > conf-ccso
	echo >> conf-ccso
	echo 'This will be used to compile .c files into shared objects.' >> conf-ccso
	echo 'gcc -s' > conf-ld
	echo >> conf-ld
	echo 'This will be used to link .o and .a files into an executable.' >> conf-ld

rpmenv:
	echo "gcc -Wall -g -O2 -s -rdynamic" > conf-ld
	echo "$(_INCDIR)/mailfront" > conf-mailfrontincs
	echo "$(_LIBDIR)/mailfront" > conf-modules
	echo "$(_INCDIR)/bglibs" > conf-bgincs
	echo "$(_LIBDIR)/bglibs" > conf-bglibs
	echo "gcc $(_RPMOPTFLAGS) -fPIC -shared" > conf-ccso
