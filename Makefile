# Makefile for cciss monitor

#
# $Header$
#
CFLAGS=-g -O2

ccissprobe: probe.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
	rm ccissprobe
