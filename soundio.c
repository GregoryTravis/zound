/* $Id: soundio.c,v 1.2 1998/07/29 03:30:43 mito Exp $ */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "soundio.h"

sound *sound_load( char *name )
{
  int fd;
  int r;
  long len;
  sample *raw;
  sound *snd;

  fd = open( name, O_RDONLY );
  if (fd<0) {
    fprintf( stderr, "Can't open %s\n", name );
    exit( 1 );
  }
  len = lseek( fd, 0, SEEK_END );
  lseek( fd, 0, SEEK_SET );
  raw = (sample*)malloc( len );
  if (!raw) {
    fprintf( stderr, "Can't alloc %d\n", len/sizeof(sample) );
    exit( 1 );
  }
  r = read( fd, raw, len );
  if (r != len) {
    fprintf( stderr, "Can't read %d bytes from %s, only %d\n", len, name, r );
    exit( 1 );
  }
  close( fd );

  snd = (sound*)malloc( sizeof(sound) );
  snd->samps = raw;
  snd->len = len/2;

  return snd;
}
