%arg out outp mono

%lib card io

%sect global

#include "io.h"
char *filename;
int fd;

%sect init

filename = param_string( "fn" );
fd = io_open( filename, IO_R );

%sect loop

int r = io_read( fd, rawbuf(outp), BUFLEN*sizeof(sample) );
if (r != BUFLEN*sizeof(sample)) DONE;
