#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

void main( int argc, char *argv[] )
{
  int delay, num, in, diff, difftotal=0, biz;
  char method;
  int factor, r;
  struct timeval tvb, tva, delaytv, start, end;
  fd_set fds;
  int fd[2];
  struct sched_param sp;

  sp.sched_priority = 99;
  r = sched_setscheduler( 0, SCHED_FIFO, &sp );
  printf( "sched %d\n", r );

  method = argv[1][0];
  delay = atoi( argv[2] );
  num = atoi( argv[3] );
  biz = atoi( argv[4] );

  pipe( fd );


  gettimeofday( &start, NULL );
  while (num--) {
    gettimeofday( &tvb, NULL );
    while (1) {
      gettimeofday( &tva, NULL );
      diff = 1000000*(tva.tv_sec-tvb.tv_sec) + (tva.tv_usec-tvb.tv_usec);
      if (diff > biz) break;
    }
//    gettimeofday( &tvb, NULL );

    delaytv.tv_sec = delay / 1000000;
    delaytv.tv_usec = delay % 1000000;

    if (method=='u') {
      usleep( delay );
    } else {
      FD_ZERO( &fds );
      FD_SET( fd[1], &fds );
      select( fd[1]+1, &fds, NULL, NULL, &delaytv );
    }

    gettimeofday( &tva, NULL );
    diff = 1000000*(tva.tv_sec-tvb.tv_sec) + (tva.tv_usec-tvb.tv_usec);
    printf( "diff %d\n", diff );
  }
  gettimeofday( &end, NULL );
  printf( "Total: %d sec %d us\n", end.tv_sec-start.tv_sec,
    end.tv_usec-start.tv_usec );
}
