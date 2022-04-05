MT_CFLAGS = -I/usr/local/linuxthreads -D_REENTRANT
MT_LDFLAGS = -L/usr/local/linuxthreads -lpthread -static

CFLAGS = -g $(MT_CFLAGS) -I/usr/lib/oss/include

PATCHES = ech fb filt jmix2 m2a maa mix2 mpar msos \
play rec sd sinwav sos sp sr tfilt splay smix ss

LIBS = z_param.o card.o io.o midi.o miditest z_time.o sm sslib.o \
       soundio.o score.o inst.o ioasync.o profile.o

EXES = feedback writelots meter

LDFLAGS = $(MT_LDFLAGS)

default: $(LIBS) $(EXES)

all: $(LIBS) $(EXES) $(PATCHES)

clean:
	rm *.o $(PATCHES) $(EXES)

depend:
	zdepend > depend.mk

z_param.o: z_param.c
	gcc $(CFLAGS) -c z_param.c

card.o: card.c card.h
	gcc $(CFLAGS) -c card.c

io.o: io.c io.h
	gcc $(CFLAGS) -c io.c

ioasync.o: ioasync.c ioasync.h
	gcc $(CFLAGS) -c ioasync.c

midi.o: midi.c midi.h
	gcc $(CFLAGS) -c midi.c

sslib.o: sslib.c sslib.h
	gcc $(CFLAGS) -c sslib.c

soundio.o: soundio.c soundio.h
	gcc $(CFLAGS) -c soundio.c

z_time.o: z_time.c z_time.h
	gcc $(CFLAGS) -c z_time.c

miditest.o: miditest.c midi.h
	gcc $(CFLAGS) -c miditest.c

miditest: miditest.o midi.o
	gcc $(CFLAGS) -o miditest miditest.o midi.o

sm.o: sm.c score.h
	gcc $(CFLAGS) -c sm.c

sm: sm.o score.o io.o ioasync.o z_time.o
	gcc $(CFLAGS) -o sm sm.o score.o io.o ioasync.o z_time.o $(LDFLAGS)

score.o: score.c score.h
	gcc $(CFLAGS) -c score.c

inst.o: inst.c inst.h
	gcc $(CFLAGS) -c inst.c

feedback.o: feedback.c
	gcc $(CFLAGS) -c feedback.c

feedback: feedback.o
	gcc $(CFLAGS) -o feedback feedback.o

writelots.o: writelots.c
	gcc $(CFLAGS) -c writelots.c

writelots: writelots.o
	gcc $(CFLAGS) -o writelots writelots.o

profile.o: profile.c profile.h
	gcc $(CFLAGS) -c profile.c

meter: meter.c
	gcc -o meter $(CFLAGS) meter.c

include depend.mk
