/* $Id: score.c,v 1.1 1998/06/30 02:56:14 mito Exp $ */

#include "score.h"

void score_dump_event( score_event *sev )
{
  printf( "EV@%x: t %d i 0x%x p %d val %x\n", sev->time, sev->track,
    sev->inst, sev->pitch, sev->value );
}
