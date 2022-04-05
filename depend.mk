msos: msos.p io.o midi.o score.o midiin.c scorewrite.c scoreread.c midiout.c scoredump.c
	z msos.p

tfilt: tfilt.p card.o inst.o io.o midi.o score.o midiin.c read.c tfilt.c write.c spkr.c
	z tfilt.p

sp: sp.p io.o midi.o score.o scoreread.c scoredump.c midiout.c
	z sp.p

sr: sr.p io.o midi.o score.o midiin.c scoredump.c scorewrite.c
	z sr.p

ss: ss.p card.o inst.o midi.o score.o soundio.o sslib.o midiin.c ss.c spkr.c
	z ss.p

fb: fb.p card.o mic.c spkr.c
	z fb.p

splay: splay.p card.o io.o read.c read.c sspkr.c
	z splay.p

jmix2: jmix2.p card.o inst.o io.o midi.o score.o midiin.c read.c read.c jmix2.c spkr.c write.c
	z jmix2.p

filt: filt.p card.o io.o read.c filt.c write.c
	z filt.p

maa: maa.p card.o io.o midi.o score.o scoreread.c read.c spkr.c midiout.c
	z maa.p

ech: ech.p card.o io.o read.c write.c echo.c
	z ech.p

mpar: mpar.p card.o io.o midi.o score.o scoreread.c scoredump.c midiout.c mic.c write.c
	z mpar.p

sos: sos.p card.o io.o read.c mic.c spkr.c write.c
	z sos.p

spar: spar.p card.o io.o midi.o score.o scoreread.c midiout.c mic.c write.c
	z spar.p

play: play.p card.o io.o read.c spkr.c
	z play.p

sd: sd.p midi.o score.o midiin.c scoredump.c
	z sd.p

sinwav: sinwav.p io.o sinwav.c write.c
	z sinwav.p

rec: rec.p card.o io.o mic.c write.c
	z rec.p

mix2: mix2.p card.o io.o read.c read.c mix2.c write.c
	z mix2.p

m2a: m2a.p card.o io.o midi.o score.o scoreread.c midiout.c mic.c write.c
	z m2a.p

smix: smix.p io.o score.o scoreread.c scoreread.c smix.c scorewrite.c
	z smix.p

