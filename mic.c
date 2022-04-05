%arg out outp mono

%lib card

%sect global

#include "card.h"

%sect init

card_init( CARD_R );

%sect loop

int r = card_read( rawbuf(outp), BUFLEN*sizeof(sample) );
//printf( "read %d\n", r );
