/* $Id: ioasync.c,v 1.1 1998/07/13 03:15:27 mito Exp $ */

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "io.h"
#include "ioasync.h"
#include "z_time.h"

#define lock(m) (_lock(m,__LINE__))
#define unlock(m) (_unlock(m,__LINE__))

void *io_reader( void *arg );
void *io_writer( void *arg );
void _lock( pthread_mutex_t *mut, int );
void _unlock( pthread_mutex_t *mut, int );
void wait( pthread_cond_t *cnd, pthread_mutex_t *mut );
void signl( pthread_cond_t *cnd );

#define BUFSIZE (65536*4)
#define WAKE_AMOUNT (16384)

typedef struct filedata {
  char *buf;
  int inp, outp;
  int fd;
  pthread_t thr;
  pthread_cond_t cnd;
  int please_term;
  int since_signal;
  int total_written;  // total entered into async system, anyway
} filedata ;

#define MAX_FD (32)
filedata *filedatas[MAX_FD];
pthread_mutex_t global_mut;

static int async_initted = 0;
void io_async_init( void )
{
  int r;

  if (async_initted) return;

  r = pthread_mutex_init( &global_mut, NULL );
  if (r) { perror( "global mutex init" ); exit( 1 ); }

  async_initted = 1;
}

int io_async_open( char *pathname, int type )
{
  int fd, r;
  filedata *fdat;

  io_async_init();

  if (type & IO_R) {
    fd = open( pathname, O_RDONLY );
  } else if (type & IO_W) {
    fd = open( pathname, O_WRONLY|O_CREAT|O_TRUNC, 0644 );
  } else {
    fprintf( stderr, "Bad: io_async_open( %d )\n", type );
    exit( 1 );
  }

  if (fd>=MAX_FD) {
    fprintf( stderr, "fd too high: %d\n", fd );
    exit( 1 );
  }

printf( "open %s: %d\n", pathname, fd );

  fdat = (filedata*)malloc( sizeof( filedata ) );
  if (!fdat) { fprintf( stderr, "Can't alloc fdat\n" ); exit( 1 ); }
  fdat->buf = (char*)malloc( BUFSIZE );
  fdat->inp = fdat->outp = fdat->please_term = fdat->since_signal = 0;
  fdat->total_written = 0;
  fdat->fd = fd;

  r = pthread_cond_init( &fdat->cnd, NULL );
  if (r) { perror( "cond init" ); exit( 1 ); }

  filedatas[fd] = fdat;

  if (type & IO_W)
    r = pthread_create( &fdat->thr, NULL, &io_writer, (void*)fd );
  else
    r = pthread_create( &fdat->thr, NULL, &io_reader, (void*)fd );

  if (r) { perror( "thread create" ); exit( 1 ); }

printf( "after tcreate, pid %d\n", getpid() );

  return fd;
}

int io_async_close( int fd )
{
  int r;
  filedata *fdat;

  lock( &global_mut );
  fdat = filedatas[fd];
  fdat->please_term = 1;

  signl( &fdat->cnd );

  unlock( &global_mut );
}

int io_async_read( int fd, void *buf, int count )
{
  return -1;
}

int io_async_write( int fd, void *buf, int count )
{
  int cur_inp, cur_outp, writeroom;
  filedata *fdat;

//printf( "Gonna write %d to %d\n", count, fd );

  lock( &global_mut );

  fdat = filedatas[fd];

  if (fdat) {
    cur_inp = fdat->inp;
    cur_outp = fdat->outp;

    // Is there room to write?  There has to be.
    writeroom = (cur_outp-cur_inp-1+BUFSIZE)%BUFSIZE;
    if (count > writeroom) {
      fprintf( stderr, "Out of room in buffer: i %d o %d, count %d\n",
	cur_inp, cur_outp, count );
      exit( 1 );
    }

    // So copy the data in
    if (cur_outp < cur_inp) {
      // One block
      memcpy( fdat->buf+cur_inp, buf, count );
      //printf( "copy single %d %d %d\n", cur_inp, cur_outp, count );
    } else {
      int block0_len, firstwrite, count2;
      // Two blocks
      block0_len = BUFSIZE-cur_inp;
      firstwrite = count > block0_len ? block0_len : count;
      memcpy( fdat->buf+cur_inp, buf, firstwrite );
      count2 = count - firstwrite;
      if (count2>0) {
	memcpy( fdat->buf, buf+firstwrite, count2 );
	//printf( "copy double %d %d %d %d\n",
	//  cur_inp, cur_outp, firstwrite, count2 );
      } else {
	//printf( "copy single (double) %d %d %d\n", cur_inp, cur_outp, count );
      }
    }
    fdat->inp = (cur_inp + count) % BUFSIZE;

    fdat->since_signal += count;
    if (fdat->since_signal >= WAKE_AMOUNT) {
      fdat->since_signal = 0;
      //printf( "signal!\n" );
      signl( &fdat->cnd );
    }

    fdat->total_written += count;
    if ((fdat->total_written%BUFSIZE) != fdat->inp)
      fprintf( stderr, "OOOOOOO BAD %d %d %d\n",
	fdat->total_written, fdat->total_written%BUFSIZE, fdat->inp );

    //printf( "wrote %d\n", count );
  }

  unlock( &global_mut );

  return count;
}

void *io_reader( void *arg )
{
  return arg;
}

void *io_writer( void *arg )
{
  int fd, r;
  filedata *fdat;

  fd = (int)arg;

printf( "Thread fd=%d born!\n", fd );

  fdat = filedatas[fd];

  lock( &global_mut );

  while (!fdat->please_term) {
    int cur_inp, cur_outp;

    cur_inp = fdat->inp;
    cur_outp = fdat->outp;

    // Is there something to write?
    if (cur_inp != cur_outp) {
      int amount_written;

      // Yes -- so write it
      unlock( &global_mut );
      if (cur_inp > cur_outp) {
        // One section
        r = write( fd, fdat->buf+cur_outp, cur_inp-cur_outp );
        if (r != cur_inp-cur_outp) {
          printf( "ASYNCH write: only %d of %d\n", r, cur_inp-cur_outp );
        }
        amount_written = cur_inp-cur_outp;
      } else {
        // Two sections: wrap
        r = write( fd, fdat->buf+cur_outp, BUFSIZE-cur_outp );
        if (r != BUFSIZE-cur_outp) {
          printf( "ASYNCH write: only %d of %d\n", r, BUFSIZE-cur_outp );
        }
        amount_written = BUFSIZE-cur_outp;
        r = write( fd, fdat->buf, cur_inp );
        if (r != cur_inp) {
          printf( "ASYNCH write: only %d of %d\n", r, cur_inp );
        }
        amount_written += cur_inp;
      }
      fdat->outp = (cur_outp+amount_written) % BUFSIZE;
      lock( &global_mut );
    }

    // Now we go to sleep
    wait( &fdat->cnd, &global_mut );
  }

  close( fd );
  free( fdat->buf );
  free( fdat );
  filedatas[fd] = 0;

  unlock( &global_mut );

  printf( "Thread fd=%d dying\n", fd );
  pthread_exit( 0 );
}

void _lock( pthread_mutex_t *mut, int line )
{
  int r;
//printf( "lock attemt from line %d\n", line );
  r = pthread_mutex_lock( mut );
//printf( "lock success from line %d\n", line );
  if (r) { perror( "lock mutex" ); exit( 1 ); }
}

void _unlock( pthread_mutex_t *mut, int line )
{
  int r;
//printf( "unlock attemt from line %d\n", line );
  r = pthread_mutex_unlock( mut );
//printf( "unlock success from line %d\n", line );
  if (r) { perror( "unlock mutex" ); exit( 1 ); }
}

void wait( pthread_cond_t *cnd, pthread_mutex_t *mut )
{
  int r = pthread_cond_wait( cnd, mut );
  if (r) { perror( "cond wait" ); exit( 1 ); }
}

void signl( pthread_cond_t *cnd )
{
  int r = pthread_cond_signal( cnd );
  if (r) { perror( "cond signal" ); exit( 1 ); }
}
