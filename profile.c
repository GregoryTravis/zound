/* $Id: profile.c,v 1.1 1998/07/14 00:35:44 mito Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "profile.h"

long profile_data[MAX_DATA];
int profile_data_p=0;

void profile_term( void )
{
  FILE *fp;
  int i;

  printf( "profile: saving....\n" );

  fp = fopen( "profile.out", "w" );
  if (!fp) {
    fprintf( stderr, "Can't open profile.out!\n" );
    return;
  }

  for (i=0; i<profile_data_p; ++i)
    fprintf( fp, "%d\n", profile_data[i] );

  fclose( fp );
}

void profile_init( void )
{
  atexit( &profile_term );
}
