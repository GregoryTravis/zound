/* $Id: soundio.h,v 1.1 1998/06/23 02:55:09 mito Exp $ */

#ifndef _soundio_h_
#define _soundio_h_

#include "z_types.h"

typedef struct sound {
  sample *samps;
  int len;
} sound ;

sound *sound_load( char *name );

#endif /* _soundio_h_ */
