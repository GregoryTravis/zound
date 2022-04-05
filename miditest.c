#include <stdio.h>

#include "midi.h"

#define DEFAULT_INFILE "/dev/midi"

void main( int argc, char *argv[] )
{
  char *infile=0;
  int a, argerr, mbp, done;
  byte *mb;

  argerr = 0;
  for (a=1; a<argc; ++a) {
    if (argv[a][0]=='-') {
      switch( argv[a][1] ) {
        case 'i':
          infile = argv[++a];
          break;
        default:
          fprintf( stderr, "Unknown option %s\n", argv[a] );
          argerr = 1;
          break;
      }
    } else {
      fprintf( stderr, "Unknown word %s\n", argv[a] );
      argerr = 1;
    }
  }

  if (argerr) {
    fprintf( stderr, "Usage: %s [-i <infile>]\n", argv[0] );
    exit( 1 );
  }

  if (!infile) infile = DEFAULT_INFILE;
printf( "Reading %s\n", infile );

  midi_open( infile );

  while (1) {
    midi_read_msg( &mb, &mbp, &done );
    if (!mbp) {
//      printf( "NADA\n" );
      continue;
    }
    if (done) break;
    if (STATUS_SYS_RT(mb[0])) {
      /* Sysex realtime */
//      if (mb[0]!=0xf8)
        printf( "%s (0x%x)\n", midi_sys_names[mb[0]&0xf], mb[0] );
    } else if (STATUS_SYS(mb[0])) {
      printf( "sys common 0x%x", mb[0] );
      if (mbp>0) printf( " 0x%x", mb[0] );
      if (mbp>1) printf( " 0x%x", mb[1] );
      printf( "\n" );
    } else {
      /*printf( "voice channel 0x%x", mb[0] );
      if (mbp>0) printf( " 0x%x", mb[0] );
      if (mbp>1) printf( " 0x%x", mb[1] );
      printf( "\n" );*/
      midi_show_vcm( mb );
    }
  }

  midi_close();
}
