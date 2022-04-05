// $Id: sm.c,v 1.1 1998/07/02 00:39:16 mito Exp $

#include "io.h"
#include "score.h"

void main( int argc, char *argv[] )
{
  int infd0, infd1, outfd;
  score_event in0, in1;
  int has0=0, has1=0;

  infd0 = io_open( argv[1], IO_R );
  infd1 = io_open( argv[2], IO_R );
  outfd = io_open( argv[3], IO_W );

printf( "ha %d %d %d\n", infd0, infd1, outfd );

  while (1) {
    if (!has0 && infd0!=-1) {
      int r = score_read_event( infd0, &in0, 1 );
printf( "0 %d\n", r );
      if (r==0) { 
printf( "close\n" );
       close( infd0 );
        infd0 = -1;
      } else {
        has0=1;
      }
    }
    if (!has1 && infd1!=-1) {
      int r = score_read_event( infd1, &in1, 1 );
printf( "1 %d\n", r );
      if (r==0) {
printf( "close\n" );
        close( infd1 );
        infd1 = -1;
      } else {
        has1=1;
      }
    }

    if (has0&&has1) {
printf( "both %d %d\n", in0.time, in1.time );
      if (in0.time < in1.time) {
        score_write_event( outfd, &in0, 1 );
        has0 = 0;
      } else {
        score_write_event( outfd, &in1, 1 );
        has1 = 0;
      }
    } else if (has0) {
      score_write_event( outfd, &in0, 1 );
      has0 = 0;
    } else if (has1) {
      score_write_event( outfd, &in1, 1 );
      has1 = 0;
    } else {
      break;
    }
  }
}
