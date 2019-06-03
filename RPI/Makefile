CROSS_COMPILE 	?=
CROSS_PREFIX	?=
CC		:= $(CROSS_COMPILE)gcc
CFLAGS 	?= -I. -g -O0 -std=gnu11
LDFLAGS	?=
LIBS	:= -larducam_mipicamera
OLIB	:= lib
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: preview capture video list_format raw_callback read_write_sensor_reg

preview: preview.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

capture: capture.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
	
capture_raw: capture_raw.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

video: video.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

list_format: list_format.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

raw_callback: raw_callback.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

read_write_sensor_reg : read_write_sensor_reg.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f *.o
	-rm -f preview capture video list_format raw_callback read_write_sensor_reg

.PHONY: install

install: 
	sudo install -m 644 $(OLIB)/libarducam_mipicamera.so /usr/lib/

