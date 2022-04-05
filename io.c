// $Id: io.c,v 1.4 1998/07/13 03:14:38 mito Exp $

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "io.h"
#include "ioasync.h"

#define ASYNC_FDFLAG 0x4000

int io_open( char *pathname, int type )
{
  if (((type & (IO_R|IO_W))==(IO_R|IO_W)) ||
      ((type & (IO_R|IO_W))==0)) {
    fprintf( stderr, "Bad open flags 0x%x\n", type );
  }

  if (type&IO_ASYNC) {
    return io_async_open( pathname, type ) | ASYNC_FDFLAG;
  } else {
    if (type & IO_R) {
      return open( pathname, O_RDONLY );
    } else if (type & IO_W) {
      return open( pathname, O_WRONLY|O_CREAT|O_TRUNC, 0644 );
    } else {
      fprintf( stderr, "Bad: io_open( %d )\n", type );
      exit( 1 );
    }
  }
}

int io_close( int fd )
{
  if (fd & ASYNC_FDFLAG) {
    return io_async_close( fd & ~ASYNC_FDFLAG );
  } else {
    return close( fd );
  }
}

int io_read( int fd, void *buf, int count )
{
  if (fd & ASYNC_FDFLAG) {
    return io_async_read( fd&~ASYNC_FDFLAG, buf, count );
  } else {
    return read( fd, buf, count );
  }
}

int io_write( int fd, void *buf, int count )
{
  if (fd & ASYNC_FDFLAG) {
    return io_async_write( fd&~ASYNC_FDFLAG, buf, count );
  } else {
    return write( fd, buf, count );
  }
}

long io_length( int fd )
{
  if (fd & ASYNC_FDFLAG) {
    fprintf( stderr, "io_length on async fd: not imlemented.\n" );
    exit( 1 );
  } else {
    long save = lseek( fd, 0, SEEK_CUR );
    long end = lseek( fd, 0, SEEK_END );
    lseek( fd, save, SEEK_SET );
    return end;
  }
}
