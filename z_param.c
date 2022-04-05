// $Id: z_param.c,v 1.2 1998/07/12 15:56:00 mito Exp $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "z_param.h"

params *new_params()
{
  params *p = (params*)malloc( sizeof(params) );
  p->num = 0;
}

void param_add( params *ps, char *fname, char *aname )
{
  if (ps->num >= MAX_PARAMS) {
    fprintf( stderr, "Too many params.\n" );
    exit( 1 );
  }

  ps->keys[ps->num] = strdup( fname );
  ps->values[ps->num] = strdup( aname );
  ps->num++;
}

void param_add_comline( params *ps, int argc, char *argv[] )
{
  int a;
  int ordc=0;
  char buf[20];

  for (a=1; a<argc; ++a) {
    char *arg = argv[a], *col, temp;

    col = index( arg, ':' );
    if (col) {
      temp = *col;
      *col = 0;

      param_add( ps, arg, col+1 );

      *col = temp;
    } else {
      sprintf( buf, "%d", ordc );
      param_add( ps, buf, arg );
      ordc++;
    }
  }
}

int param_int( params *ps, char *key )
{
  return atoi( param_string( ps, key ) );
}

double param_double( params *ps, char *key )
{
  return atof( param_string( ps, key ) );
}

char *param_string( params *ps, char *key )
{
  int i;

  for (i=0; i<ps->num; ++i) {
    if (!strcmp( ps->keys[i], key ))
      return ps->values[i];
  }

  fprintf( stderr, "Can't find param %s\n", key );
  exit( 1 );
}

int param_is_set( params *ps, char *key )
{
  int i;

  for (i=0; i<ps->num; ++i) {
    if (!strcmp( ps->keys[i], key ))
      return 1;
  }

  return 0;
}
