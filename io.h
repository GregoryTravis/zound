// $Id: io.h,v 1.4 1998/07/13 03:14:38 mito Exp $

#ifndef _io_h_
#define _io_h_

#define IO_R 1
#define IO_W 2
#define IO_ASYNC 4

int io_open_r( char *pathname, int type );
int io_close( int fd );
int io_read( int fd, void *buf, int count );
int io_write( int fd, void *buf, int count );
long io_length( int fd );

#endif /* _io_h_ */
