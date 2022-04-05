%arg in inp mono

%lib card

%sect global

#include "card.h"

%sect init

card_init( CARD_W );

%sect loop

int r = card_write( rawbuf(inp), BUFLEN*sizeof(sample) );
//printf( "wrote %d\n", r );
//printf( "TIME %d samp %d ms\n", curtime, curtime_ms );
