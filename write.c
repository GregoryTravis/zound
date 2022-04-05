%arg in inp mono

%lib io

%sect global

#include "io.h"
char *filename;
int fd;

%sect init

filename = param_string( "fn" );
fd = io_open( filename, IO_W|IO_ASYNC );

%sect term

printf( "IO closing %d\n", fd );
io_close( fd );

%sect loop

int r = io_write( fd, rawbuf(inp), BUFLEN*sizeof(sample) );

//printf( "%d\n", r );
