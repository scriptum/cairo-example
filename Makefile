SOURCES=$(wildcard *.c)
TARGETS=$(SOURCES:.c=)
LIBS=`pkg-config --cflags --libs gtk+-2.0` -lm
all: $(TARGETS)

%:%.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

install: all
	/usr/bin/install -m 755 $(TARGETS) /usr/bin

uninstall:
	cd /usr/bin/; /bin/rm $(TARGETS)

clean:
	/bin/rm $(TARGETS)
