// $Id: midi.h,v 1.3 1998/06/30 02:56:12 mito Exp $

#ifndef _midi_h_
#define _midi_h_

#include "score.h"
#include "z_types.h"

#define STATUS_SYS(b) (((b)&0xf0)==0xf0)
#define STATUS_SYS_RT(b) (((b)&0xf0)==0xf0 && ((b)&0xf)>=0x8)
#define STATUS(b) ((b)&0x80)
#define STATUS_RES(b) (STATUS_SYS(b) && reserved[(b)&0xf])
#define STATUS_DATALEN(b) \
  (((b)<0xf0) ? vcm_lens[(b)>>4] : \
   ((b)<0xf8) ? syscom_lens[(b)&0xf] : \
   ((b)==0xf8) ? 0 : (err( "STATUS_DATALEN" ),0) )
#define STATUS_SYSEX_START(b) ((b)==0xf0)
#define STATUS_SYSEX_STOP(b) ((b)==0xf7)

void midi_show_vcm( byte *mb );
void midi_open( char *infile );
void midi_close();
void midi_read_event( score_event *sev, int *more, int *done );
void midi_write_event( score_event *sev );
void midi_read_msg( byte **usr_mb, int *usr_mbp, int *done );
void midi_write_msg( byte *mb );
void midi_dump( byte *mb );
extern char *midi_sys_names[];
int midi_eof();

#define PITCHWHEELVAL(a,b) ( ((b)<<7) | (a) )
#define PITCHWHEEL0(p) ((p)&0x7F)
#define PITCHWHEEL1(p) (((p)>>7)&0x7F)

// User macros
#define pitchwheelp(b) ((b)==0xe0)
#define pitchwheelval(msg) (PITCHWHEELVAL((msg)[1],(msg)[2]))

typedef struct midi_event {
  long time;
  byte bs[3];
} midi_event;

#define midiw(fd,ev,n) io_write( (fd), (ev), (n)*sizeof(midi_event) )
#define midir(fd,ev,n) io_read( (fd), (ev), (n)*sizeof(midi_event) )

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON 0x90
#define MIDI_AFTERTOUCH 0xA0
#define MIDI_CONTROLLER 0xB0
#define MIDI_PROGCHANGE 0xC0
#define MIDI_CHANNELPRESSURE 0xD0
#define MIDI_PITCHWHEEL 0xE0

#endif /* _midi_h_ */
