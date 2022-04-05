%arg in inp0 mono
%arg in inp1 mono

%lib card

%sect global

#include "card.h"
sample rifbuf[BUFLEN*2];

%sect init

card_init( CARD_W|CARD_STEREO );

%sect loop

int i,ii, r;
for (i=0,ii=0; i<BUFLEN; ++i) {
  rifbuf[ii++] = ain(inp0,i);
  rifbuf[ii++] = ain(inp1,i);
}
r = card_write( rifbuf, BUFLEN*2*sizeof(sample) );
//printf( "wrote %d\n", r );
//printf( "TIME %d samp %d ms\n", curtime, curtime_ms );
