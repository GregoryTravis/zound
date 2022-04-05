/* $Id: score.h,v 1.2 1998/07/17 00:19:36 mito Exp $ */

#ifndef _score_h_
#define _score_h_

#include "z_types.h"

typedef struct score_event {
  long time;
  uint track;
  uint inst;
  uint pitch;
  uint vol;
  uint action;
  uint midi_channel;
  uint value;
} score_event ;

#define SCORE_NON  1
#define SCORE_NOFF 2
#define SCORE_MOD  3

#define scorebuflen(e) score_edge_len_##e
#define scorebuf(e) score_edge_buf_##e
#define scoreev(e,n) score_edge_buf_##e[n]

#define score_write_event(fd,ev,n) \
  io_write( (fd), (ev), (n)*sizeof(score_event) )
#define score_read_event(fd,ev,n) \
  io_read( (fd), (ev), (n)*sizeof(score_event) )

void score_dump_event( score_event *sev );

#endif /* _score_h_ */
