/* $Id: sslib.c,v 1.2 1998/07/29 03:31:40 mito Exp $ */

#include <stdio.h>
#include "soundio.h"
#include "sslib.h"
#include "z_ext.h"
#include "z_types.h"

typedef struct noteinst {
  sound *snd;
  long start;
  struct noteinst *aprev, *anext;
  struct noteinst *tprev, *tnext;
  int note;
} noteinst ;

#define POOLSIZE 128
#define MAXNOTE 128
noteinst *table[MAXNOTE];
noteinst *active;
noteinst poolr[POOLSIZE];
noteinst *pool;

sound *thesnd;

void ss_reset_active()
{
  noteinst *p;
  int i;

  memset( table, 0, sizeof(table) );
  active = 0;
  p = pool = poolr;
  for (i=1; i<POOLSIZE; ++i) {
    p->anext = &poolr[i];
    p = p->anext;
  }
  p->anext = 0;
}

void ss_dump()
{
  noteinst *p;
  int i, count;

  for (p=active, count=0; p; p = p->anext) {
    count++;
  }
  printf( "Active: %d\n", count );

  printf( "Pernote: " );
  for (i=0; i<MAXNOTE; ++i) {
    for (p=table[i], count=0; p; p = p->tnext) {
      count++;
    }
    printf( "%d ", count );
  }
  printf( "\n" );
}

void ss_init()
{
  ss_reset_active();
  thesnd = sound_load( "sd5050.sw" );
}

void ss_term()
{
}

void ss_note_on( long time, int note )
{
  noteinst *n;
  if (!pool) return;
  n = pool;
  pool = n->anext;
  n->anext = active;
  n->aprev = 0;
  if (active) active->aprev = n;
  active = n;
  n->tnext = table[note];
  n->tprev = 0;
  if (table[note]) table[note]->tprev = n;
  table[note] = n;

  n->start = time;
  n->snd = thesnd;
  n->note = note;
}

void ss_note_off( long time, int note )
{
  noteinst *n;
  for (n=table[note]; n; n=n->tnext) {
    if (n->anext) n->anext->aprev = n->aprev;
    if (n->aprev) n->aprev->anext = n->anext;
    if (active==n) active=n->anext;
    n->anext = pool;
    pool = n;
  }
  table[note] = 0;
}

void ss_render( sample *buf, int nsamps )
{
  noteinst *n;

  //ss_dump();

  memset( buf, 0, nsamps*sizeof(sample) );
  for (n=active; n;) {
    long start = n->start-curtime;
    long sstart=0;
    long end = n->snd->len+n->start-curtime;
    long send = n->snd->len;
    if (end<=0) {
      noteinst *loop_next = n->anext;
      if (n->tnext) n->tnext->tprev = n->tprev;
      if (n->tprev) n->tprev->tnext = n->tnext;
      if (n==table[n->note]) table[n->note] = n->tnext;
      if (n->anext) n->anext->aprev = n->aprev;
      if (n->aprev) n->aprev->anext = n->anext;
      if (active==n) active=n->anext;
      n->anext = pool;
      pool = n;
      n = loop_next;
      continue;
    }
    if (start>=nsamps) continue;
    if (start<0) {
      sstart = -start;
      start=0;
    }
    if (end>nsamps) {
      send -= end-nsamps;
      end = nsamps;
    }
    {
      sample *s = n->snd->samps+sstart;
      sample *d = buf+start;
      sample *de = buf+end;
      while (d<de) *d++ += *s++>>1;
    }

    n = n->anext;
  }
}
