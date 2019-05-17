CROSS_COMPILE 	?=
CROSS_PREFIX	?=
CC		:= $(CROSS_COMPILE)gcc
CFLAGS 	?= -I. -g -O0 -std=gnu11
LDFLAGS	?=
LIBS	:= -larducam_mipicamera
LDIR	:= lib
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: preview capture video list_format

preview: preview.c
	$(CC) $(LDFLAGS) $(CFLAGS) -L $(LDIR) -o $@ $^ $(LIBS)

capture: capture.c
	$(CC) $(LDFLAGS) $(CFLAGS) -L $(LDIR) -o $@ $^ $(LIBS)
	
video: video.c
	$(CC) $(LDFLAGS) $(CFLAGS) -L $(LDIR) -o $@ $^ $(LIBS)

list_format: list_format.c
	$(CC) $(LDFLAGS) $(CFLAGS) -L $(LDIR) -o $@ $^ $(LIBS)

clean:
	-rm -f *.o
	-rm -f preview capture video list_format

.PHONY: install

install: 
	sudo install -m 644 $(OLIB)/libarducam_mipicamera.so /usr/lib/

