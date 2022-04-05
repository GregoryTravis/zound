// $Id: z_time.c,v 1.2 1998/07/12 22:57:26 mito Exp $

#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "z_time.h"

long z_time( void )
{
  struct timeb tb;
  ftime( &tb );
  return ((long)tb.time)*1000 + (long)tb.millitm;
}

long z_wait( long ms )
{
  int r;
  fd_set rs;
  struct timeval tv;

  FD_ZERO( &rs );
  tv.tv_sec = ms/1000;
  tv.tv_usec = ms % 1000;
  r = select( 0, &rs, NULL, NULL, &tv );
}

// Rough timers

#define NTIMERS (32)
typedef struct rough_timer {
  long delta;
  long last;
} rough_timer ;

rough_timer rts[NTIMERS];
int ntimers=0;

int z_create_timer( long delta )
{
  rough_timer *rt;
  if (ntimers>=NTIMERS) {
    fprintf( stderr, "Too many timers: %d\n", ntimers );
    exit( 1 );
  }
  rt = &rts[ntimers];
  rt->delta = delta;
  rt->last = z_time();

  return ntimers++;
}

int z_timer( int tid )
{
  long now;
  rough_timer *rt;
  int ret;

  rt = &rts[tid];
  now = z_time();
  ret = rt->last+rt->delta < now;

  if (ret) rt->last = now;

  return ret;
}
