// $Id: midi.c,v 1.5 1998/07/02 00:40:21 mito Exp $

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "inst.h"
#include "midi.h"
#include "score.h"
#include "z_ext.h"

char *notenames[] =
{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
#define NOTE(b) (notenames[(b)%12])

/* Algo:

states: ready (non-rt status (elided if running status))
        in-vcm (with countdown)
        in-sysex
        in-reserved (wait until next status)

*/

/* 0x80 .. 0xef */
static byte vcm_lens[] =
{ 0, 0, 0, 0, 0, 0, 0, 0, /* unused */
  3, /* note off */
  3, /* note on */
  3, /* aftertouch */
  3, /* controller */
  2, /* program change */
  2, /* channel pressure */
  3, /* pitch wheel */
  0  /* system */
} ;

/* 0xf0 .. 0xf7 */
static byte syscom_lens[] =
{
 -1, /* system exclusive start */
  2, /* MTC Quarter Frame */
  3, /* Song Position Pointer */
  2, /* Song Select */
 -1, /* Reserved */
 -1, /* Reserved */
  1, /* Tune Request */
 -1, /* system exclusive stop */
  /* Rest are system realtime */
} ;

static byte reserved[] = { 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0 } ;

static byte msgbuf[2];
static byte rtmsgbuf[2];
static int msgbufp;

enum {
  sready,
  sinmsg,
  sinsysex,
  sinres,
} ;

char *midi_sys_names[] =
{ "System Exclusive Start",
  "MTC Quarter Frame",
  "Song Position Pointer",
  "Song Select",
  "Reserved",
  "Reserved",
  "Tune Request",
  "System Exclusive Stop",
  "MIDI Clock",
  "Reserved",
  "MIDI Start",
  "MIDI Continue",
  "MIDI Stop",
  "Reserved",
  "Active Sense",
  "Reset"
} ;

static char *states[] = { "ready", "inmsg", "insysex", "inres" };

void err( char *s )
{
  fprintf( stderr, "%s\n", s );
  exit( 1 );
}

void midi_show_vcm( byte *mb )
{
  int channel = mb[0] & 0xf;
  switch( mb[0]>>4 ) {
    case 0x8:
      printf( "note off %s channel 0x%x vel 0x%x\n",
        NOTE(mb[1]), channel, mb[2] );
      break;
    case 0x9:
      printf( "note on %s channel 0x%x vel 0x%x\n",
        NOTE(mb[1]), channel, mb[2] );
      break;
    case 0xa:
      printf( "aftertouch %s channel 0x%x pressure 0x%x\n",
        NOTE(mb[1]), channel, mb[2] );
      break;
    case 0xb:
      printf( "controller 0x%x channel 0x%x val 0x%x\n",
        mb[1], channel, mb[2] );
      break;
    case 0xc:
      printf( "prog change 0x%x channel 0x%x\n", mb[1], channel );
      break;
    case 0xd:
      printf( "channel pressure 0x%x channel 0x%x\n", mb[1], channel );
      break;
    case 0xe:
      printf( "pitch wheel 0x%x channel 0x%x\n",
        PITCHWHEELVAL( mb[1], mb[2] ), channel );
      break;
    default:
      err( "bad show vcm" );
      break;
  }
}

static char *midiin_name=0;
static int midifd;
static int running, cur_status, last_status;
static int state, sysex_len;

static int using_device;

void midi_open( char *infile )
{
  if (midiin_name) {
    if (strcmp( midiin_name, infile )) {
      fprintf( stderr, "Tried to open midi dev as %s and %s\n",
        midiin_name, infile );
      exit( 1 );
    } else {
      //printf( "double midi open -- ok\n" );
      return;
    }
  } else {
    midiin_name = strdup( infile );
  }

  using_device = !strcmp( infile, "/dev/midi" );

  if (using_device) fclose( stdin );

  midifd = open( infile, O_RDWR );

  if (midifd==-1) {
    perror( "open dev" );
    exit( 1 );
  }

  running = 0;
  msgbufp = 0;
  state = sready;
}

void midi_close()
{
  if (midiin_name) {
    free( midiin_name );
    midiin_name = 0;
    close( midifd );
  }
}

int midi_eof()
{
  if (using_device) return 0;
  return 1;
}

static int avail_read( int fd )
{
  int r;
  fd_set rs;
  struct timeval tv;

  FD_ZERO( &rs );
  FD_SET( fd, &rs );
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  r = select( 1, &rs, NULL, NULL, &tv );

  return r;
}

static byte empty_mb[3];
void sev_to_mb( byte *mb, score_event *sev )
{
  int inst, channel;
  memcpy( mb, empty_mb, sizeof(empty_mb) );
  inst = sev->inst;
  channel = sev->midi_channel;
  if (INST_ISCONTROLLER(inst)) {
    mb[0] = MIDI_CONTROLLER;
    mb[0] |= sev->midi_channel;
    mb[1] = INST_WHICHCONTROLLER(inst);
    mb[2] = sev->value>>8;
  } else if (INST_ISNOTE(inst)) {
    mb[0] = sev->action == SCORE_NOFF ? MIDI_NOTEOFF : MIDI_NOTEON;
    mb[0] |= sev->midi_channel;
    mb[1] = sev->pitch;
    mb[2] = sev->vol;
  } else if (INST_ISAFTERTOUCH(inst)) {
    mb[0] = MIDI_AFTERTOUCH;
    mb[0] |= sev->midi_channel;
    mb[1] = sev->pitch;
    mb[2] = sev->value>>8;
  } else if (INST_ISCHANNELPRESSURE(inst)) {
    mb[0] = MIDI_CHANNELPRESSURE;
    mb[0] |= channel;
    mb[1] = sev->value>>8;
  } else if (INST_ISPITCHWHEEL(inst)) {
    mb[0] = MIDI_PITCHWHEEL;
    mb[0] |= channel;
    mb[1] = PITCHWHEEL0(sev->value);
    mb[2] = PITCHWHEEL1(sev->value);
  } else {
    printf( "Can't convert MIDI inst 0x%x\n", inst );
  }
}

static score_event empty_sev;
int mb_to_sev( score_event *sev, byte *mb )
{
  *sev = empty_sev;
  if (STATUS_SYS_RT(mb[0])) {
    return 0;
  } else if (STATUS_SYS(mb[0])) {
    return 0;
  } else {
    int channel = mb[0] & 0xf;
    int type = mb[0]&0xF0;
    if (type==MIDI_NOTEOFF || type==MIDI_NOTEON) {
      sev->inst = INST_NOTE(mb[1]);
      sev->pitch = mb[1];
      sev->action = (type==MIDI_NOTEOFF?SCORE_NOFF:SCORE_NON);
      sev->vol = mb[2];
      sev->midi_channel = channel;
    } else if (type==MIDI_AFTERTOUCH) {
      sev->inst = INST_AFTERTOUCH;
      sev->action = SCORE_MOD;
      sev->pitch = mb[1];
      sev->value = mb[2]<<8;
      sev->midi_channel = channel;
    } else if (type==MIDI_CONTROLLER) {
      sev->inst = INST_CONTROLLER(mb[1]);
      sev->action = SCORE_MOD;
      sev->value = mb[2]<<8;
      sev->midi_channel = channel;
    } else if (type==MIDI_CHANNELPRESSURE) {
      sev->inst = INST_CHANNELPRESSURE;
      sev->action = SCORE_MOD;
      sev->value = mb[1]<<8;
      sev->midi_channel = channel;
    } else if (type==MIDI_PITCHWHEEL) {
      sev->inst = INST_PITCHWHEEL;
      sev->action = SCORE_MOD;
      sev->value = PITCHWHEELVAL( mb[1], mb[2] );
      sev->midi_channel = channel;
    }

    sev->time = 0;

    return 1;
  }
}

// to device
void midi_write_event( score_event *sev )
{
  byte mb[3];
  sev_to_mb( mb, sev );
  midi_write_msg( mb );
}

// from device
void midi_read_event( score_event *sev, int *more, int *done )
{
  byte *mb;
  while (1) {
    midi_read_msg( &mb, more, done );
    if (!*more) return;

    if (mb_to_sev( sev, mb ))
      return;
  }
}

// to device
void midi_write_msg( byte *mb )
{
  int r;

  r = write( midifd, mb, STATUS_DATALEN(mb[0]) );
  if (r != STATUS_DATALEN(mb[0])) {
    fprintf( stderr, "MIDI: tried to write %d, wrote %d\n",
      STATUS_DATALEN(mb[0]), r );
    exit( 1 );
  }
}

// to device
void midi_read_msg( byte **usr_mb, int *usr_mbp, int *done )
{
  byte buf;
  int b, r;
  *done = 0;
  while (1) {
    if (using_device && !avail_read( midifd )) {
      *usr_mbp = 0;
      return;
    }

    r = read( midifd, &buf, 1 );
    b = buf;

    if (r<1) {
      *usr_mbp = 0;
      return;
    }

    if (b==-1) {
      *done = 1;
      return;
    }
    //printf( "%s 0x%x\n", states[state], b );

    if (STATUS_SYS_RT(b)) {
      /* Sysex realtime */
      rtmsgbuf[0] = b;
      *usr_mb = rtmsgbuf;
      *usr_mbp = 1;
      return;
    } else if (state==sready) {
      if (!STATUS(b)) {
        if (!running) {
          err( "bad running!" );
        } else {
          cur_status = last_status;
          state = sinmsg;
          msgbuf[msgbufp++] = cur_status;
          msgbuf[msgbufp++] = b;
        }
      } else {
        if (STATUS_SYSEX_START(b)) {
          cur_status = last_status = b;
          state = sinsysex;
          sysex_len = 0;
        } else if (STATUS_SYSEX_STOP(b)) {
          printf( "Spurious sysex stop!\n" );
        } else {
	  last_status = cur_status = b;
	  running = !STATUS_SYS(b);
	  state = sinmsg;
          msgbuf[msgbufp++] = cur_status;

	  if (STATUS_RES(b)) {
	    state = sinres;
	  }
        }
      }
    } else if (state==sinsysex) {
      if (STATUS_SYSEX_STOP(b)) {
        state = sready;
        printf( "Sysex len 0x%x\n", sysex_len );
      } else {
        sysex_len++;
      }
    } else if (state==sinmsg) {
      msgbuf[msgbufp++] = b;
    } else if (state==sinres) {
      /* In reserved message.  We don't know length,
         so we don't do anything unless this is the
         start of a new message. */
      if (STATUS(b)) {
        printf( "End of reserved sys com 0x%x\n", cur_status );
        last_status = cur_status = b;
        running = 1;
        state = sinmsg;

        if (STATUS_RES(b)) {
          state = sinres;
        }
      }
    }

    /* Do we have a full message? */
    if (state == sinmsg && msgbufp >= STATUS_DATALEN(cur_status)) {
      if (msgbufp > STATUS_DATALEN(cur_status)) {
        err( "msgbufp too big" );
      } else {
        state = sready;

        *usr_mb = msgbuf;
        *usr_mbp = msgbufp;
        msgbufp = 0;

        return;
      }
    }
  }
}


void midi_dump( byte *mb )
{
  int mbp = STATUS_DATALEN( mb[0] );
  if (mb[0] != 0xf8)
    printf( "%x %x %x\n", mb[0], mb[1], mb[2] );
  if (STATUS_SYS_RT(mb[0])) {
    /* Sysex realtime */
    if (mb[0]!=0xf8)
      printf( "%s (0x%x)\n", midi_sys_names[mb[0]&0xf], mb[0] );
  } else if (STATUS_SYS(mb[0])) {
    printf( "sys common 0x%x", mb[0] );
    if (mbp>0) printf( " 0x%x", mb[1] );
    if (mbp>1) printf( " 0x%x", mb[2] );
    printf( "\n" );
  } else {
    printf( "voice channel 0x%x", mb[0] );
    if (mbp>0) printf( " 0x%x", mb[1] );
    if (mbp>1) printf( " 0x%x", mb[2] );
    printf( "\n" );
    midi_show_vcm( mb );
  }
}
