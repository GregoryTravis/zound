#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include "card.h"
#include "z_time.h"

#define NUMBUFS 64
#define BUFLENLOG 10
#define BUFLEN (1<<(BUFLENLOG))
#define USERBUFLEN 

char buf[BUFLEN];

int done=0;

void break_handler( int n )
{
  done = 1;
}

int lg[256];

void main( int argc, char *argv[] )
{
  int fragcode = (NUMBUFS<<16) | BUFLENLOG, i;
  char *fname;
  int len, r, format, stereo, speed, frag_size, rec, caps;
  int fd;
  struct sigaction sa;
  int last_lahg = -2;

  sa.sa_handler = &break_handler;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = SA_RESTART;
//  sigaction( SIGINT, &sa, NULL );

  /* open device */
  fd = open( "/dev/dsp", (O_RDONLY), 0 );
  if (fd == -1) { perror( "open dsp" ); exit( 1 ); }

  /* Let's set the buffering situation */
  printf( "Set frag to %d (code=0x%x)\n", (1<<BUFLENLOG), fragcode );
  r = ioctl( fd, SNDCTL_DSP_SETFRAGMENT, &fragcode );
  if (r==-1) { perror( "setfrag" ); exit( 1 ); }
  printf( "Frag set to 0x%x\n", fragcode );

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

  for (i=1; i<256; ++i) {
    int ii=i;
    int b=-1;
    while (1) {
//printf( "ho %d\n", ii );
      if (ii==0) {
//printf( "%d %d\n", i, b );
        lg[i] = b;
        break;
      }
      ii>>=1;
      b++;
    }
  }

  while (!done) {
    r = read( fd, buf, BUFLEN );
    if (r != BUFLEN) {
      printf( "BAD READ %d\n", r );
    }
    for (i=0; i<BUFLEN/2; ++i) {
      short samp = ((short*)buf)[i];
      int lahg;
      samp = (samp<0) ? -samp : samp;
      if ((((unsigned short)samp)&0xff00)==0) {
        lahg = lg[((unsigned short)samp)&0xff];
      } else {
        lahg = lg[((unsigned short)samp)>>8]+8;
//lahg=0;
      }
      if (i==0) {
	if (lahg != last_lahg) {
	  for (i=0; i<lahg; ++i) {
	    putchar( '*' );
	  }
	  putchar( '\n' );
	  last_lahg = lahg;
	}
      }
    }
  }
}
