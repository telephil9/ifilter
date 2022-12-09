</$objtype/mkfile

BIN=/$objtype/bin/image
CFLAGS=-FTVw
TARG=cfilter pixelate blur dither
HFILES=a.h
OFILES=common.$O
MAN=/sys/man/1

default:V: all

</sys/src/cmd/mkmany

install:V:
	mkdir -p $BIN
	for(i in $TARG)
		mk $MKFLAGS $i.install
	cp ifilter.man $MAN/ifilter
