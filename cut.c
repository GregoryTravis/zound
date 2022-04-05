#include <stdio.h>

void main( int argc, char *argv[] )
{
  int c;
  int off = atoi( argv[1] );
  off *= 2;
  while (off-->0) getchar();
  while ((c=getchar())!=EOF)
    putchar( c );
}
