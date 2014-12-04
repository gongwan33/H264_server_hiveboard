bin_PROGRAMS=pcare
pcare_SOURCES=main.c utils.c rw.c
SUBDIRS=../audio ../video ../network  
#../leds
INCLUDES=-I$(top_srcdir)/src/include -I/usr/local/alsa/include
LDADD = $(top_srcdir)/src/video/libvideo.a $(top_srcdir)/src/network/libnetwork.a $(top_srcdir)/src/audio/libaudio.a $(top_srcdir)/src/lib/libjpeg.a $(top_srcdir)/src/lib/libmad.so.0.2.1 /usr/local/alsa/lib/libasound.a -lpthread -ldl -lrt -lm
