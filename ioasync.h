/* $Id: ioasync.h,v 1.1 1998/07/13 03:15:27 mito Exp $ */

#ifndef _ioasync_h_
#define _ioasync_h_

int io_async_open( char *pathname, int type );
int io_async_close( int fd );
int io_async_read( int fd, void *buf, int count );
int io_async_write( int fd, void *buf, int count );

#endif /* _ioasync_h_ */
