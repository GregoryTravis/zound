// $Id: card.c,v 1.7 1998/07/29 00:33:14 mito Exp $

#include <stdio.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include "card.h"
#include "z_time.h"

static int modes = 0;
static int fd=-1;

#define DEFAULT_NUMBUFS 64
#define DEFAULT_BUFLENLOG 10

static int numbufs=DEFAULT_NUMBUFS;
static int buflenlog=DEFAULT_BUFLENLOG;

#define BUFLEN (1<<(buflenlog))

//unsigned char readbuf[BUFLEN], writebuf[BUFLEN];
unsigned char *readbuf, *writebuf;
int readbufp=0, writebufp=0;

void space( int fd, int *ispace, int *ospace )
{
  audio_buf_info info;
  ioctl( fd, SNDCTL_DSP_GETISPACE, &info );
  *ispace = info.fragments;
  ioctl( fd, SNDCTL_DSP_GETOSPACE, &info );
  *ospace = info.fragments;
}

void card_buffering( int u_numbufs, int u_buflenlog )
{
  if (fd!=-1) {
    fprintf( stderr, "Can't set buffering: already open.\n" );
    exit( 1 );
  }
  numbufs = u_numbufs;
  buflenlog = u_buflenlog;
  printf( "BUFF %d %d\n", numbufs, buflenlog );
}

void card_init( int mode )
{
  int len, r, format, stereo, speed, frag_size, rec, caps;
  int fragcode;

  if (!readbuf) {
    readbuf = (unsigned char*)malloc( BUFLEN );
    writebuf = (unsigned char*)malloc( BUFLEN );
  }

  fragcode = (numbufs<<16) | buflenlog;

  modes |= mode;

  if (fd!=-1) {
//    fprintf( stderr, "Double card open\n" );
//    exit( 1 );
    return;
  }

  /* open device */
  fd = open( "/dev/dsp", O_RDWR /*rec?O_RDONLY:O_WRONLY*/, 0 );
  if (fd == -1) { perror( "open dsp" ); exit( 1 ); }

  /* Let's set the buffering situation */
  printf( "Set frag to %d (code=0x%x)\n", (1<<buflenlog), fragcode );
  r = ioctl( fd, SNDCTL_DSP_SETFRAGMENT, &fragcode );
  if (r==-1) { perror( "setfrag" ); exit( 1 ); }
  printf( "Frag set to 0x%x\n", fragcode );

// No longer needed?
//  /* duplex */
//  r = ioctl( fd, SNDCTL_DSP_SETDUPLEX, 0 );
//  if (r==-1) { perror( "duplex" ); exit( 1 ); }
//  printf( "Duplex ok.\n" );

  /* What are caps? */
  r = ioctl( fd, SNDCTL_DSP_GETCAPS, &caps );
  if (r==-1) { perror( "caps" ); exit( 1 ); }
  printf( "Caps: 0x%x %x\n", caps, caps&DSP_CAP_DUPLEX );

  /* what formats do we have? */
  r = ioctl( fd, SNDCTL_DSP_GETFMTS, &format );
  if (r==-1) { perror( "get fmts" ); exit( 1 ); }
  printf( "Formats: 0x%x\n", format );

  /* set sample format */
  format = AFMT_S16_LE;
  printf( "SF Trying 0x%x\n", format );
  r = ioctl( fd, SNDCTL_DSP_SETFMT, &format );
  if (r==-1) { perror( "set format" ); exit( 1 ); }
  printf( "SF Got 0x%x\n", format );

  /* set # of channels */
  stereo = (mode & CARD_STEREO) ? 1 : 0;
  r = ioctl( fd, SNDCTL_DSP_STEREO, &stereo );
  if (r==-1) { perror( "stereo" ); exit( 1 ); }
  printf( "Stereo: got %d\n", stereo );

  /* set sampling rate */
//  speed = 44100;
  speed = 44100;
  r = ioctl( fd, SNDCTL_DSP_SPEED, &speed );
  if (r==-1) { perror( "speed" ); exit( 1 ); }
  printf( "Speed: %d\n", speed );

  /* what is block size? */
  r = ioctl( fd, SNDCTL_DSP_GETBLKSIZE, &frag_size );
  if (r==-1) { perror( "frag size" ); exit( 1 ); }
  printf( "Frag size is %d\n", frag_size );

  readbufp = writebufp = 0;

  {
    int is, os, i;
    int want_i = numbufs/2, want_o = numbufs/2;

    // Start read and wait
    read( fd, readbuf, BUFLEN );
    while (1) {
      space( fd, &is, &os );
      if (is>=want_i) break;
    }

    // Eat read buffers until *really* right
    while (1) {
      space( fd, &is, &os );
      if (is<=want_i) break;
      read( fd, readbuf, BUFLEN );
    }

    printf( "read ready %d %d\n", is, os );

    // Now fill write immediately
//    for (i=0; i<numbufs-want_o; ++i) {
    while (1) {
      space( fd, &is, &os );
      if (os == want_o) break;
      write( fd, writebuf, BUFLEN );
    }
    space( fd, &is, &os );

    printf( "write ready %d %d\n", is, os );
  }
}

void card_term()
{
  if (fd==-1) return;
  close( fd );
  fd = -1;
}

audio_buf_info info;
int has_wrote=0;
int writebuf_not_empty=0;
int card_read( void *buf, int len )
{
  //if ((modes&CARD_W) && !has_wrote) { printf( "skip\n" ); return 0; }
  if (readbufp>=BUFLEN) {
    // Refill the buffer

    if (modes & CARD_R) {
      ioctl( fd, SNDCTL_DSP_GETISPACE, &info );
      //printf( "R ISPACE %d %d %d %d\n", info.fragments, info.fragstotal,
      //  info.fragsize, info.bytes );
      if (info.fragments >= info.fragstotal) {
	printf( "READ OVERRUN\n" );
      }
    }

    if (modes & CARD_W) {
      ioctl( fd, SNDCTL_DSP_GETOSPACE, &info );
      //printf( "R OSPACE %d %d %d %d\n", info.fragments, info.fragstotal,
      //  info.fragsize, info.bytes );
      if (writebuf_not_empty) {
	if (info.fragments >= info.fragstotal) {
	  printf( "WRITE STARVE\n" );
	}
      } else {
	if (info.fragments < info.fragstotal)
	  writebuf_not_empty = 1;
      }
    }

    read( fd, readbuf, BUFLEN );
    readbufp = 0;
    //printf( "flush buf size 0x%x\n", BUFLEN );
  }
  memcpy( buf, readbuf+readbufp, len );
  readbufp += len;

  //printf( "read buffered 0x%x to 0x%x\n", len, readbufp );
}

int card_write( void *buf, int len )
{
has_wrote=1;
  if (writebufp>=BUFLEN) {
    // Drain the buffer

    if (modes & CARD_R) {
      ioctl( fd, SNDCTL_DSP_GETISPACE, &info );
      //printf( "R ISPACE %d %d %d %d\n", info.fragments, info.fragstotal,
      //  info.fragsize, info.bytes );
      if (info.fragments >= info.fragstotal) {
	printf( "READ OVERRUN\n" );
      }
    }

    if (modes & CARD_W) {
      ioctl( fd, SNDCTL_DSP_GETOSPACE, &info );
      //printf( "R OSPACE %d %d %d %d\n", info.fragments, info.fragstotal,
      //  info.fragsize, info.bytes );
      if (writebuf_not_empty) {
	if (info.fragments >= info.fragstotal) {
	  printf( "WRITE STARVE\n" );
	}
      } else {
	if (info.fragments < info.fragstotal)
	  writebuf_not_empty = 1;
      }
    }

    write( fd, writebuf, BUFLEN );
    writebufp = 0;
    //printf( "fill buf size 0x%x\n", BUFLEN );
  }
  memcpy( writebuf+writebufp, buf, len );
  writebufp += len;

  //printf( "write buffered 0x%x to 0x%x\n", len, writebufp );
}
