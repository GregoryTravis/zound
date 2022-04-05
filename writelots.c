// $Id: writelots.c,v 1.1 1998/07/12 14:46:08 mito Exp $

#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>

#define BUFSIZE 16384
unsigned char buf[BUFSIZE];

void main( int argc, char *argv[] )
{
  int fd, r;
  struct timeb tb;
  long t;

  for (r=0; r<BUFSIZE; ++r)
    buf[r] = r&0xFF;

  fd = open( "big.file", O_WRONLY|O_CREAT|O_TRUNC, 0644 );
  while (1) {
    ftime( &tb );
    t = ((long)tb.time)*1000 + (long)tb.millitm;
    r = write( fd, buf, BUFSIZE );
    t = ((long)tb.time)*1000 + (long)tb.millitm - t;
    printf( "%d\n", t );
    if (r != BUFSIZE)
      printf( "ONLY WROTE %d\n", r );
  }
}
