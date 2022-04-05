// $Id: feedback.c,v 1.1 1998/07/01 02:01:28 mito Exp $

#include <stdio.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "card.h"

#define NUMBUFS 64
#define BUFLENLOG 8
#define BUFLEN (1<<(BUFLENLOG))

unsigned char buf[BUFLEN];

int lasttyme=0;
void tyme()
{
  int tyyme;
  struct timeb tb;
  ftime( &tb );
  tyyme = ((long)tb.time)*1000 + (long)tb.millitm;
  printf( "[%d]\n", tyyme-lasttyme );
  lasttyme = tyyme;
}

int tyme2()
{
  int tyyme;
  struct timeb tb;
  ftime( &tb );
  return ((long)tb.time)*1000 + (long)tb.millitm;
}

int read_ready=0;
int write_ready=0;
void space( int fd, char c )
{
  audio_buf_info info;
  ioctl( fd, SNDCTL_DSP_GETISPACE, &info );
  if (c) printf( "%c ISPACE %d %d %d %d\n", c, info.fragments, info.fragstotal,
      info.fragsize, info.bytes );
  if (info.fragments >= info.fragstotal) {
    printf( "READ OVERRUN\n" );
  }
  read_ready = info.fragments;

  ioctl( fd, SNDCTL_DSP_GETOSPACE, &info );
  if (c) printf( "%c OSPACE %d %d %d %d\n", c, info.fragments, info.fragstotal,
    info.fragsize, info.bytes );
  if (info.fragments >= info.fragstotal) {
    printf( "WRITE STARVE\n" );
  }
  write_ready = info.fragments;
}

void space2( int fd, int *ispace, int *ospace )
{
  audio_buf_info info;
  ioctl( fd, SNDCTL_DSP_GETISPACE, &info );
  *ispace = info.fragments;
  ioctl( fd, SNDCTL_DSP_GETOSPACE, &info );
  *ospace = info.fragments;
}

void main( int argc, char *argv[] )
{
  int len, r, format, stereo, speed, frag_size, rec, caps;
  int fragcode, fd;
  int numbufs = NUMBUFS;

  if (argc>1) numbufs = atoi( argv[1] );

  fragcode = (numbufs<<16) | BUFLENLOG;

  /* open device */
  fd = open( "/dev/dsp", O_RDWR /*rec?O_RDONLY:O_WRONLY*/, 0 );
  if (fd == -1) { perror( "open dsp" ); exit( 1 ); }

  /* Let's set the buffering situation */
  printf( "Set frag to %d (code=0x%x)\n", (1<<BUFLENLOG), fragcode );
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
  stereo = 0;
  r = ioctl( fd, SNDCTL_DSP_STEREO, &stereo );
  if (r==-1) { perror( "stereo" ); exit( 1 ); }
  printf( "Stereo: got %d\n", stereo );

  /* set sampling rate */
  speed = 44100;
  r = ioctl( fd, SNDCTL_DSP_SPEED, &speed );
  if (r==-1) { perror( "speed" ); exit( 1 ); }
  printf( "Speed: %d\n", speed );

  /* what is block size? */
  r = ioctl( fd, SNDCTL_DSP_GETBLKSIZE, &frag_size );
  if (r==-1) { perror( "frag size" ); exit( 1 ); }
  printf( "Frag size is %d\n", frag_size );

  printf( "\n" );

write( fd, buf, BUFLEN );
read( fd, buf, BUFLEN );
  {
    int ispace, ospace;
    int spinstart, pushstart;
    int allstart=tyme2();
    int numpushes = 0;
    int spincycles;
    int beforeispace;

    while (1) {
      spinstart = tyme2();
      spincycles = 0;
      while (1) {
        space2( fd, &ispace, &ospace );
        if (ispace>0) break;
        spincycles++;
      }
      printf( "spin (%d) %d\n", spincycles, tyme2()-spinstart );
      if (spincycles) {
        printf( "spin was %%%d effective\n",
          (int)( ( 100 * ( ((double)((ispace*BUFLEN)/2.0)) /
                     ((tyme2()-spinstart)/1000.0) )
                 / (44100.0) ) ) );
      }

      printf( "buf %d %d\n", ispace, ospace );

      pushstart = tyme2();
      r = read( fd, buf, BUFLEN );
      if (r==BUFLEN) {
	write( fd, buf, BUFLEN );
      } else {
	printf( "BAAAAAAAAAAAAAAAAAAAAAAD\n" );
      }
      printf( "push %d\n", tyme2()-pushstart );
      numpushes++;
      printf( "effect samp rate: %f\n",
        (((double)(numpushes*BUFLEN))/2.0) / ((tyme2()-allstart)/1000.0) );
    }
  }

#if 0
write( fd, buf, BUFLEN );
  {
    fd_set rrs, wrs, ers;
    struct timeval tv;
    int ispace, ospace;

    while (1) {
      while (1) {
	FD_ZERO( &rrs ); FD_SET( fd, &rrs );
	FD_ZERO( &wrs ); FD_SET( fd, &wrs );
	FD_ZERO( &ers ); //FD_SET( fd, &ers );

	r = select( fd+1, &rrs, &wrs, &ers, NULL );
        if (FD_ISSET( fd, &rrs )) break;
      }
      space2( fd, &ispace, &ospace );
      printf( "sel %d/%d space %d/%d\n", FD_ISSET( fd, &rrs ),
        FD_ISSET( fd, &wrs ), ispace, ospace );
      tyme();
      if (ispace>0) {
        read( fd, buf, BUFLEN );
        printf( "read " ); tyme();
        write( fd, buf, BUFLEN );
        printf( "write " ); tyme();
      }
    }
  }
#endif

#if 0 // this exhibits the same load-too-much-at-one-time problem
  write( fd, buf, BUFLEN );
  write( fd, buf, BUFLEN );
  write( fd, buf, BUFLEN );
  write( fd, buf, BUFLEN );
  write( fd, buf, BUFLEN );

  {
    fd_set rrs, wrs, ers;
    struct timeval tv;

      FD_ZERO( &rrs ); FD_SET( fd, &rrs );
      FD_ZERO( &wrs ); FD_SET( fd, &wrs );
      FD_ZERO( &ers ); //FD_SET( fd, &ers );

printf( "first doing %d\n", fd );
      r = select( fd+1, &rrs, &wrs, &ers, NULL );
      printf( "ret %d\n", r );
      printf( "r %d w %d e %d\n", FD_ISSET( fd, &rrs ),
	FD_ISSET( fd, &wrs ), FD_ISSET( fd, &ers ) );
  }

  {
    fd_set rrs, wrs, ers;
    struct timeval tv;
//static int firsttime = 1;

    while (1) {
      FD_ZERO( &rrs ); FD_SET( fd, &rrs );
      FD_ZERO( &wrs ); //if (firsttime) { FD_SET( fd, &wrs ); firsttime=0; }
      FD_ZERO( &ers ); //FD_SET( fd, &ers );

      tv.tv_sec = 0;
      tv.tv_usec = 2;
printf( "doing %d\n", fd );
      r = select( fd+1, &rrs, &wrs, &ers, &tv );
      tyme();
      space( fd, 'S' );
      printf( "ret %d\n", r );
      printf( "r %d w %d e %d\n", FD_ISSET( fd, &rrs ),
	FD_ISSET( fd, &wrs ), FD_ISSET( fd, &ers ) );

      if (FD_ISSET( fd, &rrs )) {
        printf( "Ready to read!\n" );
        space( fd, 'R' );
        printf( "Reading!\n" );
        tyme();
        read( fd, buf, BUFLEN );
        tyme();
        space( fd, 'R' );
        printf( "write!\n" );
        write( fd, buf, BUFLEN );
        tyme();
        space( fd, 'W' );
      }
    }
  }
#endif

#if 0
  tyme();
  while (1) {
    space( fd, 'R' );
    printf( "READ\n" );
    read( fd, buf, BUFLEN );
    tyme();
    space( fd, 'W' );
    printf( "WRITE\n" );
    write( fd, buf, BUFLEN );
    tyme();
  }
#endif

#if 0 // this works if buffers are large (64/10)
  while (1) {
    r = read( fd, buf, BUFLEN );
    if (r==BUFLEN) write( fd, buf, BUFLEN );
  }
#endif
}
