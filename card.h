// $Id: card.h,v 1.4 1998/07/29 00:33:32 mito Exp $

#ifndef _card_h_
#define _card_h_

void card_buffering( int numbufs, int buflenlog );
void card_init( int mode );
void card_term();
int card_read( void *buf, int len );
int card_write( void *buf, int len );

#define CARD_R 1
#define CARD_W 2
#define CARD_STEREO 4

#endif /* _card_h_ */
