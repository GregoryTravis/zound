%arg in inp score

%lib io score

%sect global

#include "io.h"
#include "midi.h"
char *filename;
int fd;

%sect init

filename = param_string( "fn" );
fd = io_open( filename, IO_W );

%sect loop

if (scorebuflen(inp)) {
  score_write_event( fd, scorebuf(inp), scorebuflen(inp) );
}
