/* $Id: sslib.h,v 1.2 1998/07/29 03:31:52 mito Exp $ */

#ifndef _sslib_h_
#define _sslib_h_

void ss_init();
void ss_term();
void ss_note_on( long time, int note );
void ss_note_off( long time, int note );
void ss_render( sample *buf, int nsamps );

#endif /* _sslib_h_ */
