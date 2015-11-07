#########
# Run 'USE_IGZIP=1 make' to compile with igzip support
# https://software.intel.com/en-us/articles/igzip-a-high-performance-deflate-compressor-with-optimizations-for-genomic-data
#

CC=         gcc 
CFLAGS=		-g -Wall -O2 -pthread 
LDFLAGS=	
DFLAGS=		-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -D_USE_KNETFILE -DHAVE_LIBPTHREAD #-DDISABLE_BZ2
AOBJS=		bgzf.o knetfile.o util.o block.o consumer.o queue.o reader.o writer.o pbgzf.o pbgzip.o
PROG=		pbgzip
INCLUDES=	-I.
SUBDIRS=	. 
LIBPATH=

ifdef USE_IGZIP
INCLUDES+=  -Iigzip/c_code -Iigzip/include
LDFLAG+=    -ligzip0c #-ligzip1c
DFLAGS+=    -DHAVE_IGZIP
endif

.SUFFIXES:.c .o
.PHONY: all lib

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all-recur lib-recur clean-recur cleanlocal-recur install-recur:
		@target=`echo $@ | sed s/-recur//`; \
		wdir=`pwd`; \
		list='$(SUBDIRS)'; for subdir in $$list; do \
			cd $$subdir; \
			$(MAKE) CC="$(CC)" DFLAGS="$(DFLAGS)" CFLAGS="$(CFLAGS)" \
				INCLUDES="$(INCLUDES)" LIBPATH="$(LIBPATH)" $$target || exit 1; \
			cd $$wdir; \
		done;

all:$(PROG)

.PHONY:all lib clean cleanlocal
.PHONY:all-recur lib-recur clean-recur cleanlocal-recur install-recur

pbgzip:lib-recur $(AOBJS)
		$(CC) $(CFLAGS) -o $@ $(AOBJS) $(LDFLAGS) $(LIBPATH) -lm -lz -lbz2

#faidx_main.o:faidx.h razf.h

cleanlocal:
		rm -fr gmon.out *.o a.out *.exe *.dSYM razip bgzip $(PROG) *~ *.a *.so.* *.so *.dylib

clean:cleanlocal-recur
